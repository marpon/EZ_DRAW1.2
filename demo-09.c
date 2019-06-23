/* demo-09.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 21/06/2011 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-09.c ez-draw.c -o demo-09 -lX11 -lXext
 * Compilation on Windows :
 *     gcc -Wall demo-09.c ez-draw.c -o demo-09.exe -lgdi32
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"

#define MAX_CPT1 100

/* We can avoid global variables by using ez_set_data(), see demo-10.c
   and the next examples */
int count1 = 0, win1_w = 300, win1_h = 200, delay1 = 30;


void win1_on_expose (Ez_event *ev)
{
    /* We draw based on count1 */
    int xc = win1_w/2, rx = xc * count1 / MAX_CPT1,
        yc = win1_h/2, ry = yc * count1 / MAX_CPT1;

    ez_set_color (ez_magenta); 
    ez_set_thick (3);
    ez_draw_circle (ev->win, xc-rx, yc-ry, xc+rx, yc+ry);

    ez_set_color (ez_black); ez_set_nfont (0);
    ez_draw_text (ev->win, EZ_BL, 8, win1_h-8, "q: quit");
}


void win1_on_key_press (Ez_event *ev)
{
    switch (ev->key_sym) {
        case XK_q : ez_quit (); break;
    }
}


void win1_on_configure_notify (Ez_event *ev)
{
    win1_w = ev->width; win1_h = ev->height;
}


void win1_on_timer_notify (Ez_event *ev)            /* The timer has expired */
{
    /* We increment the counter count1 so as to modify the position of the
       object to animate */
    count1 = (count1 + 1) % MAX_CPT1;
    /* We send an Expose event so that the window will be displayed again */
    ez_send_expose (ev->win);
    /* We restart the timer to maintain a "loop" or TimerNotify events,
       which is iterated each delay1 milliseconds */
    ez_start_timer (ev->win, delay1);
}


void win1_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose          : win1_on_expose           (ev); break;
        case KeyPress        : win1_on_key_press        (ev); break;
        case ConfigureNotify : win1_on_configure_notify (ev); break;
        case TimerNotify     : win1_on_timer_notify     (ev); break;
    }
}


int main ()
{
    Ez_window win1;

    if (ez_init() < 0) exit(1);

    win1 = ez_window_create (win1_w, win1_h, "Demo 09: Hypnosis", win1_on_event);

    /* Enable double buffer to prevent window flashes */
    ez_window_dbuf (win1, 1);

    /* Start a timer to get a TimerNotify event in delay1 milliseconds:
       this is the starting point of the "loop" of TimerNotify events. */
    ez_start_timer (win1, delay1);

    ez_main_loop ();
    exit(0);
}

