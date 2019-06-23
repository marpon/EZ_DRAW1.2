
.. _chap-tutorial:

========
Tutorial
========


La façon la plus simple de travailler est de programmer directement dans le 
répertoire de EZ-Draw en partant d'un exemple, et de compléter le Makefile_
fourni, voir section :ref:`sec-tut-compilation`.



.. ############################################################################

.. index:: Boucle principale; Exemple

.. _sec-tut-first-prog:

----------------------------------
Premier programme avec une fenêtre
----------------------------------


Écrivons un premier programme qui ouvre une fenêtre, appelé ``demo-01.c``. 
Il faut commencer par inclure ez-draw.h_ (ligne 1, ci-dessous) :
ce fichier décrit les types et prototypes du module principal de EZ-Draw, 
et il inclut aussi les fichiers ``.h`` standards tels que ``<stdio.h>``, 
``<stdlib.h>``, ``<string.h>`` ; vous n'avez donc pas à vous en occuper.

Dans le ``main``, on initialise le module et le mode graphique en appelant 
:func:`ez_init` ; si l'initialisation du mode graphique échoue, la fonction 
affiche un message d'erreur dans le terminal, puis renvoie -1.
Dans ce cas, il faut sortir du programme en faisant ``exit(1)`` .

Ensuite on crée une (ou plusieurs) fenêtre(s) avec la fonction
:func:`ez_window_create`. Ces fenêtres sont affichées quand le programme atteint 
:func:`ez_main_loop` : c'est cette fonction qui fait "vivre" les fenêtres. 
Elle s'arrête lorsqu'on appelle  :func:`ez_quit`, ou lorsque toutes les fenêtres 
sont détruites.


Voici le fichier demo-01.c_ :

.. literalinclude:: ../../demo-01.c
   :language: c
   :linenos:
   :lines: 13-


On obtient cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-01.png
      :alt: demo-01


.. ############################################################################

.. index:: Compilation; Avec le module de base

.. _sec-tut-compilation:

-----------
Compilation
-----------


Pour compiler ``demo-01.c`` dans un terminal sous Unix, taper

.. code-block:: console

    gcc -Wall demo-01.c ez-draw.c -o demo-01 -lX11 -lXext

ou sous Windows, taper 

.. code-block:: console

    gcc -Wall demo-01.c ez-draw.c -o demo-01.exe -lgdi32


Pour exécuter le programme sous Unix, taper 

.. code-block:: console

    ./demo-01

ou sous Windows, taper 

.. code-block:: console

    demo-01


.. index:: Compilation; Avec un Makefile

|bullet| À l'usage, il est beaucoup plus pratique d'utiliser la commande 
``make`` au lieu de retaper chaque fois la ligne de commande de ``gcc``.
La commande ``make`` utilise le même fichier Makefile_ sous Unix et Windows.

Pour tout compiler la première fois, ou pour recompiler les fichiers 
C modifiés, il suffit de taper dans le terminal, quelque soit votre système :

.. code-block:: console

    make all


En cas d'erreur, voir le chapitre Installation_.
Si tout est à jour, ``make`` affiche : 
``make: Rien à faire pour « all ».``
Vous pouvez toujours forcer une recompilation générale en tapant :

.. code-block:: console

    make clean all

Enfin si vous souhaitez faire le ménage, par exemple avant de faire une
sauvegarde, tapez :

.. code-block:: console

    make distclean

Cela effacera tous les exécutables et fichiers temporaires, et ne conservera
que les fichiers sources.

*Exercice :*
    Éditez le fichier ``demo-01.c``, et modifiez le titre par
    ``"Mon premier programme"``. Compilez en utilisant ``make`` puis
    exécutez le programme.


|bullet| Voyons maintenant comment rajouter un nouveau programme. Il suffit 
d'éditer le fichier Makefile_, et de rajouter section C le nom de l'exécutable 
dans la liste ``EXECS``, ``EXECS_M`` ou ``EXECS_IM``, puis d'enregistrer. 
On peut alors taper ``make all`` pour compiler le nouveau programme.
Il y a plusieurs listes pour distinguer les cas :

- ``EXECS`` : pour compiler uniquement avec ez-draw.c_ ;
- ``EXECS_M`` : pour compiler avec ez-draw.c_ et ``-lm`` (la librairie math) ;
- ``EXECS_IM`` : pour compiler avec ez-draw.c_, ``-lm`` et ez-image.c_ (pour afficher des images).

Voici les listes actuelles dans le Makefile_ :

.. literalinclude:: ../../Makefile
   :language: makefile
   :lines: 34-40

Comme vous pouvez le constater, lorsqu'une liste est sur plusieurs lignes,
les lignes intermédiaires sont terminées par un ``\``.


*Exercice :*
    Recopiez le fichier ``demo-01.c`` en ``essai1.c``, et rajoutez
    ``essai1`` dans le Makefile_. 
    Compilez en utilisant ``make`` puis exécutez le programme.


|bullet| Vous pouvez utiliser le Makefile_ fourni pour des projets plus 
élaborés découpés en plusieurs modules. 
Supposons que votre exécutable s'appelle ``monappli``,
que votre projet utilise les modules ``ez-draw.c``, ``ez-image.c``, ``monprog.c``
et ``divers.c``, et que vous avez besoin des librairies ``-lm`` et ``-lgomp``.
Il vous suffit de renseigner les champs ``EXECS_PRO``, ``OBJS_PRO`` et
``LIBS_PRO``  dans le Makefile_ section D comme ceci :

.. code-block:: makefile

   EXECS_PRO = monappli
   OBJS_PRO  = ez-draw.o ez-image.o monprog.o divers.o
   LIBS_PRO  = -lm -lgomp


|bullet| Enfin, si vous voulez créer votre projet dans un 
nouveau répertoire, voici les fichiers qu'il faudra recopier dedans :

- ez-draw.h_ et ez-draw.c_ (c'est le module de base), 
- ez-image.h_ et ez-image.c_ (si vous comptez afficher des images), 
- Makefile_ et make.bat_.

Il suffira de vider les listes ``EXECS`` dans le Makefile_ et de 
compléter les champs nécessaires.


.. ############################################################################

.. index:: Évènement; Gestion des évènements

.. _sec-tut-event-handler:

----------------------
Gestion des évènements
----------------------


Nous allons maintenant nous intéresser à la gestion des évènements dans les
fenêtres.

Les paramètres de la fonction :func:`ez_window_create` responsable de la
création d'une fenêtre sont :

.. code-block:: c

    Ez_window ez_window_create (int w, int h, const char *name, Ez_func on_event);

``w`` est la largeur (width) de l'intérieur de la fenêtre en pixels,
``h`` est la hauteur (height), 
``name`` est le titre de la fenêtre ;
``on_event`` est la fonction d'évènement de la fenêtre, voir ci-dessous.

Le résultat de  :func:`ez_window_create` est de type :type:`Ez_window` ;
il sert à identifier la fenêtre, et on peut l'afficher dans le terminal 
sous une forme hexadécimale : 

.. code-block:: c

    Ez_window win1;
    win1 = ez_window_create (400, 300, "Demo 0: Hello World", NULL);
    printf ("win1 = 0x%x\n", ez_window_get_id(win1));

Pour être en mesure de réagir aux actions de l'utilisateur (touches clavier,
mouvement de souris, clics de souris, etc) il faut réaliser ce qu'on nomme une
*gestion d'évènements* ; c'est pourquoi on donne la fonction ``on_event`` 
en 4e argument de :func:`ez_window_create`.

.. index:: Callback; Exemple

La fonction d'évènement ``on_event``, encore appelée *callback*, est une fonction 
de votre programme (ou ``NULL`` comme dans l'exemple ``demo-01``). 
Cette fonction sera automatiquement appelée par  :func:`ez_main_loop` lors de 
chaque évènement concernant la fenêtre.

La fonction ``on_event``  doit obligatoirement avoir le prototype suivant :

.. code-block:: c

    void on_event (Ez_event *ev);

La variable ``ev`` pointe sur un struct dont les champs décrivent
l'évènement ; en particulier, ``ev->win`` indique la fenêtre concernée
par l'évènement. On détaille les autres champs de ``ev`` dans la section 
:ref:`sec-tut-all-events`.


Découvrons deux évènements dans l'exemple suivant 
demo-02.c_ :

* Le gestionnaire de fenêtre indique à votre programme si vos fenêtres doivent
  être redessinées  (la première fois qu'elles apparaissent, si une autre fenêtre 
  est passée devant, etc).
  Lorsque cela arrive, l'évènement ``Expose`` est déclenché. Vous devez
  alors redessiner l'ensemble du contenu de la fenêtre ``ev->win``.
..
* Lorsque l'utilisateur enfonce une touche, l'évènement ``KeyPress`` est
  déclenché. Le code de la touche est disponible dans ``ev->key_sym``
  (pour *key symbol*). Chaque code de touche correspond à une constante
  préfixée par ``XK_``, par exemple ici ``XK_q`` pour la touche "q".


.. literalinclude:: ../../demo-02.c
   :language: c
   :linenos:
   :lines: 13-


On obtient cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-02.png
      :alt: demo-02


Cet exemple demo-02.c_ est une première façon de gérer les événements, avec
un gros ``switch`` dans ``win1_on_event``. 
L'inconvénient de cette méthode est que le ``switch`` peut rapidement grossir 
lorsqu'on augmente le programme, au risque de rendre le programme illisible. 
C'est pourquoi il vaut mieux éclater ``win1_on_event`` en fonctions
spécialisées par événement (toujours avec un ``switch``), 
ce que l'on fait dans la section suivante.



.. ############################################################################

.. index:: Dessin; Exemple

.. _sec-tut-drawings-colors:

---------------------------
Les dessins et les couleurs
---------------------------


Comme expliqué dans la section précédente,
l'évènement ``Expose`` signifie qu'il faut redessiner le contenu
de la fenêtre, ce qu'on fait dans l'exemple suivant en appelant
``win1_on_expose``. À noter : pour chaque ``Expose``, EZ-Draw
vide entièrement la fenêtre (avec un fond blanc) avant de passer 
l'évènement à votre programme.


La liste des dessins est donné dans la section :ref:`sec-ref-drawings`.
Les coordonnées sont relatives à l'origine, qui est le coin en haut à gauche
de l'intérieur de la fenêtre, avec ``x`` vers la droite et ``y``
vers le bas.


Les dessins sont automatiquements coupés par le bord de la fenêtre,
il n'y a donc pas à se préoccuper de savoir si un dessin risque de
dépasser ou pas. 

Les dessins sont faits dans l'épaisseur courante (par défaut 1 pixel).
On peut changer l'épaisseur courante avec  :func:`ez_set_thick`,
voir :ref:`sec-ref-drawings`. 

Les dessins sont faits dans la couleur courante (par défaut en noir).
Pour changer la couleur courante, on appelle  :func:`ez_set_color` en lui
donnant un numéro de couleur. Quelques couleurs sont prédéfinies :
``ez_black``, ``ez_white``, ``ez_grey``, ``ez_red``, ``ez_green``, ``ez_blue``, 
``ez_yellow``, ``ez_cyan``, ``ez_magenta``.
On peut créer d'autres couleurs, voir :ref:`sec-ref-colors`.


Voici le fichier demo-03.c_ :

.. literalinclude:: ../../demo-03.c
   :language: c
   :linenos:
   :lines: 13-


On obtient cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-03.png
      :alt: demo-03


.. ############################################################################

.. index:: Texte; Exemple

.. _sec-tut-displaying-text:

------------------
Affichage de texte
------------------


On peut afficher du texte n'importe où dans la fenêtre à l'aide de
la fonction  :func:`ez_draw_text`. Elle prend en argument la fenêtre,
le type d'alignement ``align``,
puis des coordonnées ``x1,y1``, enfin une chaîne de caractères
à imprimer, ou comme dans ``printf``, un format et des paramètres.
Tout est détaillé dans :ref:`sec-ref-fonts`.

La chaîne de caractères peut comporter des ``\n``, ceci provoquera
des saut de lignes dans l'affichage. 
L'affichage de texte se fait dans la couleur courante, modifiable par
:func:`ez_set_color`.


Dans l'exemple suivant demo-04.c_ on illustre
l'affichage de texte, ainsi que l'usage de :func:`ez_window_get_size` pour 
faire un dessin qui s'adapte aux changements de taille de la fenêtre. 


.. literalinclude:: ../../demo-04.c
   :language: c
   :linenos:
   :lines: 13-


On obtient cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-04.png
      :alt: demo-04


.. ############################################################################

.. index:: Évènement; Tracer les évènements, Bouton Fermer; Exemple

.. _sec-tut-all-events:

---------------------
Tracer les évènements
---------------------


Dans l'exemple suivant on recense tous les évènements possibles
et on affiche dans le terminal les champs utilisables de la variable ``ev`` 
(les autres champs sont réinitialisés à 0).
Voir aussi la section :ref:`sec-ref-events`.

Par défaut, le bouton "Fermer" dans la barre de titre d'une des fenêtres de 
l'application provoque la fin du programme. On peut changer ce réglage
d'origine en faisant  :func:`ez_auto_quit(0) <ez_auto_quit>` : 
le bouton "Fermer" provoquera l'évènement ``WindowClose``, comme dans l'exemple 
suivant : 


Voici le fichier demo-05.c_ :

.. literalinclude:: ../../demo-05.c
   :language: c
   :linenos:
   :lines: 13-


On obtient cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-05.png
      :alt: demo-05


Remarque : les évènements ``TimerNotify`` ne sont pas traités ici,
voir :ref:`sec-ref-timers`.


.. ############################################################################

.. index:: Souris; Dessiner avec la souris

.. _sec-tut-drawing-with-mouse:

--------------------
Dessiner à la souris
--------------------

  
L'exemple présenté dans cette section permet de dessiner une ligne polygonale
à la souris. Les coordonnées des sommets sont mémorisées dans des variables globales 
(on pourrait aussi éviter d'avoir des variables globales, voir :ref:`sec-ref-client-data`). 
Chaque fois que le bouton de la souris est cliqué, un sommet est
inséré ; à chaque déplacement de la souris avec bouton enfoncé, le dernier
sommet est déplacé.


Par principe (et pour des raisons techniques), les dessins ne peuvent
être faits que lors de l'évènement ``Expose``. Si l'on veut mettre à jour
le dessin dans la fenêtre lors d'un autre évènement, il suffit d'envoyer 
l'évènement ``Expose`` avec la fonction  :func:`ez_send_expose`.
C'est ce que l'on fait ici pour les évènements 
``ButtonPress``, ``MotionNotify`` et ``KeyPress``. 


Voici le fichier demo-06.c_ :

.. literalinclude:: ../../demo-06.c
   :language: c
   :linenos:
   :lines: 13-


On obtient cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-06.png
      :alt: demo-06


.. ############################################################################

.. index:: Fenêtre; Gérer plusieurs fenêtres

.. _sec-tut-several-windows:

------------------------
Gérer plusieurs fenêtres
------------------------


On peut créer autant de fenêtre que l'on veut, par des appels
à  :func:`ez_window_create`. Chaque fenêtre créée est immédiatement affichée
par dessus les autres fenêtres. Pour faire détruire une fenêtre ``win`` (et donc
la faire disparaître de l'écran), utiliser  
:func:`ez_window_destroy(win) <ez_window_destroy>`.


On peut cacher une fenêtre ``win`` (c'est-à-dire la rendre invisible) avec  
:func:`ez_window_show(win, 0) <ez_window_show>` puis la rendre visible (par 
dessus les autres fenêtres) avec :func:`ez_window_show(win, 1) <ez_window_show>`.

Dans l'exemple qui suit, on paramètre la boucle principale en faisant
:func:`ez_auto_quit(0) <ez_auto_quit>` : le bouton "Fermer" de la barre de 
titre d'une fenêtre ne provoquera plus la fin du programme, mais provoquera 
l'évènement ``WindowClose``. Selon la fenêtre incriminée, on cache la fenêtre,
détruit la fenêtre ou quitte le programme.


Voici le fichier demo-07.c_ :

.. literalinclude:: ../../demo-07.c
   :language: c
   :linenos:
   :lines: 13-


On obtient cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-07.png
      :alt: demo-07


.. ############################################################################

.. index:: Texte; Saisie

.. _sec-tut-text-input:

---------------
Saisie de texte
---------------


L'exemple suivant demo-08.c_ montre comment lire au 
clavier une chaîne de caractères,
supprimer des caractères avec la touche Backspace, et détecter la frappe
de la touche Entrée pour déclencher une action.


.. literalinclude:: ../../demo-08.c
   :language: c
   :linenos:
   :lines: 13-


On obtient cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-08.png
      :alt: demo-08


.. ############################################################################

.. index:: Animation, Timer; Exemple

.. _sec-tut-animations:

----------
Animations
----------


Pour réaliser une animation il faut deux ingrédients supplémentaires :
un timer (voir :ref:`sec-ref-timers`) pour entretenir la séquence temporelle,
et un double buffer d'affichage (voir :ref:`sec-ref-dbuf`) pour éviter que 
l'affichage ne clignote.

Attention : 
    il ne faut surtout pas employer ``sleep`` ou ``usleep`` car
    ces fonction empêchent les callbacks de "rendre la main" à  :func:`ez_main_loop`, et donc
    elles figent l'affichage et l'interface (ou les perturbent fortement).

Le principe d'une animation est le suivant : 

*  on démarre un timer dans ``main``, qui provoque un évènement ``TimerNotify`` 
   quelques millisecondes plus tard ;
..
*  à réception de cet évènement ``TimerNotify`` dans la callback de la fenêtre, 
   on fait trois choses : 

   * on incrémente un compteur (ou des coordonnées) permettant de modifier la 
     position de l'objet à animer ; 
   * on émet un évènement ``Expose`` pour que l'affichage soit refait ;
   * enfin, on réarme le timer pour qu'il y ait un prochain évènement ``TimerNotify``
     (on obtient une espèce de "boucle" de ``TimerNotify`` temporisée) ;
..
*  chaque fois qu'un évènement ``Expose`` est reçu, on redessine la fenêtre 
   en tenant compte du compteur (ou des coordonnées) pour dessiner l'objet animé 
   dans sa position courante. Il ne faut pas dessiner pour un autre évènement, 
   car cela perturberait le double-buffer d'affichage.
   De plus, EZ-Draw optimise l'affichage en éliminant les évènements ``Expose`` 
   inutiles.


Voici un premier exemple simple avec le fichier demo-09.c_
affiché ci-dessous. L'animation consiste à faire grossir un cercle au milieu de 
la fenêtre ; elle s'adapte également à la taille de la fenêtre.

.. literalinclude:: ../../demo-09.c
   :language: c
   :linenos:
   :lines: 13-


On obtient cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-09.png
      :alt: demo-09


L'exemple suivant demo-10.c_ illustre les animations
multiples : on fait tourner les aiguilles d'une montre dans une
fenêtre (on peut faire pause avec la touche espace), tandis qu'une balle rebondit
sur une raquette dans une seconde fenêtre (que l'on peut agrandir).


On obtient ces fenêtres :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-10-1.png
      :alt: demo-10-1
   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-10-2.png
      :alt: demo-10-2


Cet exemple demo-10.c_ illustre également comment attacher une information (par
exemple un ``struct``) à une fenêtre, pour éviter les variables globales.
Pour plus d'explications voir :ref:`sec-ref-client-data`. 
On se sert beaucoup de cette possibilité dans la partie suivante.



.. ############################################################################

.. index:: Image; Exemples

.. _sec-tut-images:

------
Images
------


Dans les sections précédentes on a vu tout ce que l'on pouvait faire
avec le module de base.

EZ-Draw contient un second module, ez-image.c_,
qui permet de charger et d'afficher des images en couleur au format
PNG, JPEG, GIF ou BMP, ou encore de créer une image en mémoire et de
dessiner dedans. Tout est détaillé dans les sections :ref:`sec-ref-images-type`
et suivantes.

Pour utiliser ce module il faut inclure ez-image.h_.
Voici l'exemple demo-13.c_
dans lequel on récupère un nom de fichier image en argument
de la ligne de commande, puis on charge l'image et enfin on l'affiche :

.. literalinclude:: ../../demo-13.c
   :language: c
   :linenos:
   :lines: 13-


On obtient par exemple cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-13.png
      :alt: demo-13


.. index:: Compilation; Avec le module image

Pour compiler cet exemple ``demo-13.c`` sous Unix, taper : 

.. code-block:: console

    gcc -Wall demo-13.c ez-draw.c ez-image.c -o demo-13 -lX11 -lXext -lm

ou sous Windows, taper : 

.. code-block:: console

    gcc -Wall demo-13.c ez-draw.c ez-image.c -o demo-13.exe -lgdi32 -lmsimg32 -lm

On peut également rajouter le nom de l'exécutable à la fin de ``EXECS_IM =``
dans le Makefile_, puis taper ``make all``  pour compiler.


Les formats PNG, GIF et BMP permettent de mémoriser le degré de transparence, dans
ce qu'on appelle le canal alpha. Les formats GIF et BMP codent le canal alpha sur
1 bit ; les pixels sont soit transparents (0), soit opaques (255). Le format PNG
code le canal alpha sur 8 bits (de 0 pour transparent à 255 pour opaque).

Le module ``ez-image`` est capable d'afficher une image en tenant compte 
de la transparence, en utilisant un seuil d'opacité sur le canal alpha :
les pixels sont soit opaques (affichés), soit transparents (non affichés).

L'exemple suivant demo-14.c_ récupère deux noms de 
fichiers en argument de la ligne de commande, puis superpose les deux images. 
On peut ensuite déplacer la
seconde image avec les flèches, ou modifier le seuil d'opacité avec les touches
``+`` et ``-``.


.. literalinclude:: ../../demo-14.c
   :language: c
   :linenos:
   :lines: 13-


On obtient par exemple cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-14.png
      :alt: demo-14


Il est également possible de créer une image en mémoire, puis d'affecter
les couleurs des pixels. L'exemple demo-12.c_ affiche 
la palette de couleurs HSV à l'aide d'une image calculée en mémoire.

On obtient cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-12.png
      :alt: demo-12


Vous trouverez plus d'informations sur les images dans le :ref:`chap-ref-manual`.


