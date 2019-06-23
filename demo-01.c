/* demo-01.c : demonstration of EZ-Draw
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 02/07/2008 - version 1.2
 *
 * Compilation on Unix :
 *     gcc -Wall demo-01.c ez-draw.c -o demo-01 -lX11 -lXext
 * Compilation on Windows :
 *     gcc -Wall demo-01.c ez-draw.c -o demo-01.exe -lgdi32
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


int main ()
{
    if (ez_init() < 0) exit(1);

    ez_window_create (400, 300, "Demo 01: Hello World", NULL);

    ez_main_loop ();
    exit(0);
}

