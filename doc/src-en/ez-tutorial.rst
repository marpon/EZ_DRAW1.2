
.. _chap-tutorial:

========
Tutorial
========


The easiest way to proceed is to work directly in the EZ-Draw directory,
starting from an example, and to complete the provided Makefile_,
see section :ref:`sec-tut-compilation`.



.. ############################################################################

.. index:: Main loop; Example

.. _sec-tut-first-prog:

---------------------------
First program with a window
---------------------------


Let us write our first program that opens a window, and name it ``demo-01.c``.
We first include ez-draw.h_ (line 1, see below):
this file defines the types and prototypes of the main EZ-Draw module, 
and it also includes the standard headers such as ``<stdio.h>``, ``<stdlib.h>``, 
``<string.h>``, so we don't need to worry about.

In ``main``, we initialize the module and the graphic mode by calling
:func:`ez_init`; if the initialization fails, the function print
an error message in the terminal, then return -1.
In this case, we must terminate the program by ``exit(1)`` .

Next we create one (or several) window(s) using function
:func:`ez_window_create`. These windows are displayed when the program
reaches :func:`ez_main_loop`: that is this function which "gives life" to the
windows. It stops when one calls :func:`ez_quit`, or when all windows are
destroyed.


Here is the file demo-01.c_ :

.. literalinclude:: ../../demo-01.c
   :language: c
   :linenos:
   :lines: 13-


This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-01.png
      :alt: demo-01


.. ############################################################################

.. index:: Compilation; Using the basic module

.. _sec-tut-compilation:

-----------
Compilation
-----------

To compile ``demo-01.c`` in a terminal on Unix, type

.. code-block:: console

    gcc -Wall demo-01.c ez-draw.c -o demo-01 -lX11 -lXext

or on Windows, type 

.. code-block:: console

    gcc -Wall demo-01.c ez-draw.c -o demo-01.exe -lgdi32


To run the program in a terminal on Unix, type

.. code-block:: console

    ./demo-01

or on Windows, type

.. code-block:: console

    demo-01


.. index:: Compilation; Using Makefile

|bullet| It is more handy to use the command ``make`` instead of typing over 
and over the ``gcc`` command line.
The command ``make`` use the same Makefile_ on Unix and on Windows.

To compile everything the first time, or to compile again the modified C files,
it is sufficient to type in the terminal, whichever your system is:

.. code-block:: console

    make all


On error, see chapter Installation_.
If everything is up to date, ``make`` will display: 
``make: Nothing to be done for `all'.``
You can always force a general re-compilation by typing:

.. code-block:: console

    make clean all

Finally if you want to clean your directory, for instance before making
a backup, type:

.. code-block:: console

    make distclean

This will erase all executable and temporary files, and will keep only the
source files.

*Exercise :*
    Edit the file ``demo-01.c``, and change the title with
    ``"My first program"``. Compile with ``make`` then run the program.


|bullet| Let us see now how to add a new program. Just edit the file
Makefile_ , and add in section C the name of the executable in the list 
``EXECS``, ``EXECS_M`` or ``EXECS_IM``, then save.
We can then type ``make all`` to compile the new program.
There are several lists to distinguish between cases:

- ``EXECS`` : to compile only with ez-draw.c_ ;
- ``EXECS_M`` : to compile with ez-draw.c_ and ``-lm`` (the math library);
- ``EXECS_IM`` : to compile with ez-draw.c_, ``-lm`` and ez-image.c_ (to display images).

Here are the current lists in the Makefile_ :

.. literalinclude:: ../../Makefile
   :language: makefile
   :lines: 34-40

As you can see, when a list is on several lines, the intermediary lines
are ended by a ``\``.


*Exercise :*
    Copy the file ``demo-01.c`` as ``trial1.c``, and add
    ``trial1`` in the Makefile_. 
    Compile with ``make`` then run the program.


|bullet| You can use the provided Makefile_ for more advanced projects
divided in several modules. Suppose that your executable is named ``myappli``,
that your project uses the modules ``ez-draw.c``, ``ez-image.c``, ``myprog.c``
and ``misc.c``, and that you need the libraries ``-lm`` and ``-lgomp``.
You just have to complete the fields ``EXECS_PRO``, ``OBJS_PRO`` and
``LIBS_PRO``  in the Makefile_ section D like that:

.. code-block:: makefile

   EXECS_PRO = myappli
   OBJS_PRO  = ez-draw.o ez-image.o myprog.o misc.o
   LIBS_PRO  = -lm -lgomp


|bullet| Finally, if you want to create your project in a new directory,
here are the files that you have to copy in it:

- ez-draw.h_ and ez-draw.c_ (it is the basic module), 
- ez-image.h_ and ez-image.c_ (if you plan to display images), 
- Makefile_ and make.bat_.

Just clear the ``EXECS`` lists in the Makefile_ and fill the necessary fields.


.. ############################################################################

.. index:: Event; Event handler

.. _sec-tut-event-handler:

-------------
Event handler
-------------


We will now take a look at the handling of events in the windows.

The parameters of the function :func:`ez_window_create` which is
responsible of a window creation are:

.. code-block:: c

    Ez_window ez_window_create (int w, int h, const char *name, Ez_func on_event);

``w`` is the width of the interior of the window in pixels,
``h`` is the height, 
``name`` is the window title;
``on_event`` is the event handler of the window, see below.

The result of :func:`ez_window_create` has type :type:`Ez_window`; 
it is used to identify the window, and can be printed in the terminal
as an hexadecimal value:

.. code-block:: c

    Ez_window win1;
    win1 = ez_window_create (400, 300, "Demo 0: Hello World", NULL);
    printf ("win1 = 0x%x\n", ez_window_get_id(win1));


To be able to interact with the user (key press and release, mouse moves and
click, etc) we have to realize a so-called *event handling*;
that is why we give the function ``on_event`` as a fourth parameter of
:func:`ez_window_create`.

.. index:: Callback; Example

The event handler ``on_event`` (also named *callback*), is a function of
your program (or ``NULL`` as in example ``demo-01``). 
This function will be automatically called by :func:`ez_main_loop` 
for each event that involves the window.

The function ``on_event`` has the following prototype:

.. code-block:: c

    void on_event (Ez_event *ev);

The parameter ``ev`` is the address of a struct whose fields describe
the event; in particular, ``ev->win`` tells which window is involved.
The other fields are detailed in section :ref:`sec-tut-all-events`.

We study two events in the following example demo-02.c_ :

* The window manager tells to your program if the windows must be redrawn
  (the first time they appear, when another window is passing ahead, etc).
  When this happens, an ``Expose`` event is generated and the event handler
  is called. At this time you have to redraw the whole content of the window 
  ``ev->win``.
..
* When the user press a key, the ``KeyPress`` event is generated. 
  The key code is available in ``ev->key_sym`` (for *key symbol*). 
  Each key code is expressed as a constant prefixed by ``XK_``, 
  for instance ``XK_q`` stands for the key "q".


.. literalinclude:: ../../demo-02.c
   :language: c
   :linenos:
   :lines: 13-


This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-02.png
      :alt: demo-02


This example demo-02.c_ is a first attempt to manage the events, using a big
``switch`` into ``win1_on_event``. 
The drawback of this method is the rapid growing of the ``switch`` when
enhancing the program, leading the program potentially unreadable.
That is why it is better to split ``win1_on_event`` in functions 
(still using a ``switch``), 
each of them being specialized for an event; we do it in the next section.


.. ############################################################################

.. index:: Drawing; Example

.. _sec-tut-drawings-colors:

-------------------
Drawings and colors
-------------------


As explained in the previous section, the ``Expose`` event
means that the whole content of the windows must be redrawn.
In the next example we redraw by calling ``win1_on_expose``. 
Note: for each ``Expose`` event, EZ-Draw empties the window 
(with a white background) before passing the event to your program.

The list of drawings is given in section :ref:`sec-ref-drawings`.
The coordinates are relative to the Origin, which is the top left
corner inside the window; ``x`` goes to the right and ``y`` goes down.

The drawings are automatically cut by the window border, so that there
is no need to worry if a drawing fits or not.

The drawings are performed using the current thickness (1 pixel by default).
you can change the thickness with  :func:`ez_set_thick`,
see :ref:`sec-ref-drawings`. 

The drawings are done in the current color (black by default).
To change color, call  :func:`ez_set_color` by giving it the color number.
Some colors are predefined:
``ez_black``, ``ez_white``, ``ez_grey``, ``ez_red``, ``ez_green``, ``ez_blue``, 
``ez_yellow``, ``ez_cyan``, ``ez_magenta``.
It is possible to create other colors, see :ref:`sec-ref-colors`.


Here is the file demo-03.c_ :

.. literalinclude:: ../../demo-03.c
   :language: c
   :linenos:
   :lines: 13-


This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-03.png
      :alt: demo-03


.. ############################################################################

.. index:: Text; Example

.. _sec-tut-displaying-text:

---------------
Displaying text
---------------


It is possible to draw text at any place in the window, thanks to
the function :func:`ez_draw_text`. It takes as arguments: the window,
the kind of alignment ``align``, the coordinates ``x1,y1``, 
finally a string to draw, or as in ``printf``, a format and parameters.
Everything is detailed in section :ref:`sec-ref-fonts`.

The string can contain some ``\n``, causing line breaks in the display.
The text drawing is performed in the current color, modifiable by
:func:`ez_set_color`.

In the next example demo-04.c_ we illustrate this, as well as the use of
:func:`ez_window_get_size` to kindly adapt the drawing to the window
size changes.

.. literalinclude:: ../../demo-04.c
   :language: c
   :linenos:
   :lines: 13-


This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-04.png
      :alt: demo-04


.. ############################################################################

.. index:: Event; Tracing events, Close Button; Example

.. _sec-tut-all-events:

--------------
Tracing events
--------------


In the next example we list all possible events, and we trace in the terminal
the ``ev`` variable fields that are usable (the other ones are set to 0).
See also section :ref:`sec-ref-events`.

By default, the "Close" button in the title bar of one of the application
windows causes the termination of the program. We can change this behavior
by calling  :func:`ez_auto_quit(0) <ez_auto_quit>` : 
from now on, the "Close" button will cause a ``WindowClose`` event, as in the
following example.


Here is the file demo-05.c_ :

.. literalinclude:: ../../demo-05.c
   :language: c
   :linenos:
   :lines: 13-


This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-05.png
      :alt: demo-05


Note: the ``TimerNotify`` events are not treated here, see  :ref:`sec-ref-timers`.


.. ############################################################################

.. index:: Mouse; Drawing with the mouse

.. _sec-tut-drawing-with-mouse:

----------------------
Drawing with the mouse
----------------------


The example shown in this section allows to draw a polygonal line with
the mouse. The coordinates of the vertices are stored in global variables
(we could also avoid global variables, see :ref:`sec-ref-client-data`). 
Each time the mouse button is clicked, a new vertex is inserted;
for each mouse move with a pressed button (drag), the last vertex is moved.

By principle (and for technical reasons), the drawings must only be done
for the ``Expose`` event. If you want to update the drawings of a window for another
event, it is sufficient to send an ``Expose`` event using the function
:func:`ez_send_expose`.
We do this here, for ``ButtonPress``, ``MotionNotify`` and ``KeyPress``
events.


Here is the file demo-06.c_ :

.. literalinclude:: ../../demo-06.c
   :language: c
   :linenos:
   :lines: 13-


This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-06.png
      :alt: demo-06


.. ############################################################################

.. index:: Window; Managing several windows

.. _sec-tut-several-windows:

------------------------
Managing several windows
------------------------


You can create as much windows that you want, by calling
:func:`ez_window_create`. Each window that is created is immediately displayed
on top of the existing windows.
To destroy a window ``win`` (and thus hide it from screen),
use :func:`ez_window_destroy(win) <ez_window_destroy>`.

It is possible to hide a window  ``win`` (it still exists but is not displayed) by calling
:func:`ez_window_show(win, 0) <ez_window_show>`, then show it again (on top of the
other ones) by calling :func:`ez_window_show(win, 1) <ez_window_show>`.

In the next example, the main loop is configured by 
:func:`ez_auto_quit(0) <ez_auto_quit>`, 
which means that pressing the "Close" button of the window title bar will not end the
program, but send in place a ``WindowClose`` event.
According to the window involved, we hide the window, destroy the window
or exit the program.

Here is the file demo-07.c_ :

.. literalinclude:: ../../demo-07.c
   :language: c
   :linenos:
   :lines: 13-


This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-07.png
      :alt: demo-07


.. ############################################################################

.. index:: Text; Text input

.. _sec-tut-text-input:

----------
Text input
----------


The next example demo-08.c_ demonstrates how to read a string on keyboard,
suppress some characters using the Backspace, and detect the Enter key
to trigger an action.


.. literalinclude:: ../../demo-08.c
   :language: c
   :linenos:
   :lines: 13-


This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-08.png
      :alt: demo-08


.. ############################################################################

.. index:: Animation, Timer; Example

.. _sec-tut-animations:

----------
Animations
----------


To perform animations, two additional ingredients are needed:
a timer (see :ref:`sec-ref-timers`) to maintain the temporal sequence,
and double buffering (see :ref:`sec-ref-dbuf`) to prevent display
flashing.

Warning: 
    Never ever employ ``sleep`` or ``usleep``, because
    these functions prevent the callbacks to restore the control to :func:`ez_main_loop`, 
    and thus they freeze the display and the interface (or at least 
    severely disrupt them).

The principle of an animation is the following:

*  start a timer in ``main``, which will cause a ``TimerNotify`` event
   few milliseconds later;
..
*  upon receipt of this ``TimerNotify`` event in the window callback,
   we do three things:

   * we increment a counter (or coordinates, or whatever) in order to change the
     position of the object to animate;
   * we send an ``Expose`` event to refresh display;
   * last, we restart the timer such that there will be a next ``TimerNotify``
     event (what we obtain is a kind of timed loop of ``TimerNotify`` events);
..
*  each time an ``Expose`` event is received, we redraw the window content
   tacking account of the counter (or the coordinates) to draw the animated
   object in its current position. Never draw for another event, you would
   disturb the double display buffer.
   Moreover, EZ-Draw optimize displaying by eliminating useless  ``Expose`` 
   events.


Here is a first example demo-09.c_ below. The animation shows a growing circle
at the middle of the window; it also adapts the drawings to the window size
changes.


.. literalinclude:: ../../demo-09.c
   :language: c
   :linenos:
   :lines: 13-


This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-09.png
      :alt: demo-09


Another example is provided by demo-10.c_ to illustrate multiple animations:
in one window we turn the hands of a watch (type space for pause),
while in another window a ball is bouncing on a racket (the window can be
enlarged).


These windows are obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-10-1.png
      :alt: demo-10-1
   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-10-2.png
      :alt: demo-10-2


This example demo-10.c_ demonstrates also how to attach an information
(e.g. a  ``struct``) to a window, so as to avoid global variables.
For more explanations, see :ref:`sec-ref-client-data`. 
We use this possibility in the whole next part.


.. ############################################################################

.. index:: Image; Examples

.. _sec-tut-images:

------
Images
------


In the previous sections we have seen what we can do with the basic module.

EZ-Draw comes with a second module, ez-image.c_,
that allows to load and display color images in PNG, JPEG, GIF or BMP format,
or create an image in memory and draw into.
These possibilities are detailed in sections :ref:`sec-ref-images-type`
and following.

To use this module, just include ez-image.h_.
Here is the example demo-13.c_ where we get a file name as  argument of the
command line, then we load the image and display it:

.. literalinclude:: ../../demo-13.c
   :language: c
   :linenos:
   :lines: 13-


This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-13.png
      :alt: demo-13


.. index:: Compilation; Using the image module

To compile this file ``demo-13.c`` on Unix, type: 

.. code-block:: console

    gcc -Wall demo-13.c ez-draw.c ez-image.c -o demo-13 -lX11 -lXext -lm

or on Windows, type: 

.. code-block:: console

    gcc -Wall demo-13.c ez-draw.c ez-image.c -o demo-13.exe -lgdi32 -lmsimg32 -lm

You can also append the executable file name at the end of ``EXECS_IM =``
in the Makefile_; then type ``make all`` to compile.

The formats PNG, GIF and BMP allow to store a transparency level, in what is
called the alpha channel. The formats GIF and BMP store the alpha channel
over 1 bit; the pixels are either transparent (0), or opaque (255). 
The PNG format stores the alpha channel over 8 bits (from 0 for transparent,
to 255 for opaque).

The module ``ez-image`` is able to display an image by taking in account
the transparency, using an opacity threshold on the alpha channel:
the pixels are either opaques (displayed) or transparents (not displayed).

The next example demo-14.c_ gets two file names as arguments of the command line,
then superimpose both images.
You can move the second image using arrow keys, or change the opacity threshold
using keys ``+`` and ``-``.



.. literalinclude:: ../../demo-14.c
   :language: c
   :linenos:
   :lines: 13-


This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-14.png
      :alt: demo-14


It is also possible to create an image in memory, then set the pixels colors.
The example demo-12.c_ displays the HSV palette computed in this manner.

This window is obtained:

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-12.png
      :alt: demo-12


You will find more informations about images in the :ref:`chap-ref-manual`.


