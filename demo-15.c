/* demo-15.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 29/04/2013 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-15.c ez-draw.c ez-image.c -o demo-15 -lX11 -lXext -lm
 * Compilation on Windows :
 *     gcc -Wall demo-15.c ez-draw.c ez-image.c -o demo-15.exe -lgdi32 -lmsimg32 -lm
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"
#include "ez-image.h"


typedef struct {
    double time1, factor;
    int alpha;
    Ez_image *image1, *image2;
    Ez_window win1;
} App_data;


void app_data_init (App_data *a, char *filename)
{
    a->time1 = -1;
    a->factor = 1;
    a->alpha = 1;

    a->image1 = ez_image_load (filename);
    if (a->image1 == NULL) exit (1);

    a->image2 = ez_image_dup (a->image1);
    ez_image_set_alpha (a->image2, a->alpha);
}


void app_data_destroy (App_data *a)
{
    ez_image_destroy (a->image1);
    ez_image_destroy (a->image2);
}


void win1_on_expose (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);
    int w, h;

    ez_window_get_size (a->win1, &w, &h);
    ez_set_color (ez_get_RGB (220,255,200));
    ez_fill_rectangle (a->win1, 0, 0, w-1, h-20);
    ez_image_paint (a->win1, a->image2, 0, 0);
    ez_set_color (ez_black);
    ez_draw_text (a->win1, EZ_BLF, 4, h-4, 
        "+-: factor %.2f  a: alpha %s", 
        a->factor, a->alpha ? "ON ":"OFF"); 
    if (a->time1 > 0)
        ez_draw_text (a->win1, EZ_TRF, w-4, 4, "%.3f ms", a->time1*1000);
}


void win1_on_key_press (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);

    switch (ev->key_sym) {
        case XK_q           : ez_quit (); break;
        case XK_minus       :
        case XK_KP_Subtract : a->factor -= 0.02; break;
        case XK_plus        :
        case XK_KP_Add      : a->factor += 0.02; break;
        case XK_a           : a->alpha = !a->alpha; break;
       default              : return;
    }

    ez_image_destroy (a->image2);
    a->time1 = ez_get_time ();
    a->image2 = ez_image_scale (a->image1, a->factor);
    ez_image_set_alpha (a->image2, a->alpha);
    a->time1 = ez_get_time () - a->time1;
    ez_send_expose (a->win1);
}


void win1_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose   : win1_on_expose    (ev); break;
        case KeyPress : win1_on_key_press (ev); break;
    }
}


int main (int argc, char *argv[])
{
    char *filename = "images/tux2.gif";
    App_data a;

    if (argc-1 != 1)
         fprintf (stderr, "Usage: %s image\n", argv[0]);
    else filename = argv[1];

    if (ez_init() < 0) exit (1);
    app_data_init (&a, filename);

    a.win1 = ez_window_create (a.image1->width*2, a.image1->height*2+24, 
        "Demo 15: Scale", win1_on_event);
    ez_set_data (a.win1, &a);
    ez_window_dbuf (a.win1, 1);

    ez_main_loop ();

    app_data_destroy (&a);
    exit(0);
}

