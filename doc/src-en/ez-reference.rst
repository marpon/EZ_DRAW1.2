
.. _chap-ref-manual:

===================
Reference manual
===================


Sources: ez-draw.h_, ez-draw.c_.


.. ############################################################################

.. index:: Main loop

.. _sec-ref-main-loop:

---------
Main loop
---------


.. function:: int ez_init ()

   Initialization.

   Return 0 on success, -1 on failure.


.. function:: void ez_main_loop ()

   Main loop.

This function displays the existing windows, then waits for events;
for each incoming event the function calls the corresponding window event function 
(the *callback*).

To stop the main loop, call :func:`ez_quit` in a callback.
Once returned from :func:`ez_main_loop`, no more graphic call should be done.


.. function:: void ez_quit ()

   Exit :func:`ez_main_loop()`.

.. index:: Close Button

.. function:: void ez_auto_quit (int val)

   Modify the effect of the "Close" button in the title bar of a window,
   for each window of the program.

By default (``val = 1``), if you click on the "Close" button of any 
window of the program, it ends immediately.

You can change this behavior by invoking :func:`ez_auto_quit` with 
``val = 0`` : then, if the user clicks on the "Close" button of a window, 
the program (instead of ending) will receive the event  
``WindowClose`` for this window, so as to decide what to do:

  * ignore the event;
  * destroy the window with :func:`ez_window_destroy`;
  * hide the window with :func:`ez_window_show`;
  * create a dialog window;
  * complete the program with :func:`ez_quit`;
  * etc.

Note: when all windows are destroyed, the program stops.


.. ############################################################################

.. index:: Window

.. _sec-ref-windows:

--------
Windows
--------

Each window has a unique identifier, of type :type:`Ez_window`:

.. type:: Ez_window

   Identifier of a window.


The following functions allow to create or manipulate windows:

.. index:: Callback

.. function:: Ez_window ez_window_create (int w, int h, const char *name, Ez_func on_event)

   Create and display a window, having width ``w`` and height ``h``,
   a title ``name``, and a function ``on_event`` (the *callback*) called for each event
   (``on_event`` might be ``NULL``).

   Return the identifier of the window, of type :type:`Ez_window`.


Any callback has type :type:`Ez_func` :

.. type:: Ez_func

   The type of a callback, that is to say, the prototype of an event handler,
   the function called for each event. This type is defined as:

   .. code-block:: c

       typedef void (*Ez_func)(Ez_event *ev);

   In other words, the function ``on_event`` given to :func:`ez_create_window` 
   has to be declared as:

   .. code-block:: c

       void on_event (Ez_event *ev);


.. function:: int ez_window_get_id (Ez_window win)

   Return the window id as an ``int``.


.. function:: void ez_window_destroy (Ez_window win)

   Destroy window ``win``.


.. function:: void ez_window_show (Ez_window win, int val)

   Show (``val = 1``) or hide (``val = 0``) the window ``win``.


.. function:: void ez_window_set_size (Ez_window win, int w, int h)

   Change the size of the window.


.. function:: void ez_window_get_size (Ez_window win, int *w, int *h)

   Retrieve the window size.


.. function:: void ez_window_clear (Ez_window win)

   Empty the window (with a white background) and initialize the drawing
   parameters (color, width, font) to the default values.


.. function:: void ez_send_expose (Ez_window win)

   Send an ``Expose`` event to the window, so as to empty the window and to
   force to redraw.


.. ############################################################################

.. index:: Event; Events list, Mouse; Events and coordinates

.. _sec-ref-events:

------
Events
------

Each event is described by a struct of type :type:`Ez_event`:

.. type:: Ez_event

   Store an event.

This type is defined in ez-draw.h_ as follows:

.. code-block:: c

    typedef struct {
        int type;                       /* Expose, ButtonPress, etc                */
        Ez_window win;                  /* Identifier of the window                */
        int mx, my;                     /* Mouse coordinates                       */
        int mb;                         /* Mouse button, 0 = none                  */
        int width, height;              /* Width and height                        */
        KeySym key_sym;                 /* Key symbol: XK_Space, XK_q, etc         */
        char   key_name[80];            /* For tracing: "XK_Space", "XK_q", ..     */
        char   key_string[80];          /* Corresponding string: " ", "q", etc     */
        int    key_count;               /* String length                           */
        /* Other fields private */
    } Ez_event;

The first field gives the event type. The possible values are:

.. table::
   :class: centrer nonsouligner encadrer

   ===================  ====================================
   ``Expose``           The whole window must be redrawn.
   ``ButtonPress``      Mouse button pressed.
   ``ButtonRelease``    Mouse button released.
   ``MotionNotify``     Mouse moved.
   ``KeyPress``         Key pressed.
   ``KeyRelease``       Key released.
   ``ConfigureNotify``  The window size has changed.
   ``WindowClose``      The button "Close" was pressed.
   ``TimerNotify``      The timer has expired.
   ===================  ====================================


.. ############################################################################

.. index:: Color

.. _sec-ref-colors:

------
Colors
------

Each color is denoted by an integer of type ``Ez_uint32``.

.. function:: void ez_set_color (Ez_uint32 color)

   Store the color ``color`` for the next drawings, as well as for text displaying.

The following colors are predefined:
``ez_black``, ``ez_white``, ``ez_grey``, ``ez_red``, ``ez_green``, ``ez_blue``, 
``ez_yellow``, ``ez_cyan``, ``ez_magenta``.

Other colors can be obtained with these functions:


.. function:: Ez_uint32 ez_get_RGB (Ez_uint8 r, Ez_uint8 g, Ez_uint8 b)

   Return a color computed according to the levels ``r,g,b`` given between 0 and 255.


.. function:: Ez_uint32 ez_get_grey (Ez_uint8 g)

   Return a grey color computed according to the level ``g`` given between 0 and 255.


.. function:: Ez_uint32 ez_get_HSV (double h, double s, double v)

   Return a color defined in space Hue, Saturation, Value.


``h`` is an angle between 0 and 360 degrees which arbitrary represents pure colors;
``s`` is saturation, between 0 and 1 ; ``v`` is the luminosity value,
between 0 and 1.
For more information, see 
`Hue Saturation Value <http://en.wikipedia.org/wiki/Hue/saturation/value>`_
in Wikipedia.


.. function:: void ez_HSV_to_RGB (double h, double s, double v, Ez_uint8 *r, Ez_uint8 *g, Ez_uint8 *b)

   Convert a color from HSV to RGB.

   The ranges are: 
   ``h`` between 0 and 360,
   ``s`` and ``v`` between 0 and 1,
   ``*r``, ``*g``, ``*b`` between 0 and 255.


As examples, see demo-11.c_ and demo-12.c_.


These windows are obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-11.png
      :alt: demo-11
   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-12.png
      :alt: demo-12


.. ############################################################################

.. index:: Drawing

.. _sec-ref-drawings:

--------
Drawings
--------


The coordinates are relative to the Origin, which is the top left point
inside the window; ``x`` goes to the right and ``y`` goes down.

For rectangles and circles, ``x1,y1`` and ``y2,y2`` are the top left and
bottom right coordinates of the bounding box.
For points, line segments and triangles, the vertex coordinates are
expected.

The default color is black; it can be changed with :func:`ez_set_color`.


.. function:: void ez_draw_point (Ez_window win, int x1, int y1)

   Draw a point.


.. function:: void ez_draw_line (Ez_window win, int x1, int y1, int x2, int y2)

   Draw a line segment.


.. function:: void ez_draw_rectangle (Ez_window win, int x1, int y1, int x2, int y2)

   Draw a rectangle.


.. function:: void ez_fill_rectangle (Ez_window win, int x1, int y1, int x2, int y2)

   Fill a rectangle.


.. function:: void ez_draw_triangle (Ez_window win, int x1, int y1, int x2, int y2, int x3, int y3)

   Draw a triangle.

.. function:: void ez_fill_triangle (Ez_window win, int x1, int y1, int x2, int y2, int x3, int y3)

   Fill a triangle.


.. function:: void ez_draw_circle (Ez_window win, int x1, int y1, int x2, int y2)

   Draw a circle.


.. function:: void ez_fill_circle (Ez_window win, int x1, int y1, int x2, int y2)

   Fill a circle.


Drawings with :func:`ez_draw_point`, :func:`ez_draw_line`, 
:func:`ez_draw_rectangle`, :func:`ez_draw_triangle`, :func:`ez_draw_circle`
have a default thickness of 1 pixel. It can be changed with:


.. function:: void ez_set_thick (int thick)

   Set thickness ``thick`` (in pixels) for the next drawings.


.. ############################################################################

.. index:: Font, Text

.. _sec-ref-fonts:

--------------
Text and fonts
--------------

.. function:: int ez_font_load (int num, const char *name)

   Load the font ``name`` (e.g. ``"6x13"``) and store it as the font
   number ``num``.

   Return 0 on success, -1 on error.


The font number must be less than ``EZ_FONT_MAX``.
Some fixed fonts are preloaded by default:

  * Font number 0: ``"6x13"``
  * Font number 1: ``"8x16"``
  * Font number 2: ``"10x20"``
  * Font number 3: ``"12x24"``


Note:
   On X11, the name can be in any fashion but must correspond to an existing
   font. On Windows, the name must be in the form 
   *width*\ ``x``\ *height* (a matching font of fixed size is obtained).


.. function:: void ez_set_nfont (int num)

   Set the font number ``num`` for the next text drawings.


.. function:: void ez_draw_text (Ez_window win, Ez_Align align, int x1, int y1, const char *format, ...)

   Display text; same usage as ``printf``.


Example:

    .. code-block:: c

        ez_draw_text (win, EZ_TL, 10, 10, "Width = %d\nHeight = %d", w, h);


The coordinates ``x1,y1`` are relative to ``align``,
which takes for values:

.. table::
   :class: centrer nonsouligner encadrer

   =======================  =========================  ========================
   ``EZ_TL`` (Top Left)     ``EZ_TC`` (Top Center)     ``EZ_TR`` (Top Right)
   ``EZ_ML`` (Middle Left)  ``EZ_MC`` (Middle Center)  ``EZ_MR`` (Middle Right)
   ``EZ_BL`` (Bottom Left)  ``EZ_BC`` (Bottom Center)  ``EZ_BR`` (Bottom Right)
   =======================  =========================  ========================


The text is drawn over the current content of the window; you can also delete 
the background at the same time (with white) using these values for ``align``:

.. table::
   :class: centrer nonsouligner encadrer

   ===============================  =================================  ================================
   ``EZ_TLF`` (Top Left Filled)     ``EZ_TCF`` (Top Center Filled)     ``EZ_TRF`` (Top Right Filled)
   ``EZ_MLF`` (Middle Left Filled)  ``EZ_MCF`` (Middle Center Filled)  ``EZ_MRF`` (Middle Right Filled)
   ``EZ_BLF`` (Bottom Left Filled)  ``EZ_BCF`` (Bottom Center Filled)  ``EZ_BRF`` (Bottom Right Filled)
   ===============================  =================================  ================================


By default:
  * the text is displayed using font number 0 (``6x13``);
    it can be changed by :func:`ez_set_nfont`.
  * the text is displayed in black; the color can be changed with
    :func:`ez_set_color`.


.. ############################################################################

.. index:: Double buffering
   seealso: Image; Double buffering
   seealso: Animation; Double buffering

.. _sec-ref-dbuf:

----------------
Double buffering
----------------


Displaying with a double buffer prevents the window flashes while it is refreshed.
The idea is to draw in the double buffer, then swap it with the window content
when all the drawings are finished. Everything is handled automatically by EZ-Draw.


.. function:: void ez_window_dbuf (Ez_window win, int val)

   Enable or disable the double buffer display for the window ``win``.


By default, the double buffering is disabled (``val = 0``).

If the double buffering is enabled (``val = 1``) for a window,
every drawings in this window must be made during ``Expose`` events only.
If the double buffering is disabled,  it is no longer a requirement, 
but it is strongly advised.

As an example, see in game jeu-nim.c_ the functions
``gui_init()``, ``win1_onKeyPress()``, ``win1_onExpose()``.

In this game, you can test the display with or without the double buffer
(press key ``d`` to switch between one and the other):

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-jeu-nim-2.png
      :alt: jeu-nim-2


.. ############################################################################

.. index:: Timer
   seealso: Animation; Timer

.. _sec-ref-timers:

------
Timers
------


*Starting a timer* means storing a date in the future, which is the current date 
plus a certain delay. When we arrive at this date in the future, we say that the 
timer has *expired*.

Each window can be associated with a timer.
Upon expiration of the timer, the program receives a unique event
``TimerNotify`` for the window, then the timer is deleted.


.. function:: void ez_start_timer (Ez_window win, int delay)

   Start a timer for the window ``win`` with the delay ``delay`` expressed
   in milliseconds.

Any recall of this function before timer expiration will cancel and replace
the timer with the new delay. Moreover, if ``delay`` is ``-1`` then the timer
is deleted.
(Note: this is not an error to delete a timer already deleted or non-existent).

An an example, see demo-09.c_.


.. ############################################################################

.. index:: Client-data, Global variable

.. _sec-ref-client-data:

-----------
Client-data
-----------


Each window can store an arbitrary data of the program,
for instance a string or a struct address.
You can then recover the data at any time in the program.
This mechanism aims to avoid global variables.


.. function:: void ez_set_data (Ez_window win, void *data)

   Store the data ``data`` in the window ``win`` 


.. function:: void *ez_get_data (Ez_window win)

   Return the data stored in the window ``win``.


Here is an example of program that draws a circle, whose coordinates
are stored into a global variable ``md``:

.. code-block:: c
    :linenos:

    #include "ez-draw.h"

    typedef struct {
        int x, y, r;
    } My_data;

    My_data md;  /* 1. Global variable */


    void win1_on_expose (Ez_event *ev)
    {
        /* 3. Use */
        ez_draw_circle (ev->win, md.x-md.r, md.y-md.r, md.x+md.r, md.y+md.r);
    }


    void win1_on_event (Ez_event *ev)
    {
       switch (ev->type) {
            case Expose : win1_on_expose (ev); break;
        }
    }


    int main ()
    {
        if (ez_init() < 0) exit(1);

        /* 2. Initialization */
        md.x = 200; md.y = 100; md.r = 50;

        ez_window_create (400, 300, "Demo client-data 1", win1_on_event);

        ez_main_loop ();
        exit(0);
    }


Here is now the same program but without using global variable;
the data are stored in the window:


.. code-block:: c
    :linenos:

    #include "ez-draw.h"
    
    typedef struct {
        int x, y, r;
    } My_data;


    void win1_on_expose (Ez_event *ev)
    {
        /* 4. We retrieve the data stored in the window */
        My_data *md = ez_get_data (ev->win);

        /* 5. Use */
        ez_draw_circle (ev->win, md->x-md->r, md->y-md->r, md->x+md->r, md->y+md->r);
    }


    void win1_on_event (Ez_event *ev)
    {
       switch (ev->type) {
            case Expose : win1_on_expose (ev); break;
        }
    }


    int main ()
    {
        Ez_window win1;
        My_data md;  /* 1. Local variable to main() */
    
        if (ez_init() < 0) exit(1);

        /* 2. Initialization */
        md.x = 200; md.y = 100; md.r = 50;
    
        win1 = ez_window_create (400, 300, "Demo client-data 2", win1_on_event);

        /* 3. We store the data in the window */
        ez_set_data (win1, &md);

        ez_main_loop ();
        exit(0);
    }


As another example, see demo-10.c_.


.. ############################################################################

.. index:: Image; The image type

.. _sec-ref-images-type:

--------------
The image type
--------------


EZ-Draw allows to display or to manipulate images, thanks to the type:

.. type:: Ez_image

   Main ``struct`` type to store an image.

This type is defined in ez-image.h_ as follows:

.. code-block:: c

    typedef struct {
        int width, height;
        Ez_uint8 *pixels_rgba;
        int has_alpha;
        int opacity;
    } Ez_image;

Guess what: the image width in pixels is ``width`` and its height is ``height``.

The pixels are stored in the table ``pixels_rgba``
as R,G,B,A (for red, green, blue and alpha, that is to say transparency) 
each having a value between 0 and 255 (255 is the maximum intensity or opacity).

The R,G,B,A values of a pixel having coordinates ``x,y`` in the image
are stored in ``pixels_rgba[(y*width+x)*4 + 0..3]``.

The ``has_alpha`` field indicates if the alpha channel is used (``has_alpha = 1``)
or ignored (``has_alpha = 0``) when displaying. 
If the channel is ignored, all pixels are displayed;
if used, just the opaque pixels are displayed.

The opaque pixels are those for which the alpha channel is greater or equal
to the opacity threshold, specified by the ``opacity`` field; 
by default, the opacity threshold is 128.


**Warning:**
    do not modified the fields ``width``, ``height``, ``pixels_rgba`` of an
    image, since they describe the allocated memory.
    However, you can change the fields ``has_alpha``, ``opacity``,
    as well as the pixel values in ``pixels_rgba[]``.
    You may also use the following functions.


.. function:: void ez_image_set_alpha (Ez_image *img, int has_alpha)
              int  ez_image_has_alpha (Ez_image *img)
              void ez_image_set_opacity (Ez_image *img, int opacity)
              int  ez_image_get_opacity (Ez_image *img)

   Get back or change the fields ``has_alpha`` and ``opacity``.
   
   These functions simply do nothing if ``img`` is ``NULL``.


.. ############################################################################

.. index:: Image; Managing images

.. _sec-ref-managing-images:

---------------
Managing images
---------------

To use the following functions you must include ez-image.h_.


.. function:: Ez_image *ez_image_create (int w, int h)

   Create an image having width ``w`` and height ``h``, in pixels.

   Return the created image, or ``NULL`` on error.


.. function:: Ez_image *ez_image_load (const char *filename)

   Load an image from the file ``filename``.
   The file must be in PNG, JPEG, GIF or BMP format.

   Transparency is supported for PNG, GIF and BMP format:
   if the file contains an alpha channel, then the field ``has_alpha`` 
   of the image is set to 1.

   Return the created image, or ``NULL`` on error.


.. function:: Ez_image *ez_image_dup (Ez_image *img)

   Create a deep copy of the image ``img``.

   Return the created image, or ``NULL`` on error.


.. function:: void ez_image_destroy (Ez_image *img)

   Destroy an image in memory.

   All images created by ``ez_image_...`` should be destroyed using this function.


.. function:: void ez_image_paint (Ez_window win, Ez_image *img, int x, int y)

   Display an image in the window ``win``, with the upper left corner of the image
   at the ``x,y`` coordinates in the window.
   If ``img->has_alpha`` is true, apply transparency, that is to say,
   only display opaque pixels.


.. function:: void ez_image_paint_sub (Ez_window win, Ez_image *img, int x, int y, \
        int src_x, int src_y, int w, int h)

   Display a rectangular region of an image in the window ``win``.

   The image region is bounded by coordinates ``src_x, src_y``
   (top left corner) and ``src_x+w-1, src_y+h-1`` (bottom right corner)
   in the image. If the coordinates go beyond the image, just the region
   which actually fits in the image is displayed.

   The top left corner of the region is displayed at the ``x,y``
   coordinate in the window.
   If ``img->has_alpha`` is true, apply transparency.


.. function:: void ez_image_print (Ez_image *img, int src_x, int src_y, int w, int h)

   Display a rectangular region of an image in the terminal.

   The image region is bounded by coordinates ``src_x, src_y``
   (top left corner) and ``src_x+w-1, src_y+h-1`` (bottom right corner)
   in the image. If the coordinates go beyond the image, just the region
   which actually fits in the image is displayed.


.. ############################################################################

.. index:: Image; Operations on images

.. _sec-ref-oper-on-images:

--------------------
Operations on images
--------------------

This section presents some operations which are available by including
ez-image.h_.
These operations are done on colors and on the alpha channel.


.. function:: void ez_image_fill_rgba (Ez_image *img, Ez_uint8 r, Ez_uint8 g, Ez_uint8 b, Ez_uint8 a)

   Fill an image with color ``r,g,b,a``.

   The values are between 0 and 255.


.. function:: void ez_image_blend (Ez_image *dst, Ez_image *src, int dst_x, int dst_y)
              void ez_image_blend_sub (Ez_image *dst, Ez_image *src, int dst_x, int dst_y, \
                  int src_x, int src_y, int w, int h)

   Superimpose a region of image ``src`` into the image ``dst``.

   The region of source image ``src`` is bounded by coordinates
   ``src_x, src_y`` (top left corner) and ``src_x+w-1, src_y+h-1`` 
   (bottom right corner).
   The function superimposes this region into the destination image ``dst``
   at coordinates ``dst_x, dst_y`` (top left corner) and
   ``dst_x+w-1, dst_y+h-1`` (bottom right corner).

   If the coordinates go beyond the images ``src`` or ``dst``, 
   just the common region is superimposed.
   If the source image has no alpha channel (``src->has_alpha``
   is false), then the pixel values from the ``src`` region
   overwrite those of ``dst``.
   Otherwise, the regions are melted by transparency
   (*alpha blending*) using formulas of
   `Porter and Duff <http://fr.wikipedia.org/wiki/Alpha_blending>`_.


.. function:: Ez_image *ez_image_extract (Ez_image *img, int src_x, int src_y, int w, int h)

   Create an image containing a copy of a rectangular region of the
   source image ``img``.

   The region of the image is bounded by coordinates ``src_x, src_y``
   (top left corner) and ``src_x+w-1, src_y+h-1`` (bottom right corner)
   in ``img``. If the coordinates go beyond the image, just the region
   fitting in the image is extracted.

   Return ``NULL`` on memory error or if intersection is empty.


.. function:: Ez_image *ez_image_sym_ver (Ez_image *img)
              Ez_image *ez_image_sym_hor (Ez_image *img)

   Create an image of same size and properties as the source image ``img``,
   containing the symmetrical image with respect to the vertical or horizontal axis.

   Return the new image, or ``NULL`` on error.


.. function:: Ez_image *ez_image_scale (Ez_image *img, double factor)

   Create an image whose size is scaled by the ``factor`` with respect to
   the source image ``img``, containing the rescaled image.
   The scale factor ``factor`` must be strictly positive.

   Return the new image, or ``NULL`` on error.


.. function:: Ez_image *ez_image_rotate (Ez_image *img, double theta, int quality)

   Compute a rotation of the source image ``img`` for angle ``theta``, in degrees.
   Return a new image whose size is adjusted to contain the result, or ``NULL``
   on error.

   In the resulting image, the field ``has_alpha`` is set to 1, and the parts
   not coming from the source image are transparent, in such a manner that they
   will not appear when displaying.

   If ``quality = 1``, the function smooths the result (with a bi-linear
   interpolation); if ``quality = 0``, the function focuses on speed (using a 
   closest neighbour computation), which saves a factor about 3.

Note : 
   the result being independent from the rotation center, it is not required
   as a parameter; however you can separately choose a rotation center and
   compute its coordinates in the destination image, thanks to the
   following function:

.. function:: void ez_image_rotate_point (Ez_image *img, double theta, \
                  int src_x, int src_y, int *dst_x, int *dst_y)

   Compute for a point having coordinates ``src_x,src_y`` in the source image,
   the corresponding coordinates ``dst_x,dst_y`` of the point in the 
   destination image.


The example demo-16.c_ illustrates rotations, with or without transparency.
The rotation center (red cross) is movable with the arrow keys. You can
even modify quality.

These windows are obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-16-1.png
      :alt: demo-16-1
   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-16-2.png
      :alt: demo-16-2
   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-16-3.png
      :alt: demo-16-3


.. ############################################################################

.. index:: seealso: Image; Pixmap

.. _sec-ref-pixmaps:

----------------------
Speed up image display
----------------------

We have seen in the previous sections the type :type:`Ez_image` defined
in ez-image.h_.

This type is convenient for loading, transforming and displaying images.
However, displaying an image takes a few milliseconds to a few tens 
of milliseconds, this duration varying with the image size and computer power.

The reason is that for each display, :func:`ez_image_paint` computes again
the whole conversion from the type :type:`Ez_image`, to an intermediate
image in memory, applies the transparency if needed, sends the result
to the graphic card, which finally displays it.

We can trace the duration of operations in the terminal, by setting an
environment variable then running one of the demonstration programs:
on Unix, type

.. code-block:: console

    export EZ_IMAGE_DEBUG=1

or on Windows, type:

.. code-block:: console

    set EZ_IMAGE_DEBUG=1


We obtain this for instance on Unix: 

.. code-block:: console

    $ ./demo-14
    ez_image_load  file "images/paper1.jpg"  in 8.725 ms  w = 640  h = 480  n = 3  has_alpha = 0
    ez_image_load  file "images/tux1.png"  in 1.946 ms  w = 210  h = 214  n = 4  has_alpha = 1
    ez_xi_create  w = 640  h = 480  depth = 24  bpp = 32
    ez_xi_fill_24 2.875 ms
    ez_xi_create  w = 210  h = 214  depth = 24  bpp = 32
    ez_xi_fill_24 0.132 ms
    ez_xmask_create   fill 0.119 ms   bitmap 5.610 ms


To cancel this environment variable on Unix, type:

.. code-block:: console

    unset EZ_IMAGE_DEBUG

or on Windows, type:

.. code-block:: console

    set EZ_IMAGE_DEBUG=


.. index:: Pixmap

In an animation, all these times add up, and when there are a lot of images,
the animation may be jerky.
The solution is simple: convert the image of type :type:`Ez_image`
as a pixmap of type :type:`Ez_pixmap`, then display the pixmap.


.. type:: typedef struct Ez_pixmap

   A pixmap is an image already converted and stored in the graphic card.
   Its display is way faster, and also relieves the processor of the machine.

   Once created, a pixmap is immutable.


This solution is also interesting to display a background image
(which size is often large). In this case, we can also gain efficiency by
using an image without alpha channel.


The type :type:`Ez_pixmap` is defined as:

.. code-block:: c

    typedef struct {
        int width, height;
        /* other fields private */
    } Ez_pixmap


The following functions handle pixmaps:


.. function:: Ez_pixmap *ez_pixmap_create_from_image (Ez_image *img)

   Create a pixmap from an image ``img``.
   The pixmap keeps the image transparency.
   The image can then be freed if no longer needed.

   Return the new pixmap, or ``NULL`` on error.


.. function:: void ez_pixmap_destroy (Ez_pixmap *pix)

   Delete the pixmap ``pix``.

   All pixmaps must be freed by this function.


.. function:: void ez_pixmap_paint (Ez_window win, Ez_pixmap *pix, int x, int y)

   Display the pixmap ``pix`` in the window ``win``.

   The top left corner of the pixmap is displayed at the ``x,y`` coordinates
   in the window.


.. function:: void ez_pixmap_tile (Ez_window win, Ez_pixmap *pix, int x, int y, int w, int h)

   Display the pixmap ``pix`` repeatedly in the window ``win``.

   The pixmap is displayed as a wallpaper in the window region bounded
   by coordinates ``x,y`` (top left corner) and
   ``x+w-1,y+h-1`` (bottom right corner).


The example demo-17.c_ allows to check the display speed, measured in fps 
(*frame per second*) in an animation.
Use keys ``+`` and ``-`` to change the number of balls,
and key ``p`` to enable or disable the use of pixmaps.

This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-17.png
      :alt: demo-17


.. ############################################################################

.. _sec-ref-misc:

-------------
Miscellaneous
-------------

.. index:: Random number

.. function:: int ez_random (int n)

   Return a random integer between 0 and ``n-1``.


Note: the random generator is initialized by :func:`ez_init`.


.. index:: Timer; Measuring time

.. function:: double ez_get_time ()

   Return the time elapsed since the *Epoch* (1970, january the 1st at 0:00:00) in
   seconds, with a precision in microseconds.

This function is usefull for measuring the duration of computations: just call
:func:`ez_get_time` before and after the computation, and then display the 
difference:

.. code-block:: c

    double t1, t2;
    t1 = ez_get_time ();
    my_computation ();
    t2 = ez_get_time ();
    printf ("Duration: %.6f s\n", t2-t1)


