/* jeu-2048.c: a sliding puzzle - powers of 2 / Fibonacci series
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 15/04/2014 - version 1.2
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


/*------------------------- D E F I N I T I O N S ---------------------------*/

#define BOARD_H       4
#define BOARD_W       4
#define BOARD_T       8
#define ANIM1_NB     12
#define DELAY1       20

#define TARGET_POW2   2048
#define TARGET_FIBO   2584

typedef struct {
    enum { G_POW2, G_FIBO, G_LAST } mode;
    int board[BOARD_H][BOARD_W];
    enum { S_PLAY, S_ANIM, S_NOSHIFT, S_WIN, S_LOST } state;
    enum { D_LEFT, D_RIGHT, D_UP, D_DOWN } dir;
    int anim_step;
    int score, best_score, val_max, target_val;
} App;

#define CELL_XB  20
#define CELL_YB  70
#define CELL_YC  30
#define CELL_XM  80
#define CELL_YM  80
#define WIN1_H   (CELL_YB+CELL_YM*BOARD_H+CELL_YC)
#define WIN1_W   (CELL_XB+CELL_XM*BOARD_W+CELL_XB)

typedef struct {
    App *app;
    Ez_window win1;
} Gui;


/*--------------------------------- G A M E ---------------------------------*/


void set_mode (App *app, int mode)
{
    app->mode = mode;
    switch (mode) {
        case G_POW2 : app->target_val = TARGET_POW2; break;
        case G_FIBO : app->target_val = TARGET_FIBO; break;
    }
}


void change_mode (App *app)
{
    set_mode (app, (app->mode+1) % G_LAST);
}


int is_couple_fibo (int val1, int val2)
{
    if (val1 == 1 && val2 == 1) return 1;
    if (val1 > val2) { int tmp = val1; val1 = val2; val2 = tmp; }
    return (int)(val1*1.6180339887498949+0.5) == val2;
}


int is_couple (App *app, int val1, int val2)
{
    switch (app->mode) {
        case G_POW2 : return val1 == val2;
        case G_FIBO : return is_couple_fibo (val1, val2);
        default     : return 0;
    }
}


void init_board (App *app)
{
    int i, j;

    for (i = 0; i < BOARD_H; i++)
    for (j = 0; j < BOARD_W; j++)
        app->board[i][j] = 0;
    app->score = 0;
}


int count_empty_cells (App *app)
{
    int i, j, n = 0;

    for (i = 0; i < BOARD_H; i++)
    for (j = 0; j < BOARD_W; j++)
        if (app->board[i][j] == 0) n++;

    return n;
}


int some_cells_can_merge (App *app)
{
    int i, j;

    for (i = 0; i < BOARD_H  ; i++)
    for (j = 0; j < BOARD_W-1; j++)
        if (app->board[i][j] != 0 &&
            is_couple (app, app->board[i][j], app->board[i][j+1]))
            return 1;

    for (i = 0; i < BOARD_H-1; i++)
    for (j = 0; j < BOARD_W  ; j++)
        if (app->board[i][j] != 0 &&
            is_couple (app, app->board[i][j], app->board[i+1][j]))
            return 1;

    return 0;
}


void insert_val_at_rank (App *app, int val, int rank)
{
    int i, j, k = 0;
    for (i = 0; i < BOARD_H; i++)
    for (j = 0; j < BOARD_W; j++)
        if (app->board[i][j] == 0 && ++k == rank) { 
            app->board[i][j] = val; return; 
         }
}


void insert_random_cell (App *app)
{
    int n, p, v;

    n = count_empty_cells (app);
    if (n == 0) return;
    p = 1 + ez_random (n);
    switch (app->mode) {
        case G_POW2 : v = ez_random (10) == 0 ? 4 : 2; break;
        case G_FIBO : v = ez_random (10) == 0 ? 2 : 1; break;
        default     : v = 1;
    }
    insert_val_at_rank (app, v, p);
}


void new_game (App *app)
{
    init_board (app);
    insert_random_cell (app);
    insert_random_cell (app);
    app->score = 0;
    app->val_max = 0;
    app->state = S_PLAY;
    app->anim_step = 0;
}


void add_points (App *app, int points)
{
    app->score += points;
    if (app->best_score < app->score)
        app->best_score = app->score;
}


void update_val_max (App *app, int cur)
{
    if (app->val_max < cur)
        app->val_max = cur;
}


void board_get_bounds (App *app, int *amax, int *bmax)
{
    switch (app->dir) {
        case D_LEFT  :
        case D_RIGHT : *amax = BOARD_H; *bmax = BOARD_W; break;
        case D_UP    :
        case D_DOWN  : *amax = BOARD_W; *bmax = BOARD_H; break;
        default : *amax = *bmax = 0; /* to make compiler happy */
    }
}


int board_get_val (App *app, int a, int b)
{
    switch (app->dir) {
        case D_LEFT  : return app->board[a][b          ];
        case D_RIGHT : return app->board[a][BOARD_W-1-b];
        case D_UP    : return app->board[b          ][a];
        case D_DOWN  : return app->board[BOARD_H-1-b][a];
    }
    return 0;  /* to make compiler happy */
}


void board_set_val (App *app, int a, int b, int val)
{
    switch (app->dir) {
        case D_LEFT  : app->board[a][b          ] = val; break;
        case D_RIGHT : app->board[a][BOARD_W-1-b] = val; break;
        case D_UP    : app->board[b          ][a] = val; break;
        case D_DOWN  : app->board[BOARD_H-1-b][a] = val; break;
    }
}


int board_can_shift (App *app)
{
    int a, b, amax, bmax, k, cur, prev;

    board_get_bounds (app, &amax, &bmax);

    for (a = 0; a < amax; a++)
    for (b = k = prev = 0; b < bmax; b++)
    {
        cur = board_get_val (app, a, b);
        if (cur == 0) continue;
        if (is_couple (app, cur, prev)) return 1;
        if (k != b) return 1; 
        prev = cur;
        k++;
    }
    return 0;
}


void board_shift (App *app)
{
    int a, b, amax, bmax, k, cur, prev;

    board_get_bounds (app, &amax, &bmax);

    for (a = 0; a < amax; a++)
    for (b = k = prev = 0; b < bmax; b++)
    {
        cur = board_get_val (app, a, b);
        if (cur == 0) continue;
        if (is_couple (app, cur, prev)) {
            board_set_val (app, a, k-1, cur+prev);
            board_set_val (app, a, b, 0);
            add_points (app, cur+prev);
            update_val_max (app, cur+prev);
            prev = 0;
        } else {
            if (k != b) {
                board_set_val (app, a, k, cur);
                board_set_val (app, a, b, 0);
            }
            prev = cur;
            k++;
        }
    }
}


/*------------------------------ D R A W I N G S ----------------------------*/


void set_color_hexa (int hexa)
{
    Ez_uint8 r = (hexa >> 16) & 0xff,
             g = (hexa >>  8) & 0xff,
             b = (hexa      ) & 0xff;
    Ez_uint32 color = ez_get_RGB (r, g, b);
    ez_set_color (color);
}


int board_get_x (double j)
{
    return CELL_XB + j*CELL_XM;
}


int board_get_y (double i)
{
    return CELL_YB + i*CELL_YM;
}


int get_cell_bg_pow2 (int val)
{
    switch (val) {
        case    0 : return 0xcdc0b4;
        case    2 : return 0xeee4da;
        case    4 : return 0xede0c8;
        case    8 : return 0xf2b179;
        case   16 : return 0xf59563;
        case   32 : return 0xf67c5f;
        case   64 : return 0xf65e3b;
        case  128 : return 0xedcf72;
        case  256 : return 0xedcc61;
        case  512 : return 0xedc850;
        case 1024 : return 0xedc53f;
        case 2048 : return 0xedc22e;
        default   : return 0x3c3a32;
    }
}


int get_cell_ink_pow2 (int val)
{
    return val <= 4 ? 0x776e65 : 0xf9f6f2;
}


int get_cell_bg_fibo (int val)
{
    switch (val) {
        case    0 : return 0xcdc0b4;
        case    1 : return 0xeee4da;
        case    2 : return 0xede0c8;
        case    3 : return 0xf2b179;
        case    5 : return 0xf59563;
        case    8 : return 0xf67c5f;
        case   13 : return 0xf65e3b;
        case   21 : return 0xedcf72;
        case   34 : return 0xedcc61;
        case   55 : return 0xedc850;
        case   89 : return 0xedc53f;
        case  144 : return 0xedc22e;
        case  233 : return 0xedbc0c;
        case  377 : return 0xedb5ea;
        case  610 : return 0xedafc8;
        case  987 : return 0xeda9a6;
        case 1597 : return 0xeda384;
        case 2584 : return 0xedc22e;
        default   : return 0x3c3a32;
    }
}


int get_cell_ink_fibo (int val)
{
    return val <= 2 ? 0x776e65 : 0xf9f6f2;
}


int get_cell_bg (App *app, int val)
{
    switch (app->mode) {
        case G_POW2 : return get_cell_bg_pow2 (val);
        case G_FIBO : return get_cell_bg_fibo (val);
        default     : return 0xffffff;
    }
}


int get_cell_ink (App *app, int val)
{
    switch (app->mode) {
        case G_POW2 : return get_cell_ink_pow2 (val);
        case G_FIBO : return get_cell_ink_fibo (val);
        default     : return 0x000000;
    }
}


void draw_cell_xy (Ez_window win, App *app, int x, int y, int val)
{
    int x1 = x, y1 = y, x2 = x+CELL_XM, y2 = y+CELL_YM;

    if (val == 0) return;

    set_color_hexa (get_cell_bg (app, val));
    ez_fill_rectangle (win, x1+2, y1+2, x2-2, y2-2);

    ez_set_thick (BOARD_T);
    set_color_hexa (0xbbada0);
    ez_draw_rectangle (win, x1, y1, x2, y2);

    ez_set_nfont (3);
    set_color_hexa (get_cell_ink (app, val));
    ez_draw_text (win, EZ_MC, x+CELL_XM/2, y+CELL_YM/2, "%d", val);
}


void draw_cell_ij (Ez_window win, App *app, double i, double j, int val)
{
    draw_cell_xy (win, app, board_get_x (j), board_get_y (i), val);
}


void draw_cell_ab (Ez_window win, App *app, double a, double b, int val)
{
    switch (app->dir) {
        case D_LEFT  : draw_cell_ij (win, app, a, b          , val); break;
        case D_RIGHT : draw_cell_ij (win, app, a, BOARD_W-1-b, val); break;
        case D_UP    : draw_cell_ij (win, app, b          , a, val); break;
        case D_DOWN  : draw_cell_ij (win, app, BOARD_H-1-b, a, val); break;
    }
}


void draw_grid (Ez_window win, App *app)
{
    int i, j, x, y,
        x1 = board_get_x (0), x2 = board_get_x (BOARD_W),
        y1 = board_get_y (0), y2 = board_get_y (BOARD_H);

    set_color_hexa (get_cell_bg (app, 0));
    ez_fill_rectangle (win, x1+2, y1+2, x2-2, y2-2);

    ez_set_thick (BOARD_T);
    set_color_hexa (0xbbada0);
    ez_draw_rectangle (win, x1, y1, x2, y2);

    for (i = 1; i < BOARD_H; i++) {
        y = board_get_y (i);
        ez_draw_line (win, x1, y, x2, y);
    }
    for (j = 1; j < BOARD_W; j++) {
        x = board_get_x (j);
        ez_draw_line (win, x, y1, x, y2);
    }
}


void draw_board_current (Ez_window win, App *app)
{
    int i, j;

    for (i = 0; i < BOARD_H; i++)
    for (j = 0; j < BOARD_W; j++)
        draw_cell_ij (win, app, i, j, app->board[i][j]);
}


double interpol_step (int step, int max, int u, int v)
{
    return u + (v-u)*step/(double)max;
}


void draw_board_shifting (Ez_window win, App *app)
{
    int a, b, amax, bmax, k, cur, prev;
    double t;

    board_get_bounds (app, &amax, &bmax);

    for (a = 0; a < amax; a++)
    for (b = k = prev = 0; b < bmax; b++)
    {
        cur = board_get_val (app, a, b);
        if (cur == 0) continue;
        if (is_couple (app, cur, prev)) {
            t = interpol_step (app->anim_step, ANIM1_NB, b, k-1);
            draw_cell_ab (win, app, a, t, cur);
            prev = 0;
        } else {
            t = interpol_step (app->anim_step, ANIM1_NB, b, k);
            draw_cell_ab (win, app, a, t, cur);
            prev = cur;
            k++;
        }
    }
}


void draw_board (Ez_window win, App *app)
{
    draw_grid (win, app);
    if (app->state == S_ANIM)
         draw_board_shifting (win, app);
    else draw_board_current  (win, app);
}


void draw_sticked (Ez_window win)
{
    int x1 = board_get_x (0), x2 = board_get_x (BOARD_W),
        y1 = board_get_y (0), y2 = board_get_y (BOARD_H);

    ez_set_thick (BOARD_T);
    ez_set_color (ez_red);
    ez_draw_rectangle (win, x1, y1, x2, y2);
}


void draw_background (Ez_window win)
{
    int w, h;
    ez_window_get_size (win, &w, &h);
    set_color_hexa (0xfaf8ef);
    ez_fill_rectangle (win, 0, 0, w, h);
}


void draw_header (Ez_window win, App *app)
{
    int x = WIN1_W/2, y = 20;
    set_color_hexa (0x776e65); ez_set_nfont (2);
    switch (app->state) {
        case S_LOST : ez_draw_text (win, EZ_TC, x, y, "BAD LUCK!"); break;
        case S_WIN  : ez_draw_text (win, EZ_TC, x, y, "WELL DONE !!"); break;
        default :
            if (app->val_max >= app->target_val)
                 ez_draw_text (win, EZ_TC, x, y, "AWESOME !!"); 
            else ez_draw_text (win, EZ_TC, x, y, "Get %d!", app->target_val);
    }
    ez_set_color (ez_black);
    ez_draw_text (win, EZ_TL, 10, 10, "Score\n%d", app->score);
    ez_draw_text (win, EZ_TR, WIN1_W-10, 10, "Best\n%d", app->best_score);
}


void draw_footer (Ez_window win, App *app)
{
    char *s;

    switch (app->state) {
        case S_PLAY    : s = "Press an arrow"; break;
        case S_LOST    : s = "Press n to try again"; break;
        case S_WIN     : s = "Press n to start again"; break;
        case S_NOSHIFT : s = "Nothing to shift!"; break;
        default : s = "";
    }
    ez_set_color (ez_black); ez_set_nfont (0);
    ez_draw_text (win, EZ_BL, 10, WIN1_H-8, s);

    if (app->state == S_NOSHIFT || app->state == S_LOST) 
        draw_sticked (win);
}


/*---------------------- E V E N T S   O N   W I N   1 ----------------------*/


void win1_on_Expose (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    draw_background (ev->win);
    draw_header (ev->win, app);
    draw_board (ev->win, app);
    draw_footer (ev->win, app);
}


void play_dir (Gui *gui, int dir)
{
    App *app = gui->app;

    if (app->state != S_PLAY) return;
    app->dir = dir;
    if (!board_can_shift (app)) {
        app->state = S_NOSHIFT;
        return;
    }
    app->state = S_ANIM;
    app->anim_step = 0;
    ez_start_timer (gui->win1, DELAY1);
}


void win1_on_KeyPress (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    switch (ev->key_sym) {

        case XK_q : ez_quit (); break;
        case XK_n : change_mode (app); new_game (app); break;

        case XK_Left     :
        case XK_KP_Left  : play_dir (gui, D_LEFT ); break;
        case XK_Right    :
        case XK_KP_Right : play_dir (gui, D_RIGHT); break;
        case XK_Up       :
        case XK_KP_Up    : play_dir (gui, D_UP   ); break;
        case XK_Down     :
        case XK_KP_Down  : play_dir (gui, D_DOWN ); break;

        default : return;
    }

    ez_send_expose (ev->win);
}


void win1_on_KeyRelease (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    if (app->state != S_NOSHIFT) return;
    app->state = S_PLAY;

    ez_send_expose (ev->win);
}


void win1_on_TimerNotify (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    if (app->state != S_ANIM) return;

    app->anim_step++;
    if (app->anim_step < ANIM1_NB) 
        ez_start_timer (gui->win1, DELAY1);
    else {
        board_shift (app);
        insert_random_cell (app);
        if (count_empty_cells (app) == 0 && ! some_cells_can_merge (app))
             app->state = app->val_max >= app->target_val ? S_WIN : S_LOST;
        else app->state = S_PLAY;
    }

    ez_send_expose (ev->win);
}


void win1_on_event (Ez_event *ev)           /* Called for each event on win1 */
{
    switch (ev->type) {
        case Expose        : win1_on_Expose      (ev); break;
        case KeyPress      : win1_on_KeyPress    (ev); break;
        case KeyRelease    : win1_on_KeyRelease  (ev); break;
        case TimerNotify   : win1_on_TimerNotify (ev); break;
    }
}


/*------------------------- G E N E R A L   I N I T -------------------------*/


void app_init (App *app)
{
    set_mode (app, G_POW2);
    new_game (app);
    app->best_score = 0;
}


void gui_init (Gui *gui, App *app)
{
    gui->app = app;

    gui->win1 = ez_window_create (WIN1_W, WIN1_H, "Game 2048", win1_on_event);
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

