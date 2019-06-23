/* demo-16.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 29/04/2013 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-16.c ez-draw.c ez-image.c -o demo-16 -lX11 -lXext -lm
 * Compilation on Windows :
 *     gcc -Wall demo-16.c ez-draw.c ez-image.c -o demo-16.exe -lgdi32 -lmsimg32 -lm
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"
#include "ez-image.h"


typedef struct {
    double time1;
    int src_cx, src_cy, dst_cx, dst_cy, theta, alpha, quality;
    Ez_image *image1, *image2;
    Ez_window win1, win2;
} App_data;


void app_data_init (App_data *a, char *filename)
{
    a->time1 = -1;
    a->src_cx = 100; a->src_cy = 50;
    a->theta = 45;
    a->alpha = a->quality = 1;

    a->image1 = ez_image_load (filename);
    if (a->image1 == NULL) exit (1);

    a->image2 = ez_image_rotate (a->image1, a->theta, a->quality);
    ez_image_rotate_point (a->image1, a->theta, a->src_cx, a->src_cy, 
        &a->dst_cx, &a->dst_cy);
    ez_image_set_alpha (a->image2, a->alpha);    
}


void app_data_destroy (App_data *a)
{
    ez_image_destroy (a->image1);
    ez_image_destroy (a->image2);
}


void draw_cross (Ez_window win, int x, int y)
{
    ez_draw_line (win, x-20, y, x+20, y);
    ez_draw_line (win, x, y-20, x, y+20);
}


void win1_on_expose (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);
    int w, h;

    ez_window_get_size (a->win1, &w, &h);
    ez_image_paint (a->win1, a->image1, 0, 0);
    ez_set_color (ez_red);
    draw_cross (a->win1, a->src_cx, a->src_cy);
    ez_set_color (ez_black);
    ez_draw_text (a->win1, EZ_BLF, 4, h-4, "Arrows: rotation center");
}


void win2_on_expose (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);
    int w, h;

    ez_window_get_size (a->win2, &w, &h);
    ez_set_color (ez_get_RGB (220,255,200));
    ez_fill_rectangle (a->win2, 0, 0, w-1, h-20);
    ez_image_paint (a->win2, a->image2, 0, 0);
    ez_set_color (ez_red);
    draw_cross (a->win2, a->dst_cx, a->dst_cy);
    ez_set_color (ez_black);
    ez_draw_text (a->win2, EZ_BLF, 4, h-4, 
        "+-: angle %2d  a: alpha %s  b: quality %s", 
        a->theta, a->alpha ? "ON ":"OFF", a->quality ? "ON ":"OFF"); 
    if (a->time1 > 0)
        ez_draw_text (a->win2, EZ_TRF, w-4, 4, "%.3f ms", a->time1*1000);
}


void win1_on_key_press (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);
    int rot = 0;

    switch (ev->key_sym) {
        case XK_q           : ez_quit (); break;
        case XK_minus       :
        case XK_KP_Subtract : a->theta = (a->theta+359) % 360; rot = 1; break;
        case XK_plus        :
        case XK_KP_Add      : a->theta = (a->theta+1) % 360; rot = 1; break;
        case XK_a           : a->alpha = !a->alpha; break;
        case XK_b           : a->quality = !a->quality; rot = 1; break;
        case XK_Left        : 
        case XK_KP_Left     : a->src_cx-- ; break;
        case XK_Right       :
        case XK_KP_Right    : a->src_cx++ ; break;
        case XK_Up          : 
        case XK_KP_Up       : a->src_cy-- ; break;
        case XK_Down        : 
        case XK_KP_Down     : a->src_cy++ ; break;
        default             : return;
    }

    if (rot) {
        ez_image_destroy (a->image2);
        a->time1 = ez_get_time ();
        a->image2 = ez_image_rotate (a->image1, a->theta, a->quality);
        a->time1 = ez_get_time () - a->time1;
    }
    ez_image_set_alpha (a->image2, a->alpha);    
    ez_image_rotate_point (a->image1, a->theta, a->src_cx, a->src_cy, 
        &a->dst_cx, &a->dst_cy);

    if (!rot) ez_send_expose (a->win1);
    ez_send_expose (a->win2);
}


void win1_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose   : win1_on_expose    (ev); break;
        case KeyPress : win1_on_key_press (ev); break;
    }
}


void win2_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose   : win2_on_expose    (ev); break;
        case KeyPress : win1_on_key_press (ev); break;  /* of win1 */
    }
}


int main (int argc, char *argv[])
{
    char *filename = "images/tux2.gif";
    App_data a;
    int d;

    if (argc-1 != 1)
         fprintf (stderr, "Usage: %s image\n", argv[0]);
    else filename = argv[1];

    if (ez_init() < 0) exit (1);
    app_data_init (&a, filename);

    a.win1 = ez_window_create (a.image1->width, a.image1->height+24, 
        "Demo 16: Rotation", win1_on_event);
    ez_set_data (a.win1, &a);
    ez_window_dbuf (a.win1, 1);

    d = sqrt (a.image1->width *a.image1->width + 
             a.image1->height*a.image1->height);
    a.win2 = ez_window_create (d, d+24, "Rotation result", win2_on_event);
    ez_set_data (a.win2, &a);
    ez_window_dbuf (a.win2, 1);

    ez_main_loop ();

    app_data_destroy (&a);
    exit(0);
}

