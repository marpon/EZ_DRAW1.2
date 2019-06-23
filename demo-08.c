/* demo-08.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 21/06/2011 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-08.c ez-draw.c -o demo-08 -lX11 -lXext
 * Compilation on Windows :
 *     gcc -Wall demo-08.c ez-draw.c -o demo-08.exe -lgdi32
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"

#define BUF_MAX 80
char buf1[BUF_MAX] = "", buf2[BUF_MAX] = "";


/* Return 1 if text must be displayed again, 2 if text is validated, else 0 */

int text_input (Ez_event *ev, char *s)
{
    int i;

    switch (ev->key_sym) {

        case XK_BackSpace :                                 /* Backspace key */
            i = strlen (s);
            if (i == 0) break;
            s[i-1] = 0;
            return 1;

        case XK_Return :                                        /* Enter key */
            return 2;

        default :                                      /* Insert a character */
            if (ev->key_count != 1) break;
            i = strlen (s);
            if (i >= BUF_MAX-1) break;
            s[i] = ev->key_string[0]; s[i+1] = 0;
            return 1;
    }
    return 0;
}


void text_display (Ez_window win, int x, int y, char *s1, char *s2)
{
    ez_set_color (ez_black);
    ez_draw_text (win, EZ_TL, x, y, "Text: %s_", s1);

    if (strcmp (buf2, "") != 0) {
        ez_set_color (ez_blue);
        ez_draw_text (win, EZ_TC, 200, 70,
            "You have validated this text:\n%s", s2);
    }
}


void win1_on_expose (Ez_event *ev)
{
    text_display (ev->win, 10, 10, buf1, buf2);
}


void win1_on_key_press (Ez_event *ev)
{
    int k = text_input (ev, buf1);
    if (k == 2) strncpy (buf2, buf1, BUF_MAX);
    if (k > 0) ez_send_expose (ev->win);
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

    ez_window_create (400, 200, "Demo 08: Text input", win1_on_event);

    ez_main_loop ();
    exit(0);
}

