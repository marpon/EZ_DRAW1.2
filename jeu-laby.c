/* jeu-laby.c: a 3D maze
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 27/03/2014 - version 1.2
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


/*------------------- L A B Y R I N T H   C R E A T I O N -------------------*/


#define LABY_XM 15
#define LABY_YM 12

/* Labyrinth coding for a cell x,y: 
 *   laby[2*y+1][2*x+1] : center of cell
 *   laby[2*y  ][2*x+1] : horizontal wall
 *   laby[2*y+1][2*x  ] : verical wall
 *   laby[2*y  ][2*x  ] : unused
*/
typedef int Laby[LABY_YM*2+1][LABY_XM*2+1];

typedef struct {
    struct { int cx, cy; } l[LABY_YM*LABY_XM];
    int n;
} Neighbors;


void laby_init (Laby laby)
{
    int x, y;

    for (y = 0; y < LABY_YM*2+1; y++)
    for (x = 0; x < LABY_XM*2+1; x++)
        laby[y][x] = 1;
}


void laby_push_neighbor (Laby laby, Neighbors *neighbors, int x, int y)
{
    if (x < 0 || x >= LABY_XM || y < 0 || y >= LABY_YM) return;
    if (laby[2*y+1][2*x+1] != 1) return;
    laby[2*y+1][2*x+1] = 2;  /* Avoid several push */
    neighbors->l[neighbors->n].cx = x;
    neighbors->l[neighbors->n].cy = y;
    neighbors->n++;   
}


void laby_init_neighbors (Laby laby, Neighbors *neighbors)
{
    int x, y;

    x = ez_random(LABY_XM);
    y = ez_random(LABY_YM);
    laby[2*y+1][2*x+1] = 0;
    neighbors->n = 0;
    laby_push_neighbor (laby, neighbors, x+1,y);
    laby_push_neighbor (laby, neighbors, x-1,y);
    laby_push_neighbor (laby, neighbors, x,y+1);
    laby_push_neighbor (laby, neighbors, x,y-1);
}


void laby_crunch_wall (Laby laby, int x, int y)
{
    int dx[] = { 1, 0, -1, 0},
        dy[] = { 0, 1, 0, -1},
        xx, yy, i, j, n = 0, t[4];

    /* Store neighbours as 0 in t[] and count in n */
    for (i = 0; i < 4; i++) {
        xx = x+dx[i]; yy = y+dy[i];
        if (0 <= xx && xx < LABY_XM && 0 <= yy && yy < LABY_YM &&
            laby[2*yy+1][2*xx+1] == 0)
            t[n++] = i;
    }

    /* Randomly take one of them, then eat the wall */
    i = ez_random (n); j = t[i];
    laby[2*y+dy[j]+1][2*x+dx[j]+1] = 0;
}


int laby_step (Laby laby, Neighbors *neighbors)
{
    int n, x, y;

    if (neighbors->n == 0) return 0;  /* The labyrinth is finished */

    /* Randomly take a neighbour and set it to 0 */
    n = ez_random (neighbors->n);
    x = neighbors->l[n].cx; y = neighbors->l[n].cy; 
    laby[2*y+1][2*x+1] = 0;

    /* Eat a wall that is neighbour to a 0 */
    laby_crunch_wall (laby, x,y);

    /* Pop (x,y) from the neighbours list and push the 4 neighbours */
    neighbors->n--;
    neighbors->l[n] = neighbors->l[neighbors->n];
    laby_push_neighbor (laby, neighbors, x+1,y);
    laby_push_neighbor (laby, neighbors, x-1,y);
    laby_push_neighbor (laby, neighbors, x,y+1);
    laby_push_neighbor (laby, neighbors, x,y-1);

    return 1;
}


void laby_hide_walls (Laby laby, int a, int b)
{
    int x, y;
    for (y = 1; y < LABY_YM*2; y++)
    for (x = 1; x < LABY_XM*2; x++)
        if (laby[y][x] == a) laby[y][x] = b;
}


void laby_show_wall (Laby laby, int mx, int my)
{
    if (laby[my][mx] == 2) laby[my][mx] = 1;
}


void laby_show_walls (Laby laby, int x, int y)
{
    laby_show_wall (laby, 2*x  , 2*y+1);
    laby_show_wall (laby, 2*x+1, 2*y  );
    laby_show_wall (laby, 2*x+2, 2*y+1);
    laby_show_wall (laby, 2*x+1, 2*y+2);
}


/*-------------------- P A W N   D I S P L A C E M E N T S ------------------*/


/* Current pawn and exit */
typedef struct {
    int px, py;  /* Pawn coordinates */
    int pa;      /* Pawn angle */
    int sx, sy;  /* Exit coordinates */
} Pawn;

/* A move in several steps */
typedef struct {
    enum { M_FORWARD, M_TURN, M_BUMP1, M_BUMP2 } kind;
    double x, y, a, x1, x2, y1, y2, a1, a2;
    int k, nb;  /* step k : 0 .. nb */
} Move;


void pawn_init (Pawn *pawn)
{
    pawn->px = pawn->py = pawn->pa = 0;
    pawn->sx = LABY_XM-1; pawn->sy = LABY_YM-1;
}


void pawn_move_coords (Laby laby, Pawn *pawn, Move *move, int dx, int dy)
{
    int xx = pawn->px + dx, yy = pawn->py + dy;

    if (0 <= xx && xx < LABY_XM && 0 <= yy && yy < LABY_YM &&
        laby[2*pawn->py+dy+1][2*pawn->px+dx+1] == 0) 
    {
        move->kind = M_FORWARD;
        move->x = move->x1 = pawn->px; move->x2 = xx;
        move->y = move->y1 = pawn->py; move->y2 = yy;
        move->a = move->a1 = move->a2 = pawn->pa;
        move->k = 0; move->nb = 20;
    } else {
        move->kind = M_BUMP1;
        move->x = move->x1 = pawn->px; move->x2 = pawn->px + 0.1*dx;
        move->y = move->y1 = pawn->py; move->y2 = pawn->py + 0.1*dy;
        move->a = move->a1 = move->a2 = pawn->pa;
        move->k = 0; move->nb = 6;
    }
}


void pawn_move_forward (Laby laby, Pawn *pawn, Move *move, int dir)
{
    int dx = (pawn->pa ==  0) ? dir : (pawn->pa == 180) ? -dir : 0,
        dy = (pawn->pa == 90) ? dir : (pawn->pa == 270) ? -dir : 0;
    pawn_move_coords (laby, pawn, move, dx, dy);
}


void pawn_move_lateral (Laby laby, Pawn *pawn, Move *move, int dir)
{
    int dx = (pawn->pa == 270) ? dir : (pawn->pa ==  90) ? -dir : 0,
        dy = (pawn->pa ==   0) ? dir : (pawn->pa == 180) ? -dir : 0;
    pawn_move_coords (laby, pawn, move, dx, dy);
}


void pawn_move_turn (Pawn *pawn, Move *move, int dir)
{
    int a = pawn->pa + dir*90;

    move->kind = M_TURN;
    move->x = move->x1 = move->x2 = pawn->px;
    move->y = move->y1 = move->y2 = pawn->py;
    move->a = move->a1 = pawn->pa; move->a2 = a;
    move->k = 0; move->nb = 20 ;
}


/*---------------------------------- A P P ------ ---------------------------*/


/* Size in pixels */
#define W_X0  25
#define W_Y0  40
#define W_TA  22
#define P_RA  8

/* Windows sizes */
#define WIN1_W (W_X0+W_TA*LABY_XM+W_X0)
#define WIN1_H (W_Y0+W_TA*LABY_YM+30)
#define WIN2_W 400
#define WIN2_H 400

/* Animation delays for creation and moves */
#define DELAY1 20
#define DELAY2 20

/* Parameters for 3D */
#define HOMO_FACTOR  0.5
#define PROJ_DISTD   4

typedef struct {
    enum { S_START, S_CREATE, S_PLAY, S_MOVE, S_WIN } state;
    Laby laby;             /* The labyrinth and the walls */
    Neighbors neighbors;   /* List of neighbors during creation */
    Pawn pawn;             /* Current coordinates */
    Move move;             /* Move in progress */
    int kb_arrow;          /* Current arrow pressed */
    int kb_shift;          /* A shift key is pressed */
    double distD;          /* Distance from observer to focal plane */
    Ez_window win1, win2;  /* Windows of the map, of the 3D view */
} App;


void app_init (App *app)
{
    app->state = S_START;
    app->kb_arrow = 0;
    app->kb_shift = 0;
    app->distD = PROJ_DISTD;

    laby_init (app->laby);
}


/*----------------------- D R A W I N G S   F O R   W I N 1 -----------------*/


void draw_wall_h (Ez_window win, int x, int y)
{
    ez_draw_line (win, W_X0+ x   *W_TA, W_Y0+y*W_TA, 
                       W_X0+(x+1)*W_TA, W_Y0+y*W_TA);
}


void draw_wall_v (Ez_window win, int x, int y)
{
    ez_draw_line (win, W_X0+x*W_TA, W_Y0+ y   *W_TA, 
                       W_X0+x*W_TA, W_Y0+(y+1)*W_TA);
}


void draw_cell (Ez_window win, int x, int y)
{
    ez_fill_rectangle (win, W_X0+ x   *W_TA+1, W_Y0+ y   *W_TA+1, 
                            W_X0+(x+1)*W_TA-1, W_Y0+(y+1)*W_TA-1);
}


void draw_pawn (Ez_window win, double x, double y, double a)
{
    double b = a * M_PI / 180, c = cos (b), s = sin (b);
    int x1 = W_X0+(x+0.5)*W_TA + 0.5, y1 = W_Y0+(y+0.5)*W_TA + 0.5;
             
    ez_draw_circle (win, x1 - P_RA, y1 - P_RA, x1 + P_RA, y1 + P_RA);

    ez_draw_line (win, x1, y1, x1 + c*P_RA + 0.5, y1 + s*P_RA + 0.5);
}


void draw_exit (Ez_window win, int x, int y)
{
    ez_draw_line (win, W_X0+ x   *W_TA+3, W_Y0+ y   *W_TA+3, 
                       W_X0+(x+1)*W_TA-3, W_Y0+(y+1)*W_TA-3);
    ez_draw_line (win, W_X0+(x+1)*W_TA-3, W_Y0+ y   *W_TA+3, 
                       W_X0+ x   *W_TA+3, W_Y0+(y+1)*W_TA-3);
}


void draw_laby_2D (Ez_window win, App *app)
{
    int x, y;

    ez_set_color (ez_blue);
    for (y = 0; y <= LABY_YM; y++)
    for (x = 0; x <  LABY_XM; x++)
    if (app->laby[y*2][x*2+1] == 1) 
        draw_wall_h (win, x, y);

    for (y = 0; y <  LABY_YM; y++)
    for (x = 0; x <= LABY_XM; x++)
    if (app->laby[y*2+1][x*2] == 1) 
        draw_wall_v (win, x, y);

    if (app->state == S_CREATE) {
        ez_set_color (ez_grey);
        for (y = 0; y <  LABY_YM; y++)
        for (x = 0; x <  LABY_XM; x++)
        if (app->laby[y*2+1][x*2+1] == 1)
            draw_cell (win, x, y);

        ez_set_color (ez_yellow);
        for (y = 0; y <  LABY_YM; y++)
        for (x = 0; x <  LABY_XM; x++)
        if (app->laby[y*2+1][x*2+1] == 2)
            draw_cell (win, x, y);
    } else {
        ez_set_color (ez_red);
        if (app->state == S_MOVE)
             draw_pawn (win, app->move.x, app->move.y, app->move.a);
        else draw_pawn (win, app->pawn.px, app->pawn.py, app->pawn.pa);
        draw_exit (win, app->pawn.sx, app->pawn.sy);
    }
}


void win1_redraw (Ez_window win, App *app)
{
    char *s;

    switch (app->state) {
        case S_START : s = "\n** Labyrinth **"; break;
        case S_CREATE : s = "Creation in progress ..."; break;
        case S_PLAY :
        case S_MOVE : s = "Find the exit!"; break;
        case S_WIN : s = "Well done !!"; break;
        default : s = "";
    }
    ez_set_color (ez_blue); ez_set_nfont (2);
    ez_draw_text (win, EZ_TC, WIN1_W/2, 10, s);

    ez_set_color (ez_black); ez_set_nfont (0);
    ez_draw_text (win, EZ_BL, 10, WIN1_H-5, 
        "n: new game  r: reveal  q: quit");

    if (app->state == S_START) {

        ez_set_color (ez_magenta); ez_set_nfont (1);
        ez_draw_text (win, EZ_TC, WIN1_W/2, WIN1_H*3/7,
            "Use arrows to move\n\n"
            "Type 'n' to start ...");
        ez_set_color (ez_grey); ez_set_nfont (0);
        ez_draw_text (win, EZ_TC, WIN1_W/2, WIN1_H*5/7,
            "The program jeu-laby.c is part of EZ-Draw:\n"
            "http://pageperso.lif.univ-mrs.fr/~edouard.thiel/ez-draw");

    } else draw_laby_2D (win, app);
}


/*----------------------- E V E N T S   F O R   W I N 1 ---------------------*/


void win1_on_Expose (Ez_event *ev, App *app)
{
    win1_redraw (ev->win, app);
}


void send_2expose (App *app)
{
    ez_send_expose (app->win1);
    ez_send_expose (app->win2);
}


void move_for_arrow (App *app)
{
    switch (app->kb_arrow) {
        case XK_Left     : 
        case XK_KP_Left  : 
            if (app->kb_shift) 
                 pawn_move_lateral (app->laby, &app->pawn, &app->move, -1); 
            else pawn_move_turn (&app->pawn, &app->move, -1); 
            break;

        case XK_Right    : 
        case XK_KP_Right : 
            if (app->kb_shift) 
                 pawn_move_lateral (app->laby, &app->pawn, &app->move, 1); 
            else pawn_move_turn (&app->pawn, &app->move, 1); 
            break;

        case XK_Up       : 
        case XK_KP_Up    : 
            pawn_move_forward (app->laby, &app->pawn, &app->move, 1); 
            break;

        case XK_Down     : 
        case XK_KP_Down  : 
            pawn_move_forward (app->laby, &app->pawn, &app->move, -1); 
            break;

        default : return;
    }

    app->state = S_MOVE;
    ez_start_timer (app->win2, DELAY2);
}


void move_step (App *app)
{
    double u; 

    app->move.k++;
    u = (double) app->move.k / app->move.nb;
    app->move.x = app->move.x1 * (1-u) + app->move.x2 * u;
    app->move.y = app->move.y1 * (1-u) + app->move.y2 * u;
    app->move.a = app->move.a1 * (1-u) + app->move.a2 * u;

    if (app->move.k == app->move.nb && app->move.kind == M_BUMP1) {
        double tmp;
        app->move.kind = M_BUMP2;
        tmp = app->move.x1; app->move.x1 = app->move.x2; app->move.x2 = tmp;
        tmp = app->move.y1; app->move.y1 = app->move.y2; app->move.y2 = tmp;
        app->move.k = 0;
    } 

    if (app->move.k == app->move.nb) {
        app->state = S_PLAY;
        if (app->move.kind == M_FORWARD) {
            app->pawn.px = app->move.x2; app->pawn.py = app->move.y2;
            laby_show_walls (app->laby, app->pawn.px, app->pawn.py);
            if (app->pawn.px == app->pawn.sx && app->pawn.py == app->pawn.sy) { 
                /* We reach the exit! */
                app->state = S_WIN; laby_hide_walls (app->laby, 2, 1); 
            } else if (app->kb_arrow != 0) 
                move_for_arrow (app);
        } else if (app->move.kind == M_TURN) {
            app->pawn.pa = (int) (app->move.a2 + 360) % 360;
            if (app->kb_arrow != 0) 
                move_for_arrow (app);
        }
    } else ez_start_timer (app->win2, DELAY2);

    send_2expose (app);
}


void win1_on_KeyPress (Ez_event *ev, App *app)
{
    switch (ev->key_sym) {
        case XK_q : 
            ez_quit (); 
            break;
        case XK_Left :
        case XK_Right :
        case XK_Up :
        case XK_Down :
        case XK_KP_Left :
        case XK_KP_Right :
        case XK_KP_Up :
        case XK_KP_Down :
            app->kb_arrow = ev->key_sym;
            if (app->state == S_PLAY) move_for_arrow (app);
            break;
        case XK_Shift_L :
        case XK_Shift_R :
            app->kb_shift = 1;
            break;
        case XK_r :
            if (app->state == S_PLAY) { 
                laby_hide_walls (app->laby, 2, 1); 
                ez_send_expose (ev->win); 
            }
            break;
        case XK_d :
            app->distD /= 1.05; 
            if (app->distD < 2) app->distD = 2;
            ez_send_expose (app->win2);
            break;
        case XK_D :
            app->distD *= 1.05; 
            if (app->distD > 10) app->distD = 10;
            ez_send_expose (app->win2);
            break;
        case XK_n :
            app->state = S_CREATE;
            laby_init (app->laby);
            laby_init_neighbors (app->laby, &app->neighbors);
            ez_start_timer (ev->win, DELAY1);
            break;
    }
}


void win1_on_KeyRelease (Ez_event *ev, App *app)
{
    switch (ev->key_sym) {
        case XK_Left :
        case XK_Right :
        case XK_Up :
        case XK_Down :
        case XK_KP_Left :
        case XK_KP_Right :
        case XK_KP_Up :
        case XK_KP_Down :
            app->kb_arrow = 0;
            break;
        case XK_Shift_L :
        case XK_Shift_R :
            app->kb_shift = 0;
            break;
    }
}


void win1_on_TimerNotify (Ez_event *ev, App *app)
{
    if (app->state == S_CREATE) {
        /* Make a further step in labyrinth creation */
        int k = laby_step (app->laby, &app->neighbors);

        if (k == 1) {
            /* The labyrinth is not finished, restart the timer */
            ez_start_timer (ev->win, DELAY1);
        } else { 
            /* The labyrinth is finish, we can play */
            app->state = S_PLAY; 
            laby_hide_walls (app->laby, 1, 2); 
            pawn_init (&app->pawn); 
            laby_show_walls (app->laby, app->pawn.px, app->pawn.py);
            ez_window_show (app->win2, 1);
        }
        send_2expose (app);
    }
}


void win1_on_event (Ez_event *ev)           /* Called for each event on win1 */
{
    App *app = ez_get_data (ev->win);

    switch (ev->type) {
        case Expose        : win1_on_Expose        (ev, app); break;
        case KeyPress      : win1_on_KeyPress      (ev, app); break;
        case KeyRelease    : win1_on_KeyRelease    (ev, app); break;
        case TimerNotify   : win1_on_TimerNotify   (ev, app); break;
    }
}


/*-------------------- 3 D   T R A N S F O R M A T I O N S ------------------*/


#define ZEDIM 4
typedef double Matr[ZEDIM][ZEDIM];  /* [line][col] */
typedef double Vec[ZEDIM];


/*
 * Display the matrix for debugging
*/

void matr_print (Matr m)
{
    int i, j;
    
    for (i = 0; i < ZEDIM; i++) {
        for (j = 0; j < ZEDIM; j++) printf ("%2.3f ", m[i][j]);
        printf ("\n");
    }
}


/*
 * Initialize matrix m to unity
*/

void matr_unity (Matr m)
{
    int i, j;

    for (i = 0; i < ZEDIM; i++)
    for (j = 0; j < ZEDIM; j++)
        m[i][j] = 0;

    for (i = 0; i < ZEDIM; i++)
        m[i][i] = 1;
}


/*
 * Copy matrix m to res
*/

void matr_copy (Matr m, Matr res)
{
    int i, j;

    for (i = 0; i < ZEDIM; i++)
    for (j = 0; j < ZEDIM; j++)
        res[i][j] = m[i][j];
}


/*
 * Multiply matrix m1 by matrix m2 and store in res
*/

void matr_mult_matrix (Matr m1, Matr m2, Matr res)
{
    int i, j, k;
    double d;

    for (i = 0; i < ZEDIM; i++)
    for (j = 0; j < ZEDIM; j++)
    {
        d = 0;
        for (k = 0; k < ZEDIM; k++)
            d += m1[i][k] * m2[k][j];
        res[i][j] = d;
    }
}


/*
 * Multiply matrix m1 by vector v2 and store in res
*/

void matr_mult_vector (Matr m1, Vec v2, Vec res)
{
    int i, k;
    double d;

    for (i = 0; i < ZEDIM; i++)
    {
        d = 0;
        for (k = 0; k < ZEDIM; k++)
            d += m1[i][k] * v2[k];
        res[i] = d;
    }
}


/*
 * Make operation: m = T*m
 * where T is the conic projection matrix on plane XY and center (0,0,-D)
*/

void matr_mult_projection (Matr m, double d)
{
    Matr res, tmp;

    tmp[0][0] =  d; tmp[0][1] =  0; tmp[0][2] =  0; tmp[0][3] =  0;
    tmp[1][0] =  0; tmp[1][1] =  d; tmp[1][2] =  0; tmp[1][3] =  0;
    tmp[2][0] =  0; tmp[2][1] =  0; tmp[2][2] =  d; tmp[2][3] =  0;
    tmp[3][0] =  0; tmp[3][1] =  0; tmp[3][2] =  1; tmp[3][3] =  d;
    
    matr_mult_matrix (tmp, m, res);
    matr_copy (res, m);
}


/*
 * Make operation: m = T*m
 * where T is the rotation matrix for angle r (in degrees) and X axis
*/

void matr_mult_rot_X (Matr m, double r)
{
    Matr res, tmp;
    double a, c, s;

    a = r * M_PI / 180; c = cos(a); s = sin(a);

    tmp[0][0] =  1; tmp[0][1] =  0; tmp[0][2] =  0; tmp[0][3] =  0;
    tmp[1][0] =  0; tmp[1][1] =  c; tmp[1][2] =  s; tmp[1][3] =  0;
    tmp[2][0] =  0; tmp[2][1] = -s; tmp[2][2] =  c; tmp[2][3] =  0;
    tmp[3][0] =  0; tmp[3][1] =  0; tmp[3][2] =  0; tmp[3][3] =  1;
  
    matr_mult_matrix (tmp, m, res);
    matr_copy (res, m);
}


/*
 * Make operation: m = T*m
 * where T is the rotation matrix for angle r (in degrees) and Y axis
*/

void matr_mult_rot_Y (Matr m, double r)
{
    Matr res, tmp;
    double a, c, s;

    a = r * M_PI / 180; c = cos(a); s = sin(a);

    tmp[0][0] =  c; tmp[0][1] =  0; tmp[0][2] =  s; tmp[0][3] =  0;
    tmp[1][0] =  0; tmp[1][1] =  1; tmp[1][2] =  0; tmp[1][3] =  0;
    tmp[2][0] = -s; tmp[2][1] =  0; tmp[2][2] =  c; tmp[2][3] =  0;
    tmp[3][0] =  0; tmp[3][1] =  0; tmp[3][2] =  0; tmp[3][3] =  1;

    matr_mult_matrix (tmp, m, res);
    matr_copy (res, m);
}


/*
 * Make operation: m = T*m
 * where T is the rotation matrix for angle r (in degrees) and Z axis
*/

void matr_mult_rot_Z (Matr m, double r)
{
    Matr res, tmp;
    double a, c, s;

    a = r * M_PI / 180; c = cos(a); s = sin(a);

    tmp[0][0] =  c; tmp[0][1] =  s; tmp[0][2] =  0; tmp[0][3] =  0;
    tmp[1][0] = -s; tmp[1][1] =  c; tmp[1][2] =  0; tmp[1][3] =  0;
    tmp[2][0] =  0; tmp[2][1] =  0; tmp[2][2] =  1; tmp[2][3] =  0;
    tmp[3][0] =  0; tmp[3][1] =  0; tmp[3][2] =  0; tmp[3][3] =  1;

    matr_mult_matrix (tmp, m, res);
    matr_copy (res, m);
}


/*
 * Make operation: m = T*m
 * where T is the translation matrix (dx,dy,dz)
*/

void matr_mult_translation (Matr m, double dx, double dy, double dz)
{
    Matr res, tmp;

    tmp[0][0] =  1; tmp[0][1] =  0; tmp[0][2] =  0; tmp[0][3] = dx;
    tmp[1][0] =  0; tmp[1][1] =  1; tmp[1][2] =  0; tmp[1][3] = dy;
    tmp[2][0] =  0; tmp[2][1] =  0; tmp[2][2] =  1; tmp[2][3] = dz;
    tmp[3][0] =  0; tmp[3][1] =  0; tmp[3][2] =  0; tmp[3][3] =  1;
    
    matr_mult_matrix (tmp, m, res);
    matr_copy (res, m);
}


/*
 * Make operation: m = T*m
 * where T is the homothety matrix of center 0 and factor (rx, ry, rz)
*/

void matr_mult_homothety (Matr m, double rx, double ry, double rz)
{
    Matr res, tmp;

    tmp[0][0] = rx; tmp[0][1] =  0; tmp[0][2] =  0; tmp[0][3] =  0;
    tmp[1][0] =  0; tmp[1][1] = ry; tmp[1][2] =  0; tmp[1][3] =  0;
    tmp[2][0] =  0; tmp[2][1] =  0; tmp[2][2] = rz; tmp[2][3] =  0;
    tmp[3][0] =  0; tmp[3][1] =  0; tmp[3][2] =  0; tmp[3][3] =  1;
    
    matr_mult_matrix (tmp, m, res);
    matr_copy (res, m);
}


/*
 * Geometric transform of a vertex by using homogeneous coordinates:
 *  - we apply the transformation matrix m to the vector (x1, y1, z1, 1).
 *  - we obtain a vector (x', y', z', t').
 *  - if t' != 0, the result is *x2=x'/t', *y2=y'/t', *z2 = z'/fabs(t').
*/

void matr_transform_vertex (Matr m, double  x1, double  y1, double  z1, 
                                      double *x2, double *y2, double *z2)
{
    Vec v1, v2;
    
    v1[0] = x1; v1[1] = y1; v1[2] = z1; v1[3] = 1;
    matr_mult_vector (m, v1, v2);

    /* Point to inifinite */
    if (v2[3] == 0) { *x2 = *y2 = 0; *z2 = -1; return; }

    /* We use fabs() so that objects behind the observer get a z < 0 */
    *x2 = v2[0]/v2[3]; *y2 = v2[1]/v2[3]; *z2 = v2[2]/fabs(v2[3]); 
}


/* (Unused in this program)
 *
 * Cut segment in tz = 0 considering tz1 < 0 <= tz2.
 * A linear interpolation in tz=0 being unsufficient, we make a dichotomic
 * search of the cutting point. Usage:
 *   if (tz1 < 0)
 *       cut_segment (m, x1, y1, 1, x2, y2, 1, tx1, tx2, &tx1, &ty1, &tz1);
 *   else if (tz2 < 0)
 *       cut_segment (m, x2, y2, 1, x1, y1, 1, tx2, tx1, &tx2, &ty2, &tz2);
*/

void cut_segment (Matr m, double   x1, double   y1, double   z1, 
                          double   x2, double   y2, double   z2, 
                          double  tx1, double  tx2, 
                          double *ox3, double *oy3, double *oz3)
{
    double x3, y3, z3;

    while ( fabs(tx2 - tx1) >= 1 ) {
        x3 = (x1+x2)/2; y3 = (y1+y2)/2; z3 = (z1+z2)/2;
        matr_transform_vertex (m, x3, y3, z3, ox3, oy3, oz3);
        if (*oz3 < 0) { x1 = x3; y1 = y3; z1 = z3; tx1 = *ox3; }
        else          { x2 = x3; y2 = y3; z2 = z3; tx2 = *ox3; }
    }
}


/*----------------------- L I N E   Z - B U F F E R -------------------------*/


#define ZBUF_MAX 2048
typedef struct {
    double y[ZBUF_MAX], z[ZBUF_MAX];
    int    e[ZBUF_MAX], width;
} Zbuffer;


/* 
 * Initialize the line Z-buffer 
*/

void zbuf_init (Zbuffer *buf, int width)
{
    int x;

    buf->width = (width < ZBUF_MAX) ? width : ZBUF_MAX;

    for (x = 0; x < buf->width; x++) {
        buf->y[x] = -1;
        buf->z[x] = -1;
        buf->e[x] = 0;
    }
}


/*
 * Blend visible points of a segment in the Z-buffer
*/

void zbuf_blend_segment (Zbuffer *buf, Matr m, int x1, int y1, int x2, int y2)
{
    double tx1, ty1, tz1, tx2, ty2, tz2, tmp, z;
    int x, ix1, ix2;

    matr_transform_vertex (m, x1, y1, 1, &tx1, &ty1, &tz1);
    matr_transform_vertex (m, x2, y2, 1, &tx2, &ty2, &tz2);

    /* If both extremities are out of the window then return */
    if ((tx1 < 0 && tx2 < 0) || (tx1 >= buf->width && tx2 >= buf->width)) return;

    /* If both extremities are behind the observer then return */
    if (tz1 < 0 && tz2 < 0) return; 

    /* If one extremity is behind and the other one is out of the window, then
       this is a wall that we can not see */
    if ( (tz1 < 0 && (tx2 < 0 || tx2 >= buf->width)) ||
         (tz2 < 0 && (tx1 < 0 || tx1 >= buf->width)) ) return;

    /* Set x in ascending order */
    if (tx1 > tx2) {
        tmp = tx1; tx1 = tx2; tx2 = tmp;
        tmp = ty1; ty1 = ty2; ty2 = tmp;
        tmp = tz1; tz1 = tz2; tz2 = tmp;
    }

    /* Compute bounds of the window */
    ix1 = (tx1 >= 0) ? tx1 : 0;
    ix2 = (tx2 < buf->width) ? tx2 : buf->width-1;

    /* For each point of the segment we check if the point will be visible */
    for (x = ix1; x <= ix2; x++) {

        /* Linear interpolation in z: wrong but sufficient */
        z = tz1 + (x-tx1)*(tz2-tz1)/(tx2-tx1) + 0.5;  /* rounding */

        /* This point is behind, try the next */
        if (buf->z[x] != -1 && z >= buf->z[x]) continue;

        /* This point is visible, we store it in the Z-buffer */
        buf->z[x] = z;
        buf->y[x] = ty1 + (x-tx1)*(ty2-ty1)/(tx2-tx1) + 0.5;  /* rounding */
        /* e[x] =  0 interior, 1 extremity, 2 cut */
        buf->e[x] = (x != ix1 && x != ix2) ? 0 :  
                    (fabs(x-tx1) > 1 && fabs(x-tx2) > 1) ? 2 : 1;
    }
}


/*
 * Display the scene by using the Z-buffer
*/

void zbuf_render (Ez_window win, Zbuffer *buf, int height)
{
    int x, lx = -1;

    ez_set_color (ez_blue);

    for (x = 0; x < buf->width; x++) {
        if (buf->e[x] >= 1 || x == buf->width-1) {
            if (lx+1 < x-1) {
                /* Display top and bottom of the wall */
                ez_draw_line (win, lx+1, buf->y[lx+1], x-1, buf->y[x-1]);
                ez_draw_line (win, lx+1, height-buf->y[lx+1], x-1, height-buf->y[x-1]);
            }
            if (buf->e[x] == 1) {
                /* Display vertical side of a wall */
                ez_draw_line (win, x, height - buf->y[x], x, buf->y[x]);
            }
            lx = x;
        } 
    }
}


/*-------------------------- 3 D   R E N D E R I N G ------------------------*/


/*
 * Compute the product matrix m of the simple transformations.
*/

void compute_3D_transform (Matr m, double angle, 
    double distD, int width, int height, double factor, double cx, double cy)
{
    matr_unity (m);
    matr_mult_translation (m, -cx, -cy, 0);
    matr_mult_rot_Z (m, angle+90);
    matr_mult_rot_X (m, 90);
    matr_mult_projection (m, distD);
    matr_mult_homothety (m, factor*width, factor*height, 1);
    matr_mult_translation (m, width/2, height/2, 0);
}


/*
 * Display in perspective projection:
 *  - compute the product matrix m of the simple transformations
 *  - apply m to 3D vertices -> get 2D vertices
 *  - adapt 2D coordinates to the window size
 *  - compute visible parts with a line Z-buffer
 *  - display visible parts.
*/

void draw_in_perspective (Laby laby, Ez_window win, int width, int height, 
    double factor, double angle, double distD, double px, double py)
{
    int x, y;
    Matr m;
    Zbuffer buf;
    
    /* Compute the product matrix m of the simple transformations */
    compute_3D_transform (m, angle, distD, width, height, factor, px*2+1, py*2+1);
    
    /* Use a line Z-buffer in x to compute the visible parts */
    zbuf_init (&buf, width);

    for (y = 0; y <= LABY_YM; y++)
    for (x = 0; x <  LABY_XM; x++)
    if (laby[y*2][x*2+1] >= 1) 
        zbuf_blend_segment (&buf, m, 2*x, 2*y, 2*x+2, 2*y);

    for (y = 0; y <  LABY_YM; y++)
    for (x = 0; x <= LABY_XM; x++)
    if (laby[y*2+1][x*2] >= 1) 
        zbuf_blend_segment (&buf, m, 2*x, 2*y, 2*x, 2*y+2);

    /* Display the visible parts */
    zbuf_render (win, &buf, height);
}


/*----------------------- E V E N T S   F O R   W I N 2 ---------------------*/


void win2_on_Expose (Ez_event *ev, App *app)
{
    int w, h;

    ez_window_get_size (ev->win, &w, &h);
    if (app->state == S_MOVE)
         draw_in_perspective (app->laby, ev->win, w, h, HOMO_FACTOR, 
            app->move.a, app->distD, app->move.x, app->move.y);
    else draw_in_perspective (app->laby, ev->win, w, h, HOMO_FACTOR, 
            app->pawn.pa, app->distD, app->pawn.px, app->pawn.py);

    ez_set_color (ez_black); ez_set_nfont (0);
    ez_draw_text (ev->win, EZ_BC, w/2, h-5, 
        "Arrows: move   shift+arrows: lateral");
    ez_draw_text (ev->win, EZ_TC, w/2, 5, 
        "d,D: focal dist %.2f", app->distD);
}


void win2_on_KeyPress (Ez_event *ev, App *app)
{
    Ez_event ev2 = *ev;
    ev2.win = app->win1;
    win1_on_KeyPress (&ev2, app);
}


void win2_on_KeyRelease (Ez_event *ev, App *app)
{
    Ez_event ev2 = *ev;
    ev2.win = app->win1;
    win1_on_KeyRelease (&ev2, app);
}


void win2_on_TimerNotify (Ez_event *ev, App *app)
{
    (void) ev;  /* unused */
    if (app->state == S_MOVE) move_step (app);
}


void win2_on_event (Ez_event *ev)           /* Called for each event on win2 */
{
    App *app = ez_get_data (ev->win);

    switch (ev->type) {
        case Expose        : win2_on_Expose        (ev, app); break;
        case KeyPress      : win2_on_KeyPress      (ev, app); break;
        case KeyRelease    : win2_on_KeyRelease    (ev, app); break;
        case TimerNotify   : win2_on_TimerNotify   (ev, app); break;
    }
}


/*----------------------------------- G U I ---------------------------------*/


void gui_init (App *app)
{
    app->win1 = ez_window_create (WIN1_W, WIN1_H, "Game of labyrinth", win1_on_event);
    app->win2 = ez_window_create (WIN2_W, WIN2_H, "3D View", win2_on_event);
    ez_window_show (app->win2, 0);

    ez_set_data (app->win1, app);
    ez_set_data (app->win2, app);

    /* Double buffer to avoid blinking */
    ez_window_dbuf (app->win1, 1);
    ez_window_dbuf (app->win2, 1);
}


/*------------------------ M A I N   P R O G R A M --------------------------*/

int main ()
{
    App app;

    if (ez_init() < 0) exit(1);

    app_init (&app);
    gui_init (&app);

    ez_main_loop ();
    exit(0);
}

