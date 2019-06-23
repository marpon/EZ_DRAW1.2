/* demo-11.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 29/04/2013 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-11.c ez-draw.c -o demo-11 -lX11 -lXext
 * Compilation on Windows :
 *     gcc -Wall demo-11.c ez-draw.c -o demo-11.exe -lgdi32
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


void win1_on_expose (Ez_event *ev)
{
    char *color_names [9] = { "ez_black", "ez_white", "ez_grey", "ez_red", 
        "ez_green", "ez_blue", "ez_yellow", "ez_cyan", "ez_magenta" };
    Ez_uint32 c[9];
    int i, x, y, r, g, b;

    c[0] = ez_black; c[1] = ez_white; c[2] = ez_grey; c[3] = ez_red ;
    c[4] = ez_green; c[5] = ez_blue; c[6] = ez_yellow; c[7] = ez_cyan;
    c[8] = ez_magenta;

    for (i = 0; i < 9; i++) {
        x = 20+(i%3)*100; y = 20+(i/3)*60;
        ez_set_color (ez_black);
        ez_draw_text (ev->win, EZ_TL, x, y, color_names[i]);
        ez_draw_rectangle (ev->win, x, y+20, x+60, y+40);
        ez_set_color (c[i]);
        ez_fill_rectangle (ev->win, x+1, y+21, x+59, y+39);
    }

    ez_set_color (ez_black);
    ez_draw_text (ev->win, EZ_TL, 20, 200, "Grey levels: 0..255");
    ez_draw_rectangle (ev->win, 20, 220, 277, 240);
    for (i = 0; i < 256; i++) {
        ez_set_color (ez_get_grey (i));
        ez_draw_line (ev->win, 21+i, 221, 21+i, 239);
    }

    ez_set_color (ez_black);
    ez_draw_text (ev->win, EZ_TL, 20, 260, "HSV color: H = 0..360, S = V = 1");
    ez_draw_rectangle (ev->win, 20, 280, 380, 300);
    for (i = 0; i < 360; i++) {
        ez_set_color (ez_get_HSV (i, 1, 1));
        ez_draw_line (ev->win, 21+i, 281, 21+i, 299);
    }

    r = ez_random (256); g = ez_random (256); b = ez_random (256);
    ez_set_color (ez_black);
    ez_draw_text (ev->win, EZ_TLF, 320, 160, "Random RGB\n%3d %3d %3d", r, g, b);
    ez_draw_rectangle (ev->win, 320, 200, 380, 240);
    ez_set_color (ez_get_RGB (r, g, b));
    ez_fill_rectangle (ev->win, 321, 201, 379, 239);
}


void win1_on_key_press (Ez_event *ev)
{
    switch (ev->key_sym) {
        case XK_q     : ez_quit (); break;
        case XK_space : ez_send_expose (ev->win); break;
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
    Ez_window win1;

    if (ez_init() < 0) exit(1);

    win1 = ez_window_create (410, 320, "Demo 11: Colors", win1_on_event);
    ez_window_dbuf (win1, 1);

    ez_main_loop ();
    exit(0);
}

