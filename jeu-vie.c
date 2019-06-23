/* jeu-vie.c: John Conway's game of life
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 30/03/2014 - version 1.2
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


/*------------------------- D E F I N I T I O N S ---------------------------*/

/* Board size */
#define BOARD_XM 100
#define BOARD_YM 80

/* Size in pixels */
#define W_X0  3
#define W_Y0  30
#define W_TA  7

/* Size of windows */
#define WIN1_W (W_X0+W_TA*BOARD_XM+W_X0)
#define WIN1_H (W_Y0+W_TA*BOARD_YM+20)

/* Delay for animation */
#define DELAY1 200

/* State of a cell */
enum { C_EMPTY, C_DEAD, C_ALIVE, C_BORN };

typedef struct {
    char *tab;
    int xmax, ymax,
        lastx, lasty,
        anim, delay, generation;
} App;

typedef struct {
    App *app;
    Ez_window win1;
} Gui;


/*-------------------------------- B O A R D --------------------------------*/


int board_create (App *app, int xmax, int ymax)
{
    app->xmax = xmax;
    app->ymax = ymax;
    app->tab = malloc (xmax*ymax*sizeof(char));
    if (app->tab == NULL) { perror ("board_create"); return -1; }
    return 0;
}


void board_destroy (App *app)
{
    free (app->tab); app->tab = NULL;
}


void board_clear (App *app)
{
    int t;
    for (t = app->xmax*app->ymax-1; t >= 0; t--) app->tab[t] = 0;
    app->anim = app->generation = 0;
}


void board_example1 (App *app)
{
    /* Gosper's Glider Gun */
    int x[] = {24, 22, 24, 12, 13, 20, 21, 34, 35, 11, 15, 20, 21, 34, 35,
               0, 1, 10, 16, 20, 21, 0, 1, 10, 14, 16, 17, 22, 24, 10, 16,
               24, 11, 15, 12, 13},
        y[] = {0, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 
               4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 7, 7, 8, 8 }, 
        i, n = sizeof(x)/sizeof(int);

    for (i = 0; i < n; i++)
        app->tab[(y[i]+10)*app->xmax+x[i]+10] = C_ALIVE;
}


void board_step (App *app)
{
    int dx[] = {1, 1, 0, -1}, dy[] = {0, 1, 1, 1}, x, y, t, i, n, a, b;

    /* We scan sequentially all points of the board except on the border */
    for (y = 1; y < app->ymax-1; y++)
    for (x = 1; x < app->xmax-1; x++)
    {
        n = 0;  /* Number of neighbors */

        /* This neighbor is already processed: if it just died or survived, 
           then the cell was already occupied. */
        for (i = 0; i < 4; i++) {
            a = app->tab[(y-dy[i])*app->xmax + (x-dx[i])];
            if (a == C_DEAD || a == C_ALIVE) n++;
        }

        /* This neighbor is not already processed: if it survived or was 
           just born, then the cell is occupied. */
        for (i = 0; i < 4; i++) {
            a = app->tab[(y+dy[i])*app->xmax + (x+dx[i])];
            if (a == C_ALIVE || a == C_BORN) n++;
        }

        t = y*app->xmax+x; b = app->tab[t];
        if (b == C_EMPTY || b == C_DEAD) {
            /* The cell is empty or was just cleared. A birth will happen 
               if there are exactly 3 neighbors. */
            app->tab[t] = (n == 3) ? C_BORN : C_EMPTY;
        } else {  /* b == C_ALIVE || b == C_BORN */
            /* The cell is occupied (survivor or birth).
               It will survive if there are 2 or 3 neighbors. */
            app->tab[t] = (n == 2 || n == 3) ? C_ALIVE : C_DEAD;
       }
    }
    app->generation++;
}


void board_modify (Ez_window win, App *app, int mx, int my, int click, int button)
{
    int x = (mx - W_X0) / W_TA,
        y = (my - W_Y0) / W_TA, a, t;

    if (app->anim) { ez_start_timer (win, -1); app->anim = 0; }
    if (x < 1 || x >= app->xmax-1 || y < 1 || y >= app->ymax-1) return;

    if (!click && app->lastx == x && app->lasty == y) return;
    app->lastx = x; app->lasty = y;

    t = y * app->xmax + x; a = app->tab[t];
    if (click && button == 1)
         app->tab[t] = (a == C_DEAD || a == C_EMPTY) ? C_ALIVE : C_EMPTY;
    else app->tab[t] = (button == 1) ? C_ALIVE : C_EMPTY;
    ez_send_expose (win);
}


/*------------------------------ D R A W I N G S ----------------------------*/


void draw_cell_legend (Ez_window win, int x, int y, Ez_uint32 color, char *s)
{
    ez_set_color (color);
    ez_fill_rectangle (win, x, y, x+W_TA-2, y+W_TA-2);
    ez_set_color (ez_black);
    ez_draw_text (win, EZ_BL, x+W_TA+5, y+W_TA+1, "%s", s);
}


void draw_header (Ez_window win, App *app)
{
    draw_cell_legend (win,  10, 15, ez_green, "Birth if 3 neighbors");
    draw_cell_legend (win, 155, 15, ez_blue , "Survival if 2 or 3 neighbors");

    ez_set_color (ez_black);
    ez_draw_text (win, EZ_TL, 400, 10, 
        "Generation %-7d  Delay %-4d ms   Animation %s", 
        app->generation, app->delay, app->anim ? "ON " : "OFF");
}


void draw_board (Ez_window win, App *app)
{
    int x, y, a;

    ez_set_color (ez_grey);
    ez_draw_rectangle (win,
        W_X0+              W_TA-2, W_Y0+              W_TA-2, 
        W_X0+(app->xmax-1)*W_TA  , W_Y0+(app->ymax-1)*W_TA  );

    for (y = 0; y < app->ymax; y++)
    for (x = 0; x < app->xmax; x++)
    {
        a = app->tab[y*app->xmax+x];
        if (a == C_EMPTY || a == C_DEAD) continue;
        switch (a) {
            case C_BORN  : ez_set_color (ez_green); break;
            case C_ALIVE : ez_set_color (ez_blue ); break;
        }
        ez_fill_rectangle (win, W_X0+ x   *W_TA,   W_Y0+ y   *W_TA, 
                                W_X0+(x+1)*W_TA-2, W_Y0+(y+1)*W_TA-2);
    }
}


void draw_footer (Ez_window win)
{
    ez_set_color (ez_black);
    ez_draw_text (win, EZ_BL, 10, WIN1_H-5, 
        "Space: animate   d,D: delay   c: clear   q: quit    "
        "mouse: button 1 draw/erase, button 3 erase");
}


/*------------------------------- E V E N T S -------------------------------*/


void win1_on_Expose (Ez_event *ev, Gui *gui)
{
    App *app = gui->app;

    draw_header (ev->win, app);
    draw_board  (ev->win, app);
    draw_footer (ev->win);
}


void win1_on_MotionNotify (Ez_event *ev, Gui *gui)
{
    if (ev->mb >= 1)
        board_modify (ev->win, gui->app, ev->mx, ev->my, 0, ev->mb);
}


void win1_on_ButtonPress (Ez_event *ev, Gui *gui)
{
    board_modify (ev->win, gui->app, ev->mx, ev->my, 1, ev->mb);
}


void win1_on_KeyPress (Ez_event *ev, Gui *gui)
{
    App *app = gui->app;

    switch (ev->key_sym) {
        case XK_q : 
            ez_quit (); 
            break;
        case XK_space :
            app->anim = !app->anim;
            ez_start_timer (ev->win, app->anim ? 0 : -1);
            ez_send_expose (ev->win);
            break;
        case XK_c :
            if (app->anim) ez_start_timer (ev->win, -1);
            board_clear (app);
            ez_send_expose (ev->win);
            break;
        case XK_e :
            if (app->anim) ez_start_timer (ev->win, -1);
            board_clear (app);
            board_example1 (app);
            ez_send_expose (ev->win);
            break;
        case XK_d : 
            app->delay /= 1.1;
            if (app->delay < 0) app->delay = 0;
            ez_send_expose (ev->win);
            break;
        case XK_D : 
            app->delay = app->delay*1.1 + 1;
            if (app->delay > 2000) app->delay = 2000;
            ez_send_expose (ev->win);
            break;
    }
}


void win1_on_TimerNotify (Ez_event *ev, Gui *gui)
{
    App *app = gui->app;
    board_step (app);
    ez_send_expose (ev->win);
    ez_start_timer (ev->win, app->delay);
}


void win1_on_event (Ez_event *ev)           /* Called for each event on win1 */
{
    Gui *gui = ez_get_data (ev->win);

    switch (ev->type) {
        case Expose        : win1_on_Expose        (ev, gui); break;
        case MotionNotify  : win1_on_MotionNotify  (ev, gui); break;
        case ButtonPress   : win1_on_ButtonPress   (ev, gui); break;
        case KeyPress      : win1_on_KeyPress      (ev, gui); break;
        case TimerNotify   : win1_on_TimerNotify   (ev, gui); break;
    }
}


/*-------------------------- G E N E R A L   I N I T ------------------------*/


void app_init (App *app)
{
    board_create (app, BOARD_XM, BOARD_YM);
    board_clear (app);
    board_example1 (app);
    app->delay = DELAY1;
}


void gui_init (Gui *gui, App *app)
{
    gui->app = app;

    gui->win1 = ez_window_create (WIN1_W, WIN1_H, "Game of life", win1_on_event);
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


