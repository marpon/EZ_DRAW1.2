/* jeu-taquin.c: a sliding puzzle
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 30/03/2014 - version 1.2
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


/*------------------------- D E F I N I T I O N S ---------------------------*/

#define BOARD_H    5
#define BOARD_W    5
#define ANIM_NB   10
#define MIX_NB    (BOARD_H*BOARD_W*2)
#define DELAY1    30
#define DELAY2    10

typedef struct {
    int board[BOARD_H][BOARD_W];
    int click_i, click_j,
        empty_i, empty_j;
    enum { S_START, S_PLAY, S_ANIM, S_WIN, S_MIX } state;
    int anim_step, mix_step;
} App;

#define CELL_XB  20
#define CELL_YB  40
#define CELL_YC  30
#define CELL_XM  50
#define CELL_YM  50
#define WIN1_H   (CELL_YB+CELL_YM*BOARD_H+CELL_YC)
#define WIN1_W   (CELL_XB+CELL_XM*BOARD_W+CELL_XB)

typedef struct {
    App *app;
    Ez_window win1;
} Gui;


/*--------------------------------- G A M E ---------------------------------*/


void init_board (App *app)
{
    int i, j, k = 1;

    for (i = 0; i < BOARD_H; i++)
    for (j = 0; j < BOARD_W; j++)
        app->board[i][j] = k++;

    app->empty_i = BOARD_H-1;
    app->empty_j = BOARD_W-1;
    app->board[app->empty_i][app->empty_j] = 0;
}


int move_is_possible (App *app)
{
    if (app->click_i < 0 || app->click_i >= BOARD_H ||
        app->click_j < 0 || app->click_j >= BOARD_W)
      return 0;

    if ( (app->click_i == app->empty_i && app->click_j != app->empty_j) ||
         (app->click_i != app->empty_i && app->click_j == app->empty_j) )
      return 1;

    return 0;
}


int cell_is_pushed (App *app, int i, int j)
{
    if ( ( (app->click_i <= i && i <= app->empty_i) ||
           (app->empty_i <= i && i <= app->click_i) ) &&
         ( (app->click_j <= j && j <= app->empty_j) ||
           (app->empty_j <= j && j <= app->click_j) ) )
      return 1;

    return 0;
}


void compute_displacement (App *app, int *si, int *sj, int *n)
{
    int ni = app->click_i - app->empty_i,
        nj = app->click_j - app->empty_j;

    *n = abs(ni)+abs(nj);
    *si = ni / *n;
    *sj = nj / *n;
}


void push_cells (App *app)
{
    int si, sj, n, ki, kj, k;

    compute_displacement (app, &si, &sj, &n);

    for (k = 0; k < n; k++) {
        ki = app->empty_i+k*si;
        kj = app->empty_j+k*sj;
        app->board[ki][kj] = app->board[ki+si][kj+sj];
    }

    app->empty_i = app->click_i;
    app->empty_j = app->click_j;
    app->board[app->empty_i][app->empty_j] = 0;
}


int cell_is_good (App *app, int i, int j)
{
    if (i == BOARD_H-1 && j == BOARD_W-1)
        return app->board[i][j] == 0;
    return app->board[i][j] == i*BOARD_W+j+1;
}


int game_is_won (App *app)
{
    int i, j;

    for (i = 0; i < BOARD_H; i++)
    for (j = 0; j < BOARD_W; j++)
        if (! cell_is_good (app, i, j))
            return 0;

    return 1;
}


void random_move (App *app)
{
    /* We alternate between lines and columns depending on mix_step */
    if (app->mix_step % 2 == 0) {
        app->click_i = ez_random (BOARD_H-1);
        if (app->click_i >= app->empty_i) app->click_i ++;
        app->click_j = app->empty_j;
    } else {
        app->click_j = ez_random (BOARD_W-1);
        if (app->click_j >= app->empty_j) app->click_j ++;
        app->click_i = app->empty_i;
    }
}


/*------------------------------ D R A W I N G S ----------------------------*/


/* Coordinates board j,i --> coordinates pixels x,y */

int board_get_x (int j)
{
    return CELL_XB + j*CELL_XM;
}

int board_get_y (int i)
{
    return CELL_YB + i*CELL_YM;
}


/* Coordinates pixels x,y --> coordinates board j,i */

int board_get_j (int x)
{
    return (x - CELL_XB) / CELL_XM - (x < CELL_XB);
}

int board_get_i (int y)
{
    return (y - CELL_YB) / CELL_YM - (y < CELL_YB);
}


void draw_cell_xy (Ez_window win, int x, int y, int val, int click, int good)
{
    int x1 = x+2, y1 = y+2, x2 = x+CELL_XM-2, y2 = y+CELL_YM-2;

    if (val == 0) return;

    ez_set_thick (click ? 3 : 1);
    ez_set_color (click ? ez_black : ez_grey);
    ez_draw_rectangle (win, x1, y1, x2, y2);

    ez_set_color (good ? ez_blue : ez_red);
    ez_set_nfont (3);
    ez_draw_text (win, EZ_MC, x+CELL_XM/2, y+CELL_YM/2, "%d", val);
}


void draw_cell_ij (Ez_window win, int i, int j, int val, int click, int good)
{
    int xb = board_get_x (j),
        yb = board_get_y (i);

    draw_cell_xy (win, xb, yb, val, click, good);
}


void draw_cell_animated (Ez_window win, App *app, int i, int j)
{
    int xb = board_get_x (j),
        yb = board_get_y (i),
        si, sj, n; 

    compute_displacement (app, &si, &sj, &n);

    xb -= sj * CELL_XM * app->anim_step / ANIM_NB;
    yb -= si * CELL_YM * app->anim_step / ANIM_NB;

    draw_cell_xy (win, xb, yb, app->board[i][j], 0, 0);
}


void draw_board (Ez_window win, App *app)
{
    int i, j;

    for (i = 0; i < BOARD_H; i++)
    for (j = 0; j < BOARD_W; j++)
    {
        if ( (app->state == S_ANIM || app->state == S_MIX) && 
             cell_is_pushed (app, i, j) )
            draw_cell_animated (win, app, i, j);
        else draw_cell_ij (win, i, j, app->board[i][j],
                i == app->click_i && j == app->click_j,
                cell_is_good (app, i, j));
    }
}


/*---------------------- E V E N T S   O N   W I N   1 ----------------------*/


void win1_on_Expose (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;
    char *s;

    ez_set_color (ez_magenta); ez_set_nfont (2);
    ez_draw_text (ev->win, EZ_TC, WIN1_W/2, 10, 
        app->state == S_WIN ? "WELL DONE !!" : "SLIDING PUZZLE");

    draw_board (ev->win, app);

    switch (app->state) {
        case S_START : s = "Press space to mix"; break;
        case S_PLAY   : s = "Click on a cell"; break;
        case S_ANIM : s = "Displacement ..."; break;
        case S_WIN : s = "Press Space to start again"; break;
        case S_MIX : s = "Mix in progress ..."; break;
        default : s = "";
    }
    ez_set_color (ez_black); ez_set_nfont (0);
    ez_draw_text (ev->win, EZ_BL, 10, WIN1_H-8, s);

}


void win1_on_MotionNotify (Ez_event *ev)
{
    (void) ev;  /* unused */
}


void win1_on_ButtonPress (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    if (app->state != S_PLAY) return;

    app->click_i = board_get_i (ev->my);
    app->click_j = board_get_j (ev->mx);

    ez_send_expose (ev->win);
}


void win1_on_ButtonRelease (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    if (app->state != S_PLAY) return;

    if (move_is_possible (app)) {
        app->state = S_ANIM;
        app->anim_step = 0;
        ez_start_timer (ev->win, DELAY1);
    } else {
        app->click_i = app->click_j = -1;
    }
    ez_send_expose (ev->win);
}


void win1_on_KeyPress (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    switch (ev->key_sym) {

        case XK_q : 
            ez_quit (); 
            break;

        case XK_space :
            if (app->state == S_START || app->state == S_WIN) {
                app->state = S_MIX;
                app->mix_step = 0;
                app->anim_step = 0;
                random_move (app);
                ez_start_timer (ev->win, DELAY1);
                ez_send_expose (ev->win);
            }
            break;
    }
}


void win1_on_TimerNotify (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    if (app->state != S_ANIM && app->state != S_MIX)
        return;

    app->anim_step ++;
    if (app->anim_step >= ANIM_NB) {
        push_cells (app);
        app->click_i = app->click_j = -1;

        if (app->state == S_MIX) {
            app->mix_step ++;
            if (app->mix_step >= MIX_NB)
                app->state = S_PLAY;
            else {
                random_move (app);
                app->anim_step = 0;
            }
        } else {
            app->state = S_PLAY;
            if (game_is_won(app))
                app->state = S_WIN;
        }
    }

    if (app->state == S_ANIM)
        ez_start_timer (ev->win, DELAY1);
    else if (app->state == S_MIX)
        ez_start_timer (ev->win, DELAY2);
    ez_send_expose (ev->win);
}


void win1_on_event (Ez_event *ev)           /* Called for each event on win1 */
{
    switch (ev->type) {
        case Expose        : win1_on_Expose (ev); break;
        case MotionNotify  : win1_on_MotionNotify (ev); break;
        case ButtonPress   : win1_on_ButtonPress (ev); break;
        case ButtonRelease : win1_on_ButtonRelease (ev); break;
        case KeyPress      : win1_on_KeyPress (ev); break;
        case TimerNotify   : win1_on_TimerNotify (ev); break;
    }
}


/*-------------------------- G E N E R A L   I N I T ------------------------*/


void app_init (App *app)
{
    init_board (app);
    app->click_i = app->click_j = -1;
    app->state = S_START;
    app->anim_step = -1;
}


void gui_init (Gui *gui, App *app)
{
    gui->app = app;

    gui->win1 = ez_window_create (WIN1_W, WIN1_H, "Sliding puzzle", win1_on_event);
    ez_window_dbuf (gui->win1, 1);
    ez_set_data (gui->win1, gui);
}


/*----------------------------------- M A I N  ------------------------------*/


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

