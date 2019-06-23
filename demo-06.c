/* demo-06.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 21/06/2011 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-06.c ez-draw.c -o demo-06 -lX11 -lXext
 * Compilation on Windows :
 *     gcc -Wall demo-06.c ez-draw.c -o demo-06.exe -lgdi32
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"

#define VER_MAX 100
int ver_nb = 0, ver_x[VER_MAX], ver_y[VER_MAX];


void vertex_clear ()
{
    ver_nb = 0;
}


void vertex_insert (int x, int y)
{
    if (ver_nb >= VER_MAX) return;
    ver_x[ver_nb] = x;
    ver_y[ver_nb] = y;
    ver_nb++;
}


void vertex_move (int x, int y)
{
    if (ver_nb <= 0 || ver_nb >= VER_MAX) return;
    ver_x[ver_nb-1] = x;
    ver_y[ver_nb-1] = y;
}


void draw_vertices (Ez_window win)
{
    int i;

    ez_set_color (ez_blue);
    for (i = 0; i < ver_nb; i++)
        ez_draw_rectangle (win, ver_x[i]-2, ver_y[i]-2, ver_x[i]+2, ver_y[i]+2);
}

void draw_segments (Ez_window win)
{
    int i;

    ez_set_color (ez_grey);
    for (i = 1; i < ver_nb; i++)
        ez_draw_line (win, ver_x[i-1], ver_y[i-1], ver_x[i], ver_y[i]);
}


void win1_on_expose (Ez_event *ev)
{
    ez_set_color (ez_black);
    ez_draw_text (ev->win, EZ_TL, 10, 10,
        "Click and drag the mouse in the window to draw.\n"
        "Type Space to clear the window, 'q' to quit.");
    draw_segments (ev->win);
    draw_vertices (ev->win);
}


void win1_on_button_press (Ez_event *ev)
{
    vertex_insert (ev->mx, ev->my);
    ez_send_expose (ev->win);
}


void win1_on_motion_notify (Ez_event *ev)
{
    if (ev->mb == 0) return;                            /* No button pressed */
    vertex_move (ev->mx, ev->my);
    ez_send_expose (ev->win);
}


void win1_on_key_press (Ez_event *ev)
{
    switch (ev->key_sym) {
        case XK_q : 
            ez_quit (); 
            break;
        case XK_space : 
            vertex_clear ();
            ez_send_expose (ev->win); 
            break;
    }
}


void win1_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose       : win1_on_expose        (ev); break;
        case ButtonPress  : win1_on_button_press  (ev); break;
        case MotionNotify : win1_on_motion_notify (ev); break;
        case KeyPress     : win1_on_key_press     (ev); break;
    }
}


int main ()
{
    Ez_window win1;

    if (ez_init() < 0) exit(1);

    win1 = ez_window_create (400, 300, "Demo 06: Drawing wih the mouse", win1_on_event);

    /* Enable double buffer to prevent window flashes */
    ez_window_dbuf (win1, 1);

    ez_main_loop ();
    exit(0);
}

