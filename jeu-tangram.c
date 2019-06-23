/* jeu-tangram.c: a random tangram puzzle
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 29/03/2014 - version 1.2
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


/*------------------------- D E F I N I T I O N S ---------------------------*/

#define VERTEX_MAX     20
#define PIECE_MAX      50
#define BSPLINE_MAX     8
#define ANGLE_MAX     540
#define DELAY1       1500
#define DELAY2         20
#define DELAY3        500
#define ANIM2_NB      200
#define LEVEL_INIT      5


typedef struct {
    double x, y;                        /* Coordinates */
} Vertex;

typedef struct {
    Vertex vertex[VERTEX_MAX];          /* List of vertices */
    int nb_ver;                         /* Number of polygon vertices */
} Polyg;

#define DIM 3
typedef double Matr[DIM][DIM];          /* [line][column] */
typedef double Vec[DIM];

typedef struct {
    double tx, ty;                      /* Translation */
    double rx, ry;                      /* Rotation center */
    double ra;                          /* Angle in degrees */
    Matr matr;                          /* Transformation matrix */
} Transfo;

typedef struct {
    Polyg pol_orig;                     /* Original polygon */
    Vertex gc_orig;                     /* Gravity center of pol_orig */
    Transfo transfo;                    /* Transformation on pol_orig */
    Polyg pol_trans;                    /* Transformed polygon */
    Vertex gc_trans;                    /* Gravity center of pol_trans */
    Ez_uint32 color_inter;              /* Color of interior */
    Ez_uint32 color_bord;               /* Color of border */
} Piece;

typedef struct {
    Piece piece[PIECE_MAX];             /* List of pieces */
    int order_z[PIECE_MAX];             /* Stacking order of pieces */
    int nb_pieces;                      /* Number of pieces */
} Tangram;

typedef struct {
    double px[BSPLINE_MAX];             /* Coordinates of control points */
    double py[BSPLINE_MAX];             /*   of a cubic B-spline */
    double pa[BSPLINE_MAX];             /* same for the angle */
} Trajec;

typedef struct {
    Trajec trajec[PIECE_MAX];           /* Trajectory of pieces */
    int step;
    int nb_steps;
} Anim;

typedef struct {
    Tangram tangram;                    /* Tangram puzzle */
    int click;                          /* Current piece number, or -1 */
    Transfo transfo_click;              /* Transfo of piece during click */
    double click_x, click_y;            /* Coordinates of click */
    int level;                          /* Number of tangram pieces */
    Anim anim;                          /* Parameters for animation */
    int auto_mix;                       /* Flag */
    int kb_shift;                       /* Control key pressed */
    enum { S_START, S_PLAY, S_ANIM2, S_MAGNET, S_WIN } state;
} App;


#define TANG_X1  150
#define TANG_Y1  150
#define TANG_X2  450
#define TANG_Y2  450
#define TANG_IS_SQUARE (TANG_X2-TANG_X1 == TANG_Y2-TANG_Y1)

#define WIN1_H 600
#define WIN1_W 600

typedef struct {
    App *app;
    Ez_window win1;
} Gui;


/*------------------------------ P O L Y G O N S ----------------------------*/


void polyg_init (Polyg *polyg)
{
    polyg->nb_ver = 0;
}


/*
 * Insert the vertex in the polygon at position pos.
 * Return the index of the vertex, else -1.
*/

int polyg_insert_vertex_pos (Polyg *polyg, Vertex vertex, int pos)
{
    int i;

    if (polyg->nb_ver >= VERTEX_MAX) {
        fprintf (stderr, "ERROR: polyg_insert_vertex_pos: array is full\n");
        return -1;
    }

    if (pos < 0 || pos > polyg->nb_ver) {
        fprintf (stderr, "ERROR: polyg_insert_vertex_pos: bad pos\n");
        return -1;
    }

    for (i = polyg->nb_ver-1; i >= pos; i--)
        polyg->vertex[i+1] = polyg->vertex[i];
    polyg->vertex[pos] = vertex;
    polyg->nb_ver ++;

    return pos;
}


int polyg_insert_vertex_end (Polyg *polyg, Vertex vertex)
{
    return polyg_insert_vertex_pos (polyg, vertex, polyg->nb_ver);
}


int polyg_insert_point_pos (Polyg *polyg, double x, double y, int pos)
{
    Vertex vertex;

    vertex.x = x; vertex.y = y;
    return polyg_insert_vertex_pos (polyg, vertex, pos);
}


int polyg_insert_point_end (Polyg *polyg, double x, double y)
{
    return polyg_insert_point_pos (polyg, x, y, polyg->nb_ver);
}


/*
 * Delete vertices [a..b] in the polygon.
*/

void polyg_delete_interval (Polyg *polyg, int a, int b)
{
    int i;

    if (a < 0 || b >= polyg->nb_ver) {
        fprintf (stderr, "ERROR: polyg_delete_interval: bad vertex index\n");
        return;
    }
    if (a > b) return;

    for (i = b+1; i < polyg->nb_ver; i++)
        polyg->vertex[i+a-b-1] = polyg->vertex[i];
    polyg->nb_ver -= b-a+1;
}


/*
 * Split the polygon pol1 in two polygons pol1 and pol2 on edge a,b.
 * a and b are index of vertices in pol1.
*/

void polyg_cut_by_edge (Polyg *pol1, Polyg *pol2, int a, int b)
{
    int k;

    if (a < 0 || a >= pol1->nb_ver || b < 0 || b >= pol1->nb_ver || a == b) {
        fprintf (stderr, "ERROR: polyg_cut_by_edge: bad vertex index\n");
        return;
    }
    if (a > b) { int tmp = a; a = b; b = tmp; }

    polyg_init (pol2);
    for (k = a; k <= b; k++)
        polyg_insert_vertex_end (pol2, pol1->vertex[k]);

    polyg_delete_interval (pol1, a+1, b-1);
}


/*
 * Create a square polygon having coordinates x1..x2, y1..y2.
*/

void polyg_create_square (Polyg *pol, double x1, double y1, double x2, double y2)
{
    polyg_init (pol);
    polyg_insert_point_end (pol, x1, y1);
    polyg_insert_point_end (pol, x2, y1);
    polyg_insert_point_end (pol, x2, y2);
    polyg_insert_point_end (pol, x1, y2);
}


void polyg_compute_gravity_center (Polyg *pol, double *xg, double *yg)
{
    int i;

    *xg = *yg = 0;
    if (pol->nb_ver == 0) return;

    for (i = 0; i < pol->nb_ver; i++) {
        *xg += pol->vertex[i].x;
        *yg += pol->vertex[i].y;
    }
    *xg /= pol->nb_ver;
    *yg /= pol->nb_ver;    
}


double compute_triangle_area (double x1, double y1, double x2, double y2,
    double x3, double y3)
{
    double xa = x2-x1, ya = y2-y1, xb = x3-x1, yb = y3-y1;
    return fabs (xa*yb - ya*xb) / 2;
}


double polyg_compute_area (Polyg *polyg)
{
    int i;
    double area = 0;

    for (i = 0; i < polyg->nb_ver-2; i++)
        area += compute_triangle_area (
            polyg->vertex[0].x, polyg->vertex[0].y,
            polyg->vertex[i+1].x, polyg->vertex[i+1].y,
            polyg->vertex[i+2].x, polyg->vertex[i+2].y);
    return area;
}


/*--------------------- 2 D   T R A N S F O R M A T I O N S -----------------*/


void matr_print (Matr m)
{
    int i, j;
    
    for (i = 0; i < DIM; i++) {
        for (j = 0; j < DIM; j++) printf ("%2.3f ", m[i][j]);
        printf ("\n");
    }
}


void matr_init_unity (Matr m)
{
    int i, j;

    for (i = 0; i < DIM; i++)
    for (j = 0; j < DIM; j++)
        m[i][j] = 0;

    for (i = 0; i < DIM; i++)
        m[i][i] = 1;
}


void matr_copy (Matr m, Matr res)
{
    int i, j;

    for (i = 0; i < DIM; i++)
    for (j = 0; j < DIM; j++)
        res[i][j] = m[i][j];
}


void matr_multiply_by_matr (Matr m1, Matr m2, Matr res)
{
    int i, j, k;
    double d;

    for (i = 0; i < DIM; i++)
    for (j = 0; j < DIM; j++)
    {
        d = 0;
        for (k = 0; k < DIM; k++)
            d += m1[i][k] * m2[k][j];
        res[i][j] = d;
    }
}


void matr_multiply_by_vec (Matr m1, Vec v2, Vec res)
{
    int i, k;
    double d;

    for (i = 0; i < DIM; i++)
    {
        d = 0;
        for (k = 0; k < DIM; k++)
            d += m1[i][k] * v2[k];
        res[i] = d;
    }
}


void matr_multiply_rotation (Matr m, double ra)
{
    Matr res, tmp;
    double a, c, s;

    a = ra * M_PI / 180; c = cos(a); s = sin(a);

    tmp[0][0] =  c; tmp[0][1] =  s; tmp[0][2] =  0;
    tmp[1][0] = -s; tmp[1][1] =  c; tmp[1][2] =  0;
    tmp[2][0] =  0; tmp[2][1] =  0; tmp[2][2] =  1;
  
    matr_multiply_by_matr (m, tmp, res);
    matr_copy (res, m);
}


void matr_multiply_translation (Matr m, double tx, double ty)
{
    Matr res, tmp;

    tmp[0][0] =  1; tmp[0][1] =  0; tmp[0][2] = tx;
    tmp[1][0] =  0; tmp[1][1] =  1; tmp[1][2] = ty;
    tmp[2][0] =  0; tmp[2][1] =  0; tmp[2][2] =  1;
  
    matr_multiply_by_matr (m, tmp, res);
    matr_copy (res, m);
}


void transfo_init (Transfo *t)
{
    t->tx = t->ty = 0;
    t->rx = t->ry = 0;
    t->ra = 0;
    matr_init_unity (t->matr);
}


void transfo_store_translation (Transfo *t, double tx, double ty)
{
    t->tx = tx; t->ty = ty;
}


void transfo_store_rotation (Transfo *t, double rx, double ry, double ra)
{
    t->rx = rx; t->ry = ry; t->ra = ra;
}


void transfo_compute_matr (Transfo *t)
{
    matr_init_unity (t->matr);
    matr_multiply_translation (t->matr, t->tx, t->ty);
    matr_multiply_translation (t->matr, t->rx, t->ry);
    matr_multiply_rotation    (t->matr, t->ra);
    matr_multiply_translation (t->matr, -t->rx, -t->ry);
}


void transfo_apply_on_point (Transfo *t, 
    double x1, double y1, double *x2, double *y2)
{
    Vec v1, v2;
    
    v1[0] = x1; v1[1] = y1; v1[2] = 1;
    matr_multiply_by_vec (t->matr, v1, v2);

    /* Point to infinite */
    if (v2[2] == 0) { *x2 = *y2 = -1; return; }

    *x2 = v2[0]/v2[2]; *y2 = v2[1]/v2[2]; 
}


void transfo_apply_on_vertex (Transfo *t, Vertex *s1, Vertex *s2)
{
    transfo_apply_on_point (t, s1->x, s1->y, &s2->x, &s2->y);
}


void transfo_apply_on_polyg (Transfo *t, Polyg *p1, Polyg *p2)
{
    int i;
    double x, y;

    polyg_init (p2);

    for (i = 0; i < p1->nb_ver; i++) {
        transfo_apply_on_point (t, p1->vertex[i].x, p1->vertex[i].y, &x, &y);
        polyg_insert_point_end (p2, x, y);
    }
}


/*--------------------------------- P I E C E -------------------------------*/


void piece_init (Piece *piece)
{
    polyg_init (&piece->pol_orig);
    polyg_init (&piece->pol_trans);
    transfo_init (&piece->transfo);
    piece->color_inter = ez_grey;
    piece->color_bord = ez_black;
}


void piece_compute_gravity_center (Piece *piece)
{
    polyg_compute_gravity_center (&piece->pol_orig,
        &piece->gc_orig.x, &piece->gc_orig.y);
}


void piece_store_translation (Piece *piece, double tx, double ty)
{
    transfo_store_translation (&piece->transfo, tx, ty);
}


void piece_store_rotation (Piece *piece, double ra)
{
    transfo_store_rotation (&piece->transfo,
        piece->gc_orig.x, piece->gc_orig.y, ra);
}


void piece_compute_matr (Piece *piece)
{
    transfo_compute_matr (&piece->transfo);
}


void piece_apply_transformation (Piece *piece)
{
    transfo_apply_on_polyg  (&piece->transfo, &piece->pol_orig, &piece->pol_trans);
    transfo_apply_on_vertex (&piece->transfo, &piece->gc_orig , &piece->gc_trans);
}


/*------------------------------- T A N G R A M -----------------------------*/


void tangram_init (Tangram *tangram)
{
    tangram->nb_pieces = 0;
}


/*
 * Insert and initialize a piece in the tangram.
 * Return the index of the piece, else -1.
*/

int tangram_insert_piece (Tangram *tangram)
{
    if (tangram->nb_pieces >= PIECE_MAX) {
        fprintf (stderr, "ERROR: tangram_insert_piece: array full\n");
        return -1;
    }

    piece_init (&tangram->piece[tangram->nb_pieces]);
    tangram->order_z[tangram->nb_pieces] = tangram->nb_pieces;
    return tangram->nb_pieces ++;
}


void tangram_place_piece_over (Tangram *tangram, int i)
{
    int j;

    if (i < 0 || i >= tangram->nb_pieces) {
        fprintf (stderr, "ERROR: tangram_place_piece_over: bad index\n");
        return;
    }

    for (j = tangram->nb_pieces-1; j >= 0; j--)
        if (tangram->order_z[j] == i) break;
    for (j-- ; j >= 0; j--)
        tangram->order_z[j+1] = tangram->order_z[j];
    tangram->order_z[0] = i;
}


void tangram_store_translation (Tangram *tangram, int i, double tx, double ty)
{
    if (i < 0 || i >= tangram->nb_pieces) {
        fprintf (stderr, "ERROR: tangram_store_translation: bad index\n");
        return;
    }
    piece_store_translation (&tangram->piece[i], tx, ty);
}


void tangram_store_rotation (Tangram *tangram, int i, double ra)
{
    if (i < 0 || i >= tangram->nb_pieces) {
        fprintf (stderr, "ERROR: tangram_store_rotation: bad index\n");
        return;
    }
    piece_store_rotation (&tangram->piece[i], ra);
}


void tangram_compute_matr (Tangram *tangram, int i)
{
    if (i < 0 || i >= tangram->nb_pieces) {
        fprintf (stderr, "ERROR: tangram_compute_matr: bad index\n");
        return;
    }
    piece_compute_matr (&tangram->piece[i]);
}


/*
 * Return the angle between horizontal Cx and CP in [0..360[
*/

double compute_angle_horizontal (double xc, double yc, double xp, double yp)
{
    double dx = xp-xc, dy = yp-yc, a;

    if (fabs(dx) < 1E-5 && fabs(dy) < 1E-5) return 0;
    a = atan2 (dy, dx) * 180/M_PI;
    return (a < 0) ? a+360 : a;
}


double compute_difference_angle (double xc, double yc, 
    double xp, double yp, double xq, double yq)
{
    return compute_angle_horizontal (xc, yc, xq, yq) -
           compute_angle_horizontal (xc, yc, xp, yp);
}


double tangram_compute_angle_gc_trans (Tangram *tangram, int i,
    double xp, double yp, double xq, double yq)
{
    if (i < 0 || i >= tangram->nb_pieces) {
        fprintf (stderr, "ERROR: tangram_compute_angle_gc_trans: bad index\n");
        return 0;
    }

    return compute_difference_angle (
        tangram->piece[i].gc_trans.x, tangram->piece[i].gc_trans.y,
        xp, yp, xq, yq);
}


void tangram_compute_gravity_center (Tangram *tangram, int i)
{
    if (i < 0 || i >= tangram->nb_pieces) {
        fprintf (stderr, "ERROR: tangram_compute_gravity_center: bad index\n");
        return;
    }
    piece_compute_gravity_center (&tangram->piece[i]);
}


void tangram_compute_gravity_centers (Tangram *tangram)
{
    int i;

    for (i = 0; i < tangram->nb_pieces; i++)
        piece_compute_gravity_center (&tangram->piece[i]);
}


void tangram_transform_piece (Tangram *tangram, int i)
{
    if (i < 0 || i >= tangram->nb_pieces) {
        fprintf (stderr, "ERROR: tangram_transform_piece: bad index\n");
        return;
    }
    piece_apply_transformation (&tangram->piece[i]);
}


void tangram_transform_pieces (Tangram *tangram)
{
    int i;

    for (i = 0; i < tangram->nb_pieces; i++)
        piece_apply_transformation (&tangram->piece[i]);
}


void bound_into_rectangle (int xmin, int ymin, int xmax, int ymax, 
    int *x, int *y)
{
    if (*x < xmin) *x = xmin;
    if (*y < ymin) *y = ymin;
    if (*x > xmax-1) *x = xmax-1;
    if (*y > ymax-1) *y = ymax-1;
}


void tangram_bound_translation (Tangram *tangram, int i, Transfo *trans,
    int *dx, int *dy)
{
    int gx, gy, bx, by;

    if (i < 0 || i >= tangram->nb_pieces) {
        fprintf (stderr, "ERROR: tangram_bound_translation: bad index\n");
        return;
    }

    gx = tangram->piece[i].gc_orig.x;
    gy = tangram->piece[i].gc_orig.y;
    bx = trans->tx + gx;
    by = trans->ty + gy;

    bound_into_rectangle (-bx, -by, WIN1_W-bx, WIN1_H-by, dx, dy);
}


/*
 * Return 1 if P is on the right of (or on) the line AB, else 0.
*/

int point_is_on_right (double xp, double yp, double xa, double ya,
    double xb, double yb)
{
    double d = (xb-xa)*(yp-ya) - (yb-ya)*(xp-xa);
    return d >= 0 ? 1 : 0;
}


/*
 * Return 1 if P is inside (or on the boundary) of the polygon, else 0.
 * We assume that the polygon is convex.
*/

int point_is_inside (double xp, double yp, Polyg *pol)
{
    int i, j;

    for (i = 0; i < pol->nb_ver; i++) {
        j = (i+1) % pol->nb_ver;
        if (! point_is_on_right (xp, yp, 
                pol->vertex[i].x, pol->vertex[i].y, 
                pol->vertex[j].x, pol->vertex[j].y) )
            return 0; 
    }
    return 1;
}


/*
 * Return the number of the piece that contains P (the first found), else -1.
*/

int tangram_search_clicked_piece (Tangram *tangram, double xp, double yp)
{
    int i, j;

    for (j = 0; j < tangram->nb_pieces; j++) {
        i = tangram->order_z[j];
        if (point_is_inside (xp, yp, &tangram->piece[i].pol_trans))
            return i;
    }
    return -1;
}


/*
 * Return a % b in [0..b[. We assume b > 0 
*/

double real_modulo (double a, double b)
{
    if (a < 0) a += ((int) (1-a/b))*b;
    return a - ((int) (a/b))*b;
}


/*
 * Return the minimum angle difference in [0..360[ 
*/

double gap_angle (double a, double b)
{
    double c = real_modulo (fabs(a-b), 360),
           d = 360 - c;
    return c < d ? c : d;
}


/*
 * Attract the pieces on the original locations (by keeping track of the 90
 * degrees rotations).
 * Return 1 if all pieces are at their place.
*/

int tangram_apply_magnetic_grid (Tangram *tangram)
{
    Transfo t;
    int rot_a = TANG_IS_SQUARE ? 90 : 180;

    t.tx = t.ty = 0;
    t.rx = (TANG_X1 + TANG_X2)/2; t.ry = (TANG_Y1 + TANG_Y2)/2;

    /* Rotations to 90 degrees of the original solution */
    for (t.ra = 0; t.ra < 360; t.ra += rot_a) {
        int i;
        int ok = 1;

        transfo_compute_matr (&t);

        for (i = 0; i < tangram->nb_pieces ; i++) {
            Piece *piece = &tangram->piece[i];
            Vertex c;
            double dx, dy, da;

            transfo_apply_on_vertex (&t, &piece->gc_orig, &c);

            dx = fabs (c.x - piece->gc_trans.x);
            dy = fabs (c.y - piece->gc_trans.y);
            da =  gap_angle (t.ra, piece->transfo.ra);

            /* The piece is in place */
            if (dx < 1E-5 && dy < 1E-5 && da < 1E-5) continue;

            /* The piece is alsmost in place */
            if (dx < 15 && dy < 15 && da < 10) 
            {
                piece->transfo.tx = c.x - piece->gc_orig.x;
                piece->transfo.ty = c.y - piece->gc_orig.y;
                piece->transfo.ra = t.ra;
                transfo_compute_matr (&piece->transfo);
                piece_apply_transformation (piece);
            } else {
                ok = 0;  /* not in place */
            }
        }

        /* Do all the pieces be in place for rot_a ? */
        if (ok) return 1;
    }
    return 0;
}


/*-------------------------------- C O L O R S ------------------------------*/


void tangram_choose_colors (Tangram *tangram)
{
    double h, g = 0.1 * ez_random (3600), s = 0.23;
    int i;

    for (i = 0; i < tangram->nb_pieces; i++) {
        h = g + i * 360.0 / tangram->nb_pieces;
        h = h - ((int) h / 360)*360;
        tangram->piece[i].color_inter = ez_get_HSV (h, s, 0.99);
        tangram->piece[i].color_bord  = ez_get_HSV (h, s, 0.8);
    }
}


/*--------------------------- G E N E R A T I N G ---------------------------*/


/*
 * Return the index of a piece having maximum area
*/

int choose_piece_of_max_area (Tangram *tangram)
{
    int i, j, k, area_ind;
    double area, area_max;

    area_max = 0; 
    area_ind = 0;
    i = ez_random (tangram->nb_pieces);

    for (j = 0; j < tangram->nb_pieces; j++) {
        k = (i+j) % tangram->nb_pieces;
        area = polyg_compute_area (&tangram->piece[k].pol_orig);
        if (area > area_max) { area_max = area; area_ind = k; }
    }
    return area_ind;
}


/*
 * Return area of the smallest piece
*/

double tangram_compute_area_of_smallest_piece (Tangram *tangram)
{
    int i;
    double area, area_min;

    area_min = -1; 

    for (i = 0; i < tangram->nb_pieces; i++) {
        area = polyg_compute_area (&tangram->piece[i].pol_orig);
        if (area_min < 0 || area < area_min) area_min = area;
    }
    return area_min;
}


/*
 * Return index k of a longer edge k,k+1 of the polygon
*/

int choose_edge_of_max_length (Polyg *polyg)
{
    int i, j, k, l, lg_ind;
    double dx, dy, lg, lg_max;

    lg_max = 0;
    lg_ind = 0;
    i = ez_random (polyg->nb_ver);

    for (j = 0; j < polyg->nb_ver; j++) {
        k = (i+j) % polyg->nb_ver;
        l = (k+1) % polyg->nb_ver;
        dx = polyg->vertex[l].x - polyg->vertex[k].x;
        dy = polyg->vertex[l].y - polyg->vertex[k].y;
        lg = sqrt (dx*dx + dy*dy);
        if (lg > lg_max) { lg_max = lg; lg_ind = k; }
    }
    return lg_ind;
}


/*
 * Generate a tangram of nb_pieces.
*/

void tangram_generate_split (Tangram *tangram, int nb_pieces)
{
    int i;

    tangram_init (tangram);

    i = tangram_insert_piece (tangram);
    if (i < 0) return;
    polyg_create_square (&tangram->piece[i].pol_orig, 
        TANG_X1, TANG_Y1, TANG_X2, TANG_Y2);
    
    for (i = 1; i < nb_pieces; i++) {
        int p, q, a, b, c, n;
        double t;
        Polyg *polyg;

        p = choose_piece_of_max_area (tangram);
        polyg = &tangram->piece[p].pol_orig;

        a = choose_edge_of_max_length (polyg);
        n = polyg->nb_ver;
        b = (a+1) % n;
        t = (300 + ez_random (400)) / 1000.0;

        polyg_insert_point_pos (polyg, 
            polyg->vertex[a].x * t + polyg->vertex[b].x * (1-t),
            polyg->vertex[a].y * t + polyg->vertex[b].y * (1-t),
            b);

        c = (b + 2 + ez_random (n-2)) % (n+1);
        q = tangram_insert_piece (tangram);
        if (q < 0) return;
        polyg_cut_by_edge (polyg, &tangram->piece[q].pol_orig, b, c);
    }

    tangram_compute_gravity_centers (tangram);
    tangram_transform_pieces (tangram);
    tangram_choose_colors (tangram);
}


/*
 * Generate several tangrams, and copy the "best" in *tangram.
 * The aim is to avoid the very flat triangles.
 * To do that, we choose the tangram whose area of the smallest piece
 * is maximal.
*/

void tangram_generate_problem (Tangram *tangram, int nb_pieces)
{
    Tangram trial[3];
    int i, j = 0;
    double area, area_max = 0;
    
    for (i = 0; i < 3; i++) {
        tangram_generate_split (&trial[i], nb_pieces);
        area = tangram_compute_area_of_smallest_piece (&trial[i]);
        if (area > area_max) { area_max = area; j = i; }
    }
    *tangram = trial[j];
}


void tangram_place_pieces_on_circle (Tangram *tangram)
{
    int i;
    double a = ez_random (360);

    for (i = 0; i < tangram->nb_pieces; i++) {
        double b = (a + i * 360 / tangram->nb_pieces) * M_PI / 180;

        tangram_store_translation (tangram, i, 
            WIN1_W * (0.5 + 0.3*cos(b)) - tangram->piece[i].gc_orig.x, 
            WIN1_H * (0.5 + 0.3*sin(b)) - tangram->piece[i].gc_orig.y); 
        tangram_store_rotation (tangram, i, ez_random (360)); 

        tangram_compute_matr (tangram, i);
        tangram_transform_piece (tangram, i);
    }
}


/*---------------------------- A N I M A T I O N ----------------------------*/


/*
 * Return the cubic polynom of Bezier in t
 * b[0..3], t in [0..1]
*/

double compute_bezier_cubic (double *b, double t)
{
    double s = 1.0 - t;
    return s*s*(s*b[0] + 3*t*b[1]) + t*t*(3*s*b[2] + t*b[3]);
}


/*
 * Return the B-spline cubic polynom in r
 * p[0..n-1], r in [0..n-3]
*/

double compute_bspline_cubic (double *p, int n, double r)
{
    int i = r;
    double t = r-i;
    double b[4];

    if (i >= n-3) { i = n-4; t = 1.0; }

    b[0] = (p[i] + 4*p[i+1] + p[i+2])/6;
    b[1] = (2*p[i+1] + p[i+2])/3;
    b[2] = (p[i+1] + 2*p[i+2])/3;
    b[3] = (p[i+1] + 4*p[i+2] + p[i+3])/6;

    return compute_bezier_cubic (b, t);
}


/*
 * Choose the coordinates of control points such that:
 *  - the trajectory starts in a and ends in b;
 *  - the values are in [val_min .. val_max]
*/

void app_prepare_curve (double *p, int n, double a, double b, 
    double val_min, double val_max)
{
    int j;

    /* Integers are sufficient */
    for (j = 2; j < n-2; j++)
        p[j] = val_min + ez_random (val_max - val_min);  

    p[1] = a; p[0] = 2*p[1] - p[2];
    p[n-2] = b; p[n-1] = 2*p[n-2] - p[n-3];
}


void app_prepare_mixing_trajectories (App *app)
{
    int i;
    double a = ez_random (360);

    for (i = 0; i < app->tangram.nb_pieces; i++) {
        Piece *piece = &app->tangram.piece[i];
        double b = (a + i * 360 / app->tangram.nb_pieces) * M_PI / 180;

        app_prepare_curve (app->anim.trajec[i].px, BSPLINE_MAX,
            piece->gc_orig.x + piece->transfo.tx, 
            WIN1_W * (0.5 + 0.3*cos(b)),
            0, WIN1_W);
        app_prepare_curve (app->anim.trajec[i].py, BSPLINE_MAX,
            piece->gc_orig.y + piece->transfo.ty,
            WIN1_H * (0.5 + 0.3*sin(b)),            
            0, WIN1_H);
        app_prepare_curve (app->anim.trajec[i].pa, BSPLINE_MAX,
            piece->transfo.ra, 
            ez_random (360),
            -ANGLE_MAX, ANGLE_MAX);
    }
}


void app_prepare_solution_trajectories (App *app)
{
    int i;

    for (i = 0; i < app->tangram.nb_pieces; i++) {
        Piece *piece = &app->tangram.piece[i];

        app_prepare_curve (app->anim.trajec[i].px, BSPLINE_MAX,
            piece->gc_orig.x + piece->transfo.tx, 
            piece->gc_orig.x,
            0, WIN1_W);
        app_prepare_curve (app->anim.trajec[i].py, BSPLINE_MAX,
            piece->gc_orig.y + piece->transfo.ty,
            piece->gc_orig.y,            
            0, WIN1_H);
        app_prepare_curve (app->anim.trajec[i].pa, BSPLINE_MAX,
            piece->transfo.ra, 
            0,
            -ANGLE_MAX, ANGLE_MAX);
    }
}


/*
 * Compute the transformations in such a manner that the gravity centers
 * lie on the trajectory.
 * step in [0..nsteps-1]
*/

void app_anim_step (App *app, int step, int nsteps)
{
    int i;
    double tx, ty, ta;
    double r = (double) step * (BSPLINE_MAX-3) / (nsteps-1);

    for (i = 0; i < app->tangram.nb_pieces; i++) {
        tx = compute_bspline_cubic (app->anim.trajec[i].px, BSPLINE_MAX, r);
        ty = compute_bspline_cubic (app->anim.trajec[i].py, BSPLINE_MAX, r);
        ta = compute_bspline_cubic (app->anim.trajec[i].pa, BSPLINE_MAX, r);

        tangram_store_translation (&app->tangram, i, 
            tx - app->tangram.piece[i].gc_orig.x, 
            ty - app->tangram.piece[i].gc_orig.y); 
        tangram_store_rotation (&app->tangram, i, ta); 

        tangram_compute_matr (&app->tangram, i);
        tangram_transform_piece (&app->tangram, i);
    }
}


/*------------------------------ D R A W I N G S ----------------------------*/


void draw_vertex (Ez_window win, Vertex *s)
{
    ez_draw_rectangle (win, s->x-3, s->y-3, s->x+3, s->y+3);
}


void draw_gravity_center (Ez_window win, Vertex *s)
{
    ez_draw_circle (win, s->x-3, s->y-3, s->x+3, s->y+3);
}


void draw_polyg (Ez_window win, Polyg *polyg, 
    Ez_uint32 c_vertices, Ez_uint32 c_edges)
{
    int i, j;

    ez_set_color (c_vertices);
    for (i = 0; i < polyg->nb_ver; i++)
        draw_vertex (win, &polyg->vertex[i]);

    ez_set_color (c_edges);
    for (i = 0; i < polyg->nb_ver; i++) {
        j = (i == 0) ? polyg->nb_ver-1 : i-1;
        ez_draw_line (win, polyg->vertex[j].x, polyg->vertex[j].y,
                           polyg->vertex[i].x, polyg->vertex[i].y);
    }
}

void fill_polyg (Ez_window win, Polyg *polyg, 
    Ez_uint32 c_inter, Ez_uint32 c_border)
{
    int i, j;

    ez_set_color (c_inter);
    for (i = 0; i < polyg->nb_ver-2; i++)
        ez_fill_triangle (win,
            polyg->vertex[0  ].x + 0.5, polyg->vertex[0  ].y + 0.5,
            polyg->vertex[i+1].x + 0.5, polyg->vertex[i+1].y + 0.5,
            polyg->vertex[i+2].x + 0.5, polyg->vertex[i+2].y + 0.5);

    ez_set_color (c_border);
    for (i = 0; i < polyg->nb_ver; i++) {
        j = (i == 0) ? polyg->nb_ver-1 : i-1;
        ez_draw_line (win,
            polyg->vertex[j].x + 0.5, polyg->vertex[j].y + 0.5,
            polyg->vertex[i].x + 0.5, polyg->vertex[i].y + 0.5);
    }
}


void draw_tangram (Ez_window win, Tangram *tangram, int click)
{
    int j;

    /* Painter's algorithm */
    for (j = tangram->nb_pieces-1; j >= 0; j--) {
        int i = tangram->order_z[j];
        Piece *piece = &tangram->piece[i];
        Ez_uint32 c_border = i == click ? ez_red : piece->color_bord;

        fill_polyg (win, &piece->pol_trans, piece->color_inter, c_border);

        if (i == click) {
            ez_set_color (c_border);
            draw_gravity_center (win, &piece->gc_trans);
        }
    }
}


/*---------------------- E V E N T S   O N   W I N   1 ----------------------*/


void win1_on_Expose (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;
    int w, h, d = 5;

    ez_set_color (ez_grey);
    ez_fill_rectangle (ev->win,
        TANG_X1-d, TANG_Y1-d, TANG_X2+d, TANG_Y2+d);
    ez_set_color (ez_white);
    ez_fill_rectangle (ev->win,
        TANG_X1, TANG_Y1, TANG_X2, TANG_Y2);

    draw_tangram (ev->win, &app->tangram, app->click);

    ez_set_nfont (2);
    if (app->state == S_WIN) {
        ez_set_color (ez_red);
        ez_draw_text (ev->win, EZ_TC, WIN1_W/2, 10, 
            "Well done !! Tangram of %d pieces solved", app->tangram.nb_pieces);
    } else {
        ez_set_color (ez_magenta); 
        ez_draw_text (ev->win, EZ_TC, WIN1_W/2, 10, 
            "Place the %d pieces in the %s !", app->tangram.nb_pieces,
            TANG_IS_SQUARE ? "square" : "rectangle");
    }

    ez_window_get_size (ev->win, &w, &h);
    ez_set_color (ez_black); ez_set_nfont (0);
    switch (app->state) {
        case S_START : 
            ez_draw_text (ev->win, EZ_BC, WIN1_W/2, h-10,
                "The program jeu-tangram.c is part of EZ-Draw:\n"
                "http://pageperso.lif.univ-mrs.fr/~edouard.thiel/ez-draw"); 
            break;
        case S_PLAY :
        case S_MAGNET :
            ez_draw_text (ev->win, EZ_BC, WIN1_W/2, h-10,
                "n: new tangram   up,down: number of pieces   "
                "m: mix   s: solve   q: quit\n"
                "button1: drag to move the piece    c: colors   "
                "a: auto mix %s\n"
                "To turn a piece, drag button3 or shift+button1, "
                "or use the scroll wheel",
                app->auto_mix ? "ON " : "OFF");
                break;
        case S_ANIM2 : 
        case S_WIN :
            ez_draw_text (ev->win, EZ_BC, WIN1_W/2, h-10,
                "Animation in progress ..."); 
            break;
        default : break;
    }
}


void win1_on_ButtonPress (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;
    int da = 1;

    if (app->state == S_MAGNET) app->state = S_PLAY;
    if (app->state != S_PLAY) return;

    if (app->click < 0 || ev->mb <= 3)
        app->click = tangram_search_clicked_piece (&app->tangram,
            ev->mx, ev->my);

    if (app->click < 0) { ez_send_expose (ev->win); return; }

    app->transfo_click = app->tangram.piece[app->click].transfo;
    app->click_x = ev->mx;
    app->click_y = ev->my;
    tangram_place_piece_over (&app->tangram, app->click);

    switch (ev->mb) {
        case 4 :
            da = -1;
        case 5 :
            tangram_store_rotation (&app->tangram, app->click,
                (int)(app->transfo_click.ra - da));
            tangram_compute_matr (&app->tangram, app->click);
            tangram_transform_piece (&app->tangram, app->click);
            break;
    }
    ez_send_expose (ev->win);
}


void win1_on_MotionNotify (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;
    enum { A_NONE, A_TRANSLATION, A_ROTATION} action = A_NONE;

    if (app->state != S_PLAY) return;
    if (app->click < 0) return;

    switch (ev->mb) {
        case 1 :
            action = A_TRANSLATION;
            if (app->kb_shift == 1) action = A_ROTATION;
            break;
        case 2 :
        case 3 :
            action = A_ROTATION;
            break;
    }

    switch (action) {
        case A_TRANSLATION : {
            int dx = ev->mx - app->click_x, 
                dy = ev->my - app->click_y;
            tangram_bound_translation (&app->tangram, app->click,
                &app->transfo_click, &dx, &dy);
            tangram_store_translation (&app->tangram, app->click, 
                app->transfo_click.tx + dx, 
                app->transfo_click.ty + dy);
            tangram_compute_matr (&app->tangram, app->click);
            tangram_transform_piece (&app->tangram, app->click);
            ez_send_expose (ev->win);
          } break;
        case A_ROTATION : {
            double a = tangram_compute_angle_gc_trans (&app->tangram,
                app->click, app->click_x, app->click_y, ev->mx, ev->my);
            tangram_store_rotation (&app->tangram, app->click,
                (int)(app->transfo_click.ra - a));
            tangram_compute_matr (&app->tangram, app->click);
            tangram_transform_piece (&app->tangram, app->click);
            ez_send_expose (ev->win);
          } break;
        default : break;
    }
}


void win1_on_ButtonRelease (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    if (app->state == S_PLAY) {
        app->state = S_MAGNET;
        ez_start_timer (ev->win, DELAY3);
    }
}


void win1_on_KeyPress (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    /* Behavior independent from state */
    switch (ev->key_sym) {
        case XK_q : 
            ez_quit (); 
            break;

        case XK_c : {
            tangram_choose_colors (&app->tangram);
            ez_send_expose (ev->win);
          } break;

        case XK_a :
            app->auto_mix = ! app->auto_mix;
            ez_send_expose (ev->win);
            break;

        case XK_Shift_L :
        case XK_Shift_R :
            app->kb_shift = 1;
            if (app->click >= 0) {
                app->click = -1;
                ez_send_expose (ev->win);
            }
            break;
    }

    if (app->state == S_START || app->state == S_MAGNET) app->state = S_PLAY;
    if (app->state != S_PLAY) return;

    switch (ev->key_sym) {
        case XK_n :
            tangram_generate_problem (&app->tangram, app->level);
            if (app->auto_mix)
                tangram_place_pieces_on_circle (&app->tangram);
            app->click = -1;
            ez_send_expose (ev->win);
            break;

        case XK_Up :
        case XK_KP_Up :
            app->level ++;
            if (app->level >= PIECE_MAX) app->level = PIECE_MAX-1;
            tangram_generate_problem (&app->tangram, app->level);
            if (app->auto_mix)
                tangram_place_pieces_on_circle (&app->tangram);
            app->click = -1;
            ez_send_expose (ev->win);
            break;

        case XK_Down :
        case XK_KP_Down :
            app->level --;
            if (app->level < 2) app->level = 2;
            tangram_generate_problem (&app->tangram, app->level);
            if (app->auto_mix)
                tangram_place_pieces_on_circle (&app->tangram);
            app->click = -1;
            ez_send_expose (ev->win);
            break;

        case XK_m : 
        case XK_s : {
              if (app->state == S_START || app->state == S_PLAY) {
                  app->state = S_ANIM2;
                  app->click = -1;
                  if (ev->key_sym == XK_m)
                       app_prepare_mixing_trajectories (app);
                  else app_prepare_solution_trajectories (app);
                  app->anim.step = 0;
                  app->anim.nb_steps = ANIM2_NB;
                  ez_start_timer (ev->win, DELAY2);
                  ez_send_expose (ev->win);
              }
          } break;
    }
}


void win1_on_KeyRelease (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    /* Behavior depending on state */
    switch (ev->key_sym) {
        case XK_Shift_L :
        case XK_Shift_R :
            app->kb_shift = 0;
            if (app->click >= 0) {
                app->click = -1;
                ez_send_expose (ev->win);
            }
            break;
    }
}


void win1_on_TimerNotify (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    if (app->state == S_START) {

        app->state = S_ANIM2;
        app_prepare_mixing_trajectories (app);
        app->anim.step = 0;
        app->anim.nb_steps = ANIM2_NB;
    }

    if (app->state == S_ANIM2 || app->state == S_WIN) {

        app->click = -1;
        app->anim.step ++;
        app_anim_step (app, app->anim.step, app->anim.nb_steps);
        ez_send_expose (ev->win);

        if (app->anim.step < app->anim.nb_steps-1)
            ez_start_timer (ev->win, DELAY2);
        else app->state = S_PLAY;

    } else if (app->state == S_MAGNET) {

        if (tangram_apply_magnetic_grid (&app->tangram)) {
            app->state = S_WIN;
            app_prepare_solution_trajectories (app);
            app->anim.step = 0;
            app->anim.nb_steps = ANIM2_NB;
            ez_start_timer (ev->win, DELAY2);
        } else app->state = S_PLAY;
        ez_send_expose (ev->win);
    }
}


void win1_on_event (Ez_event *ev)           /* Called for each event on win1 */
{
    switch (ev->type) {
        case Expose        : win1_on_Expose (ev); break;
        case MotionNotify  : win1_on_MotionNotify (ev); break;
        case ButtonPress   : win1_on_ButtonPress (ev); break;
        case ButtonRelease : win1_on_ButtonRelease (ev); break;
        case KeyPress      : win1_on_KeyPress (ev); break;
        case KeyRelease    : win1_on_KeyRelease (ev); break;
        case TimerNotify   : win1_on_TimerNotify (ev); break;
    }
}


/*------------------------ G E N E R A L   I N I T --------------------------*/


void app_init (App *app)
{
    app->level = LEVEL_INIT;
    tangram_generate_problem (&app->tangram, app->level);
    app->click = -1;
    app->state = S_START;
    app->auto_mix = 1;
    app->kb_shift = 0;
}


void gui_init (Gui *gui, App *app)
{
    gui->app = app;

    gui->win1 = ez_window_create (WIN1_W, WIN1_H, "Tangram puzzle", win1_on_event);
    ez_window_dbuf (gui->win1, 1);
    ez_set_data (gui->win1, gui);
    ez_start_timer (gui->win1, DELAY1);
}


/*-------------------------- M A I N   P R O G R A M ------------------------*/


int main ()
{
    App app;
    Gui gui;

    if (ez_init() < 0) exit(1);

    app_init (&app);
    gui_init (&gui, &app);

    ez_main_loop ();
    exit(0);
}

