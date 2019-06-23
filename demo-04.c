/* demo-04.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 02/07/2008 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-04.c ez-draw.c -o demo-04 -lX11 -lXext
 * Compilation on Windows :
 *     gcc -Wall demo-04.c ez-draw.c -o demo-04.exe -lgdi32
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


void win1_on_expose (Ez_event *ev)
{
    int i, w, h;

    ez_window_get_size (ev->win, &w, &h);

    ez_set_color (ez_black);
    for (i = 0; i <= 3; i++) {
        ez_set_nfont (i);
        ez_draw_text (ev->win, EZ_TC, w/2, h/2 + 25*(i-2), 
            "Font number %d", i);                           /* like a printf */
    }

    ez_set_nfont (0);
    ez_set_color (ez_red);

    ez_draw_text (ev->win, EZ_TL,   2,   1, "Top\nLeft");
    ez_draw_text (ev->win, EZ_TC, w/2,   1, "Top\nCenter");
    ez_draw_text (ev->win, EZ_TR, w-2,   1, "Top\nRight");
    ez_draw_text (ev->win, EZ_ML,   2, h/2, "Middle\nLeft");
    ez_draw_text (ev->win, EZ_MR, w-2, h/2, "Middle\nRight");
    ez_draw_text (ev->win, EZ_BL,   2, h-2, "Bottom\nLeft");
    ez_draw_text (ev->win, EZ_BC, w/2, h-2, "Bottom\nCenter");
    ez_draw_text (ev->win, EZ_BR, w-2, h-2, "Bottom\nRight");
}


void win1_on_key_press (Ez_event *ev)
{
    switch (ev->key_sym) {
        case XK_q : ez_quit (); break;
    }

}


void win1_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose   : win1_on_expose    (ev); break;
        case KeyPress : win1_on_key_press (ev); break;
    }
}


int main ()
{
    if (ez_init() < 0) exit(1);

    ez_window_create (400, 300, "Demo 04: Displaying text", win1_on_event);

    ez_main_loop ();
    exit(0);
}

