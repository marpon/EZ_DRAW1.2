/* demo-03.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 10/04/2008 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-03.c ez-draw.c -o demo-03 -lX11 -lXext
 * Compilation on Windows :
 *     gcc -Wall demo-03.c ez-draw.c -o demo-03.exe -lgdi32
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


void win1_on_expose (Ez_event *ev)
{
    ez_set_color (ez_magenta);
    ez_draw_text      (ev->win, EZ_BL, 10, 20, "Thickness 1");
    ez_set_thick (1);
    ez_draw_point     (ev->win,  30,  50);
    ez_draw_line      (ev->win,  60,  35, 130,  70);
    ez_draw_rectangle (ev->win, 160,  30, 220,  70);
    ez_draw_circle    (ev->win, 240,  30, 300,  70);
    ez_draw_triangle  (ev->win, 320,  30, 380,  40, 350,  70);

    ez_set_color (ez_black);
    ez_draw_text      (ev->win, EZ_BL, 10, 100, "Thickness 2");
    ez_set_color (ez_cyan);
    ez_set_thick (2);
    ez_draw_point     (ev->win,  30, 130);
    ez_draw_line      (ev->win,  60, 115, 130, 150);
    ez_draw_rectangle (ev->win, 160, 110, 220, 150);
    ez_draw_circle    (ev->win, 240, 110, 300, 150);
    ez_draw_triangle  (ev->win, 320, 110, 380, 120, 350, 150);

    ez_set_color (ez_blue);
    ez_draw_text      (ev->win, EZ_BL, 10, 180, "Thickness 9");
    ez_set_color (ez_green);
    ez_set_thick (9);
    ez_draw_point     (ev->win,  30, 210);
    ez_draw_line      (ev->win,  60, 195, 130, 230);
    ez_draw_rectangle (ev->win, 160, 190, 220, 230);
    ez_draw_circle    (ev->win, 240, 190, 300, 230);
    ez_draw_triangle  (ev->win, 320, 190, 380, 200, 350, 230);

    ez_set_color (ez_red);
    ez_draw_text      (ev->win, EZ_BL, 10, 260, "Fill");
    ez_set_color (ez_yellow);
    ez_fill_rectangle (ev->win, 160, 270, 220, 310);
    ez_fill_circle    (ev->win, 240, 270, 300, 310);
    ez_fill_triangle  (ev->win, 320, 270, 380, 280, 350, 310);
}


void win1_on_key_press (Ez_event *ev)
{
    switch (ev->key_sym) {
        case XK_q : ez_quit (); break;
    }

}


void win1_on_event (Ez_event *ev)                /* Called by ez_main_loop() */
{                                                /* for each event on win1   */
    switch (ev->type) {
        case Expose   : win1_on_expose    (ev); break;
        case KeyPress : win1_on_key_press (ev); break;
    }
}


int main ()
{
    if (ez_init() < 0) exit(1);

    ez_window_create (400, 320, "Demo 03: All drawings", win1_on_event);

    ez_main_loop ();
    exit(0);
}

