/* demo-12.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 29/04/2013 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-12.c ez-draw.c ez-image.c -o demo-12 -lX11 -lXext -lm
 * Compilation on Windows :
 *     gcc -Wall demo-12.c ez-draw.c ez-image.c -o demo-12.exe -lgdi32 -lmsimg32 -lm
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"
#include "ez-image.h"


double compute_hsv_image (Ez_image *img, double value)
{
    int x, y, t, dx, dy, xc, yc;
    double H, S, dc, rc, rc2, dc2, t1, t2;
    Ez_uint8 R, G, B;

    if (img == NULL) return 0;

    xc = img->width/2; yc = img->height/2; rc = xc-1; rc2 = rc*rc;
    t1 = ez_get_time ();

    for (y = 0, t = 0; y < img->height; y++)
    for (x = 0; x < img->width ; x++, t += 4)
    {
        dx = x-xc; dy = y-yc; dc2 = dx*dx + dy*dy;
        R = G = B = 255;

        if (dc2 <= rc2) {
            dc = sqrt (dc2);
            S = dc/rc;

            /* acos() returns angle between 0 and +M_PI */
            H = (S == 0) ? 0 : 
                (dy <= 0) ? acos (dx / dc) * 180 / M_PI :
                          - acos (dx / dc) * 180 / M_PI + 360;

            ez_HSV_to_RGB (H, S, value, &R, &G, &B);
        }

        img->pixels_rgba[t  ] = R;
        img->pixels_rgba[t+1] = G;
        img->pixels_rgba[t+2] = B;
    }

    t2 = ez_get_time ();
    return t2-t1;
}


typedef struct {
    Ez_image *image1;
    double time1, value;
    Ez_window win1;
} App_data;


void app_data_init (App_data *a)
{
    a->value = 1;
    a->image1 = ez_image_create (400, 400);
    a->time1 = compute_hsv_image (a->image1, a->value);
}


void app_data_destroy (App_data *a)
{
    ez_image_destroy (a->image1);
}


void win1_on_expose (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);
    int w, h;

    ez_window_get_size (a->win1, &w, &h);
    ez_image_paint (a->win1, a->image1, 10, 20);
    ez_set_color (ez_black);
    ez_draw_text (a->win1, EZ_TLF,   10,   10, "V = %.2f", a->value);
    ez_draw_text (a->win1, EZ_TRF, w-10,   10, "%.3f ms", a->time1*1000);
    ez_draw_text (a->win1, EZ_BL ,   10, h-10, "Press Space ...");
}


void win1_on_key_press (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);

    switch (ev->key_sym) {
        case XK_q     : ez_quit (); break;
        case XK_space : a->value -= 0.05; break;
        default       : return;
    }

    if (a->value < 0) a->value = 1;
    a->time1 = compute_hsv_image (a->image1, a->value);
    ez_send_expose (a->win1);
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
    App_data a;

    if (ez_init() < 0) exit (1);
    app_data_init (&a);

    a.win1 = ez_window_create (a.image1->width+20, a.image1->height+40, 
        "Demo 12: HSV palette", win1_on_event);
    ez_set_data (a.win1, &a);
    ez_window_dbuf(a.win1, 1);

    ez_main_loop ();

    app_data_destroy (&a);
    exit(0);
}

