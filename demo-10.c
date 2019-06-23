/* demo-10.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 24/03/2009 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-10.c ez-draw.c -o demo-10 -lX11 -lXext -lm
 * Compilation on Windows :
 *     gcc -Wall demo-10.c ez-draw.c -o demo-10.exe -lgdi32 -lm
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


/* Window 1: we turn the hands of a watch */

typedef struct {
    int count, state, win_w, win_h, delay;
    Ez_window win;
} Clock_data;


void clock_data_init (Clock_data *a)
{
    a->count = 0;
    a->state = 1;
    a->win_w = 250; a->win_h = 250;
    a->delay = 30;
}


void draw_hands (Ez_window win, Ez_uint32 color, double angle,
    int xc, int yc, int d1, int d2, int ra)
{
    double a = (angle-90) * M_PI / 180, c = cos (a), s = sin (a),
           x1 = xc + c*d1, y1 = yc + s*d1, x2 = xc + c*d2, y2 = yc + s*d2,
           x3 = x1 - s*ra, y3 = y1 + c*ra, x4 = x1 + s*ra, y4 = y1 - c*ra,
           x5 = x2 - s*ra, y5 = y2 + c*ra, x6 = x2 + s*ra, y6 = y2 - c*ra;

    ez_set_color (color);
    ez_draw_line (win, x3, y3, x5, y5); ez_draw_line (win, x5, y5, x6, y6);
    ez_draw_line (win, x6, y6, x4, y4); ez_draw_line (win, x4, y4, x3, y3);
}


void win1_on_expose (Ez_event *ev)
{
    Clock_data *a = ez_get_data (ev->win);
    double h = a->count / 60.0, m = a->count % 60;

    ez_set_color (ez_magenta); ez_set_nfont (2);
    ez_draw_text (a->win, EZ_TC, a->win_w/2, 6, "%02d:%02d", (int) h, (int) m);

    draw_hands (a->win, ez_red, h*30, a->win_w/2, a->win_h/2, 20, 70, 10);
    draw_hands (a->win, ez_blue, m*6, a->win_w/2, a->win_h/2, 15, 92, 6);

    ez_set_color (ez_black); ez_set_nfont (0);
    ez_draw_text (a->win, EZ_BL, 8, a->win_h-8, "Space: pause   q: quit");
}


void win1_on_key_press (Ez_event *ev)
{
    Clock_data *a = ez_get_data (ev->win);

    switch (ev->key_sym) {
        case XK_q : ez_quit (); break;
        case XK_space : 
            a->state = !a->state;
            ez_start_timer (a->win, a->state ? a->delay : -1);
            break;
    }
}


void win1_on_timer_notify (Ez_event *ev)
{
    Clock_data *a = ez_get_data (ev->win);

    a->count = (a->count + 1) % 720;
    ez_send_expose (a->win);
    ez_start_timer (a->win, a->delay);
}


void win1_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose      : win1_on_expose       (ev); break;
        case KeyPress    : win1_on_key_press    (ev); break;
        case TimerNotify : win1_on_timer_notify (ev); break;
    }
}


/* Window 2: a ball is bouncing on a racket */

typedef struct {
    int ba_x, ba_y, ba_dx, ba_dy, win_w, win_h, delay;
    Ez_window win;
} Ball_data;


void ball_data_init (Ball_data *a)
{
    a->ba_x  = 100; a->ba_y  = 100;
    a->ba_dx =   4; a->ba_dy =   3;
    a->win_w = 280; a->win_h = 180;
    a->delay = 20;
}


void progress_ball (Ball_data *a, int x1, int y1, int x2, int y2)
{
    a->ba_x += a->ba_dx; a->ba_y += a->ba_dy;
    if (a->ba_x <= x1) { a->ba_x = x1; a->ba_dx = -a->ba_dx; }
    else if (a->ba_x >= x2) { a->ba_x = x2; a->ba_dx = -a->ba_dx; } 
    if (a->ba_y <= y1) { a->ba_y = y1; a->ba_dy = -a->ba_dy; }
    else if (a->ba_y >= y2) { a->ba_y = y2; a->ba_dy = -a->ba_dy; } 
}


void win2_on_expose (Ez_event *ev)
{
    Ball_data *a = ez_get_data (ev->win);

    ez_set_color (ez_black);
    ez_draw_circle (a->win, a->ba_x-10, a->ba_y-10, a->ba_x+10, a->ba_y+10);

    ez_set_color (ez_blue);
    ez_fill_rectangle (a->win, a->ba_x-25, a->win_h-8, a->ba_x+25, a->win_h-3);
}


void win2_on_timer_notify (Ez_event *ev)
{
    Ball_data *a = ez_get_data (ev->win);

    progress_ball (a, 10, 10, a->win_w-10, a->win_h-19);
    ez_send_expose (a->win);
    ez_start_timer (a->win, a->delay);
}


void win2_on_configure_notify (Ez_event *ev)
{
    Ball_data *a = ez_get_data (ev->win);

    a->win_w = ev->width; a->win_h = ev->height;
}


void win2_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose          : win2_on_expose           (ev); break;
        case TimerNotify     : win2_on_timer_notify     (ev); break;
        case ConfigureNotify : win2_on_configure_notify (ev); break;
    }
}


int main ()
{
    Clock_data clo;
    Ball_data  bal;

    if (ez_init() < 0) exit(1);

    clock_data_init (&clo);
    clo.win = ez_window_create (clo.win_w, clo.win_h, "Demo 10: Clock", 
        win1_on_event);
    ez_set_data (clo.win, &clo);
    ez_window_dbuf (clo.win, 1);

    ball_data_init (&bal);
    bal.win = ez_window_create (bal.win_w, bal.win_h, "Demo 10: Rebounds", 
        win2_on_event);
    ez_set_data (bal.win, &bal);
    ez_window_dbuf (bal.win, 1);

    /* Start timers to get TimerNotify events in delay milliseconds */
    ez_start_timer (clo.win, clo.delay);
    ez_start_timer (bal.win, bal.delay);

    ez_main_loop ();
    exit(0);
}

