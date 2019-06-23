/* demo-13.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 29/04/2013 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-13.c ez-draw.c ez-image.c -o demo-13 -lX11 -lXext -lm
 * Compilation on Windows :
 *     gcc -Wall demo-13.c ez-draw.c ez-image.c -o demo-13.exe -lgdi32 -lmsimg32 -lm
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"
#include "ez-image.h"


typedef struct {
    Ez_image *image1;
    Ez_window win1;
} App_data;


void app_data_init (App_data *a, char *filename)
{
    a->image1 = ez_image_load (filename);                   /* Load an image */
    if (a->image1 == NULL) exit (1);
}


void app_data_destroy (App_data *a)
{
    ez_image_destroy (a->image1);                           /* Destroy image */
}


void win1_on_expose (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);

    ez_image_paint (a->win1, a->image1, 0, 0);              /* Display image */
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


int main (int argc, char *argv[])
{
    char *filename = "images/tux2.gif";
    App_data a;

    if (argc-1 != 1)
         fprintf (stderr, "Usage: %s image\n", argv[0]);
    else filename = argv[1];

    if (ez_init() < 0) exit (1);
    app_data_init (&a, filename);

    a.win1 = ez_window_create (                   /* Resize window for image */
        a.image1->width, a.image1->height, 
        filename, win1_on_event);
    ez_set_data (a.win1, &a);
    ez_window_dbuf(a.win1, 1);

    ez_main_loop ();

    app_data_destroy (&a);
    exit(0);
}

