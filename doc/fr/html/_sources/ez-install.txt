
.. _chap-install:

============
Installation
============


Découvrez comment installer EZ-Draw sur GNU/Linux (Ubuntu, Debian, Fedora, etc),
sur Mac OS X, ou sur Windows, toutes versions.


.. ############################################################################

----------------
Ubuntu ou Debian
----------------

Il faut d'abord installer les packages de développement. 
Ouvrez un terminal puis tapez (ou copiez-collez) :

.. code-block:: console

   sudo apt-get install libx11-dev x11proto-xext-dev libxext-dev


Téléchargez le fichier |EZ-Draw-x.y.tgz|_ avec votre navigateur web,
enregistrez le fichier, puis déplacez-le dans votre répertoire principal
à l'aide du gestionnaire de fichier. Tapez ensuite dans le terminal :

.. parsed-literal::

   tar xvfz |EZ-Draw-x.y.tgz|
   cd |EZ-Draw-x.y|
   make -j all

Après quelques instants, les modules de EZ-Draw, les démos et les jeux sont
compilés. Pour exécuter ``demo-01``, tapez dans le terminal :

.. code-block:: console

   ./demo-01

et ainsi de suite pour les autres démos et les jeux
(par exemple ``./jeu-bubblet`` ou ``./jeu-doddle``).
Lisez ensuite le tutorial_.


.. ############################################################################

------
Fedora
------

Il faut d'abord installer les packages de développement. 
Ouvrez un terminal puis tapez (ou copiez-collez) :

.. code-block:: console

   sudo yum install gcc libX11-devel libXext-devel xorg-x11-fonts-misc

puis validez en tapant ``y``.

Téléchargez le fichier |EZ-Draw-x.y.tgz|_ avec votre navigateur web,
enregistrez le fichier, puis déplacez-le dans votre répertoire principal
à l'aide du gestionnaire de fichier. Tapez ensuite dans le terminal :

.. parsed-literal::

   tar xvfz |EZ-Draw-x.y.tgz|
   cd |EZ-Draw-x.y|
   make -j all

Après quelques instants, les modules de EZ-Draw, les démos et les jeux sont
compilés. Pour exécuter ``demo-01``, tapez dans le terminal :

.. code-block:: console

   ./demo-01

et ainsi de suite pour les autres démos et les jeux
(par exemple ``./jeu-bubblet`` ou ``./jeu-doddle``).
Lisez ensuite le tutorial_.


.. ############################################################################

--------
Mac OS X
--------

Installez d'abord ``XCode`` depuis https://developer.apple.com/xcode/download/
via l'App Store. 
Il s'agit de l'environnement de développement de Mac OS X, qui inclut 
en particulier ``gcc`` et ``make``.

Installez ensuite ``Xquartz`` depuis http://www.xquartz.org/ . Ce logiciel fournit
les bibliothèques X11 d'affichage, qui sont nécessaires pour EZ-Draw.

Téléchargez le fichier |EZ-Draw-x.y.tgz|_ avec votre navigateur web,
enregistrez le fichier ; à l'aide du Finder, décompactez le fichier
en |pre-EZ-Draw-x.y.tar|, puis comme dossier |pre-EZ-Draw-x.y| ;
enfin déplacez ce dossier dans votre répertoire principal.

Note:
    si votre Finder n'arrive pas à décompacter le fichier, ouvrez un
    terminal et tapez ceci :

    .. parsed-literal::

       tar xvfz Downloads/|EZ-Draw-x.y|.tgz

Tapez ensuite dans un terminal :

.. parsed-literal::

   cd |EZ-Draw-x.y|
   make -j all

Remarque :
   Si vous obtenez une erreur à la compilation, par exemple

   .. code-block:: console

      ./ez-draw.h:36:10: fatal error: 'X11/Xlib.h' file not found

   cela signifie qu'il faut préciser des chemins dans le Makefile_.
   Vous pouvez utiliser le script ``x11-paths.sh`` fourni avec EZ-Draw ; 
   ce script explore les répertoires système et vous donne des informations. 
   Tapez simplement ceci dans le terminal et patientez :

   .. code-block:: console

      ./x11-paths.sh

   puis modifiez le Makefile_ section A selon les instructions affichées 
   par le script.


Si tout est correct, après quelques instants, les modules de EZ-Draw, 
les démos et les jeux sont compilés. Pour exécuter ``demo-01``, tapez dans 
un terminal :

.. code-block:: console

   ./demo-01

et ainsi de suite pour les autres démos et les jeux
(par exemple ``./jeu-bubblet`` ou ``./jeu-doddle``), 
ou bien double-cliquez sur les icones dans le finder.
Lisez ensuite le tutorial_.

Remarque :
    à la première exécution, le programme met un certain temps pour
    se lancer car le système doit d'abord démarrer un serveur X11.
    Sur certaines versions de Mac OS X il peut même être nécessaire
    d'ouvrir auparavant un ``xterm``.


.. ############################################################################

-----------
Autres Unix
-----------

Téléchargez le fichier |EZ-Draw-x.y.tgz|_ avec votre navigateur web,
enregistrez le fichier, puis déplacez-le dans votre répertoire principal
à l'aide du gestionnaire de fichier. Tapez ensuite dans le terminal :

.. parsed-literal::

   gunzip |EZ-Draw-x.y|.tgz
   tar xvf |EZ-Draw-x.y|.tar
   cd |EZ-Draw-x.y|
   make all

Remarque : 
   en cas d'erreur à la compilation, vérifiez que vous disposez sur votre 
   système de :

   * ``bash``
   * ``gcc`` et les fichiers de développement ``.h``
   * ``make`` (version GNU)
   * ``X11`` version de développement :
     vérifiez la présence de ``Xlib.h`` (probablement dans ``/usr/include/X11``),
     celle de ``libX11.so`` (probablement dans ``/usr/X11R6/lib`` ou ``/usr/lib``),
     et celle de ``libXext.so`` (même endroit).

   Pour vérifier tout cela vous pouvez utiliser le script ``x11-paths.sh`` 
   fourni avec EZ-Draw ; ce script explore les répertoires système et vous 
   donne des informations. Tapez simplement ceci dans le terminal et patientez :

   .. code-block:: console

      ./x11-paths.sh


Si tout est correct, après quelques instants, les modules de EZ-Draw, 
les démos et les jeux sont compilés. 
Pour exécuter ``demo-01``, tapez dans un terminal :

.. code-block:: console

   ./demo-01

et ainsi de suite pour les autres démos et les jeux
(par exemple ``./jeu-bubblet`` ou ``./jeu-doddle``).
Lisez ensuite le tutorial_.


.. ############################################################################

.. _sec-install-windows:

-------
Windows
-------

Téléchargez le fichier |EZ-Draw-x.y.tgz|_ avec votre navigateur web,
enregistrez le fichier, puis déplacez-le dans votre répertoire principal
à l'aide du gestionnaire de fichier.

Décompressez ensuite le fichier, par exemple avec le logiciel libre ``7-Zip`` :

#. récupérez 7-Zip sur http://www.7-zip.org et installez-le ;
#. ensuite, dans l'explorateur de fichier, cliquez avec le bouton droit sur 
   |pre-EZ-Draw-x.y.tgz|, menu :menuselection:`7-Zip --> Extraire ici`
   puis de même sur |pre-EZ-Draw-x.y.tar|. 
#. Vous pouvez ensuite laisser ce dossier sur le bureau, ou le glisser dans 
   "Mes documents" par exemple.

Installez ``MinGW`` ("Minimal GNU for Windows") pour obtenir ``gcc`` et GNU ``make`` : 

#. allez dans : http://sourceforge.net/projects/tdm-gcc/ ;
#. cliquez sur "Download tdm-gcc-xxx.ex", patientez, puis sur 
   "Enregistrer le fichier".
#. Double-cliquez sur le fichier, puis cliquez sur "Exécuter" : 
   la fenêtre d'installation apparaît. 
#. Cliquez sur "Create", puis "MinGW/TDM (32-bit)", "Next". 
#. Laissez "Installation Directory" a ``c:\MinGW32``, cliquez sur "Next".
#. Sélectionnez un miroir, par exemple "[Europe - France] Free France", puis "Next".
#. Sélectionnez le type d'installation "TDM-GCC Recommended, C/C++"
   puis cliquez sur "Install".
#. Lorsque l'installation est "Completed successfully", cliquez "Next" puis "Finish".
#. Si vous aviez un terminal ouvert il faut le fermer maintenant.

Ouvrez un nouveau terminal : 

* sous Windows 8, faites apparaître la barre latérale à droite de l'écran,
  cliquez sur "Rechercher", puis sur "Invite de commandes" ;
* sous Windows 7 ou versions antérieures, cliquez dans 
  :menuselection:`Démarrer --> Tous les programmes --> Accessoires --> Invite de commandes`.

Toujours dans votre terminal, allez dans le répertoire |pre-EZ-Draw-x.y| en
tapant : 

.. parsed-literal::

   cd |c-chemin-EZ-Draw-x.y|

en remplaçant |pre-c-chemin-EZ-Draw-x.y| par le chemin réel. Pour le connaître, 
il suffit de glisser l'icone du dossier |pre-EZ-Draw-x.y| dans le terminal.

Compilez en tapant : 

.. code-block:: console

   make.bat all


Remarque :
   si la compilation ne s'effectue pas mais que vous obtenez le message d'erreur
   "Le chemin d'accès spécifié est introuvable", il faut modifier le ``PATH``, par
   exemple en tapant dans le terminal : 

   .. code-block:: console

      set PATH=c:\MinGW32\bin;%PATH%

Après quelques instants, les modules de EZ-Draw, les démos et les jeux sont
compilés. Pour exécuter ``demo-01``, tapez dans le terminal : 

.. code-block:: console

   demo-01

et ainsi de suite pour les autres démos et les jeux
(par exemple ``jeu-bubblet`` ou ``jeu-doddle``).

Lisez ensuite le tutorial_.
Pour éditer les exemples, installez par exemple GVim disponible sur
http://www.vim.org .

Remarque : 
    je n'ai pas testé d'autre compilateur. Si tel est votre cas, merci de 
    me donner les informations pour que je puisse compléter cette aide.


.. ############################################################################

------
Cygwin
------

Vous pouvez installer EZ-Draw de 2 façons :

a) En utilisant les librairies natives de Windows. Il vous suffit de suivre 
   les instructions de la section :ref:`sec-install-windows`.
   Le point important est de ne pas utiliser le terminal Cygwin, mais bien
   le terminal de Windows ("invite de commande" ou ``cmd``). 
..
b) En utilisant les librairies Unix de Cygwin. C'est un peu plus compliqué
   et plus long, voici les étapes à suivre.


Si Cygwin n'est pas déjà installé sur votre ordinateur, c'est le moment
de le faire. Allez sur http://cygwin.com/ puis téléchargez
le programme de "setup" ``setup-x86.exe`` (sur un Windows 32-bit) ou 
``setup-x86_64.exe`` (64-bit). 
Suivez ensuite les instructions ; vous pouvez conserver tous les réglages par
défaut. Le setup va télécharger tous les paquets de base et les installer
(cela peut prendre des heures selon votre connexion ... mais ce sera fait
un fois pour toutes).

Il faut installer maintenant certains paquets supplémentaires pour EZ-Draw.
Relancez le setup de Cygwin puis, après les étapes habituelles du setup,
sélectionnez les paquets suivants pour installation :

- :menuselection:`Devel --> gcc-core`
- :menuselection:`Devel --> make`
- :menuselection:`Editors --> nedit`
- :menuselection:`Libs --> libX11-devel`
- :menuselection:`X11 --> libXext-devel`
- :menuselection:`X11 --> xorg-server`
- :menuselection:`X11 --> xinit`
- :menuselection:`X11 --> xterm`
- :menuselection:`X11 --> font-sony-misc`

Ouvrez le terminal Cygwin (il doit y avoir un icone sur votre bureau)
puis tapez 

.. code-block:: console

   startxwin &

Ceci démarre le serveur X11 (c'est-à-dire le mode graphique Unix) et ouvre
un nouveau terminal "XWin". Toutes les commandes qui suivent devront être
tapées dans ce terminal. 

Remarque :
    si ``startxwin`` affiche ``Another X server instance is running on
    display :0``, tapez ``ps``, repérez le numéro au début de la ligne où 
    apparaît ``/usr/bin/XWin``,  puis tapez ``kill <pid>`` en remplaçant 
    ``<pid>`` par le numéro ; vous pouvez enfin retaper ``startxwin &``

Téléchargez le fichier |EZ-Draw-x.y.tgz|_ avec votre navigateur web,
enregistrez le fichier, puis déplacez-le dans votre répertoire principal
Cygwin à l'aide du gestionnaire de fichier ; ce répertoire est probablement
``c:\cygwin32\home\<votre-login>`` ou ``c:\cygwin64\home\<votre-login>``.

Tapez ``ls`` dans le terminal XWin pour vérifier que vous avez bien le 
fichier |pre-EZ-Draw-x.y.tgz| dans votre répertoire principal Cygwin. 
Tapez ensuite :

.. parsed-literal::

   tar xvfz |EZ-Draw-x.y.tgz|
   cd |EZ-Draw-x.y|

On peut ensuite passer à la compilation de EZ-Draw : tapez

.. code-block:: console

   make -j all

Après quelques instants, les modules de EZ-Draw, les démos et les jeux sont
compilés. Pour exécuter ``demo-01``, tapez dans le terminal :

.. code-block:: console

   ./demo-01

Si vous voyez le message ``ez_init: XOpenDisplay failed for ""``, alors vous vous êtes
trompé de terminal, c'est dans le terminal XWin qu'il faut taper les commandes.

Faites de même pour lancer les autres démos et les jeux
(par exemple ``./jeu-bubblet`` ou ``./jeu-doddle``).
Lisez ensuite le tutorial_.


.. ############################################################################

----------------------
Environnements testés
----------------------

Testé sur :

- Linux Debian et Ubuntu sur i86 (intel 32 bits)
- Linux Ubuntu sur x86_64 (intel Xeon/AMD, 64 bits)
- Linux Raspbian sur Raspberry Pi
- Linux Xandros sur Eee-PC
- Linux Fedora 20 sur x86_64 avec Virtualbox
- MacOS X 10.9, 6, 5, 4 sur Mac (intel 32 bits)
- Solaris sur Sun
- Windows XP en natif et avec VirtualBox
- Windows Vista 64 bits avec MinGW 32 bits
- Windows 7 32 bits en natif et avec VirtualBox
- Windows 8 32 bits avec Virtualbox
- Windows 8.1 64 bits avec MinGW 32/64, en natif et avec Virtualbox
- Cygwin 64 bits 1.7.28 sur Windows 8.1 64 bits avec Virtualbox


.. ############################################################################

-----------------------
Historique des versions
-----------------------

1.2 : *octobre 2013, avril 2014, avril 2016, juin 2016*

- réécriture des timers windows
- réécriture de la documentation avec `Sphinx <http://sphinx-doc.org/index.html>`_
- traduction de tout le projet en anglais
- réécriture des jeux
- intégration du jeu Doodle de Julien Prudhomme
- ajout du jeu 2048
- ajout: ez_window_get_id()
- installation pour : fedora, cygwin
- suppression de Makefile.win
- détection automatique de cygwin dans le Makefile
- jeu-tetris.c

1.1 : *avril-juin 2013*

- ez-image.c : intégration du code de Benoit Favre et de stb_image.c
- affichage images sous windows avec AlphaBlend() et -lmsimg32
- ajout ez_get_time, ez_HSV_to_RGB, ez_get_HSV
- mise à jour du tutorial
- renumérotation des démos 0..8b -> 1..10, réécriture
- nouvelles démos 11..17
- mise à jour des snapshots
- remplacement DEBUG par variables d'environnement
- ajout pixmaps pour vitesse affichage
- Window -> Ez_window

1.0 : *juin-octobre 2011*

- split demo8a/demo8b ; maj tutorial
- amélioration jeu-sudoku, jeu-nim
- réécriture demo5, demo7
- ez_error, ez_error_handler
- ez_win_destroy, ez_win_destroy_all
- jeux : taquin, bubblet, ezen, heziom, tangram
- snapshots : page web, tutorial

1.0-rc5 : *juin 2010*

- client-data : ez_set_data, ez_get_data 
- jeu-sudoku.c : patron MVC, emplacement pour résolution
- mémorisation infos fenêtres avec struct Ez_win_info
- liste de fenêtres, show différé
- ez_state : contrôle ordre et unicité appels ez_init, ez_main_loop
- respecte -Wall -W -ansi -pedantic et -std=c99

0.9 : *juillet 2009*

- index en xhtml 1.0 strict + css 2.1

0.9 : *avril - mai 2009*

- ez_random, ez_set_thick, ez_draw_triangle, ez_fill_triangle
- ez_window_show
- touches ctrl et alt sous windows (à améliorer)
- mise à jour : demo2.c, demo3.c, demo6.c, tutorial
- jeu-vie.c
- Naissance de EZ-Draw-GTK

0.8 : *mars - avril 2009*

- Gestion des timers, demo8.c
- Rajout d'un timer dans jeu-nim.c
- Labyrinthe 3D avec Z-buffer et animations (jeu-laby.c)

0.7 : *mars 2009*

- Double buffer d'affichage pour X11 et pour Windows ;
- Sous X11, dorénavant tout compiler avec : -lX11 -lXext
- Jeu de Nim (jeu-nim.c)
- Sortie de EZ-Draw++ pour le C++, par Eric REMY

0.6 : *juin-juillet 2009, puis correction janvier 2009*

- Adaptation a Windows

0.2 .. 0.5 : *mai 2008*

- Versions uniquement X11

0.1 : *mai 2008*

- Naissance du projet pour X11


.. ############################################################################

-------------
Contributeurs
-------------

- Regis Barbanchon : gestion des couleurs ; 
  `jeu-ezen.c <../../../jeu-ezen.c>`_,
  `jeu-heziom.c <../../../jeu-heziom.c>`_.

- Eric Remy : design de terminaison, de gestion des messages d'erreur.

- Benoit Favre : affichage des images et interface avec 
  `stb_image.c <http://www.nothings.org/stb_image.c>`_.

- Julien Prudhomme : 
  `jeu-doodle.c <../../../jeu-doodle.c>`_,
  `jeu-tetris.c <../../../jeu-tetris.c>`_.


