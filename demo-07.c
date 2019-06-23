/* demo-07.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 02/07/2008 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-07.c ez-draw.c -o demo-07 -lX11 -lXext
 * Compilation on Windows :
 *     gcc -Wall demo-07.c ez-draw.c -o demo-07.exe -lgdi32
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"

/* Global variables */
Ez_window win1, win2, win3 = None; int show2 = 0;


void win3_on_expose (Ez_event *ev)
{
    ez_draw_text (ev->win, EZ_TL, 10, 10,
        "If you close this window, it will be destroyed.");
}


/* The user has clicked on the Close button of the window */

void win3_on_window_close (Ez_event *ev)
{
    (void) ev;  /* Tell the compiler that ev is unused */
    ez_window_destroy (win3); win3 = None;
}


void win3_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose      : win3_on_expose       (ev); break;
        case WindowClose : win3_on_window_close (ev); break;
    }
}


void win2_on_expose (Ez_event *ev)
{
    ez_draw_text (ev->win, EZ_TL, 10, 10,
        "If you close this window, it will be hidden.");
}


void win2_on_window_close (Ez_event *ev)
{
    (void) ev;
    ez_window_show (win2, 0); show2 = 0;
}


void win2_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose      : win2_on_expose       (ev); break;
        case WindowClose : win2_on_window_close (ev); break;
    }
}


void win1_on_expose (Ez_event *ev)
{
    ez_draw_text (ev->win, EZ_TL, 10, 10,
        "Click in this window (to get the keyboard focus),\n"
        "then type :\n"
        "    - on 'm' to show or hide window 2;\n"
        "    - on 'c' to create or destroy window 3;\n"
        "    - on 'q' to quit.\n"
        "\n"
        "If you close this window, the program will end.");
}


void win1_on_key_press (Ez_event *ev)
{
    switch (ev->key_sym) {
        case XK_q : ez_quit (); break;

        case XK_m :
            show2 = !show2;                       /* show or hide the window */
            ez_window_show (win2, show2);
        break;

        case XK_c :
            if (win3 == None)      /* if the window doesn't exist, create it */
                win3 = ez_window_create (380, 220, "Window 3", win3_on_event);
            else { ez_window_destroy (win3); win3 = None; }
        break;
    }
}


void win1_on_window_close (Ez_event *ev)
{
    (void) ev;
    ez_quit ();
}


void win1_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose      : win1_on_expose       (ev); break;
        case KeyPress    : win1_on_key_press    (ev); break;
        case WindowClose : win1_on_window_close (ev); break;
    }
}


int main ()
{
    if (ez_init() < 0) exit(1);

    win1 = ez_window_create (400, 300, "Demo 07: Several windows", win1_on_event);
    win2 = ez_window_create (400, 200, "Window 2", win2_on_event);
    ez_window_show (win2, show2);

    /* By default, closing any window will cause the end of the program.
       We change this behaviour: for now on, closing any window will send 
       a WindowClose event for this window. */
    ez_auto_quit (0);

    ez_main_loop ();
    exit(0);
}

