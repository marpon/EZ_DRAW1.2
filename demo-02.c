/* demo-02.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 02/07/2008 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-02.c ez-draw.c -o demo-02 -lX11 -lXext
 * Compilation on Windows :
 *     gcc -Wall demo-02.c ez-draw.c -o demo-02.exe -lgdi32
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


void win1_on_event (Ez_event *ev)                /* Called by ez_main_loop() */
{                                                /* for each event on win1   */
    switch (ev->type) {

        case Expose :                           /* We must redraw everything */
            ez_set_color (ez_red);
            ez_draw_text (ev->win, EZ_MC, 200, 150, 
                "To quit, press the key 'q', or click\n"
                "on the Close button of the window");
            break;

        case KeyPress :                                 /* A key was pressed */
            switch (ev->key_sym) {
                case XK_q : ez_quit (); break;
            }
            break;
    }
}


int main ()
{
    if (ez_init() < 0) exit(1);

    ez_window_create (400, 300, "Demo 02: Window and events", win1_on_event);

    ez_main_loop ();
    exit(0);
}

