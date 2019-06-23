/* demo-17.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 29/04/2013 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-17.c ez-draw.c ez-image.c -o demo-17 -lX11 -lXext -lm
 * Compilation on Windows :
 *     gcc -Wall demo-17.c ez-draw.c ez-image.c -o demo-17.exe -lgdi32 -lmsimg32 -lm
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"
#include "ez-image.h"


#define BALL_MAX   500
#define BALL_NB     50
#define WIN1_W     900
#define WIN1_H     700
#define DELAY1       5


typedef struct {
    int x, y, dx, dy;
} Ball;


void ball_init (Ball *ball, int ball_w, int ball_h, int win_h, int win_w)
{
    ball->x = ez_random (win_w - ball_w);
    ball->y = ez_random (win_h - ball_h);
    ball->dx = 2+ez_random(6);
    ball->dy = 2+ez_random(6);
}


void ball_next_step (Ball *ball, int ball_w, int ball_h, int win_h, int win_w)
{
    ball->x += ball->dx;
    ball->y += ball->dy;

    if (ball->x < 0) 
        ball->x = 0, ball->dx = -ball->dx;
    else if (ball->x > win_w - ball_w)
        ball->x = win_w - ball_w, ball->dx = -ball->dx;

    if (ball->y < 0) 
        ball->y = 0, ball->dy = -ball->dy;
    else if (ball->y > win_h - ball_h)
        ball->y = win_h - ball_h, ball->dy = -ball->dy;
}


void ball_draw_image (Ez_window win, Ez_image *img, Ball *ball)
{
    ez_image_paint (win, img, ball->x, ball->y);
}


void ball_draw_pixmap (Ez_window win, Ez_pixmap *pix, Ball *ball)
{
    ez_pixmap_paint (win, pix, ball->x, ball->y);
}


typedef struct {
    Ez_image  *image1;
    Ez_pixmap *pixmap1;
    Ez_window  win1;
    int        win1_h, win1_w;
    Ball       ball[BALL_MAX];
    int        ball_nb, expose_nb, flag_pix;
    double     time_ref, fps;
} App_data;


void app_data_init (App_data *a, char *filename)
{
    int i;

    a->image1 = ez_image_load (filename);
    if (a->image1 == NULL) exit (1);

    a->pixmap1 = ez_pixmap_create_from_image (a->image1);
    if (a->pixmap1 == NULL) exit (1);

    a->win1_h = WIN1_H; a->win1_w = WIN1_W;

    a->ball_nb = BALL_NB;
    for (i = 0; i < a->ball_nb; i++)
        ball_init (&a->ball[i], a->image1->width, a->image1->height,
            a->win1_h, a->win1_w);

    a->time_ref = ez_get_time ();
    a->fps = -1;
    a->expose_nb = 0;

    a->flag_pix = 0;
}


void app_data_destroy (App_data *a)
{
    ez_image_destroy (a->image1);
    ez_pixmap_destroy (a->pixmap1);
}


void update_fps (App_data *a)
{
    double time_now = ez_get_time ();
    a->expose_nb++;
    if (time_now - a->time_ref < 0.5) return;
    a->fps = a->expose_nb / (time_now - a->time_ref);
    a->expose_nb = 0;
    a->time_ref = time_now;
}


void win1_on_expose (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);
    int i;

    update_fps (a);

    for (i = 0; i < a->ball_nb; i++)
        if (a->flag_pix)
             ball_draw_pixmap (a->win1, a->pixmap1, &a->ball[i]);
        else ball_draw_image  (a->win1, a->image1 , &a->ball[i]);

    ez_set_color (ez_black);
    ez_draw_text (a->win1, EZ_BLF, 10, a->win1_h-10, 
        "+-: balls %d   p: pixmap %s",
        a->ball_nb, a->flag_pix ? "ON ":"OFF");
    if (a->fps > 0)
        ez_draw_text (a->win1, EZ_BRF, a->win1_w-10, a->win1_h-10, "fps %.1f",
            a->fps);
}


void win1_on_key_press (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);

    switch (ev->key_sym) {
        case XK_q : 
            ez_quit (); break;
        case XK_minus       :
        case XK_KP_Subtract : 
            a->ball_nb--; 
            if (a->ball_nb < 1) a->ball_nb = 1;
            break;
        case XK_plus        :
        case XK_KP_Add      : 
            a->ball_nb++; 
            if (a->ball_nb > BALL_MAX)
                a->ball_nb = BALL_MAX;
            else ball_init (&a->ball[a->ball_nb-1], a->image1->width, 
                a->image1->height, a->win1_h, a->win1_w);
            break;
        case XK_p : 
            a->flag_pix = !a->flag_pix;
            break;
        default : return;
    }
    ez_send_expose (a->win1);
}


void win1_on_configure_notify (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);

    a->win1_w = ev->width; a->win1_h = ev->height;
}


void win1_on_timer_notify (Ez_event *ev)
{
    App_data *a = ez_get_data (ev->win);
    int i;

    for (i = 0; i < a->ball_nb; i++)
        ball_next_step (&a->ball[i], a->image1->width, a->image1->height,
            a->win1_h, a->win1_w);

    ez_start_timer (a->win1, DELAY1);
    ez_send_expose (a->win1);
}


void win1_on_event (Ez_event *ev)
{
    switch (ev->type) {
        case Expose          : win1_on_expose           (ev); break;
        case KeyPress        : win1_on_key_press        (ev); break;
        case ConfigureNotify : win1_on_configure_notify (ev); break;
        case TimerNotify     : win1_on_timer_notify     (ev); break;
    }
}


int main (int argc, char *argv[])
{
    char *filename = "images/ball2.gif";
    App_data a;

    if (argc-1 != 1)
         fprintf (stderr, "Usage: %s image\n", argv[0]);
    else filename = argv[1];

    if (ez_init() < 0) exit (1);
    app_data_init (&a, filename);

    a.win1 = ez_window_create (WIN1_W, WIN1_H, "Demo 17: Pixmaps", win1_on_event);
    ez_set_data (a.win1, &a);
    ez_window_dbuf(a.win1, 1);

    ez_start_timer (a.win1, DELAY1);
    ez_main_loop ();

    app_data_destroy (&a);
    exit(0);
}

