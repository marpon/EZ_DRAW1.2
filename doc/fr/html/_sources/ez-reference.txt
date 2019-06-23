
.. _chap-ref-manual:

===================
Manuel de référence
===================


Pour les sources, voir ez-draw.h_, ez-draw.c_.


.. ############################################################################

.. index:: Boucle principale

.. _sec-ref-main-loop:

-----------------
Boucle principale
-----------------


.. function:: int ez_init ()

   Initialisation générale.

   Renvoie 0 succès, -1 échec.


.. function:: void ez_main_loop ()

   Boucle principale.

Cette fonction affiche les fenêtres créées, puis attend les évènements et 
appelle au fur et à mesure la fonction d'évènements correspondante (la *callback*)
de la fenêtre concernée.

Pour interrompre la boucle principale, appeler :func:`ez_quit` dans une callback.
Une fois revenu de :func:`ez_main_loop`, il ne faut plus faire aucun graphisme.


.. function:: void ez_quit ()

   Fait sortir de :func:`ez_main_loop()`.

.. index:: Bouton Fermer

.. function:: void ez_auto_quit (int val)

   Modifie l'effet du bouton "Fermer" de la barre de titre d'une fenêtre,
   pour toutes les fenêtres du programme.

Par défaut (cas où ``val = 1``), si on clique sur le bouton "Fermer" de n'importe 
quelle fenêtre d'un programme, il se termine immédiatement. 

Vous pouvez changer ce comportement en invoquant :func:`ez_auto_quit` avec 
``val = 0`` : si ensuite l'utilisateur clique sur le bouton "Fermer" d'une 
fenêtre,  le programme (au lieu de se terminer) recevra l'évènement 
``WindowClose`` pour cette fenêtre ; libre alors à votre programme de décider 
ce qu'il veut faire :

  * ignorer l'évènement ;
  * détruire la fenêtre avec :func:`ez_window_destroy` ;
  * masquer la fenêtre avec :func:`ez_window_show` ;
  * créer une fenêtre de dialogue ;
  * terminer le programme avec :func:`ez_quit` ;
  * etc.

Remarque : lorsque toutes les fenêtres sont détruites, le programme s'arrête.


.. ############################################################################

.. index:: Fenêtre

.. _sec-ref-windows:

--------
Fenêtres
--------

Chaque fenêtre a un identifiant unique, de type :type:`Ez_window` :

.. type:: Ez_window

   Identifiant d'une fenêtre.


Les fonctions suivantes permettent de créer ou manipuler des fenêtres :

.. index:: Callback

.. function:: Ez_window ez_window_create (int w, int h, const char *name, Ez_func on_event)

   Crée et affiche une fenêtre, de largeur ``w`` et hauteur ``h``,
   avec un titre ``name``, et une fonction ``on_event`` (la *callback*)
   appelée pour chaque évènement (``on_event`` peut être ``NULL``).

   Renvoie l'identifiant de la fenêtre, de type :type:`Ez_window`.


Toute callback est de type :type:`Ez_func` :

.. type:: Ez_func

   Le type des callbacks, c'est-à-dire le prototype des fonctions appelées 
   pour chaque évènement. Ce type est défini ainsi :

   .. code-block:: c

       typedef void (*Ez_func)(Ez_event *ev);

   Autrement dit, la fonction ``on_event`` passée en paramètre à
   :func:`ez_create_window` doit être de cette forme :

   .. code-block:: c

       void on_event (Ez_event *ev);


.. function:: int ez_window_get_id (Ez_window win)

   Renvoie le numéro de la fenêtre comme un ``int``.


.. function:: void ez_window_destroy (Ez_window win)

   Détruit la fenêtre ``win``.


.. function:: void ez_window_show (Ez_window win, int val)

   Rend visible (``val = 1``) ou cache (``val = 0``) la fenêtre ``win``.


.. function:: void ez_window_set_size (Ez_window win, int w, int h)

   Change la taille de la fenêtre.


.. function:: void ez_window_get_size (Ez_window win, int *w, int *h)

   Récupère la taille de la fenêtre.


.. function:: void ez_window_clear (Ez_window win)

   Vide la fenêtre (avec un fond blanc) et réinitialise les paramètres de 
   dessin (couleur, épaisseur, fonte) aux valeurs par défaut.


.. function:: void ez_send_expose (Ez_window win)

   Envoie un évènement ``Expose`` a la fenêtre, pour la vider et la forcer à 
   se redessiner.


.. ############################################################################

.. index:: Évènement; Liste des évènements, Souris; Évènements et coordonnées

.. _sec-ref-events:

----------
Évènements
----------

Chaque évènement est décrit par un struct de type :type:`Ez_event` :

.. type:: Ez_event

   Mémorise un évènement.

Ce type est défini dans ez-draw.h_ de la façon suivante :

.. code-block:: c

    typedef struct {
        int type;                       /* Expose, ButtonPress, etc                */
        Ez_window win;                  /* Identifiant de la fenêtre               */
        int mx, my;                     /* Coordonnées souris                      */
        int mb;                         /* Numéro bouton de la souris, 0 = aucun   */
        int width, height;              /* Largeur et hauteur                      */
        KeySym key_sym;                 /* Symbole touche : XK_Space, XK_q, etc    */
        char   key_name[80];            /* Pour affichage : "XK_Space", "XK_q", .. */
        char   key_string[80];          /* Chaine correspondante : " ", "q", etc   */
        int    key_count;               /* Taille de la chaine                     */
        /* Autres champs privés */
    } Ez_event;

Le premier champ permet de connaître le type de l'évènement. Les différentes
valeurs possibles sont :

.. table::
   :class: centrer nonsouligner encadrer

   ===================  ====================================
   ``Expose``           Il faut redessiner toute la fenêtre.
   ``ButtonPress``      Bouton souris enfoncé.
   ``ButtonRelease``    Bouton souris relaché.
   ``MotionNotify``     Souris déplacée.
   ``KeyPress``         Touche clavier enfoncée.
   ``KeyRelease``       Touche clavier relachée.
   ``ConfigureNotify``  La fenêtre a changé de taille.
   ``WindowClose``      Le bouton "fermer" a été pressé.
   ``TimerNotify``      Le timer est arrivé à échéance.
   ===================  ====================================


.. ############################################################################

.. index:: Couleur

.. _sec-ref-colors:

--------
Couleurs
--------

Chaque couleur est désignée par un entier de type ``Ez_uint32``.

.. function:: void ez_set_color (Ez_uint32 color)

   Mémorise la couleur ``color`` pour les prochains dessins, ainsi que pour 
   l'affichage de texte.

Les couleurs suivantes sont prédéfinies :
``ez_black``, ``ez_white``, ``ez_grey``, ``ez_red``, ``ez_green``, ``ez_blue``, 
``ez_yellow``, ``ez_cyan``, ``ez_magenta``.

On peut fabriquer d'autres couleurs avec les fonctions suivantes :


.. function:: Ez_uint32 ez_get_RGB (Ez_uint8 r, Ez_uint8 g, Ez_uint8 b)

   Renvoie une couleur calculée à partir des niveaux ``r,g,b`` donnés entre 0 et 255.


.. function:: Ez_uint32 ez_get_grey (Ez_uint8 g)

   Renvoie une couleur grise calculée à partir de son niveau ``g`` donné entre 0 et 255.


.. function:: Ez_uint32 ez_get_HSV (double h, double s, double v)

   Renvoie une couleur calculée dans l'espace Hue, Saturation, Value.


``h`` est un angle entre 0 et 360 degrés qui représente arbitrairement
les couleurs pures ; ``s`` est la saturation, entre 0 et 1 ; ``v`` est
la valeur de luminosité, entre 0 et 1. 
Pour en savoir plus, consulter l'article
`Teinte Saturation Valeur <http://fr.wikipedia.org/wiki/Teinte_Saturation_Valeur>`_
dans Wikipedia.


.. function:: void ez_HSV_to_RGB (double h, double s, double v, Ez_uint8 *r, Ez_uint8 *g, Ez_uint8 *b)

   Convertit une couleur de l'espace HSV en RGB.

   Les intervalles sont : 
   ``h`` entre 0 et 360,
   ``s`` et ``v`` entre 0 et 1,
   ``*r``, ``*g``, ``*b`` entre 0 et 255.


Comme exemples, voir demo-11.c_ et demo-12.c_.


On obtient ces fenêtres :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-11.png
      :alt: demo-11
   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-12.png
      :alt: demo-12


.. ############################################################################

.. index:: Dessin

.. _sec-ref-drawings:

-------
Dessins
-------


Les coordonnées sont relatives à l'origine, qui est le point en haut à gauche 
de l'intérieur de la fenêtre ; ``x`` va vers la droite et ``y`` va vers le bas.

Pour les rectangles et les cercles, ``x1,y1`` et ``y2,y2`` sont les coordonnées 
en haut à gauche et en bas à droite de la boîte englobante. 
Pour les points, les segments de droite et les triangles, on passe les
coordonnées des sommets.

Par défaut, la couleur est le noir ; elle se change avec :func:`ez_set_color`.


.. function:: void ez_draw_point (Ez_window win, int x1, int y1)

   Dessine un point.


.. function:: void ez_draw_line (Ez_window win, int x1, int y1, int x2, int y2)

   Dessine un segment.


.. function:: void ez_draw_rectangle (Ez_window win, int x1, int y1, int x2, int y2)

   Dessine un rectangle.


.. function:: void ez_fill_rectangle (Ez_window win, int x1, int y1, int x2, int y2)

   Dessine un rectangle plein.


.. function:: void ez_draw_triangle (Ez_window win, int x1, int y1, int x2, int y2, int x3, int y3)

   Dessine un triangle.

.. function:: void ez_fill_triangle (Ez_window win, int x1, int y1, int x2, int y2, int x3, int y3)

   Dessine un triangle plein.


.. function:: void ez_draw_circle (Ez_window win, int x1, int y1, int x2, int y2)

   Dessine un cercle.


.. function:: void ez_fill_circle (Ez_window win, int x1, int y1, int x2, int y2)

   Dessine un cercle plein.


Les dessins réalisés avec :func:`ez_draw_point`, :func:`ez_draw_line`, 
:func:`ez_draw_rectangle`, :func:`ez_draw_triangle`, :func:`ez_draw_circle`
ont une épaisseur par défaut de 1 pixel. On peut modifier l'épaisseur avec :


.. function:: void ez_set_thick (int thick)

   Mémorise l'épaisseur ``thick`` (en pixels) pour les prochains dessins.


.. ############################################################################

.. index:: Fonte, Texte

.. _sec-ref-fonts:

---------------
Texte et fontes
---------------

.. function:: int ez_font_load (int num, const char *name)

   Charge une fonte à partir de son nom (par exemple ``"6x13"``) et la stocke
   pour le numéro ``num``.

   Renvoie 0 succès, -1 erreur.


Le numéro de fonte doit être inférieur à ``EZ_FONT_MAX``.
Quelques fontes fixes sont préchargées par défaut :

  * Fonte numéro 0 : ``"6x13"``
  * Fonte numéro 1 : ``"8x16"``
  * Fonte numéro 2 : ``"10x20"``
  * Fonte numéro 3 : ``"12x24"``


Remarque : 
   sous X11, le nom peut avoir une forme quelconque mais doit correspondre à une
   fonte existante. Sous Windows, le nom doit être sous la forme
   *largeur*\ ``x``\ *hauteur* (une fonte approchante de taille fixe est obtenue).


.. function:: void ez_set_nfont (int num)

   Mémorise le numéro de fonte ``num`` pour les prochains affichages de texte.


.. function:: void ez_draw_text (Ez_window win, Ez_Align align, int x1, int y1, const char *format, ...)

   Affiche du texte ; s'utilise comme ``printf``.


Exemple :

    .. code-block:: c

        ez_draw_text (win, EZ_TL, 10, 10, "Largeur = %d\nHauteur = %d", w, h);


Les coordonnées ``x1,y1`` sont données par rapport à ``align``,
qui prend pour valeurs :

.. table::
   :class: centrer nonsouligner encadrer

   =======================  =========================  ========================
   ``EZ_TL`` (Top Left)     ``EZ_TC`` (Top Center)     ``EZ_TR`` (Top Right)
   ``EZ_ML`` (Middle Left)  ``EZ_MC`` (Middle Center)  ``EZ_MR`` (Middle Right)
   ``EZ_BL`` (Bottom Left)  ``EZ_BC`` (Bottom Center)  ``EZ_BR`` (Bottom Right)
   =======================  =========================  ========================


Le texte est tracé par dessus le dessin actuel ; on peut aussi faire effacer
le fond en même temps (avec du blanc) en utilisant pour ``align`` les
constantes :

.. table::
   :class: centrer nonsouligner encadrer

   ===============================  =================================  ================================
   ``EZ_TLF`` (Top Left Filled)     ``EZ_TCF`` (Top Center Filled)     ``EZ_TRF`` (Top Right Filled)
   ``EZ_MLF`` (Middle Left Filled)  ``EZ_MCF`` (Middle Center Filled)  ``EZ_MRF`` (Middle Right Filled)
   ``EZ_BLF`` (Bottom Left Filled)  ``EZ_BCF`` (Bottom Center Filled)  ``EZ_BRF`` (Bottom Right Filled)
   ===============================  =================================  ================================


Par défaut :
  * le texte est affiché avec la fonte numero 0 (``6x13``) ;
    se change avec :func:`ez_set_nfont`.
  * le texte est affiché en noir ; se change avec :func:`ez_set_color`.


.. ############################################################################

.. index:: Double buffer
   seealso: Image; Double buffer
   seealso: Animation; Double buffer

.. _sec-ref-dbuf:

-------------------------
Double-buffer d'affichage
-------------------------


L'affichage avec double-buffer permet d'éviter que la fenêtre ne clignote
pendant qu'elle est rafraichie. Le principe est de dessiner dans le double-buffer,
puis d'échanger celui-ci avec le contenu de la fenêtre quand tous les dessins sont 
finis. Tout est automatiquement géré par EZ-Draw.


.. function:: void ez_window_dbuf (Ez_window win, int val)

   Active ou inactive l'affichage double-buffer pour le window ``win``.


Par défaut, l'affichage double-buffer est désactivé (``val = 0``).

Si l'affichage double-buffer est activé (``val = 1``) pour un window, 
les dessins dans ce window doivent obligatoirement être faits uniquement
lors des évènements ``Expose`` de ce window.
Si le double-buffer est inactivé, ce n'est plus une obligation, mais cela
reste fortement conseillé. 

Comme exemple, voir dans jeu-nim.c_ les fonctions 
``gui_init()``, ``win1_onKeyPress()``, ``win1_onExpose()``.

Dans ce jeu, on peut tester l'affichage avec et sans le double-buffer
(presser la touche  ``d`` pour basculer entre l'un et l'autre) :

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


*Armer un timer* signifie mémoriser une date dans le futur, qui est la date
actuelle plus un certain délai. Lorsqu'on arrive à cette date future, on dit que
le timer est *arrivé à échéance*.

Chaque fenêtre peut être associée à un timer. 
À l'échéance du timer, l'application reçoit un évènement unique
``TimerNotify`` pour le window concerné, puis le timer est supprimé.


.. function:: void ez_start_timer (Ez_window win, int delay)

   Arme un timer pour le window ``win`` avec un délai ``delay`` en millisecondes.


Tout rappel de cette fonction avant l'échéance du timer annule et remplace le timer.
Si de plus ``delay`` vaut ``-1``, alors le timer est supprimé
(remarque : ce n'est pas une erreur de supprimer un timer déjà supprimé ou inexistant).

Comme exemple, voir demo-09.c_.


.. ############################################################################

.. index:: Client-data, Variable globale

.. _sec-ref-client-data:

-----------
Client-data
-----------


Chaque fenêtre peut mémoriser une donnée arbitraire du programme, 
par exemple une chaîne de caractères ou l'adresse d'un ``struct``.
On peut ensuite récupérer cette donnée à tout moment dans le programme.
Ce mécanisme permet ainsi d'éviter l'emploi de variables globales.


.. function:: void ez_set_data (Ez_window win, void *data)

   Mémorise la donnée ``data`` dans la fenêtre ``win`` 


.. function:: void *ez_get_data (Ez_window win)

   Renvoie la donnée associée à la fenêtre ``win``.


Voici un exemple de programme qui affiche un cercle, dont les coordonnées
sont placées dans une variable globale ``md`` :

.. code-block:: c
    :linenos:

    #include "ez-draw.h"

    typedef struct {
        int x, y, r;
    } Mes_donnees;

    Mes_donnees md;  /* 1. Variable globale */


    void win1_on_expose (Ez_event *ev)
    {
        /* 3. Utilisation */
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

        /* 2. Initialisation */
        md.x = 200; md.y = 100; md.r = 50;

        ez_window_create (400, 300, "Demo client-data 1", win1_on_event);

        ez_main_loop ();
        exit(0);
    }


Voici maintenant le même programme sans variable globale,
en mémorisant la donnée dans la fenêtre :

.. code-block:: c
    :linenos:

    #include "ez-draw.h"
    
    typedef struct {
        int x, y, r;
    } Mes_donnees;


    void win1_on_expose (Ez_event *ev)
    {
        /* 4. On retrouve les données attachées à la fenêtre */
        Mes_donnees *md = ez_get_data (ev->win);

        /* 5. Utilisation */
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
        Mes_donnees md;  /* 1. Variable locale à main() */
    
        if (ez_init() < 0) exit(1);

        /* 2. Initialisation */
        md.x = 200; md.y = 100; md.r = 50;
    
        win1 = ez_window_create (400, 300, "Demo client-data 2", win1_on_event);

        /* 3. On mémorise la donnée dans la fenêtre */
        ez_set_data (win1, &md);

        ez_main_loop ();
        exit(0);
    }


Comme autre exemple, voir demo-10.c_.


.. ############################################################################

.. index:: Image; Le type image

.. _sec-ref-images-type:

-------------
Le type image
-------------


EZ-Draw permet d'afficher ou de manipuler des images avec le type suivant :

.. type:: Ez_image

   Type ``struct`` principal pour mémoriser une image.

Ce type est défini dans ez-image.h_ de la façon
suivante :

.. code-block:: c

    typedef struct {
        int width, height;
        Ez_uint8 *pixels_rgba;
        int has_alpha;
        int opacity;
    } Ez_image;

La largeur de l'image en pixels est ``width`` et sa hauteur est ``height``.

Les pixels sont mémorisés dans le tableau ``pixels_rgba``
sous la forme R,G,B,A (pour rouge, vert, bleu, alpha, c'est-à-dire transparence) 
avec une valeur entre 0 et 255
(255 est l'intensité ou l'opacité maximale).

Les valeurs R,G,B,A d'un pixel de coordonnées ``x,y`` dans l'image sont
mémorisées dans ``pixels_rgba[(y*width+x)*4 + 0..3]``.

Le champ ``has_alpha`` indique si le canal alpha est utilisé (``has_alpha = 1``)
ou ignoré (``has_alpha = 0``) lors de l'affichage. 
Si le canal est ignoré, tous les pixels seront affichés ;
s'il est utilisé, seuls les pixels opaques seront affichés.

Les pixels opaques sont les pixels dont le canal alpha est supérieur ou égal
au seuil d'opacité, stipulé par le champ ``opacity`` ; 
par défaut le seuil d'opacité est 128.


**Attention :**
    ne modifiez pas les champs ``width``, ``height``, ``pixels_rgba`` d'une image, 
    car ils décrivent la mémoire qui a été allouée.
    En revanche, vous pouvez modifier les champs ``has_alpha``, ``opacity``,
    ainsi que la  valeur des pixels dans ``pixels_rgba[]``.
    On peut aussi utiliser les fonctions suivantes.


.. function:: void ez_image_set_alpha (Ez_image *img, int has_alpha)
              int  ez_image_has_alpha (Ez_image *img)
              void ez_image_set_opacity (Ez_image *img, int opacity)
              int  ez_image_get_opacity (Ez_image *img)

   Récupére ou modifie les champs ``has_alpha`` et ``opacity``.
   
   Ces fonctions ne font rien si l'image ``img`` est ``NULL``.


.. ############################################################################

.. index:: Image; Gestion des images

.. _sec-ref-managing-images:

------------------
Gestion des images
------------------

Pour utiliser les fonctions suivantes il faut inclure 
ez-image.h_.


.. function:: Ez_image *ez_image_create (int w, int h)

   Crée une image de largeur ``w`` et hauteur ``h`` en pixels.

   Renvoie l'image créée, ou ``NULL`` si erreur.


.. function:: Ez_image *ez_image_load (const char *filename)

   Charge une image depuis le fichier ``filename``.
   Le fichier doit être au format PNG, JPEG, GIF ou BMP.

   La transparence est gérée pour les formats PNG, GIF et BMP :
   si le fichier contient un canal alpha, le champ ``has_alpha`` de
   l'image est mis à 1.

   Renvoie l'image créée, ou ``NULL`` si erreur.


.. function:: Ez_image *ez_image_dup (Ez_image *img)

   Crée une copie profonde de l'image ``img``.

   Renvoie l'image créée, ou ``NULL`` si erreur.


.. function:: void ez_image_destroy (Ez_image *img)

   Détruit une image en mémoire.

   Toutes les images créées par les fonctions ``ez_image_...``
   doivent être libérées avec cette fonction.


.. function:: void ez_image_paint (Ez_window win, Ez_image *img, int x, int y)

   Affiche une image dans la fenêtre ``win``, avec le coin supérieur gauche de 
   l'image aux coordonnées ``x,y`` dans la fenêtre.
   Si ``img->has_alpha`` est vrai, applique la transparence, c'est-à-dire 
   n'affiche que les pixels opaques.


.. function:: void ez_image_paint_sub (Ez_window win, Ez_image *img, int x, int y, \
        int src_x, int src_y, int w, int h)

   Affiche une région rectangulaire d'une image dans la fenêtre ``win``.

   La région de l'image est délimitée par les coordonnées ``src_x, src_y``
   (coin supérieur gauche) et ``src_x+w-1, src_y+h-1`` (coin inférieur droit)
   dans l'image. Si les coordonnées dépassent l'image, seule la région appartenant
   effectivement à l'image est affichée.

   Le coin supérieur gauche de la région est affiché aux coordonnées
   ``x,y`` dans la fenêtre.
   Si ``img->has_alpha`` est vrai, applique la transparence.


.. function:: void ez_image_print (Ez_image *img, int src_x, int src_y, int w, int h)

   Affiche une région rectangulaire d'une image dans le terminal.

   La région de l'image ``img`` est délimitée par les coordonnées ``src_x, src_y``
   (coin supérieur gauche) et ``src_x+w-1, src_y+h-1`` (coin inférieur droit)
   dans ``img``. Si les coordonnées dépassent l'image, seule la région appartenant
   effectivement à l'image est affichée.


.. ############################################################################

.. index:: Image; Opérations sur les images

.. _sec-ref-oper-on-images:

-------------------------
Opérations sur les images
-------------------------

Cette section présente quelques opérations disponibles en incluant 
ez-image.h_.
Ces opérations sont faites sur les couleurs et sur le canal alpha.


.. function:: void ez_image_fill_rgba (Ez_image *img, Ez_uint8 r, Ez_uint8 g, Ez_uint8 b, Ez_uint8 a)

   Remplit une image avec une couleur ``r,g,b,a``.

   Les valeurs sont entre 0 et 255.


.. function:: void ez_image_blend (Ez_image *dst, Ez_image *src, int dst_x, int dst_y)
              void ez_image_blend_sub (Ez_image *dst, Ez_image *src, int dst_x, int dst_y, \
                  int src_x, int src_y, int w, int h)

   Incruste une région de l'image ``src`` dans l'image ``dst``.

   La région de l'image source ``src`` est délimitée par les coordonnées 
   ``src_x, src_y`` (coin supérieur gauche) et ``src_x+w-1, src_y+h-1`` 
   (coin inférieur droit). 
   Cette région est incrustée dans l'image destination ``dst``
   aux coordonnées ``dst_x, dst_y`` (coin supérieur gauche) et 
   ``dst_x+w-1, dst_y+h-1`` (coin inférieur droit).

   Si les coordonnées dépassent les images ``src`` ou ``dst``, 
   seule la région commune est incrustée.
   Si l'image source n'a pas de canal alpha (c'est-à-dire si ``src->has_alpha``
   est faux), alors les valeurs de la région de ``src`` écrasent
   celle de ``dst``.
   Dans le cas contraîre, les régions sont mélangées par transparence
   (*alpha blending*) avec les formules de
   `Porter et Duff <http://fr.wikipedia.org/wiki/Alpha_blending>`_.


.. function:: Ez_image *ez_image_extract (Ez_image *img, int src_x, int src_y, int w, int h)

   Crée une image contenant une copie d'une région rectangulaire de l'image 
   source ``img``.

   La région de l'image est délimitée par les coordonnées ``src_x, src_y``
   (coin supérieur gauche) et ``src_x+w-1, src_y+h-1`` (coin inférieur droit)
   dans ``img``. Si les coordonnées dépassent l'image, seule la région appartenant
   effectivement à l'image est extraite.

   Renvoie ``NULL`` en cas d'erreur mémoire ou si l'intersection est vide.


.. function:: Ez_image *ez_image_sym_ver (Ez_image *img)
              Ez_image *ez_image_sym_hor (Ez_image *img)

   Crée une image de même taille et propriétés que l'image source ``img``,
   contenant le symétrique de l'image par rapport à l'axe vertical ou horizontal.

   Renvoie la nouvelle image, ou ``NULL`` si erreur.


.. function:: Ez_image *ez_image_scale (Ez_image *img, double factor)

   Crée une image de taille multipliée par ``factor`` pour
   l'image source ``img``, contenant l'image mise à l'échelle.
   Le facteur d'échelle ``factor`` doit être strictement positif.

   Renvoie la nouvelle image, ou ``NULL`` si erreur.


.. function:: Ez_image *ez_image_rotate (Ez_image *img, double theta, int quality)

   Effectue une rotation de l'image source ``img`` d'angle ``theta`` en degrés.
   Renvoie une nouvelle image dont la taille est ajustée pour contenir le
   résultat, ou ``NULL`` en cas d'erreur.

   Dans l'image résultat, le champs ``has_alpha`` est mis à 1, et les parties 
   ne provenant pas de l'image source sont transparentes ; le but est qu'elles
   n'apparaissent pas lors de l'affichage.

   Si ``quality = 1``, l'algorithme utilisé lisse le résultat (avec une interpolation
   bilinéaire) ; si ``quality = 0``, l'algorithme privilégie la rapidité (avec un
   calcul de plus proche voisin), ce qui permet de gagner environ un facteur 3.

Note : 
   le résultat étant indépendant du centre de rotation, il n'est pas demandé
   en paramètre ; on peut toutefois et indépendamment de la rotation elle-même,
   choisir arbitrairement un centre de rotation et calculer ses nouvelles coordonnées
   avec la fonction suivante :


.. function:: void ez_image_rotate_point (Ez_image *img, double theta, \
                  int src_x, int src_y, int *dst_x, int *dst_y)

   Calcule pour un point de coordonnées ``src_x,src_y`` dans l'image source,
   les nouvelles coordonnées ``dst_x,dst_y`` du point correspondant dans l'image 
   résultat.


L'exemple demo-16.c_ illustre les rotations,
sans ou avec transparence. Le centre de rotation (croix rouge) est déplaçable 
avec les flèches. On peut aussi modifier la qualité.

On obtient ces fenêtres :

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

--------------------------------
Accélérer l'affichage des images
--------------------------------

Nous avons vu dans les sections précédentes le type :type:`Ez_image` défini
dans ez-image.h_.

Ce type est commode pour charger des images, les transformer, les afficher.
Toutefois, l'affichage d'une image prend quelques millisecondes à quelques
dizaines de millisecondes, durée variable 
selon la taille de l'image et la puissance de la machine. La raison en est
qu'à chaque affichage, :func:`ez_image_paint` refait toute la conversion depuis le type
:type:`Ez_image` en une image intermédiaire en mémoire, applique la transparence
éventuelle, envoie le résultat à la carte graphique qui enfin l'affiche.

On peut afficher la durée des opérations dans le terminal, en définissant 
une variable d'environnement puis en relançant une démo : sous Unix, taper 

.. code-block:: console

    export EZ_IMAGE_DEBUG=1

ou sous Windows, taper 

.. code-block:: console

    set EZ_IMAGE_DEBUG=1


On obtient par exemple sous Unix : 

.. code-block:: console

    $ ./demo-14
    ez_image_load  file "images/paper1.jpg"  in 8.725 ms  w = 640  h = 480  n = 3  has_alpha = 0
    ez_image_load  file "images/tux1.png"  in 1.946 ms  w = 210  h = 214  n = 4  has_alpha = 1
    ez_xi_create  w = 640  h = 480  depth = 24  bpp = 32
    ez_xi_fill_24 2.875 ms
    ez_xi_create  w = 210  h = 214  depth = 24  bpp = 32
    ez_xi_fill_24 0.132 ms
    ez_xmask_create   fill 0.119 ms   bitmap 5.610 ms


Pour supprimer cette variable d'environnement sous Unix, taper 

.. code-block:: console

    unset EZ_IMAGE_DEBUG

ou sous Windows, taper 

.. code-block:: console

    set EZ_IMAGE_DEBUG=


.. index:: Pixmap

Dans une animation, tous ces temps s'additionnent, et lorsqu'il y a beaucoup
d'images, l'animation risque d'être saccadée.
La solution est simple : convertir l'image de type :type:`Ez_image`
en un pixmap de type :type:`Ez_pixmap`, puis afficher le pixmap. 


.. type:: typedef struct Ez_pixmap

   Un pixmap est une image déjà convertie et stockée dans la carte graphique. 
   Son affichage est donc beaucoup plus rapide, et de plus soulage
   le processeur de la machine.

   Une fois créé, un pixmap est non modifiable.


Cette solution est aussi intéressante pour afficher une image de fond
(qui est souvent de taille importante). Dans ce cas, on gagne encore en 
efficacité en utilisant une image sans canal alpha.

Le type :type:`Ez_pixmap` est défini comme ceci :

.. code-block:: c

    typedef struct {
        int width, height;
        /* autres champs privés */
    } Ez_pixmap


Les fonctions suivantes manipulent les pixmaps :


.. function:: Ez_pixmap *ez_pixmap_create_from_image (Ez_image *img)

   Crée un pixmap à partir d'une image ``img``.
   Le pixmap conserve la transparence de l'image.
   L'image peut ensuite être libérée s'il n'y en a plus besoin.

   Renvoie le nouveau pixmap, ou ``NULL`` si erreur.


.. function:: void ez_pixmap_destroy (Ez_pixmap *pix)

   Détruit le pixmap ``pix``.

   Les pixmaps doivent être détruits par cette fonction.


.. function:: void ez_pixmap_paint (Ez_window win, Ez_pixmap *pix, int x, int y)

   Affiche le pixmap ``pix`` dans la fenêtre ``win``.

   Le coin supérieur gauche du pixmap est affiché aux coordonnées
   ``x,y`` dans la fenêtre.


.. function:: void ez_pixmap_tile (Ez_window win, Ez_pixmap *pix, int x, int y, int w, int h)

   Affiche le pixmap ``pix`` de manière répétitive dans la fenêtre ``win``.

   Le pixmap est affiché à la manière d'un papier peint dans la zone de la fenêtre
   délimitée par les coordonnées ``x,y`` (coin supérieur gauche) et 
   ``x+w-1,y+h-1`` (coin inférieur droit).


L'exemple demo-17.c_ permet de tester la
vitesse d'affichage, mesurée en fps (pour *frame per second*) dans une animation.
Utiliser les touches ``+`` et ``-`` pour modifier le nombre de balles,
et la touche ``p`` pour activer l'utilisation des pixmaps.

On obtient cette fenêtre :

.. container:: centrer

   .. image:: http://pageperso.lif.univ-mrs.fr/~edouard.thiel/snap-EZ-Draw-1.2/snap-demo-17.png
      :alt: demo-17


.. ############################################################################

.. _sec-ref-misc:

------
Divers
------

.. index:: Nombre aléatoire

.. function:: int ez_random (int n)

   Renvoie un entier aléatoire entre 0 et ``n-1``.


Remarque : le générateur de nombres aléatoires est initialisé par :func:`ez_init`.

.. index:: Timer; Mesurer le temps

.. function:: double ez_get_time ()

   Renvoie le temps écoulé depuis l'*Epoch* (le 1er janvier 1970 à 0h) en
   secondes avec une précision en microsecondes.

Cette fonction est utile pour mesurer la durée d'un calcul : il suffit d'appeler
:func:`ez_get_time` avant et après le calcul, puis d'afficher la différence :

.. code-block:: c

    double t1, t2;
    t1 = ez_get_time ();
    calcul ();
    t2 = ez_get_time ();
    printf ("Durée du calcul : %.6f s\n", t2-t1)


