
/*=============================================================================
 * jeu-heziom.c: a variant of the puzzle Hexiom
 
 * regis.barbanchon@lif.univ-mrs.fr - 13/08/2011 - version 1.2
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
 *=============================================================================
 */

#include <limits.h>
#include <math.h>
#include "ez-draw.h"

#define PI  3.14159265358979
#define EPS 0.000001

#define MIN_GRID_RADIUS  1
#define MAX_GRID_RADIUS  5
#define MAX_GRID_DIAM (1 + 2*MAX_GRID_RADIUS)

#define DEF_GRID_RADIUS  1

#define DEF_CELL_RADIUS 35
#define NAIL_RADIUS 3.0

#define CELL_STAR_RATIO ( 25.0/35.0)
#define CELL_TILE_RATIO ( 30.0/35.0)

#define DEF_NB_TILES 4
#define DEF_NB_LOCKS 0

#define WIN_LEFT_MARGIN  10
#define WIN_RIGHT_MARGIN 10
#define WIN_TOP_MARGIN   50
#define WIN_BOT_MARGIN   70
#define TEXT_WIDTH 550

#define ANIM_MILLISEC_STEP 20
#define ANIM_DEG_STEP       1
#define ANIM_RAD_STEP       ( ANIM_DEG_STEP * PI/180)

typedef Ez_uint32 ULong;

/*=============================================================================
 * Math
 *=============================================================================
 */

#define ABS(x) ((x) >= 0 ? (x) : -(x))

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))

#define INRANGE(a,x,b) MAX (a, MIN ((x), (b)))

double 
myround (double x)
{
  double f= floor (x);
  return (x < f + 0.5) ? f : f + 1.0;
}

double 
mytrunc (double x)
{
  return (x >= 0) ? (int) x : (int) x - 1;
}

/*=============================================================================
 * Math in the plane
 *=============================================================================
 */

typedef struct Coord { 
  double x, y; 
} Coord;

Coord
Coord_xy (double x, double y)
{
  Coord coord;
  coord.x= x;
  coord.y= y;
  return coord;
}


/*=============================================================================
 * snap to integer components
 *=============================================================================
 */

Coord
Coord_round (Coord p)
{
  return Coord_xy (myround (p.x),
                   myround (p.y));
}

Coord
Coord_trunc (Coord p)
{
  return Coord_xy (mytrunc (p.x),
                   mytrunc (p.y));
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
 * products
 *=============================================================================
 */

double
Coord_cross (Coord p, Coord q)
{
  return p.x * q.y
    -    p.y * q.x;
}

double
Coord_dot (Coord p, Coord q)
{
  return p.x * q.x
    +    p.y * q.y;
}

/*=============================================================================
 * complex multiplications (j denotes -i)
 *=============================================================================
 */

Coord Coord_imul (Coord p) { return Coord_xy (-p.y, p.x); }
Coord Coord_jmul (Coord p) { return Coord_xy ( p.y,-p.x); }

Coord Coord_cmul (Coord p, Coord q) { 
  double x= p.x * q.x - p.y * q.y;
  double y= p.x * q.y + p.y * q.x;
  return Coord_xy (x, y);
}

/*=============================================================================
 * angle
 *=============================================================================
 */

double
Coord_angle (Coord p)
{
  return atan2 (p.y, p.x);
}

Coord
Coord_rphi (double r, double phi)
{
  return Coord_rotate (Coord_xy (r, 0), phi);
}

/*=============================================================================
 * distances
 *=============================================================================
 */

double
Coord_length_sqr (Coord p)
{
  return Coord_dot (p, p);
}

double
Coord_length (Coord p)
{
  return sqrt (Coord_length_sqr (p));
}

double
Coord_distance_sqr (Coord p, Coord q)
{
  return Coord_length_sqr (Coord_sub (q, p));
}

double
Coord_distance (Coord p, Coord q)
{
  return sqrt (Coord_distance_sqr (p, q));
}

Coord *
Coord_nearest_2 (Coord pointer, Coord * a, Coord * b)
{
  double pa= Coord_distance_sqr (pointer, * a);
  double pb= Coord_distance_sqr (pointer, * b);
  return (pa < pb) ? a : b;
}

Coord *
Coord_nearest_3 (Coord pointer, Coord * a, Coord * b, Coord * c)
{
  Coord * ab= Coord_nearest_2 (pointer, a , b);
  return      Coord_nearest_2 (pointer, ab, c);
}

Coord *
Coord_nearest_4 (Coord pointer, Coord * a, Coord * b, Coord * c, Coord * d)
{
  Coord * ab= Coord_nearest_2 (pointer, a , b );
  Coord * cd= Coord_nearest_2 (pointer, c , d );
  return      Coord_nearest_2 (pointer, ab, cd);
}

/*=============================================================================
 * 
 *=============================================================================
 */

void
Coord_regularPolygon (Coord vertices[], int nb_vertices,
                      Coord center, double radius, double start_angle)
{
  double sector= 2.0 * PI / nb_vertices;
  Coord start= Coord_xy (radius, 0.0);
  int k;
  for (k= 0; k < nb_vertices; k++) {
    double angle= start_angle + k * sector;
    Coord rel= Coord_rotate (start, angle);
    vertices [k]= Coord_add (center, rel);
  }
}

/*=============================================================================
 * Homogeneous coordinates
 *=============================================================================
 */

typedef struct Homo {
  Coord  c;
  double t;
} Homo;

Homo 
Homo_ct (Coord c, double t) 
{
  Homo homo;
  homo.c= c;
  homo.t= t;
  return homo;
}

Homo
Homo_xyt (double x, double y, double t)
{
  return Homo_ct (Coord_xy (x, y), t);
}

Homo
Homo_scale (Homo h, double scalar)
{
  return Homo_ct (Coord_scale (h.c, scalar), h.t * scalar);
}

Homo
Homo_add (Homo g, Homo h)
{
  return Homo_ct (Coord_add (g.c, h.c), g.t + h.t);
}

Homo
Homo_sub (Homo g, Homo h)
{
  return Homo_ct (Coord_sub (g.c, h.c), g.t - h.t);
}

double
Homo_dot (Homo g, Homo h)
{
  return Coord_dot (g.c, h.c) + g.t * h.t; 
}

/*============================================================================
 * homo canonic
 *============================================================================
 */

Homo
Homo_canonic (Homo  h)
{
  if (fabs (h.t) > EPS) return Homo_ct (Coord_scale (h.c, 1.0/h.t), 1.0); 
  if (      h.t  < 0  ) return Homo_ct (Coord_neg   (h.c)         , 0.0);
  return h;      
}

/*=============================================================================
 * homo complex multiplication by i and j (with j= -i)
 *=============================================================================
 */

Homo Homo_imul (Homo h) { return Homo_ct (Coord_imul (h.c), h.t); }
Homo Homo_jmul (Homo h) { return Homo_ct (Coord_jmul (h.c), h.t); }

/*=============================================================================
 * homo negations
 *=============================================================================
 */

Homo Homo_neg    (Homo h) { return Homo_ct (Coord_neg    (h.c), -h.t); }
Homo Homo_neg_xy (Homo h) { return Homo_ct (Coord_neg    (h.c),  h.t); }

Homo Homo_neg_x  (Homo h) { return Homo_ct (Coord_neg_x  (h.c),  h.t); }
Homo Homo_neg_y  (Homo h) { return Homo_ct (Coord_neg_y  (h.c),  h.t); }
Homo Homo_neg_t  (Homo h) { return Homo_ct (              h.c , -h.t); }

/*=============================================================================
 * homo cross products
 *=============================================================================
 */

double 
Homo_cross_xy (Homo g, Homo h) 
{ 
  return Coord_cross (g.c, h.c);
}

double
Homo_cross_xt (Homo g, Homo h)
{
  return g.c.x * h.t 
    -    g.t   * h.c.x;
}

double
Homo_cross_yt (Homo g, Homo h)
{
  return g.c.y * h.t 
    -    g.t   * h.c.y;
}

Homo
Homo_cross (Homo g, Homo h)
{
  return Homo_xyt (+Homo_cross_yt (g, h),
                   -Homo_cross_xt (g, h),
                   +Homo_cross_xy (g, h));
}

/*=============================================================================
 * Homogeneous Matrix
 *=============================================================================
 */

typedef struct Matrix { 
  Homo u, v, w;
} Matrix;

Matrix
Matrix_uvw (Homo u, Homo v, Homo w)
{
  Matrix matrix;
  matrix.u= u;
  matrix.v= v;
  matrix.w= w;
  return matrix;
}

/*=============================================================================
 * rows, transpose
 *=============================================================================
 */

Homo Matrix_row_x (Matrix m){ return Homo_xyt (m.u.c.x,  m.v.c.x,  m.w.c.x); }
Homo Matrix_row_y (Matrix m){ return Homo_xyt (m.u.c.y,  m.v.c.y,  m.w.c.y); }
Homo Matrix_row_t (Matrix m){ return Homo_xyt (m.u.t  ,  m.v.t  ,  m.w.t  ); }

Matrix
Matrix_transpose (Matrix m)
{
  return Matrix_uvw (Matrix_row_x (m),
                     Matrix_row_y (m),
                     Matrix_row_t (m));
}

/*=============================================================================
 * determinant, cofactors, adjugate, inverse
 *=============================================================================
 */

Matrix
Matrix_scale (Matrix m, double scalar)
{
  return Matrix_uvw (Homo_scale (m.u, scalar),
                     Homo_scale (m.v, scalar),
                     Homo_scale (m.w, scalar));
}

double
Matrix_det (Matrix m)
{
  return m.u.t * Homo_cross_xy (m.v, m.w)
    -    m.v.t * Homo_cross_xy (m.u, m.w) 
    +    m.w.t * Homo_cross_xy (m.u, m.v);
}

Matrix
Matrix_com (Matrix m)
{
  Homo u= Homo_xyt (+Homo_cross_yt (m.v, m.w),
                    -Homo_cross_xt (m.v, m.w),
                    +Homo_cross_xy (m.v, m.w));
  
  Homo v= Homo_xyt (-Homo_cross_yt (m.u, m.w),
                    +Homo_cross_xt (m.u, m.w),
                    -Homo_cross_xy (m.u, m.w));
    
  Homo w= Homo_xyt (+Homo_cross_yt (m.u, m.v),
                    -Homo_cross_xt (m.u, m.v),
                    +Homo_cross_xy (m.u, m.v));
  
  return Matrix_uvw (u, v, w);
}

Matrix
Matrix_adj (Matrix m)
{
  return Matrix_transpose (Matrix_com (m));
}

Matrix
Matrix_inv (Matrix m)
{
  double idet= 1.0 / Matrix_det (m);
  return Matrix_scale (Matrix_adj (m), idet);
}

/*=============================================================================
 * application, multiplication.
 * is matrix addition ever useful?
 *=============================================================================
 */

Homo
Matrix_apply (Matrix m, Homo h)
{
  Matrix mt= Matrix_transpose (m);

  double x= Homo_dot (mt.u, h);
  double y= Homo_dot (mt.v, h);
  double t= Homo_dot (mt.w, h);
  
  return Homo_xyt (x, y, t);
}

Matrix
Matrix_mul (Matrix a, Matrix b)
{
  Matrix at= Matrix_transpose (a);

  Homo u= Homo_xyt (Homo_dot (at.u, b.u),
                    Homo_dot (at.v, b.u),
                    Homo_dot (at.w, b.u));

  Homo v= Homo_xyt (Homo_dot (at.u, b.v),
                    Homo_dot (at.v, b.v),
                    Homo_dot (at.w, b.v));

  Homo w= Homo_xyt (Homo_dot (at.u, b.w),
                    Homo_dot (at.v, b.w),
                    Homo_dot (at.w, b.w));

  return Matrix_uvw (u, v, w);
}

Matrix
Matrix_add (Matrix a, Matrix b)
{
  return Matrix_uvw (Homo_add (a.u, b.u),
                     Homo_add (a.v, b.v),
                     Homo_add (a.w, b.w));
}

/*=============================================================================
 * basic transformations: unused, untested
 *=============================================================================
 */

Matrix
Matrix_id (void)
{
  Homo u= Homo_xyt (1.0, 0.0, 0.0);
  Homo v= Homo_xyt (0.0, 1.0, 0.0);
  Homo w= Homo_xyt (0.0, 0.0, 1.0);
  return Matrix_uvw (u, v, w);
}

Matrix
Matrix_shift (Coord delta) 
{
  Homo u= Homo_xyt (  1.0, 0.0, 0.0);
  Homo v= Homo_xyt (  0.0, 1.0, 0.0);
  Homo w= Homo_ct  (   delta  , 1.0);
  return Matrix_uvw (u, v, w); 
}

Matrix
Matrix_zoom (Coord f) 
{
  Homo u= Homo_xyt (  f.x, 0.0, 0.0);
  Homo v= Homo_xyt (  0.0, f.y, 0.0);
  Homo w= Homo_xyt (  0.0, 0.0, 1.0);
  return Matrix_uvw (u, v, w); 
}

Matrix
Matrix_rot (double angle) 
{
  double cos_a= cos (angle);
  double sin_a= sin (angle);
  Homo u= Homo_xyt ( cos_a, sin_a, 0.0);
  Homo v= Homo_xyt (-sin_a, cos_a, 0.0);
  Homo w= Homo_xyt (   0.0,   0.0, 1.0);
  return Matrix_uvw (u, v, w); 
}

Matrix
Matrix_hshear (double dx)
{
  Homo u= Homo_xyt (1.0, 0.0, 0.0);
  Homo v= Homo_xyt ( dx, 1.0, 0.0);
  Homo w= Homo_xyt (0.0, 0.0, 1.0);
  return Matrix_uvw (u, v, w);  
}

Matrix
Matrix_vshear (double dy)
{
  Homo u= Homo_xyt (1.0,  dy, 0.0);
  Homo v= Homo_xyt (0.0, 1.0, 0.0);
  Homo w= Homo_xyt (0.0, 0.0, 1.0);
  return Matrix_uvw (u, v, w);  
}

/*=============================================================================
 * more handy transformations: unused, untested
 *=============================================================================
 */

Matrix
Matrix_zoom_around (Coord f, Coord center) 
{
  Matrix shift1= Matrix_shift (center);
  Matrix zoom  = Matrix_zoom  (f);
  Matrix shift2= Matrix_shift (Coord_neg (center));
  return Matrix_mul (shift2, Matrix_mul (zoom, shift1));
}

Matrix
Matrix_rot_around (double angle, Coord center) 
{
  Matrix shift1= Matrix_shift (center);
  Matrix rot   = Matrix_rot   (angle);
  Matrix shift2= Matrix_shift (Coord_neg (center));
  return Matrix_mul (shift2, Matrix_mul (rot, shift1));
}

Matrix
Matrix_hshear_around (double dx, double y_axis) 
{
  Matrix shift1= Matrix_shift (Coord_xy (0, +y_axis));
  Matrix hshear= Matrix_hshear (dx);
  Matrix shift2= Matrix_shift (Coord_xy (0, -y_axis));
  return Matrix_mul (shift2, Matrix_mul (hshear, shift1));
}

Matrix
Matrix_vshear_around (double dy, double x_axis) 
{
  Matrix shift1= Matrix_shift (Coord_xy (+x_axis, 0));
  Matrix vshear= Matrix_hshear (dy);
  Matrix shift2= Matrix_shift (Coord_xy (-x_axis, 0));
  return Matrix_mul (shift2, Matrix_mul (vshear, shift1));
}

/*=============================================================================
 * Additional EZ drawing routines using Coord
 *=============================================================================
 */

int
myez_random (int min, int max)
{
  return min + ez_random (max-min+1);
}

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
myez_draw_hexagon (Ez_window win, Coord hexa [6])
{
  int k, j;
  for (k= 0, j= 5; k < 6; j=k, k++) {
    myez_draw_line (win, hexa [j], hexa [k]);
  }
}

void
myez_fill_hexagon (Ez_window win, Coord hexa [6])
{
  myez_fill_quadrangle (win, hexa[0], hexa[1], hexa[2], hexa[3]);
  myez_fill_quadrangle (win, hexa[3], hexa[4], hexa[5], hexa[0]);
}

void
myez_draw_circle (Ez_window win, Coord center, double r)
{
  ez_draw_circle (win, 
                  myround (center.x - r), myround (center.y - r), 
                  myround (center.x + r), myround (center.y + r));
}

void
myez_fill_circle (Ez_window win, Coord center, double r)
{
  ez_fill_circle (win, 
                  myround (center.x - r    ), myround (center.y - r    ), 
                  myround (center.x + r - 1), myround (center.y + r - 1));
}

void
myez_draw_circles (Ez_window win, int n, Coord centers [], double radius)
{
  int k;
   for (k= 0; k < n; k++) {
    myez_draw_circle (win, centers[k], radius);
  }
}

void
myez_fill_circles (Ez_window win, int n, Coord centers [], double radius)
{
  int k;
   for (k= 0; k < n; k++) {
    myez_fill_circle (win, centers[k], radius);
  }
}

char myez_dmd_digits [10][7][6]= {
  { ".000.","0...0","0...0","0...0","0...0","0...0",".000." },
  { "..1..",".11..","1.1..","..1..","..1..","..1..",".111." },
  { ".222.","2...2","....2","...2.","..2..",".2...","22222" },
  { ".333.","3...3","....3",".333.","....3","3...3",".333." },
  { "4....","4..4.","4..4.","4..4.","44444","...4.","...4." },
  { "5555.","5....","5....","5555.","....5","....5","5555." },
  { ".666.","6...6","6....","6666.","6...6","6...6",".666." },
  { "77777","7...7","....7","...7.","..7..",".7...",".7..." },
  { ".888.","8...8","8...8",".888.","8...8","8...8",".888." },
  { ".999.","9...9","9...9",".9999","....9","9...9",".999." }
};

void
myez_draw_dmd (Ez_window win, int n, Coord center, ULong color)
{
  int i, j;
  for (i= 0; i < 7; i++) {
    for (j= 0; j < 5; j++) {
      Coord p= Coord_xy (center.x+(j-2.0)*6 +0.5,
                         center.y+(i-3.0)*6 +0.5);
      Coord q= Coord_add (p, Coord_xy ( 1, -1));
      if (myez_dmd_digits [n][i][j]== '.') continue;
      ez_set_color (color) ;    myez_fill_circle (win, p, 3.5);
      ez_set_color (ez_yellow); myez_fill_circle (win, q, 2.0);
      ez_set_color (ez_black);  myez_draw_circle (win, p, 3.5);
    }
  }
}

void
myez_draw_crown (Ez_window win, int n, Coord in[], Coord out[], 
                 int start, int length)
{
  int k, j;
  for (k= start; length; k=j, length--) {
    j= (k+1)%n;
    myez_draw_quadrangle (win, in[k], out[k], out[j], in[j]);
  }
}

void
myez_fill_crown (Ez_window win, int n, Coord in[], Coord out[],
                 int start, int length)
{
  int k, j;
  for (k= start; length; k=j, length--) {
    j= (k+1)%n;
    myez_fill_quadrangle (win, in[k], out[k], out[j], in[j]);
  }
}

void
myez_draw_star (Ez_window win, int n, Coord in[], Coord out[], 
                int start, int length)
{
  int k, j;
  for (k= start; length; k=j, length--) {
    j= (k+1)%n;
    myez_draw_line (win, in[k], out[j]);
    myez_draw_line (win, in[j], out[j]);
  }
}

void
myez_fill_star (Ez_window win, int n, Coord in[], Coord out[], 
                int start, int length)
{
  int k, j;
  for (k= start; length; k=j, length--) {
    j= (k+1)%n;
    myez_fill_triangle (win, in[k], out[j], in [j]);
  }
}

/*=============================================================================
 * Additional EZ drawing routines... unused in final version 
 *=============================================================================
 */

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
 * Indexes
 *=============================================================================
 */

typedef struct Index { int i, j; } Index;

Index
Index_ij (int i, int j) 
{
  Index index;
  index.i= i;
  index.j= j;
  return index;
}

Index Index_null (void) { return Index_ij (INT_MIN, INT_MIN); }


int
Index_isEqual (Index index, Index other)
{
  return index.i == other.i 
    &&   index.j == other.j;
}

int 
Index_isNull (Index index) 
{
  return Index_isEqual (index, Index_null ());
}

/*=============================================================================
 * Puzzle structs
 *=============================================================================
 */

typedef struct Cell {
  int constraint; /* -2: tilemark, -1: hole, >=0 neighbors constraint */
  Index initial; /* official solution */
  int is_locked; 
} Cell;

void
Cell_swap (Cell * cell1, Cell * cell2)
{
  Cell tmp= * cell1;
  * cell1= * cell2;
  * cell2= tmp;
}

typedef struct Grid {
  int radius, nb_tiles, nb_locks;
  Cell cells [MAX_GRID_DIAM][MAX_GRID_DIAM];
  Cell saved;
} Grid;

int 
hive_area (int r)
{
  return 3*r*r + 3*r + 1;
}

int
Grid_getArea (Grid * grid)
{
  return hive_area (grid->radius);
}

int 
Grid_getIMin (Grid * grid, int j)
{
  return - MIN (grid->radius + j, grid->radius);
}

int
Grid_getIMax (Grid * grid, int j)
{
  return + MIN (grid->radius - j, grid->radius);
}

int
Grid_indexIsOutside (Grid * grid, Index index)
{
  return index.j > +grid->radius
    ||   index.j < -grid->radius
    ||   index.i < Grid_getIMin (grid, index.j) 
    ||   index.i > Grid_getIMax (grid, index.j);
}

Index
Grid_getRandomIndex (Grid * grid)
{
  int j= myez_random (-grid->radius, +grid->radius);
  int iMin= Grid_getIMin (grid, j);
  int iMax= Grid_getIMax (grid, j);
  int i= myez_random (iMin, iMax);
  return Index_ij (i, j);
}

Cell *
Grid_getCellAtIndex (Grid * grid, Index index)
{
  int i, j;
  if (Grid_indexIsOutside (grid, index)) return NULL;
  i= MAX_GRID_RADIUS + index.i;
  j= MAX_GRID_RADIUS + index.j;
  return & grid->cells [i][j]; 
}

void  
Grid_swapSavedCell (Grid * grid, Index index)
{
  Cell * cell= Grid_getCellAtIndex (grid, index);
  Cell_swap (cell, & grid->saved);
}

int
Grid_getWeightAtIndex (Grid * grid, Index index)
{
  Cell * cell= Grid_getCellAtIndex (grid, index);
  return cell && cell->constraint != -1;
}

int
Grid_countCellNeighbors (Grid * grid, Index index)
{
  return Grid_getWeightAtIndex (grid, Index_ij (index.i+1, index.j  ))
    +    Grid_getWeightAtIndex (grid, Index_ij (index.i-1, index.j  ))
    +    Grid_getWeightAtIndex (grid, Index_ij (index.i  , index.j+1))
    +    Grid_getWeightAtIndex (grid, Index_ij (index.i  , index.j-1)) 
    +    Grid_getWeightAtIndex (grid, Index_ij (index.i+1, index.j-1))
    +    Grid_getWeightAtIndex (grid, Index_ij (index.i-1, index.j+1));
}

int
Grid_countConflicts (Grid * grid)
{
  Index index;
  int nb_neighbors, count= (grid->saved.constraint != -1);
  for (index.j= -grid->radius; index.j<= grid->radius; index.j++) {
    int iMin= Grid_getIMin (grid, index.j);
    int iMax= Grid_getIMax (grid, index.j);
    for (index.i= iMin; index.i<= iMax; index.i++) {
      Cell * cell= Grid_getCellAtIndex (grid, index);
      if (cell->constraint == -1) continue;
      nb_neighbors= Grid_countCellNeighbors (grid, index);
      count+= (cell->constraint != nb_neighbors);
    }
  }
  return count;
}

void
Grid_emptyField (Grid * grid)
{
  int i, j;
  grid->saved.constraint= -1;
  grid->saved.initial= Index_null ();
  grid->saved.is_locked= 0;

  for (i= 0; i < MAX_GRID_DIAM; i++) {
    for (j= 0; j < MAX_GRID_DIAM; j++) {
      Cell * cell= & grid->cells [i][j];
      cell->constraint= -1;
      cell->initial= Index_ij (i - MAX_GRID_RADIUS, 
                               j - MAX_GRID_RADIUS);
      cell->is_locked= 0;
    }
  }
}

void
Grid_premarkField (Grid * grid, int premark) 
{
  Index index; 
  for (index.j= -grid->radius; index.j<= grid->radius; index.j++) {
    int iMin= Grid_getIMin (grid, index.j);
    int iMax= Grid_getIMax (grid, index.j);
    for (index.i= iMin; index.i<= iMax; index.i++) {
      Cell * cell= Grid_getCellAtIndex (grid, index);
      cell->constraint= premark;
    }
  }
}

void
Grid_prelockField (Grid * grid, int prelock) 
{
  Index index; 
  for (index.j= -grid->radius; index.j<= grid->radius; index.j++) {
    int iMin= Grid_getIMin (grid, index.j);
    int iMax= Grid_getIMax (grid, index.j);
    for (index.i= iMin; index.i<= iMax; index.i++) {
      Cell * cell= Grid_getCellAtIndex (grid, index);
      cell->is_locked= prelock;
    }
  }
}

void
Grid_postmarkField (Grid * grid, int postmark, int nb_postmarks) 
{
  int k;
  for (k= 0; k < nb_postmarks; /* dummy */) {
    Index alea= Grid_getRandomIndex (grid);
    Cell * cell= Grid_getCellAtIndex (grid, alea);
    if (cell->constraint == postmark) continue;
    cell->constraint= postmark;
    k++;
  }
}

void
Grid_postlockField (Grid * grid, int postlock, int nb_postlocks) 
{
  int k;
  for (k= 0; k < nb_postlocks; /* dummy */) {
    Index alea= Grid_getRandomIndex (grid);
    Cell * cell= Grid_getCellAtIndex (grid, alea);
    if (cell->is_locked == postlock) continue;
    cell->is_locked= postlock;
    k++;
  }
}


void
Grid_markField (Grid * grid)
{
  int area= Grid_getArea (grid);
  int nb_holes= area - grid->nb_tiles;
  int dense= (grid->nb_tiles > area/2);

  int nb_postmarks= dense ? nb_holes : grid->nb_tiles;
  int postmark    = dense ? -1       : -2;
  int premark     = dense ? -2       : -1;

  Grid_premarkField  (grid, premark);
  Grid_postmarkField (grid, postmark, nb_postmarks);
}

void
Grid_lockField (Grid * grid)
{
  int area= Grid_getArea (grid);
  int nb_free= area - grid->nb_locks;
  int dense= (grid->nb_locks > area/2);
 
  int nb_postlocks= dense ? nb_free : grid->nb_locks;
  int postlock    = dense ?  0      : 1;
  int prelock     = dense ?  1      : 0;
 
  Grid_prelockField  (grid, prelock);
  Grid_postlockField (grid, postlock, nb_postlocks);
}

void
Grid_weightField (Grid * grid)
{
  Index index;
  for (index.j= -grid->radius; index.j<= grid->radius; index.j++) {
    int iMin= Grid_getIMin (grid, index.j);
    int iMax= Grid_getIMax (grid, index.j);
    for (index.i= iMin; index.i<= iMax; index.i++) {
      Cell * cell= Grid_getCellAtIndex (grid, index);
      if (cell->constraint != -2) continue;
      cell->constraint= Grid_countCellNeighbors (grid, index); 
    }
  }
}

void
Grid_shuffleWave (Grid * grid)
{
  int k, n= Grid_getArea (grid);
  for (k= 0; k < n; /* dummy */) {
    Index index1= Grid_getRandomIndex (grid);
    Index index2= Grid_getRandomIndex (grid); 
    Cell * cell1= Grid_getCellAtIndex (grid, index1);
    Cell * cell2= Grid_getCellAtIndex (grid, index2);
    if (cell1->is_locked || cell2->is_locked) continue;
    Cell_swap (cell1, cell2);
    k++;
  }
}

void
Grid_shuffle (Grid * grid)
{
  int k, n= 10;
  for (k= 0; k < n; k++) {
    Grid_shuffleWave (grid);
    if (Grid_countConflicts (grid)) return;
  }
}

int
Grid_unshuffleWave (Grid * grid)
{
  Index index;
  int nb_swaps= 0;
  for (index.j= -grid->radius; index.j<= grid->radius; index.j++) {
    int iMin= Grid_getIMin (grid, index.j);
    int iMax= Grid_getIMax (grid, index.j);
    for (index.i= iMin; index.i<= iMax; index.i++) {
      Cell tmp, * cell2, * cell1;
      cell1= Grid_getCellAtIndex (grid, index);
      cell2= Grid_getCellAtIndex (grid, cell1->initial);
      if (cell1 == cell2) continue;
      tmp= * cell1;
      * cell1= * cell2;
      * cell2= tmp;
      nb_swaps ++;
    }
  }
  return nb_swaps;
}

void
Grid_unshuffle (Grid * grid)
{
  int nb_swaps;
  do { 
    nb_swaps= Grid_unshuffleWave (grid); 
  } while (nb_swaps);
}

void
Grid_getWeights (Grid * grid, int counted[7])
{
  int k;
  Index index;
  for (k= 0; k < 7; k++) {
    counted [k]= 0;
  }
  for (index.j= -grid->radius; index.j<= grid->radius; index.j++) {
    int iMin= Grid_getIMin (grid, index.j);
    int iMax= Grid_getIMax (grid, index.j);
    for (index.i= iMin; index.i<= iMax; index.i++) {
      Cell * cell = Grid_getCellAtIndex (grid, index);
      if (cell->constraint < 0) continue;
      counted [cell->constraint]++;
    }
  }
}

int
Grid_checkWeights (Grid * grid, int counted[7], int wanted[7])
{
  int k, n= 0;
  (void) grid;
  for (k= 0; k < 7; k++) {
    printf ("%d:(%d/%d)  ", k, counted[k], wanted [k]);
    n+= (counted  [k] != wanted [k]); 
  }
  printf ("  N=%d\n",n);
  return n;
}

void
Grid_init (Grid * grid, int radius, int nb_tiles, int nb_locks)
{
  int area;
  grid->radius  = INRANGE (MIN_GRID_RADIUS, radius, MAX_GRID_RADIUS);
  area= Grid_getArea (grid);

  grid->nb_tiles= INRANGE (2, nb_tiles, area-1);
  grid->nb_locks= INRANGE (0, nb_locks, area/3);
 
  Grid_emptyField  (grid);
  Grid_markField   (grid);
  Grid_weightField (grid);
  Grid_lockField   (grid);
  Grid_shuffle     (grid);
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

  double cell_radius, cell_angle;
  Matrix world_sys, local_sys;
  
  Ez_window win;
  Coord pointer;
  Index selected;
} Gui;

Coord
Gui_getIdealGridSize (Gui * gui)
{
  Grid * grid= & gui->app->grid;
  Coord a= Coord_rphi (gui->cell_radius, +1*PI/3);
  Coord b= Coord_rphi (gui->cell_radius, +2*PI/3);
  Coord m= Coord_add (a, b);
  double side= myround (2 * m.y * grid->radius + m.y); 
  return Coord_xy (MAX (side, TEXT_WIDTH), side);
}

Coord
Gui_getIdealWinSize (Gui * gui)
{
  Coord ideal= Gui_getIdealGridSize (gui); 
  Coord margin= Coord_xy (WIN_LEFT_MARGIN + WIN_RIGHT_MARGIN,
                          WIN_TOP_MARGIN  + WIN_BOT_MARGIN); 
  return Coord_add (ideal, margin);
}

Coord
Gui_getGridCenter (Gui * gui)
{
  Coord ideal= Gui_getIdealGridSize (gui);
  Coord margin= Coord_xy (WIN_LEFT_MARGIN, WIN_TOP_MARGIN);
  return Coord_add (margin, Coord_scale (ideal, 0.5));
}

Coord
Gui_getTitleCenter (Gui * gui)
{
  Coord ideal= Gui_getIdealWinSize (gui);
  return Coord_xy (ideal.x / 2, WIN_TOP_MARGIN/2);
}

Coord
Gui_getKeyInfoCenter (Gui * gui)
{
  Coord ideal= Gui_getIdealWinSize (gui);
  return Coord_xy (ideal.x / 2, ideal.y - WIN_BOT_MARGIN/2);
}



void
Gui_initCoordSystems (Gui * gui)
{
  Coord a= Coord_rphi (gui->cell_radius, gui->cell_angle);
  Coord b= Coord_rotate (a, -1*PI/3);
  Coord c= Coord_rotate (a, -2*PI/3);

  Coord u= Coord_add (a, b);
  Coord v= Coord_add (b, c); 
  Coord w= Gui_getGridCenter (gui);

  gui->local_sys= Matrix_uvw (Homo_ct (u,0), Homo_ct (v,0), Homo_ct (w,1));
  gui->world_sys= Matrix_inv (gui->local_sys);
}

Coord 
Gui_worldToLocal (Gui *gui, Coord world_coord, double t)

{
  Homo world_homo=  Homo_ct (world_coord, t);
  Homo local_homo=  Matrix_apply (gui->world_sys, world_homo);
  return local_homo.c;
}

Coord 
Gui_localToWorld (Gui *gui, Coord local_coord, double t)
{
  Homo local_homo=  Homo_ct (local_coord, t);
  Homo world_homo=  Matrix_apply (gui->local_sys, local_homo);
  return world_homo.c;
}

Coord
Gui_getCoordAtindex (Gui * gui, Index index)
{
  Coord  local_coord= Coord_add (Coord_scale (gui->local_sys.u.c, index.j),
                                 Coord_scale (gui->local_sys.v.c, index.i));
  return Coord_add (gui->local_sys.w.c, local_coord);
}

Index
Gui_getLozangeIndexAtCoord (Gui * gui, Coord world_coord)
{
  Coord local_coord= Gui_worldToLocal (gui, world_coord, 1.0);
  Coord snapped= Coord_trunc (local_coord);
  return Index_ij (snapped.y, snapped.x);
}

void
Gui_getLozangeIndexesAtCoord (Gui * gui, Coord world_coord, Index indexes[4])
{
  indexes[0]= Gui_getLozangeIndexAtCoord (gui, world_coord);
  indexes[1]= Index_ij (indexes[0].i+1, indexes[0].j  );
  indexes[2]= Index_ij (indexes[0].i+1, indexes[0].j+1);
  indexes[3]= Index_ij (indexes[0].i  , indexes[0].j+1);
}

Index
Gui_getIndexAtCoord (Gui * gui, Coord world_coord)
{
  int k;
  Index i[4],   nearest_i; 
  Coord c[4], * nearest_c;
  Gui_getLozangeIndexesAtCoord (gui, world_coord, i);
  for (k= 0; k < 4; k++) {
    c[k]= Gui_localToWorld (gui, Coord_xy (i[k].j, i[k].i), 1.0);
  }
  nearest_c= Coord_nearest_4 (world_coord, c+0, c+1, c+2, c+3);
  nearest_i= i[nearest_c - c];
  return Grid_indexIsOutside (& gui->app->grid, nearest_i) ?
    Index_null () : nearest_i;
}

void
Gui_drawCellSlot (Gui * gui, Cell * cell, Coord coord)
{
  Coord in [6], out [6];
  double out_radius= gui->cell_radius;
  double  in_radius= gui->cell_radius * CELL_STAR_RATIO;
  double out_angle= gui->cell_angle;
  double  in_angle= out_angle + PI/6;

  Coord_regularPolygon (in,  6, coord,  in_radius,  in_angle);
  Coord_regularPolygon (out, 6, coord, out_radius, out_angle);

  if (! cell->is_locked) {
    ez_set_color (ez_grey  ); myez_fill_star    (gui->win, 6, in, out, 0, 6);
    ez_set_color (ez_grey  ); myez_fill_hexagon (gui->win, in);
  } else {
    ez_set_color (ez_grey  ); myez_draw_star    (gui->win, 6,in, out, 0, 6);
    ez_set_color (ez_yellow); myez_fill_circles (gui->win, 6,in, NAIL_RADIUS);
    ez_set_color (ez_black ); myez_draw_circles (gui->win, 6,in, NAIL_RADIUS);
  }
}

void
Gui_drawCellTile (Gui * gui, Cell * cell, Coord coord, ULong dmd_color)
{
  Coord in [6], out [6];
  double out_radius= gui->cell_radius;
  double  in_radius= gui->cell_radius * CELL_TILE_RATIO;
  double angle= gui->cell_angle;
  ULong crown1= cell->is_locked ? ez_red    : ez_green;
  ULong crown2= cell->is_locked ? ez_magenta: ez_cyan;

  Coord_regularPolygon (in,  6, coord,  in_radius, angle);
  Coord_regularPolygon (out, 6, coord, out_radius, angle);

  ez_set_color (ez_black); myez_fill_hexagon (gui->win, in);
  ez_set_color (crown1  ); myez_fill_crown   (gui->win, 6, in, out, 1, 3);
  ez_set_color (crown2  ); myez_fill_crown   (gui->win, 6, in, out, 4, 3);
  ez_set_color (ez_black); myez_draw_crown   (gui->win, 6, in, out, 0, 6);
 
  myez_draw_dmd (gui->win, cell->constraint, coord, dmd_color);

  if (! cell->is_locked) return;
  ez_set_color (ez_yellow); myez_fill_circles (gui->win, 6, in, NAIL_RADIUS);
  ez_set_color (ez_black ); myez_draw_circles (gui->win, 6, in, NAIL_RADIUS);
}

void
Gui_drawCell (Gui * gui, Cell * cell, Coord coord, ULong dmd_color)
{
  if (cell->constraint == -1) {
    Gui_drawCellSlot (gui, cell, coord);
  } else {
    Gui_drawCellTile (gui, cell, coord, dmd_color);
  }
}

ULong
Gui_getCellDmdColor (Gui * gui, Index index)
{
  Grid * grid= & gui->app->grid;
  Cell * cell= Grid_getCellAtIndex (grid, index);
  int count= Grid_countCellNeighbors (grid, index);
  return (count <  cell->constraint) ? ez_blue
    :    (count == cell->constraint) ? ez_green
    :                                  ez_red;
}

void
Gui_drawGrid (Gui * gui)
{
  Grid * grid= & gui->app->grid;
  Index index;
  for (index.j= -grid->radius; index.j<= grid->radius; index.j++) {
    int iMin= Grid_getIMin (grid, index.j);
    int iMax= Grid_getIMax (grid, index.j);
    for (index.i= iMin; index.i<= iMax; index.i++) {
      Coord coord= Gui_getCoordAtindex (gui, index);
      Cell * cell= Grid_getCellAtIndex (grid, index);
      ULong dmd_color= Gui_getCellDmdColor (gui, index);
      Gui_drawCell (gui, cell, coord, dmd_color);
    }
  }
}

void
Gui_drawSelected (Gui * gui)
{
  Grid * grid= & gui->app->grid;
  Gui_drawCell (gui, & grid->saved, gui->pointer, ez_magenta);
}  

void
Gui_drawTitle (Gui * gui, int nb_conflicts)
{
  Grid * grid= & gui->app->grid;
  Coord coord= Gui_getTitleCenter (gui);
  char * caption= nb_conflicts ? "MAKE ALL TILES GREEN!" : "WELL DONE!";
  ez_set_nfont (1); 
  ez_set_color (nb_conflicts ? ez_red : ez_blue);
  ez_draw_text (gui->win, EZ_MC, coord.x, coord.y,
                "Grid Size: %d, Locks: %d, Tiles: %d\n%s",
                grid->radius, grid->nb_locks, grid->nb_tiles, caption);
}

void
Gui_drawKeyInfo (Gui * gui)
{
  Coord coord= Gui_getKeyInfoCenter (gui);
  ez_set_nfont (0); ez_set_color (ez_blue);
  ez_draw_text (gui->win, EZ_MC, coord.x, coord.y,
                "[Home/End] [L/l]: Change number of locks, "
                "[Ins/Del] [T/t]: Change number of tiles,\n"
                "[PgUp/PgDn] [G/g]: Change grid size, "
                "[+/-]: Rotate grid, "
                "[Return]: Rotate grid to Zero,\n"
                "[Space]: New grid, "
                "[S]: Shuffle, "
                "[U]: Unshuffle, "
                "[Esc/Q]: Quit.");
}

void
Gui_resizeWinToIdeal (Gui * gui)
{
  Coord ideal= Gui_getIdealWinSize (gui);
  ez_window_set_size (gui->win, ideal.x, ideal.y);
  Gui_initCoordSystems (gui);
  ez_send_expose (gui->win);
}

/*=============================================================================
 * Grid reset by user
 *=============================================================================
 */ 

void
Gui_checkConflicts (Gui * gui)
{
 Grid * grid= & gui->app->grid;
 if (! Grid_countConflicts (grid)) 
   ez_start_timer (gui->win, ANIM_MILLISEC_STEP);
}

void
Gui_unshuffleGrid (Gui * gui)
{
  Grid * grid= & gui->app->grid;
  ez_start_timer (gui->win, -1);
  Grid_unshuffle (grid);
  Gui_checkConflicts (gui);
  ez_send_expose (gui->win);
}

void
Gui_shuffleGrid (Gui * gui)
{
  Grid * grid= & gui->app->grid;  
  ez_start_timer (gui->win, -1);
  Grid_shuffle (grid);
  Gui_checkConflicts (gui);
  ez_send_expose (gui->win);
}

void
Gui_changeGridTiles (Gui * gui)
{
  Grid * grid= & gui->app->grid;
  ez_start_timer (gui->win, -1);
  Grid_init (grid, grid->radius, grid->nb_tiles, grid->nb_locks);
  Grid_shuffle (grid);
  Gui_checkConflicts (gui);
  ez_send_expose (gui->win);
}

void
Gui_changeGridNbLocks (Gui * gui, int dl)
{
  Grid * grid= & gui->app->grid;
  ez_start_timer (gui->win, -1);
  Grid_init (grid, grid->radius, grid->nb_tiles, grid->nb_locks + dl);
  Grid_shuffle (grid);
  Gui_checkConflicts (gui);
  ez_send_expose (gui->win);
}

void
Gui_changeGridNbTiles (Gui * gui, int dt)
{
  Grid * grid= & gui->app->grid;
  ez_start_timer (gui->win, -1);
  Grid_init (grid, grid->radius, grid->nb_tiles + dt, grid->nb_locks);
  Grid_shuffle (grid);
  Gui_checkConflicts (gui);
  ez_send_expose (gui->win);
}

void
Gui_changeGridRadius (Gui * gui, int dr)
{
  Grid * grid= & gui->app->grid;
  int radius= INRANGE (MIN_GRID_RADIUS, grid->radius + dr, MAX_GRID_RADIUS);
  int old_area= hive_area (grid->radius);
  int new_area= hive_area (radius);
  double tile_ratio= grid->nb_tiles / (double) old_area;
  double lock_ratio= grid->nb_locks / (double) old_area;
  int nb_tiles= myround (new_area * tile_ratio);
  int nb_locks= myround (new_area * lock_ratio);
  
  ez_start_timer (gui->win, -1);
  Grid_init (grid, radius, nb_tiles, nb_locks);
  Grid_shuffle (grid);
  Gui_checkConflicts (gui);
  Gui_resizeWinToIdeal (gui);
}

void
Gui_setAngle (Gui * gui, double angle)
{
  gui->cell_angle= angle;
  Gui_initCoordSystems (gui);
  ez_send_expose (gui->win);
}

void
Gui_rotate (Gui * gui, double delta)
{
  gui->cell_angle += delta;
  if      (gui->cell_angle < 0   ) gui->cell_angle += 2*PI;
  else if (gui->cell_angle > 2*PI) gui->cell_angle -= 2*PI;  
  Gui_initCoordSystems (gui);
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
  Gui * gui = ez_get_data (e->win);
  Grid * grid= & gui->app->grid;
  int nb_conflicts= Grid_countConflicts (grid);

  Gui_drawGrid (gui);
  Gui_drawTitle (gui, nb_conflicts);
  Gui_drawKeyInfo (gui);
  if (! Index_isNull (gui->selected)) Gui_drawSelected (gui);
}

void 
onTimerNotify (Ez_event * e)
{
  Gui * gui = ez_get_data (e->win);
  Gui_rotate (gui, ANIM_RAD_STEP);
  ez_start_timer (gui->win, ANIM_MILLISEC_STEP);
}

void 
onButtonPress (Ez_event * e)
{
  Gui * gui = ez_get_data (e->win);
  Grid * grid= & gui->app->grid;
  Index index; Cell * cell;
  ez_start_timer (gui->win, -1);

  gui->pointer= Coord_xy (e->mx, e->my);
  index= Gui_getIndexAtCoord (gui, gui->pointer);
  if (Index_isNull (index)) return;
  cell= Grid_getCellAtIndex (grid, index);
  if (cell->constraint == -1) return;
  if (cell->is_locked) return;
  gui->selected= index;
  Grid_swapSavedCell (grid, index);
  ez_send_expose (gui->win);
}

void 
onMotionNotify (Ez_event * e)
{
  Gui * gui = ez_get_data (e->win);
  if (Index_isNull (gui->selected)) return;
  ez_start_timer (gui->win, -1);

  gui->pointer= Coord_xy (e->mx, e->my);
  ez_send_expose (gui->win);
}

void 
onButtonRelease (Ez_event * e)
{
  Gui * gui= ez_get_data (e->win);
  Grid * grid= & gui->app->grid;
  Index target;
  Cell * cell;
  if (Index_isNull (gui->selected)) return;
  ez_start_timer (gui->win, -1);

  gui->pointer= Coord_xy (e->mx, e->my);
  target= Gui_getIndexAtCoord (gui, gui->pointer);
  cell= Grid_getCellAtIndex (grid, target);

  if (Grid_indexIsOutside (grid, target)    || 
      Index_isEqual (target, gui->selected) ||
      cell->is_locked) {
    Grid_swapSavedCell (grid, gui->selected);
  } else {
    Grid_swapSavedCell (grid, target);
    Grid_swapSavedCell (grid, gui->selected);
  }
  gui->selected= Index_null ();
  Gui_checkConflicts (gui);
  ez_send_expose (gui->win);
}

void 
onKeyPress (Ez_event * e)
{  
  Gui * gui = ez_get_data (e->win);
  if (! Index_isNull (gui->selected)) return;

  switch (e->key_sym) {
  case XK_Q: case XK_q: case XK_Escape: Gui_quit (gui); break;
  
  case XK_U: case XK_u: Gui_unshuffleGrid   (gui); break;
  case XK_S: case XK_s: Gui_shuffleGrid     (gui); break; 
 
  case XK_space:  Gui_changeGridTiles (gui); break;

  case XK_KP_Add:      case XK_minus: Gui_rotate (gui, +ANIM_RAD_STEP); break;
  case XK_KP_Subtract: case XK_plus : Gui_rotate (gui, -ANIM_RAD_STEP); break;
  
  case XK_Return: case XK_KP_Enter: Gui_setAngle (gui, 0); break; 
    
  case XK_T: case XK_Insert: Gui_changeGridNbTiles (gui, +1); break;
  case XK_t: case XK_Delete: Gui_changeGridNbTiles (gui, -1); break;

  case XK_G: case XK_Prior: Gui_changeGridRadius (gui, +1); break;
  case XK_g: case XK_Next:  Gui_changeGridRadius (gui, -1); break;

  case XK_L: case XK_Home: Gui_changeGridNbLocks (gui, +1); break;
  case XK_l: case XK_End:  Gui_changeGridNbLocks (gui, -1); break;

  default: break;
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
  case Expose       : onExpose         (e); break;

  case ButtonPress  : onButtonPress    (e); break;
  case MotionNotify : onMotionNotify   (e); break;
  case ButtonRelease: onButtonRelease  (e); break;
  case KeyPress     : onKeyPress       (e); break;
  case TimerNotify  : onTimerNotify    (e); break;
  default           :                       break;
  }
}

/*=============================================================================
 * App/Gui init
 *=============================================================================
 */

void 
App_init (App * app, int grid_radius, int nb_tiles, int nb_locks)
{
  Grid_init (& app->grid, grid_radius, nb_tiles, nb_locks);
}

void 
Gui_init (Gui * gui, App * app, const char win_title [], 
          double cell_radius)
{
  Coord ideal;
  gui->app= app;
  gui->cell_radius= cell_radius;
  gui->cell_angle= 0;
  Gui_initCoordSystems (gui);

  ideal= Gui_getIdealWinSize (gui);
  gui->win= ez_window_create (ideal.x, ideal.y, win_title, onEvent);
 
  gui->pointer= Coord_xy (0,0);
  gui->selected= Index_null ();
  
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
  App_init (& app, DEF_GRID_RADIUS, DEF_NB_TILES, DEF_NB_LOCKS);
  Gui_init (& gui, & app, "Heziom", DEF_CELL_RADIUS);  
  ez_main_loop ();
  return 0;
}

