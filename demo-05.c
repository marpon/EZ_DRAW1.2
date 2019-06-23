/* demo-05.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 02/07/2008 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-05.c ez-draw.c -o demo-05 -lX11 -lXext
 * Compilation on Windows :
 *     gcc -Wall demo-05.c ez-draw.c -o demo-05.exe -lgdi32
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


void win1_on_expose (Ez_event *ev)              /* We must redraw everything */
{
    ez_draw_text (ev->win, EZ_MC, 200, 150, 
        "Events are traced\nin the terminal.\n\n"
        "Type 'q' to quit.");
    printf ("Expose           win = 0x%x\n", ez_window_get_id(ev->win));
}


void win1_on_button_press (Ez_event *ev)             /* Mouse button pressed */
{
    printf ("ButtonPress      win = 0x%x  mx = %d  my = %d  mb = %d\n",
        ez_window_get_id(ev->win), ev->mx, ev->my, ev->mb);
}


void win1_on_button_release (Ez_event *ev)          /* Mouse button released */
{
    printf ("ButtonRelease    win = 0x%x  mx = %d  my = %d  mb = %d\n",
        ez_window_get_id(ev->win), ev->mx, ev->my, ev->mb);
}


void win1_on_motion_notify (Ez_event *ev)                     /* Mouse moved */
{
    printf ("MotionNotify     win = 0x%x  mx = %d  my = %d  mb = %d\n",
        ez_window_get_id(ev->win), ev->mx, ev->my, ev->mb);
}


void win1_on_key_press (Ez_event *ev)                         /* Key pressed */
{
    printf ("KeyPress         win = 0x%x  mx = %d  my = %d  "
            "key_sym = 0x%x  key_name = %s  key_count = %d  key_string = \"%s\"\n",
        ez_window_get_id(ev->win), ev->mx, ev->my,
        (int) ev->key_sym, ev->key_name, ev->key_count,
        ev->key_sym == XK_Return || ev->key_sym == XK_KP_Enter ? "" : ev->key_string);
}


void win1_on_key_release (Ez_event *ev)                      /* Key released */
{
    printf ("KeyRelease       win = 0x%x  mx = %d  my = %d  "
            "key_sym = 0x%x  key_name = %s  key_count = %d  key_string = \"%s\"\n",
        ez_window_get_id(ev->win), ev->mx, ev->my,
        (int) ev->key_sym, ev->key_name, ev->key_count,
        ev->key_sym == XK_Return || ev->key_sym == XK_KP_Enter ? "" : ev->key_string);
     switch (ev->key_sym) {
        case XK_q : ez_quit (); break;
    }
}


void win1_on_configure_notify (Ez_event *ev)          /* Window size changed */
{
    printf ("ConfigureNotify  win = 0x%x  width = %d  height = %d\n",
        ez_window_get_id(ev->win), ev->width, ev->height);
}


void win1_on_window_close (Ez_event *ev)             /* Close button pressed */
{
    printf ("WindowClose      win = 0x%x\n", ez_window_get_id(ev->win));
}


void win1_on_event (Ez_event *ev)                /* Called by ez_main_loop() */
{                                                /* for each event on win1   */
    switch (ev->type) {
        case Expose          : win1_on_expose           (ev); break;
        case ButtonPress     : win1_on_button_press     (ev); break;
        case ButtonRelease   : win1_on_button_release   (ev); break;
        case MotionNotify    : win1_on_motion_notify    (ev); break;
        case KeyPress        : win1_on_key_press        (ev); break;
        case KeyRelease      : win1_on_key_release      (ev); break;
        case ConfigureNotify : win1_on_configure_notify (ev); break;
        case WindowClose     : win1_on_window_close     (ev); break;
        default :
             printf ("Unknown event: %d\n", ev->type);
   }
}


int main ()
{
    if (ez_init() < 0) exit(1);

    ez_window_create (400, 300, "Demo 05: Tracing events", win1_on_event);

    ez_auto_quit (0);  /* to get WindowClose event */

    ez_main_loop ();
    exit(0);
}

