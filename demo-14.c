/* demo-14.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 29/04/2013 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-14.c ez-draw.c ez-image.c -o demo-14 -lX11 -lXext -lm
 * Compilation on Windows :
 *     gcc -Wall demo-14.c ez-draw.c ez-image.c -o demo-14.exe -lgdi32 -lmsimg32 -lm
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"
#include "ez-image.h"


typedef struct {
    int i2_x, i2_y;
    Ez_image *image1, *image2;
    Ez_window win1;
} App_data;


void app_data_init (App_data *a, char *filename1, char *filename2)
{
    a->image1 = ez_image_load (filename1);
    if (a->image1 == NULL) exit (1);

    a->image2 = ez_image_load (filename2);
    if (a->image2 == NULL) exit (1);

    /* Initial position is centered */
    a->i2_x = (a->image1->width  - a->image2->width ) / 2;
    a->i2_y = (a->image1->height - a->image2->height) / 2;
}


void app_data_destroy (App_data *a)
{
    ez_image_destroy (a->image1);
    ez_image_destroy (a->image2);
}


void win1_on_expose (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);

    ez_image_paint (a->win1, a->image1, 0, 0);
    ez_image_paint (a->win1, a->image2, a->i2_x, a->i2_y); 
    ez_draw_text (a->win1, EZ_BLF, 10, a->image1->height+15, 
        "[Arrows] to move");
    ez_draw_text (a->win1, EZ_BRF, a->image1->width-10, a->image1->height+15, 
        "Opacity [+-] : %d", a->image2->opacity);
}


void win1_on_key_press (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);

    switch (ev->key_sym) {
        case XK_q : ez_quit (); break;
        case XK_Left        : 
        case XK_KP_Left     : a->i2_x-- ; break;
        case XK_Right       :
        case XK_KP_Right    : a->i2_x++ ; break;
        case XK_Up          : 
        case XK_KP_Up       : a->i2_y-- ; break;
        case XK_Down        : 
        case XK_KP_Down     : a->i2_y++ ; break;
        case XK_minus       :
        case XK_KP_Subtract : a->image2->opacity--; break;
        case XK_plus        :
        case XK_KP_Add      : a->image2->opacity++; break;
        default             : return;
    }
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
    char *file1 = "images/paper1.jpg", *file2 = "images/tux1.png";
    App_data a;

    if (argc-1 != 2)
        fprintf (stderr, "Usage: %s image1 image2\n", argv[0]);
    else { file1 = argv[1]; file2 = argv[2]; }

    if (ez_init() < 0) exit(1);
    app_data_init (&a, file1, file2);

    a.win1 = ez_window_create (a.image1->width, a.image1->height+15, 
        "Demo 14: Images with transparency", win1_on_event);
    ez_set_data (a.win1, &a);
    ez_window_dbuf(a.win1, 1);

    ez_main_loop ();

    app_data_destroy (&a);
    exit(0);
}

