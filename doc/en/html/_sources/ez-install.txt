
.. _chap-install:

============
Installation
============


Learn how to install EZ-Draw on GNU/Linux (Ubuntu, Debian, Fedora, etc.), 
on Mac OS X or Windows, all versions.


.. ############################################################################

----------------
Ubuntu or Debian
----------------

First install the development packages:
open a terminal and type (or copy-paste):

.. code-block:: console

   sudo apt-get install libx11-dev x11proto-xext-dev libxext-dev


Download the file |EZ-Draw-x.y.tgz|_ with your web browser,
save the file, then move it into your home directory using the file manager.
Next, type in the terminal:

.. parsed-literal::

   tar xvfz |EZ-Draw-x.y.tgz|
   cd |EZ-Draw-x.y|
   make -j all

After a few moments, modules EZ-Draw, demos and games are compiled. 
To run ``demo-01``, type in the terminal:

.. code-block:: console

   ./demo-01

and so on for the other demos and games
(for instance ``./jeu-bubblet`` or ``./jeu-doddle``).
Next, read the tutorial_.


.. ############################################################################

------
Fedora
------

First install the development packages:
open a terminal and type (or copy-paste):

.. code-block:: console

   sudo yum install gcc libX11-devel libXext-devel xorg-x11-fonts-misc

then type ``y`` to validate.

Download the file |EZ-Draw-x.y.tgz|_ with your web browser,
save the file, then move it into your home directory using the file manager.
Next, type in the terminal:

.. parsed-literal::

   tar xvfz |EZ-Draw-x.y.tgz|
   cd |EZ-Draw-x.y|
   make -j all

After a few moments, modules EZ-Draw, demos and games are compiled. 
To run ``demo-01``, type in the terminal:

.. code-block:: console

   ./demo-01

and so on for the other demos and games
(for instance ``./jeu-bubblet`` or ``./jeu-doddle``).
Next, read the tutorial_.


.. ############################################################################

--------
Mac OS X
--------

First install ``XCode`` from https://developer.apple.com/xcode/download/ using
the App Store. 
This is the development environment of Mac OS X, which includes in particular
``gcc`` and ``make``.

Then install ``Xquartz`` from http://www.xquartz.org/ , which provides the
X11 drawing libraries, necessary for EZ-Draw to work.

Download the file |EZ-Draw-x.y.tgz|_ with your web browser,
save the file; using the Finder, unpack it as |pre-EZ-Draw-x.y.tar|,
next as folder |pre-EZ-Draw-x.y|, then move this folder into your
home directory.

Note:
    if your Finder is not able to unpack, open a terminal and type this:

    .. parsed-literal::

       tar xvfz Downloads/|EZ-Draw-x.y|.tgz


Next type in a terminal:

.. parsed-literal::

   cd |EZ-Draw-x.y|
   make -j all


Note:
   If you get an error at compile time, for instance

   .. code-block:: console

      ./ez-draw.h:36:10: fatal error: 'X11/Xlib.h' file not found

   this means that you must give some paths in the Makefile_.
   You can use the script ``x11-paths.sh`` that comes with EZ-Draw; 
   this script explores the system directories and
   give you some informations. Simply type this in the terminal and wait:

   .. code-block:: console

      ./x11-paths.sh

   next, modify the Makefile_ in section A according to the instructions 
   displayed by the script.

If everything is ok, after a few moments, the modules EZ-Draw, demos and games 
are compiled. To run ``demo-01``, type in the terminal:

.. code-block:: console

   ./demo-01

and so on for the other demos and games
(for instance ``./jeu-bubblet`` or ``./jeu-doddle``), 
or double-click the icons in the Finder.
Next, read the tutorial_.

Note:
    the first run, the program takes some time to launch because the 
    system must first start an X11 server.
    On some versions of Mac OS X, it might be even necessary to 
    open an ``xterm`` before.


.. ############################################################################

------------
Other Unices
------------

Download the file |EZ-Draw-x.y.tgz|_ with your web browser,
save the file, then move it into your home directory using the file manager.
Next, type in the terminal:

.. parsed-literal::

   gunzip |EZ-Draw-x.y|.tgz
   tar xvf |EZ-Draw-x.y|.tar
   cd |EZ-Draw-x.y|
   make all

Note: 
   in case of errors at compile time, make sure you have on your system:

   * ``bash``
   * ``gcc`` and development files ``.h``
   * ``make`` (GNU version)
   * ``X11`` development version:
     check for ``Xlib.h`` (probably in ``/usr/include/X11``),
     for ``libX11.so`` (probably in ``/usr/X11R6/lib`` or ``/usr/lib``),
     and for ``libXext.so`` (same place).

   To check all of this you can use the script ``x11-paths.sh`` 
   that comes with EZ-Draw; this script explores the system directories and
   give you some informations. Simply type this in the terminal and wait:

   .. code-block:: console

      ./x11-paths.sh


If everything is ok, after a few moments, the modules EZ-Draw, demos and games 
are compiled. To run ``demo-01``, type in the terminal:

.. code-block:: console

   ./demo-01

and so on for the other demos and games
(for instance ``./jeu-bubblet`` or ``./jeu-doddle``).
Next, read the tutorial_.


.. ############################################################################

.. _sec-install-windows:

-------
Windows
-------

Download the file |EZ-Draw-x.y.tgz|_ with your web browser and
save the file.

Next, uncompress the file, for example with the free software ``7-Zip``:

#. get 7-Zip on http://www.7-zip.org then install it;
#. in the file manager, right-click on 
   |pre-EZ-Draw-x.y.tgz|, menu :menuselection:`7-Zip --> Extract here`
   then again on |pre-EZ-Draw-x.y.tar|. 
#. You can then leave this folder on the desktop, or drag it
   in "My Documents" for example.

Install ``MinGW`` ("Minimal GNU for Windows") to get ``gcc`` and GNU ``make`` : 

#. go to: http://sourceforge.net/projects/tdm-gcc/ ;
#. click on "Download tdm-gcc-xxx.ex", wait, then click on 
   "Save file".
#. Double-click on the file, then click on "Run": 
   the installation window appears. 
#. Click on "Create", then "MinGW/TDM (32-bit)", "Next". 
#. Leave "Installation Directory" as ``c:\MinGW32``, click on "Next".
#. Select a mirror, for example "[Europe - France] Free France", then "Next".
#. Select the installation "TDM-GCC Recommended, C/C++"
   then click on "Install".
#. When the installation is "Completed successfully", click "Next" then "Finish".
#. If a terminal was open, close it now.

Open a new terminal: 

* On Windows 8, bring up the sidebar to the right of the screen, 
  click on "Search", then "Command Prompt";
* On Windows 7 or earlier versions, click on
  :menuselection:`Start --> All programs --> Accessories --> Command Prompt`.

Still in your terminal, go into the directory |pre-EZ-Draw-x.y| by
typing : 

.. parsed-literal::

   cd |c-chemin-EZ-Draw-x.y|

replacing |pre-c-chemin-EZ-Draw-x.y| by the actual path. To know,
just drag the folder icon |pre-EZ-Draw-x.y| in the terminal.

Compile by typing: 

.. code-block:: console

   make.bat all


Note:
   If the compilation fails and you get the error message
   "The specified path was not found", you must change the ``PATH``, by
   typing in the terminal:

   .. code-block:: console

      set PATH=c:\MinGW32\bin;%PATH%

After a few moments, modules EZ-Draw, demos and games are compiled. 
To run ``demo-01``, type in the terminal:

.. code-block:: console

   demo-01

and so on for the other demos and games
(for instance ``jeu-bubblet`` or ``jeu-doddle``).

Next, read the tutorial_.
To edit examples, install GVim for instance, see 
http://www.vim.org .

Note: 
    I did not test other compiler.
    If it is your case, thanks to share the informations with me so 
    I can complete this document.


.. ############################################################################

------
Cygwin
------

You can install EZ-Draw in two ways:

a) By using the native Windows libraries. You just have to follow the
   instructions in section :ref:`sec-install-windows`.
   The key point is to not use the Cygwin terminal, but the Windows terminal
   ("Command Prompt" or ``cmd``).
..
b) By using the Unix libraries of Cygwin. It is a little more complicated and
   longer; here are the steps to follow.


If Cygwin is not already installed on your computer, this is time to do it.
Go on http://cygwin.com/ then download the "setup" program ``setup-x86.exe`` 
(on Windows 32-bit) or ``setup-x86_64.exe`` (64-bit). 
Follow the instructions; you can keep all the default settings.
The setup will download all the basic packages and install them
(this may take hours depending on your connexion ... but once for all).


We must now install some additional packages for EZ-Draw.
Restart the Cygwin setup then, after the usual setup steps,
select the following packages for installation:

- :menuselection:`Devel --> gcc-core`
- :menuselection:`Devel --> make`
- :menuselection:`Editors --> nedit`
- :menuselection:`Libs --> libX11-devel`
- :menuselection:`X11 --> libXext-devel`
- :menuselection:`X11 --> xorg-server`
- :menuselection:`X11 --> xinit`
- :menuselection:`X11 --> xterm`
- :menuselection:`X11 --> font-sony-misc`

Open the Cygwin terminal (there is an icon on the desktop) then type

.. code-block:: console

   startxwin &

This starts the X11 server (that is to say, the graphic mode of Unix)
and open a new terminal "XWin". All the next commands will have to be typed
in this terminal.

Note:
    if ``startxwin`` displays ``Another X server instance is running on
    display :0``, type ``ps``, get the number at the beginning of the line
    where stands ``/usr/bin/XWin``,  then type ``kill <pid>`` by replacing 
    ``<pid>`` by the number; you can finally type again ``startxwin &``

Download the file |EZ-Draw-x.y.tgz|_ with your web browser, save the file, 
then move it into your Cygwin home directory using the file manager.
This directory is like to be ``c:\cygwin32\home\<your-login>`` or
``c:\cygwin64\home\<your-login>``.

Type ``ls`` in the XWin terminal to check if you actually have the
file |pre-EZ-Draw-x.y.tgz| in your Cygwin home directory.
Type next:

.. parsed-literal::

   tar xvfz |EZ-Draw-x.y.tgz|
   cd |EZ-Draw-x.y|

We can now compile EZ-Draw: type

.. code-block:: console

   make -j all

After a few moments, modules EZ-Draw, demos and games are compiled. 
To run ``demo-01``, type in the terminal:

.. code-block:: console

   ./demo-01

If you get the message ``ez_init: XOpenDisplay failed for ""``, then you used 
the wrong terminal; remember, you must type the commands in the XWin terminal.

Do the same to run the other demos and games
(for instance ``./jeu-bubblet`` or ``./jeu-doddle``).
Next, read the tutorial_.


.. ############################################################################

-------------------
Tested Environments
-------------------

Tested on:

- Linux Debian and Ubuntu on i86 (intel 32 bits)
- Linux Ubuntu on x86_64 (intel Xeon/AMD, 64 bits)
- Linux Raspbian on Raspberry Pi
- Linux Xandros on Eee-PC
- Linux Fedora 20 on x86_64 with Virtualbox
- MacOS X 10.9, 6, 5, 4 on Mac (intel 32 bits)
- Solaris on Sun
- Windows XP on PC and on VirtualBox
- Windows Vista 64 bits with MinGW 32 bits
- Windows 7 32 bits on PC and on VirtualBox
- Windows 8 32 bits on Virtualbox
- Windows 8.1 64 bits with MinGW 32/64, on PC and on Virtualbox
- Cygwin 64 bits 1.7.28 on Windows 8.1 64 bits with Virtualbox


.. ############################################################################

-------
Changes
-------

1.2 : *october 2013, april 2014, april 2016, june 2016*

- rewrite Windows timers
- rewrite documentation using `Sphinx <http://sphinx-doc.org/index.html>`_
- translate whole project in english
- rewrite games
- integration of Doodle game by Julien Prudhomme
- add game 2048
- add ez_window_get_id()
- add install for: fedora, cygwin
- suppress Makefile.win
- autodetect Cygwin in Makefile
- jeu-tetris.c

1.1 : *april-june 2013*

- ez-image.c : integration of Benoit Favre's code and stb_image.c
- display images in windows using AlphaBlend() and -lmsimg32
- add ez_get_time, ez_HSV_to_RGB, ez_get_HSV
- update tutorial
- renumbering demos 0..8b -> 1..10, rewriting
- new demos 11..17
- update snapshots
- remplace DEBUG by environment variables
- add pixmaps for display speed
- Window -> Ez_window

1.0 : *june-october 2011*

- split demo8a/demo8b ; update tutorial
- improve jeu-sudoku, jeu-nim
- rewrite demo5, demo7
- ez_error, ez_error_handler
- ez_win_destroy, ez_win_destroy_all
- games : taquin, bubblet, ezen, heziom, tangram
- snapshots : web page, tutorial

1.0-rc5 : *june 2010*

- client-data : ez_set_data, ez_get_data 
- jeu-sudoku.c : MVC pattern, space for resolution
- store windows informations with struct Ez_win_info
- windows list, delayed show
- ez_state : check order and unicity of calls ez_init, ez_main_loop
- meets -Wall -W -ansi -pedantic and -std=c99

0.9 : *jully 2009*

- index in xhtml 1.0 strict + css 2.1

0.9 : *april - may 2009*

- ez_random, ez_set_thick, ez_draw_triangle, ez_fill_triangle
- ez_window_show
- keys ctrl and alt on windows (to improve)
- update : demo2.c, demo3.c, demo6.c, tutorial
- jeu-vie.c
- Birth of EZ-Draw-GTK

0.8 : *march - april 2009*

- Manage timers, demo8.c
- Add a timer in jeu-nim.c
- 3D maze with Z-buffer and animations (jeu-laby.c)

0.7 : *march 2009*

- Double display buffer for X11 and Windows 
- On X11, compile everything with: -lX11 -lXext
- game of Nim (jeu-nim.c)
- Birth of EZ-Draw++ for C++, by Eric REMY

0.6 : *june-jully 2009, correction in january 2009*

- Adaptation to Windows

0.2 .. 0.5 : *may 2008*

- X11 only

0.1 : *may 2008*

- birth of the project for X11


.. ############################################################################

------------
Contributors
------------

- Regis Barbanchon : colors management; 
  `jeu-ezen.c <../../../jeu-ezen.c>`_,
  `jeu-heziom.c <../../../jeu-heziom.c>`_.

- Eric Remy : terminating design, handling of error messages.

- Benoit Favre : display images and interface with
  `stb_image.c <http://www.nothings.org/stb_image.c>`_.

- Julien Prudhomme : 
  `jeu-doodle.c <../../../jeu-doodle.c>`_,
  `jeu-tetris.c <../../../jeu-tetris.c>`_.


