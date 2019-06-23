
/*=============================================================================
 * jeu-ezen.c: a variant of the puzzle Loops of Zen
 *
 * regis.barbanchon@lif.univ-mrs.fr - 13/08/2011 - version 1.2
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
 *=============================================================================
 */

#include <math.h>
#include "ez-draw.h"

#define MIN_EDGE_SLOTS 2
#define MAX_EDGE_SLOTS 5

#define MAX_CELL_SLOTS (4 * (MAX_EDGE_SLOTS))
#define MAX_CELL_PATHS (MAX_CELL_SLOTS / 2 + 1)

#define MAX_GRID_WIDTH  15
#define MAX_GRID_HEIGHT 15

#define MIN_CELL_RADIUS 15
#define MAX_CELL_RADIUS 45

#define DEF_EDGE_SLOTS   3
#define DEF_GRID_WIDTH   8
#define DEF_GRID_HEIGHT  8
#define DEF_CELL_RADIUS 35
#define DEF_TORIC        0
#define DEF_SHOW_TILES   0
#define DEF_SHUFFLE      1

#define WIN_MARGIN 50
#define TEXT_WIDTH 650

#define BEZIER_STEPS   15
#define ANIM_STEPS     10
#define ANIM_MILLISECS 25

typedef Ez_uint32 ULong;

/*=============================================================================
 * Math
 *=============================================================================
 */

#define PI  3.14159265358979
#define EPS 0.000001

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))

#define INRANGE(a,x,b) MAX (a, MIN ((x), (b)))

double 
myround (double x)
{
  double f= floor (x);
  return (x < f + 0.5) ? f : f + 1.0;
}

/*=============================================================================
 * Math in the plane
 *=============================================================================
 */

typedef struct Coord { double x, y; } Coord;

Coord
Coord_xy (double x, double y)
{
  Coord coord;
  coord.x= x;
  coord.y= y;
  return coord;
}

/*=============================================================================
 * symmetric points wrt origin and axis
 *=============================================================================
 */

Coord Coord_neg   (Coord p) { return Coord_xy (-p.x, -p.y); }
Coord Coord_neg_x (Coord p) { return Coord_xy (-p.x,  p.y); }
Coord Coord_neg_y (Coord p) { return Coord_xy ( p.x, -p.y); }

/*=============================================================================
 * addition, subtraction of vectors
 *=============================================================================
 */

Coord Coord_sub (Coord p, Coord q) { return Coord_xy (p.x-q.x, p.y-q.y); }
Coord Coord_add (Coord p, Coord q) { return Coord_xy (p.x+q.x, p.y+q.y); }

/*=============================================================================
 * scaling point wrt origin or specified center
 *=============================================================================
 */

Coord 
Coord_scale (Coord p, double scalar)
{
  return Coord_xy (scalar * p.x, scalar * p.y);
}

Coord 
Coord_scale_around (Coord p, double scalar, Coord center)
{
  Coord rel= Coord_sub (p, center); 
  return Coord_add (center, Coord_scale (rel, scalar));
}

/*=============================================================================
 * rotating point wrt origin or specified center
 *=============================================================================
 */
  
Coord
Coord_rotate (Coord p, double angle)
{
  double cos_a= cos (angle);
  double sin_a= sin (angle);
  return Coord_xy (p.x * cos_a + p.y * -sin_a,
                   p.x * sin_a + p.y *  cos_a);
}

Coord
Coord_rotate_around (Coord p, double angle, Coord center)
{
  Coord rel= Coord_sub (p, center); 
  return Coord_add (center, Coord_rotate (rel, angle));
}

/*=============================================================================
 * linear interpolation of two points
 *=============================================================================
 */
  
Coord
Coord_lerp (Coord p, Coord q, double alpha)
{
  Coord pq= Coord_sub (q, p);
  return Coord_add (p, Coord_scale (pq, alpha));
}

/*=============================================================================
 * Bezier cubic
 *=============================================================================
 */

Coord
Coord_bezierCubic (Coord a, Coord b, Coord c, Coord d, double t)
{
  double s= 1.0 - t;

  double ss= s*s, sss= ss*s, sst3= 3*ss*t;
  double tt= t*t, ttt= tt*t, stt3= 3*tt*s;

  double x= sss* a.x + sst3* b.x + stt3* c.x + ttt* d.x;
  double y= sss* a.y + sst3* b.y + stt3* c.y + ttt* d.y;

  return Coord_xy (x, y);
}

/*=============================================================================
 * bounding box
 *=============================================================================
 */

typedef struct Bound { double left, right, top, bot; } Bound;

Bound 
Bound_create (Coord center, double radius) 
{
  Bound b;
  b.left = center.x - radius; b.top= center.y - radius;
  b.right= center.x + radius; b.bot= center.y + radius;
  return b;
}

Coord Bound_getA (Bound b) { return Coord_xy (b.left , b.top); }
Coord Bound_getB (Bound b) { return Coord_xy (b.right, b.top); }
Coord Bound_getC (Bound b) { return Coord_xy (b.right, b.bot); }
Coord Bound_getD (Bound b) { return Coord_xy (b.left , b.bot); }

/*=============================================================================
 * Additional EZ drawing routines using Coord
 *=============================================================================
 */

void
myez_draw_line (Ez_window win, Coord a, Coord b)
{
  ez_draw_line (win, 
                myround (a.x), myround (a.y),
                myround (b.x), myround (b.y));
}

void
myez_draw_triangle (Ez_window win, Coord a, Coord b, Coord c)
{
  ez_draw_triangle (win, 
                    myround (a.x), myround (a.y), 
                    myround (b.x), myround (b.y), 
                    myround (c.x), myround (c.y)); 
}

void
myez_fill_triangle (Ez_window win, Coord a, Coord b, Coord c)
{
  ez_fill_triangle (win, 
                    myround (a.x), myround (a.y), 
                    myround (b.x), myround (b.y), 
                    myround (c.x), myround (c.y)); 
}

void
myez_draw_quadrangle (Ez_window win, Coord a,  Coord b, Coord c, Coord d)
{
  myez_draw_line (win, a, b); myez_draw_line (win, b, c);
  myez_draw_line (win, c, d); myez_draw_line (win, d, a);
}

void
myez_fill_quadrangle (Ez_window win, Coord a,  Coord b, Coord c, Coord d)
{
  myez_fill_triangle (win, a, b, d); 
  myez_fill_triangle (win, c, b, d); 
}

void
myez_draw_bezier (Ez_window win, Coord a, Coord b, Coord c, Coord d, int n)
{
  int k;
  double t, dt= 1.0 / n;
  Coord prev= a, curr;
  for (k= dt, t= 0.0; k < n; k++, t+= dt) {
    curr= Coord_bezierCubic (a, b, c, d, t);
    myez_draw_line (win, prev, curr);
    prev= curr;
  } 
  curr= d;
  myez_draw_line (win, prev, curr);
}

/*=============================================================================
 * Additional EZ drawing routines... unused in final version 
 *=============================================================================
 */

void
myez_draw_circle (Ez_window win, Coord center, double r)
{
  ez_draw_circle (win, 
                  myround (center.x - r), myround (center.y - r), 
                  myround (center.x + r), myround (center.y + r));
}

void
myez_draw_cross (Ez_window win, Coord center, double r)
{
   ez_draw_line (win, 
                myround (center.x - r), myround (center.y - r),
                myround (center.x + r), myround (center.y + r));
   ez_draw_line (win, 
                myround (center.x + r), myround (center.y - r),
                myround (center.x - r), myround (center.y + r));
}

void
myez_draw_plus (Ez_window win, Coord center, double r)
{
   ez_draw_line (win, 
                myround (center.x), myround (center.y - r),
                myround (center.x), myround (center.y + r));
   ez_draw_line (win, 
                myround (center.x - r), myround (center.y),
                myround (center.x + r), myround (center.y));
}

/*=============================================================================
 * Puzzle structs
 *=============================================================================
 */

typedef struct Path {
  int lo_slot, hi_slot;
} Path;

typedef struct Cell {
  int  slot_flags [MAX_CELL_SLOTS];
  int  slots [MAX_CELL_SLOTS]; int nb_slots;
  Path paths [MAX_CELL_PATHS]; int nb_paths;
  int nb_turns, slerp, max_slerp;
} Cell;

typedef struct Index { int i, j; } Index;

typedef struct Grid {
  int is_toric, level;
  int nb_cell_slots, nb_edge_slots;
  Cell cells [MAX_GRID_HEIGHT][MAX_GRID_WIDTH];
  int width, height;
  Index selected;
} Grid;

/*=============================================================================
 * Indexes
 *=============================================================================
 */

Index
Index_ij (int i, int j) 
{
  Index index;
  index.i= i;
  index.j= j;
  return index;
}

Index Index_null (void) { return Index_ij (-1, -1); }

int 
Index_isNull (Index index) 
{
  return index.i < 0 
    ||   index.j < 0;
}

int
Index_isEqual (Index index, Index other)
{
  return index.i == other.i 
    &&   index.j == other.j;
}

/*=============================================================================
 * Cells
 *=============================================================================
 */

void 
Cell_initEmpty (Cell * cell)
{
  int k;
  cell->nb_turns = 0;
  cell->slerp    = 0;
  cell->max_slerp= ANIM_STEPS;

  cell->nb_paths= 0; cell->nb_slots= 0;
  for (k= 0; k < MAX_CELL_SLOTS; k++) {
    cell->slot_flags [k]= 0;
  }
}

double
Cell_getAngle (Cell * cell)
{
  double frac= (double) cell->slerp / (double) cell->max_slerp;
  return (cell->nb_turns + frac) * PI/2.0; 
}

void
Cell_incSlerp (Cell * cell, double delta)
{
  cell->slerp += delta;
  while (cell->slerp >= cell->max_slerp) { 
    cell->nb_turns ++; 
    cell->slerp -= cell->max_slerp; 
  } 
  while (cell->slerp <= - cell->max_slerp) { 
    cell->nb_turns --; 
    cell->slerp += cell->max_slerp; 
  }
  cell->nb_turns= (cell->nb_turns + 4) % 4; 
}

void
Cell_addSlot (Cell * cell, int slot)
{
  if (cell->slot_flags [slot]) return;
  cell->slot_flags [slot]= -1;
  cell->slots [cell->nb_slots]= slot;
  cell->nb_slots ++;
}

void
Cell_addPath (Cell * cell, int slot1, int slot2)
{
  Path * path= & cell->paths [cell->nb_paths];
  path->lo_slot= MIN (slot1, slot2);
  path->hi_slot= MAX (slot1, slot2);
  cell->nb_paths ++;
}

void
Cell_initPaths (Cell * cell)
{
  int k, slot1, slot2, n= cell->nb_slots;
  for (k= 1; k < n; k+= 2) {
    slot1= cell->slots [k-1];
    slot2= cell->slots [k  ];
    Cell_addPath (cell, slot1, slot2);
  }
  if (n % 2 == 0) return;
  slot1= cell->slots [n-1];
  slot2= cell->slots [0  ];
  Cell_addPath (cell, slot1, slot2);
}

/*=============================================================================
 * Grid edges and slots
 *=============================================================================
 */

int 
Grid_getSlotEdge (Grid * grid, int slot) 
{ 
  return slot / grid->nb_edge_slots; 
}

int 
Grid_getSlotOffset (Grid * grid, int slot) 
{ 
  return slot % grid->nb_edge_slots; 
}

Index
Grid_warpIndex (Grid * grid, Index index)
{
  if (! grid->is_toric) return index;
  return Index_ij ((index.i + grid->height) % grid->height,
                   (index.j + grid->width ) % grid->width);
}

Index
Grid_getIndexTwin (Grid * grid, Index index, int slot)
{
  Index twin;
  int edge= Grid_getSlotEdge (grid, slot);
  switch (edge) {
  case  0: twin= Index_ij (index.i-1, index.j  ); break;
  case  1: twin= Index_ij (index.i  , index.j-1); break;
  case  2: twin= Index_ij (index.i+1, index.j  ); break;
  case  3: twin= Index_ij (index.i  , index.j+1); break;
  default: twin= index;                           break;
  }
  return Grid_warpIndex (grid, twin);
}

int 
Grid_getEdgeTwin   (Grid * grid, int edge) 
{ 
  (void) grid;
  return (edge + 2) % 4; 
}

int
Grid_getSlotTwin (Grid * grid, int slot) 
{
  int edge     = Grid_getSlotEdge   (grid, slot);
  int offset   = Grid_getSlotOffset (grid, slot);
  int twin_edge= Grid_getEdgeTwin   (grid, edge);
  return (twin_edge + 1) * grid->nb_edge_slots - (offset + 1);
}

int
Grid_slotEdgesAreEqual (Grid * grid, int slot1, int slot2)
{
  int edge1= Grid_getSlotEdge (grid, slot1);
  int edge2= Grid_getSlotEdge (grid, slot2);
  return edge1 == edge2;
}

int
Grid_slotEdgesAreFacing (Grid * grid, int slot1, int slot2)
{
  int edge1= Grid_getSlotEdge (grid, slot1);
  int edge2= Grid_getSlotEdge (grid, slot2);
   return edge2 == edge1 + 2
     ||   edge1 == edge2 + 2;
}

int
Grid_slotEdgesAreContiguous (Grid * grid, int slot1, int slot2)
{
  int edge1= Grid_getSlotEdge (grid, slot1);
  int edge2= Grid_getSlotEdge (grid, slot2);
   return edge2 == (edge1 + 1) % 4
     ||   edge1 == (edge2 + 1) % 4;
}

int 
Grid_slotsAreContiguous (Grid * grid, int slot1, int slot2)
{
  return slot1 == (slot2 + 1) % grid->nb_cell_slots
    ||   slot2 == (slot1 + 1) % grid->nb_cell_slots;
}

double
Grid_getSlotLerp (Grid * grid, int slot1, int slot2)
{
  return Grid_slotEdgesAreFacing     (grid, slot1, slot2) ? 0.5 
    :    Grid_slotEdgesAreContiguous (grid, slot1, slot2) ? 0.6 
    :    Grid_slotsAreContiguous     (grid, slot1, slot2) ? 0.5
    :                                                       0.7;
} 

Cell *
Grid_getCellAtIndex (Grid * grid, Index index)
{
  if (index.i < 0 || index.i >= grid->height) return NULL;
  if (index.j < 0 || index.j >= grid->width ) return NULL;
  return & grid->cells [index.i][index.j];
}

void
Grid_initEmpty (Grid * grid, int height, int width, int density)
{
  Index index;
  grid->nb_edge_slots= density;
  grid->nb_cell_slots=  grid->nb_edge_slots * 4;

  grid->width = width;
  grid->height= height;
  grid->level= 0;
  grid->selected= Index_null ();
  for (index.i= 0; index.i < grid->height; index.i++) {
    for (index.j= 0; index.j < grid->width; index.j++) {
      Cell * cell= Grid_getCellAtIndex (grid, index);
      Cell_initEmpty (cell);
    }
  }
}

int
Grid_isUnicellular (Grid * grid)
{
  return  grid->width == 1 
    &&   grid->height == 1;
}

int
Grid_isPlanarUnicellular (Grid * grid)
{
  return Grid_isUnicellular (grid) && ! grid->is_toric;
}

void
Grid_initSlots (Grid * grid, int level)
{
  int min_count= MIN (2, grid->nb_edge_slots);
  Index curr_index;
  grid->level= level;
  if (Grid_isPlanarUnicellular (grid)) return;
  for (curr_index.i= 0; curr_index.i < grid->height; curr_index.i++) {
    for (curr_index.j= 0; curr_index.j < grid->width; curr_index.j++) {
      Cell * curr_cell= Grid_getCellAtIndex (grid, curr_index);
      int k, count= curr_cell->nb_slots;
      for (k= 0; k < level || count < min_count; k++) {
        int curr_slot= ez_random (grid->nb_cell_slots);
        int twin_slot= Grid_getSlotTwin (grid, curr_slot);
        Index twin_index= Grid_getIndexTwin   (grid, curr_index, curr_slot);
        Cell * twin_cell= Grid_getCellAtIndex (grid, twin_index);
        if (curr_cell->slot_flags [curr_slot]) continue; 
        if (! twin_cell)                       continue;
      
        count++;
        Cell_addSlot (curr_cell, curr_slot);
        Cell_addSlot (twin_cell, twin_slot);
      }
    }
  }
}

void
Grid_initPaths (Grid * grid)
{
  Index index;
  for (index.i= 0; index.i < grid->height; index.i++) {
    for (index.j= 0; index.j < grid->width; index.j++) {
      Cell * cell= Grid_getCellAtIndex (grid, index);
      Cell_initPaths (cell);
    }
  }
}

int
Grid_countClashesAtIndex (Grid * grid, Index curr_index)
{
  Cell * curr_cell= Grid_getCellAtIndex (grid, curr_index);
  int k, e= grid->nb_edge_slots, n= grid->nb_cell_slots, count= 0;
  for (k= 0; k < n; k++) {
    int k_flag= curr_cell->slot_flags [k];
    int k_turned= (k + curr_cell->nb_turns * e ) % n; 
    int j_turned= Grid_getSlotTwin (grid, k_turned);
    Index twin_index= Grid_getIndexTwin (grid, curr_index, k_turned);
    Cell * twin_cell= Grid_getCellAtIndex (grid, twin_index);
    if (! twin_cell) { 
      count += !! k_flag;
    } else {
      int j= (j_turned - twin_cell->nb_turns * e + n) % n; 
      int j_flag= twin_cell->slot_flags [j];
      count += (k_flag && ! j_flag);
    }
  }
  return count;
}

int
Grid_countAllClashes (Grid * grid)
{
  Index index;
  int count= 0;
  for (index.i= 0; index.i < grid->height; index.i++) {
    for (index.j= 0; index.j < grid->width; index.j++) {
      count += Grid_countClashesAtIndex (grid, index);
    }
  }
  return count;
}
 
void
Grid_shuffle (Grid * grid)
{
  Index index;
  for (index.i= 0; index.i < grid->height; index.i++) {
    for (index.j= 0; index.j < grid->width; index.j++) {
      Cell * cell= Grid_getCellAtIndex (grid, index);
      cell->nb_turns= ez_random (4);
    }
  }
}

void
Grid_unshuffle (Grid * grid)
{
  Index index;
  for (index.i= 0; index.i < grid->height; index.i++) {
    for (index.j= 0; index.j < grid->width; index.j++) {
      Cell * cell= Grid_getCellAtIndex (grid, index);
      cell->nb_turns= 0;
    }
  }
}

void
Grid_init (Grid * grid, int is_toric, 
           int height, int width, 
           int level, int density, int shuffle)
{
  grid->is_toric= is_toric;
  Grid_initEmpty (grid, height, width, density);
  Grid_initSlots (grid, level);
  Grid_initPaths (grid);
  if (shuffle) Grid_shuffle (grid);
}

int
Grid_indexIsOffLimit (Grid * grid, Index index)
{
  return index.i < 0 || index.i >= grid->height
    ||   index.j < 0 || index.j >= grid->width;
}

/*=============================================================================
 * App/Gui
 *=============================================================================
 */

typedef struct App {
  Grid grid;
} App;

typedef struct Gui {
  App * app;
  double cell_radius;
  int show_tiles;
  Ez_window win;
  int rotation_pending;
} Gui;


/*=============================================================================
 * Coords for drawing
 *=============================================================================
 */

Coord 
Gui_getGridSize (Gui * gui)
{
  Grid * grid= & gui->app->grid;
  Coord grid_dim= Coord_xy (grid->width, grid->height);
  return Coord_scale (grid_dim, 2 * gui->cell_radius);
}

Coord
Gui_getIdealWinSize (Gui * gui)
{
  Coord grid_size= Gui_getGridSize (gui);
  Coord win_margin= Coord_xy (WIN_MARGIN, WIN_MARGIN);
  Coord ideal_size= Coord_add (grid_size, Coord_scale (win_margin, 2));
  return Coord_xy (MAX (ideal_size.x, TEXT_WIDTH), ideal_size.y);
}

Coord
Gui_getGridCenterCoord (Gui * gui)
{
  Coord ideal_size= Gui_getIdealWinSize (gui);
  return Coord_scale (ideal_size, 0.5);
}

Coord
Gui_getBotLeftCellCoord (Gui * gui) 
{
  Grid * grid= & gui->app->grid;
  Coord center= Gui_getGridCenterCoord (gui);
  Coord v= Coord_xy (- (grid->width -1), grid->height-1);
  return Coord_add (center, Coord_scale (v, gui->cell_radius));
}

Index
Gui_getIndexAtCoord (Gui * gui, Coord pointer)
{
  Grid * grid= & gui->app->grid;
  Coord bot_left= Gui_getBotLeftCellCoord (gui);
  double scalar=  1.0/(2.0 * gui->cell_radius);
  Coord v= Coord_neg_y (Coord_scale (Coord_sub (pointer, bot_left), scalar));
  Index index= Index_ij (myround (v.y), myround (v.x));
  return Grid_indexIsOffLimit (grid, index) ? Index_null () : index;
}

Coord
Gui_getCellCoord (Gui * gui, Index index)
{
  Coord bot_left= Gui_getBotLeftCellCoord (gui);
  Coord v= Coord_xy (index.j, -index.i);
  return Coord_add (bot_left, Coord_scale (v, 2*gui->cell_radius));
}

Coord
Gui_getSlotCoord (Gui * gui, Index index, int slot)
{
  Grid * grid= & gui->app->grid;
  Cell * cell= Grid_getCellAtIndex (grid, index);
  double angle= Cell_getAngle (cell);
  Coord center= Gui_getCellCoord (gui, index);
  int edge  = Grid_getSlotEdge   (grid, slot);
  int offset= Grid_getSlotOffset (grid, slot);

  double delta= 2.0 / grid->nb_edge_slots;
  Coord unit= Coord_xy (1.0 - 0.5*delta - offset*delta, -1.0); 
  Coord scaled = Coord_scale (unit, gui->cell_radius);
  Coord rotated= Coord_rotate (scaled, - angle - edge * PI/2.0);
  Coord shifted= Coord_add (center, Coord_neg_y (rotated));
  return shifted;
}

/*=============================================================================
 * Drawing
 *=============================================================================
 */

void
Gui_drawCellTile (Gui * gui, Index index, ULong fill, ULong stroke)
{
  Grid * grid= & gui->app->grid;
  Cell * cell= Grid_getCellAtIndex (grid, index);
  double angle= Cell_getAngle (cell);
  Coord coord= Gui_getCellCoord (gui, index);
  Bound box= Bound_create (coord, gui->cell_radius-1);
 
  Coord a= Coord_rotate_around (Bound_getA (box), angle, coord);
  Coord b= Coord_rotate_around (Bound_getB (box), angle, coord);
  Coord c= Coord_rotate_around (Bound_getC (box), angle, coord);
  Coord d= Coord_rotate_around (Bound_getD (box), angle, coord); 

  ez_set_color (  fill); myez_fill_quadrangle (gui->win, a, b, c, d);
  ez_set_color (stroke); myez_draw_quadrangle (gui->win, a, b, c, d);
}

int
Gui_getStrokeThick (Gui * gui)
{
  return gui->cell_radius > 30 ? 6
    :    gui->cell_radius > 20 ? 4
    :                            3;
}

int
Gui_getFillThick (Gui * gui)
{
  return gui->cell_radius > 30 ? 4
    :    gui->cell_radius > 20 ? 2
    :                            1;
}

void
Gui_drawCellPath (Gui * gui, Index index, int k, ULong fill, ULong stroke)
{
  Grid * grid= & gui->app->grid;
  Cell * cell= Grid_getCellAtIndex (grid, index);
  Path * path= & cell->paths [k];
 
  int slot1= path->lo_slot, twin1= Grid_getSlotTwin (grid, slot1);
  int slot2= path->hi_slot, twin2= Grid_getSlotTwin (grid, slot2);

  Coord cs1= Gui_getSlotCoord (gui, index, slot1);
  Coord ct1= Gui_getSlotCoord (gui, index, twin1);
  Coord cs2= Gui_getSlotCoord (gui, index, slot2); 
  Coord ct2= Gui_getSlotCoord (gui, index, twin2);
  
  double alpha= Grid_getSlotLerp (grid, slot1, slot2); 
  Coord cm1= Coord_lerp (cs1, ct1, alpha);
  Coord cm2= Coord_lerp (cs2, ct2, alpha);
  
  ez_set_thick (Gui_getStrokeThick (gui)); ez_set_color (stroke);
  myez_draw_bezier (gui->win, cs1, cm1, cm2, cs2, BEZIER_STEPS);
  ez_set_thick (Gui_getFillThick (gui)); ez_set_color (fill);
  myez_draw_bezier (gui->win, cs1, cm1, cm2, cs2, BEZIER_STEPS);
  ez_set_thick (1);
}

void
Gui_drawCellPaths (Gui * gui, Index index, ULong fill, ULong stroke)
{
  Grid * grid= & gui->app->grid;
  Cell * cell= Grid_getCellAtIndex (grid, index);
  int k;
  for (k= 0; k < cell->nb_paths; k++) {
    Gui_drawCellPath (gui, index, k, fill, stroke);
  }
}

void
Gui_drawCell (Gui * gui, Index index, ULong fill, ULong stroke)
{
  if (gui->show_tiles) 
    Gui_drawCellTile  (gui, index, ez_yellow, ez_blue);
  Gui_drawCellPaths (gui, index, fill, stroke);
}

void
Gui_drawGrid (Gui * gui)
{
  Grid * grid= & gui->app->grid;
  Index index;
  for (index.i= 0; index.i < grid->height; index.i++) {
    for (index.j= 0; index.j < grid->width; index.j++) {
      int nb_clashes= Grid_countClashesAtIndex (grid, index);
      ULong fill=   nb_clashes ? ez_red   : ez_green;
      ULong stroke= nb_clashes ? ez_black : ez_blue; 
      if (Index_isEqual (index, grid->selected)) continue;
      Gui_drawCell (gui, index, fill, stroke);  
    }
  }
  if (Index_isNull (grid->selected)) return;
  Gui_drawCell (gui, grid->selected, ez_magenta, ez_black);
}

void
Gui_drawKeyInfo (Gui * gui)
{
  Coord win_size= Gui_getIdealWinSize (gui);

  ez_set_nfont (0); ez_set_color (ez_blue);
  ez_draw_text (gui->win, EZ_MC, 
                win_size.x / 2, 
                win_size.y - WIN_MARGIN / 2, 
                "[P]: Planar mode, "
                "[T]: Toric mode, "
                "[Arrows]: Resize grid, "
                "[PgDn/PgUp]: Change edge density,\n"
                "[S]: Shuffle grid, "
                "[U]: Unshuffle grid, "
                "[-/+]: Zoom, "
                "[V]: Toggle tiles visibility, "
                "[Esc/Q]: Quit.");  
}

void
Gui_drawStatus (Gui * gui)
{
  Grid * grid= & gui->app->grid;
  Coord  win_size= Gui_getIdealWinSize (gui);
  char * mode []= { "Planar", "Toric" }; 
  int nb_clashes= Grid_countAllClashes (grid);

  if (nb_clashes) {
    ez_set_nfont (3); ez_set_color (ez_red);
    ez_draw_text (gui->win, EZ_MC, win_size.x/2, WIN_MARGIN/2, 
                  "Restore %d-Dense %dx%d-%s Harmony (%d conflicts)", 
                  grid->nb_edge_slots, grid->height, grid->width, 
                  mode [grid->is_toric], nb_clashes);
  } else {
    ez_set_nfont (3); ez_set_color (ez_blue);
    ez_draw_text (gui->win, EZ_MC, win_size.x/2, WIN_MARGIN/2, 
                  "%d-Dense %dx%d-%s Harmony Achieved !",
                  grid->nb_edge_slots, grid->height, grid->width, 
                  mode [grid->is_toric]);
  }
}

/*=============================================================================
 * Grid reset by user
 *=============================================================================
 */ 

void
Gui_unshuffleGrid (Gui * gui)
{
  Grid_unshuffle (& gui->app->grid);
  ez_send_expose (gui->win);
}

void
Gui_reshuffleGrid (Gui * gui)
{
  Grid_shuffle (& gui->app->grid);
  ez_send_expose (gui->win);
}

void
Gui_regenerateGrid (Gui * gui, int is_toric)
{
  Grid * grid= & gui->app->grid;
  Grid_init (grid, is_toric, grid->height, grid->width, 
             grid->level, grid->nb_edge_slots, 1);
  ez_send_expose (gui->win);
}

void
Gui_resizeWinToIdeal (Gui * gui)
{
  Coord ideal= Gui_getIdealWinSize (gui);
  ez_window_set_size (gui->win, ideal.x, ideal.y);
}

void
Gui_resizeGrid (Gui * gui, int dh, int dw)
{
  Grid * grid= & gui->app->grid;
  int h= INRANGE (1, grid->height + dh, MAX_GRID_HEIGHT);
  int w= INRANGE (1, grid->width  + dw, MAX_GRID_WIDTH );
  
  Grid_init (grid, grid->is_toric, h, w, 
             grid->level, grid->nb_edge_slots, 1);
  Gui_resizeWinToIdeal (gui);
  ez_send_expose (gui->win);
}

void
Gui_densifyGrid (Gui * gui, double de)
{
  Grid * grid= & gui->app->grid;
  int e= grid->nb_edge_slots + de;
  int density= INRANGE (MIN_EDGE_SLOTS, e, MAX_EDGE_SLOTS);
  Grid_init (grid, grid->is_toric, grid->height, grid->width, 
             density, density, 1);
  ez_send_expose (gui->win);
}

void
Gui_zoomGrid (Gui * gui, double dr)
{
  double r= gui->cell_radius + dr;
  gui->cell_radius= INRANGE (MIN_CELL_RADIUS, r, MAX_CELL_RADIUS);
  Gui_resizeWinToIdeal (gui);
  ez_send_expose (gui->win);
}

void
Gui_toggleTileVisibility (Gui * gui)
{
  gui->show_tiles= ! gui->show_tiles;
  ez_send_expose (gui->win);
}

void
Gui_quit (Gui * gui)
{
  (void) gui;
  ez_quit (); 
}

/*=============================================================================
 * EZ specific callbacks
 *=============================================================================
 */


void 
onExpose (Ez_event * e)
{
  Gui *gui = ez_get_data (e->win);
  Gui_drawGrid (gui);
  Gui_drawStatus (gui);
  Gui_drawKeyInfo (gui);
}

void 
onButtonPress (Ez_event * e)
{
  Gui * gui = ez_get_data (e->win);
  Grid * grid= & gui->app->grid;
  Coord pointer= Coord_xy (e->mx, e->my);
  Index index= Gui_getIndexAtCoord (gui, pointer);
  if (  Index_isNull (index         )) return;
  if (! Index_isNull (grid->selected)) return;
  if (gui->rotation_pending)           return;
  grid->selected= index;
 
  gui->rotation_pending= (e->mb % 2) ? 1 : -1;
  ez_start_timer (e->win, 0);
}

void 
onKeyPress (Ez_event * e)
{  
  Gui * gui = ez_get_data (e->win);
  if (gui->rotation_pending) return;
                        
  switch (e->key_sym) {
  case XK_Q: case XK_q: case XK_Escape: Gui_quit (gui); break;

  case XK_U: case XK_u: Gui_unshuffleGrid  (gui); break;
  case XK_S: case XK_s: Gui_reshuffleGrid  (gui); break;
    
  case XK_P: case XK_p: Gui_regenerateGrid (gui, 0); break;
  case XK_T: case XK_t: Gui_regenerateGrid (gui, 1); break;
  
  case XK_Left : Gui_resizeGrid (gui, 0,-1); break;
  case XK_Right: Gui_resizeGrid (gui, 0, 1); break; 
  case XK_Down : Gui_resizeGrid (gui,-1, 0); break;
  case XK_Up   : Gui_resizeGrid (gui, 1, 0); break;

  case XK_Next : Gui_densifyGrid (gui, -1); break;
  case XK_Prior: Gui_densifyGrid (gui,  1); break;

  case XK_minus: case XK_KP_Subtract: Gui_zoomGrid (gui, -1); break;
  case XK_plus:  case XK_KP_Add:      Gui_zoomGrid (gui,  1); break;
  
  case XK_V: case XK_v: Gui_toggleTileVisibility (gui); break;
  default: break;
  }
}

void 
onTimerNotify (Ez_event * e)
{
  Gui * gui = ez_get_data (e->win);
  Grid * grid= & gui->app->grid;
  Cell * cell= Grid_getCellAtIndex (grid, grid->selected);
  int nb_turns= cell->nb_turns;
  Cell_incSlerp (cell, gui->rotation_pending);
  ez_send_expose (e->win);
  if (cell->nb_turns != nb_turns) { 
    gui->rotation_pending= 0;
    grid->selected= Index_null ();
  } else {
    ez_start_timer (e->win, ANIM_MILLISECS);
  }
}

/*=============================================================================
 * EZ callback diverter
 *=============================================================================
 */

void 
onEvent (Ez_event * e)
{
  switch (e->type) { 
  case Expose     : onExpose         (e); break;
  case ButtonPress: onButtonPress    (e); break;
  case KeyPress   : onKeyPress       (e); break;
  case TimerNotify: onTimerNotify    (e); break;
  default         :                       break;
  }
}

/*=============================================================================
 * App/Gui init
 *=============================================================================
 */

void 
App_init (App * app, int is_toric, 
          int grid_height, int grid_width, 
          int level, int density, int shuffle)
{
  Grid * grid= & app->grid;
  Grid_init (grid, is_toric, grid_height, grid_width, 
             level, density, shuffle);
}

void 
Gui_init (Gui * gui, App * app, const char win_title [], 
          double cell_radius, int show_tiles)
{
  Coord win_size;

  gui->app = app;
  gui->cell_radius= cell_radius;
  gui->show_tiles= show_tiles;
  win_size= Gui_getIdealWinSize (gui);
  gui->rotation_pending= 0;

  gui->win = ez_window_create (win_size.x, win_size.y, win_title, onEvent); 
  ez_window_dbuf (gui->win, 1);
  ez_set_data (gui->win, gui);
}

/*=============================================================================
 * main
 *=============================================================================
 */

int 
main (void)
{
  App app; Gui gui;

  if (ez_init() < 0) exit(1);
  
  App_init (& app, DEF_TORIC, DEF_GRID_HEIGHT, DEF_GRID_WIDTH, 
            DEF_EDGE_SLOTS, DEF_EDGE_SLOTS, DEF_SHUFFLE);

  Gui_init (& gui, & app, "Loops of EZen", 
            DEF_CELL_RADIUS, DEF_SHOW_TILES);
  
  ez_main_loop ();
  return 0;
}

