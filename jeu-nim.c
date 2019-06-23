/* jeu-nim.c: the game of NIM
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 27/03/2014 - version 1.2
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


/*------------------------- D E F I N I T I O N S ---------------------------*/


/* Min and max lines, max colomns */
#define MIN_I    3
#define MAX_I    5
#define MAX_J   15

/* Position of the last pencil */
#define PEN_X0   50
#define PEN_Y0   60

/* Width, lead height, full height of a pencil */
#define PEN_L    10
#define PEN_P    14
#define PEN_H    50

/* Gap between two pencils */
#define PEN_DX   16
#define PEN_DY   10

/* Window size */
#define WIN1_L   (PEN_X0+(PEN_L+PEN_DX)*MAX_J+PEN_DX+20)
#define WIN1_H   (PEN_Y0+(PEN_H+PEN_DY)*MAX_I+30)


typedef struct {
    int pens[MAX_I];    /* array of pencils: nb per line */
    int nli;            /* line number for this game     */
    int cur_i, cur_j;   /* current position              */

    enum { S_BEG, S_H1, S_H2, S_O1, S_O2, S_HG, S_OG } state;
} App;

typedef struct {
    App *app;
    Ez_window win1;
    int win1_use_dbuf;
} Gui;


/*--------------------------------- G A M E ---------------------------------*/


void pens_init (App *app)
{
    int i;

    app->nli = MIN_I + ez_random (MAX_I-MIN_I+1);

    for (i = 0; i < app->nli; i++)
        app->pens[i] = 1 + ez_random (MAX_J);
}


int pens_delete (App *app, int i, int j)
{
    if (i < 0 || i >= app->nli || 
        j < 0 || j >= app->pens[i]) return -1;
    app->pens[i] = j;
    return 0;
}


/* Strategy - references :
 *
 *  [1] Delahaye JP, Strategies magiques au pays de Nim.
 *      Pour La Science, mars 2009, n. 307, p 88-93.
 *
 *  [2] http://en.wikipedia.org/wiki/Nim
*/

int game_is_over (App *app)
{
    int i, k = 0;
    for (i = 0; i < app->nli; i++)
        k += app->pens[i];
    return k > 0 ? 0 : 1;
}


int nim_addition (int a, int b)
{
    int i, j, k = 0;

    for (i = 0, j = 1;  a >= j || b >= j; i++, j *= 2)
        k += (a & j) ^ (b & j);

    return k;
}


int nim_sum (App *app)
{
    int i, k = 0;

    for (i = 0; i < app->nli; i++)
        k = nim_addition (k, app->pens[i]);

    return k;
}


void compute_next_shot (App *app)
{
    int i, j = 0, k, ns;

    ns = nim_sum (app);
    /* printf ("\n(+) total = %d\n", ns); */

    if (ns != 0) {  /* Winning configuration; the computer can't loose */

        for (i = 0; i < app->nli; i++) {
            j = nim_addition (app->pens[i], ns);
            /* printf ("%2d (+) %2d = %2d : %s\n", pens[i], ns, j, 
                j <= pens[i] ? "descending ok" : "ascending"); */
            if (j <= app->pens[i]) break; 
        }

    } else {  /* We suppress a pencil and wait a misstep of the player */

        for (i = 0, k = 1; k < app->nli; k++)
            if (app->pens[k] > app->pens[i]) i = k;
        j = app->pens[i] - 1;
    }

    app->cur_i = i; app->cur_j = j;
}


/*------------------------------ D R A W I N G S ----------------------------*/


/* Coordinates pixels x,y --> coordinates grid j,i */

int pen_jtox (int j)
{
    return PEN_X0 + (PEN_L + PEN_DX) * j;
}

int pen_itoy (int i)
{
    return PEN_Y0 + (PEN_H + PEN_DY) * i;
}

/* Coordinates grid j,i --> coordinates pixels x,y */

int pen_xtoj (int x)
{
    int k = x - PEN_X0 + PEN_DX/2;
    if (k < 0) return -1;
    k = k / (PEN_L + PEN_DX);
    if (k >= MAX_J) return -1;
    return k;
}

int pen_ytoi (int y)
{
    int k = y - PEN_Y0 + PEN_DY/2;
    if (k < 0) return -1;
    k = k / (PEN_H + PEN_DY);
    if (k >= MAX_I) return -1;
    return k;
}


void find_pencil (App *app, int mx, int my)
{
    app->cur_j = pen_xtoj (mx);
    app->cur_i = pen_ytoi (my);
    if (app->cur_i >= app->nli) app->cur_i = -1;
}


void draw_pencil_xy (Ez_window win, int x, int y, Ez_uint32 color)
{
    ez_set_color (color);
    ez_fill_rectangle (win, x, y+PEN_P, x+PEN_L, y+PEN_H);
    ez_set_color (ez_grey);
    ez_draw_rectangle (win, x, y+PEN_P, x+PEN_L, y+PEN_H);
    ez_set_color (ez_black);
    ez_draw_line (win, x+PEN_L/2, y, x, y+PEN_P);
    ez_draw_line (win, x+PEN_L/2, y, x+PEN_L, y+PEN_P);
    ez_set_color (ez_red);
    ez_draw_line (win, x+PEN_L/2, y, x+PEN_L/2, y+PEN_P/2);
}


void draw_pencil_ij (Ez_window win, int i, int j, Ez_uint32 color)
{
    draw_pencil_xy (win, pen_jtox (j), pen_itoy (i), color);
}


void draw_selection (Ez_window win, App *app, int color)
{
    int i = app->cur_i,
        j = app->cur_j,
        xa = pen_jtox (j) - PEN_DX/2,
        ya = pen_itoy (i) - PEN_DY/2,
        xb = WIN1_L - 5,
        yb = pen_itoy (i) + PEN_H + PEN_DY/2,
        xc = WIN1_L - 10,
        yc = pen_itoy (i) + PEN_H/2;

    ez_set_color (color); ez_set_nfont (2);
    ez_draw_rectangle (win, xa, ya, xb, yb);

    if (i >= 0 && i < app->nli) {
        if (j < 0) 
            j = 0; 
        else if (j > app->pens[i]) 
            j = app->pens[i];
        ez_draw_text (win, EZ_MR, xc, yc, "%2d", j);
    }
}


/*-------------------- E V E N T S   F O R   W I N   1 ----------------------*/


void win1_on_Expose (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;
    int i, j;
    Ez_uint32 c;
    char *s;

    switch (app->state) {
        case S_BEG : s = "\n** Game of Nim **"; break;
        case S_H1  : s = "Your turn to play: take pencils"; break;
        case S_H2  : s = ""; break;
        case S_O1  : s = "The computer plays ..."; break;
        case S_O2  : s = ""; break;
        case S_HG  : s = "Well done, you win !!"; break;
        case S_OG  : s = "Sorry, you lost !!"; break;
        default : s = "";
    }
    ez_set_color (ez_blue); ez_set_nfont (2);
    ez_draw_text (ev->win, EZ_TC, WIN1_L/2, 10, s);

    if (app->state == S_BEG) {

        ez_set_color (ez_magenta); ez_set_nfont (1);
        ez_draw_text (ev->win, EZ_TC, WIN1_L/2, WIN1_H*2/7,
            "You play against the computer.\n\n"
            "Each plays alternately, and take in\n"
            "a line some pencils.\n\n"
            "The one which takes the last pencil wins.\n\n"
            "Press 'n' to start ...");
        ez_set_color (ez_red); ez_set_nfont (0);
        ez_draw_text (ev->win, EZ_TC, WIN1_L/2, WIN1_H*4/5,
            "The game jeu-nim.c is part of EZ-Draw:\n"
            "http://pageperso.lif.univ-mrs.fr/~edouard.thiel/ez-draw");

    } else if (app->state == S_H1 || app->state == S_H2 || 
               app->state == S_O1 || app->state == S_O2) {

        ez_set_color (ez_grey);
        for (i = 0; i < app->nli; i++)
            ez_draw_text (ev->win, EZ_ML, 10, pen_itoy (i) + PEN_H/2,
                "%2d", app->pens[i]);

        for (i = 0; i < app->nli; i++)
        for (j = 0; j < app->pens[i]; j++)
        {
            c = ez_yellow;
            if (i == app->cur_i && j >= app->cur_j && app->cur_j > -1) 
                c = ez_white;
            draw_pencil_ij (ev->win, i, j, c);
        }
    }

    if (app->cur_i != -1 && app->cur_j != -1) {
        if (app->state == S_H1 || app->state == S_H2)
            draw_selection (ev->win, app, ez_green);
        else if (app->state == S_O1 || app->state == S_O2)
            draw_selection (ev->win, app, ez_blue);
    }

    ez_set_color (ez_black); ez_set_nfont (0);
    ez_draw_text (ev->win, EZ_BL, 10, WIN1_H-8, 
        "r: rules   n: new game   d: double buffer (is %s)   q: quit",
        gui->win1_use_dbuf ? "ON" : "OFF");
}


void win1_on_MotionNotify (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    if (app->state == S_H1) {
        find_pencil (app, ev->mx, ev->my);
        ez_send_expose (ev->win);
    }
}


void win1_on_ButtonPress (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    if (app->state == S_H1) {
        if (pens_delete (app, app->cur_i, app->cur_j) == 0) {
            app->state = S_H2;
            ez_send_expose (ev->win);
        }
    }
}


void win1_on_ButtonRelease (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    if (app->state == S_H2) {
        if (game_is_over (app)) app->state = S_HG;
        else { app->state = S_O1; compute_next_shot (app); }
        ez_send_expose (ev->win);
        ez_start_timer (ev->win, 500);
    }
}


void win1_on_KeyPress (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    switch (ev->key_sym) {
        case XK_q : 
            ez_quit (); 
            break;
        case XK_d :
            /* Enable or disable double buffer */
            gui->win1_use_dbuf = ! gui->win1_use_dbuf;
            ez_window_dbuf (ev->win, gui->win1_use_dbuf);
            ez_send_expose (ev->win); 
            break;
        case XK_r : 
            app->state = S_BEG;
            ez_send_expose (ev->win); 
            ez_start_timer (ev->win, -1);
            break;
        case XK_n : 
            app->state = S_H1;
            pens_init (app); 
            find_pencil (app, ev->mx, ev->my);
            ez_send_expose (ev->win); 
            ez_start_timer (ev->win, -1);
            break;
    }
}


void win1_on_TimerNotify (Ez_event *ev)
{
    Gui *gui = ez_get_data (ev->win);
    App *app = gui->app;

    if (app->state == S_O1) {
        app->state = S_O2;
        pens_delete (app, app->cur_i, app->cur_j);
        ez_send_expose (ev->win); 
        ez_start_timer (ev->win, 300);
    } else if (app->state == S_O2) { 
        if (game_is_over (app)) app->state = S_OG;
        else { app->state = S_H1; find_pencil (app, ev->mx, ev->my); }
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
        case TimerNotify   : win1_on_TimerNotify (ev); break;
    }
}


/*------------------------ G E N E R A L   I N I T --------------------------*/


void app_init (App *app)
{
    pens_init (app);
    app->state = S_BEG;
    app->cur_i = app->cur_j = -1;
}


void gui_init (Gui *gui, App *app)
{
    gui->app = app;

    gui->win1 = ez_window_create (WIN1_L, WIN1_H, "Game of NIM", win1_on_event);
    gui->win1_use_dbuf = 1;
    ez_window_dbuf (gui->win1, gui->win1_use_dbuf);
    ez_set_data (gui->win1, gui);
}


/*-------------------------------- M A I N ----------------------------------*/


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


