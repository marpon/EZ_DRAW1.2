/* jeu-bubblet.c: a bubblet game
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 25/03/2014 - version 1.2
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"
#include "ez-image.h"


/*--------------------------------- G A M E ---------------------------------*/

#define BOARD_H     12
#define BOARD_W     11
#define BUBBLES_NB   5


/* The board is surrounded by a border of 1 pixel */
typedef int Board[BOARD_H+2][BOARD_W+2];

typedef struct {
    Board bubbles, blocks, bubbles_prev;
    int nblock;
    int undo_possible;
    int count[BOARD_H*BOARD_W];
    int max_count;
    int click_i, click_j, cur_block;
    int score, score_prev, gains, bonus;
    enum { S_START, S_HELP, S_PLAY, S_SEL1, S_SEL2, 
           S_SEL_ANIM, S_END_ANIM, S_END } state;
    int anim_step;
} Game;


void generate_bubbles (Board bubbles)
{
    int i, j;

    for (i = 0; i < BOARD_H+2; i++)
    for (j = 0; j < BOARD_W+2; j++)
       bubbles[i][j] = (i == 0 || i == BOARD_H+1 ||
                        j == 0 || j == BOARD_W+1 ) ?
           0 : 1 + ez_random (BUBBLES_NB*100) % BUBBLES_NB; 
}


int mark_block (Board bubbles, Board blocks, int id_block, int i, int j)
{
    int dj[] = { 1, 0, -1, 0 }, di[] = { 0, -1, 0, 1 };
    int pi[BOARD_H*BOARD_W], pj[BOARD_H*BOARD_W], pn;
    int k, ki, kj, val_bulle, count;

    pi[0] = i; pj[0] = j; pn = 1;
    blocks[i][j] = id_block; count = 1;
    val_bulle = bubbles[i][j]; 

    while (pn > 0) {
        pn--; i = pi[pn]; j = pj[pn];
        for (k = 0; k < 4; k++) {
            ki = i + di[k];
            kj = j + dj[k];
            if (bubbles[ki][kj] == val_bulle && blocks[ki][kj] == 0) {
                pi[pn] = ki; pj[pn] = kj; pn++;
                blocks[ki][kj] = id_block; count++;
            }
        }
    }

    return count;
}


void fill_blocks (Game *game)
{
    int i, j, count;

    for (i = 0; i < BOARD_H+2; i++)
    for (j = 0; j < BOARD_W+2; j++)
        game->blocks[i][j] = 0;

    game->nblock = 0;
    game->max_count = 0;
    for (i = 1; i < BOARD_H+1; i++)
    for (j = 1; j < BOARD_W+1; j++)
        if (game->bubbles[i][j] != 0 && game->blocks[i][j] == 0) {
            game->nblock++;
            count = mark_block (game->bubbles, game->blocks, game->nblock, i, j);
            game->count[game->nblock] = count;
            if (game->max_count < count) game->max_count = count; 
        }
}


int find_block_ij (Game *game, int i, int j)
{
    if (i >= 1 && i < BOARD_H+1 && j >= 1 && j < BOARD_W+1)
        return game->blocks[i][j];
    return -1;
}


int find_block_count (Game *game, int id_block)
{
    if (id_block >= 0 && id_block < BOARD_H*BOARD_W)
        return game->count[id_block];
    return 0;
}


int find_block_gains (Game *game, int id_block)
{
    int count = find_block_count (game, id_block),
        gains = count * (count-1);

    return gains;
}


int compute_bonus (Game *game)
{
    int n, i, j;

    n = 0;
    for (i = 1; i < BOARD_H+1; i++)
    for (j = 1; j < BOARD_W+1; j++)
        if (game->bubbles[i][j] != 0) n++;

    return (n > 5) ? 0 :
           (n > 1) ? (6-n)*20 :
           n == 1 ? 200 : 1000;
}


void delete_block (Board bubbles, Board blocks, int id_block)
{
    int i, j;

    for (i = 1; i < BOARD_H+1; i++)
    for (j = 1; j < BOARD_W+1; j++)
        if (blocks[i][j] == id_block)
            bubbles[i][j] = 0;
}


int bubbles_will_fall (Board bubbles, Board blocks, int bang)
{
    int i, j, k;

    for (j = 1; j < BOARD_W+1; j++) {
        for (i = k = BOARD_H; i >= 1; i--)
            if (bubbles[i][j] != 0 && blocks[i][j] != bang) {
                if (i != k) return 1;
                k--;
            }
    }
    return 0;
}


void pack_bubble_down (Board bubbles)
{
    int i, j, k;

    for (j = 1; j < BOARD_W+1; j++) {
        for (i = k = BOARD_H; i >= 1; i--)
            if (bubbles[i][j] != 0)
                bubbles[k--][j] = bubbles[i][j];
        for ( ; k >= 1; k--)
            bubbles[k][j] = 0;
    }
}


void pack_bubble_right (Board bubbles)
{
    int i, j, k;

    for (j = k = BOARD_W; j >=1 ; j--) {
        if (j != k)
            for (i = 1; i < BOARD_H+1; i++)
                bubbles[i][k] = bubbles[i][j];
        if (bubbles[BOARD_H][j] != 0) k--;
    }
    for ( ; k >= 1; k--)
        for (i = 1; i < BOARD_H+1; i++)
            bubbles[i][k] = 0;
}


void new_game (Game *game)
{
    generate_bubbles (game->bubbles);
    fill_blocks (game);
    game->click_i = game->click_j = game->cur_block = -1;
    game->score = game->gains = game->bonus =  0;
}


void copy_board (Board dest, Board src)
{
    int i, j;

    for (i = 0; i < BOARD_H+2; i++)
    for (j = 0; j < BOARD_W+2; j++)
        dest[i][j] = src[i][j];
}


void store_for_undo (Game *game)
{
    copy_board (game->bubbles_prev, game->bubbles);
    game->score_prev = game->score;
    game->undo_possible = 1;
}


int make_undo (Game *game)
{
    if (game->undo_possible == 0) return 0;
    copy_board (game->bubbles, game->bubbles_prev);
    game->score = game->score_prev;
    game->undo_possible = 0;
    return 1;
}


/*--------------------------------- G U I -----------------------------------*/

#define ANIM_SEL    12
#define DELAY_SEL   30
#define ANIM_END    40
#define DELAY_END   20

#define CELL_XB     10
#define CELL_YB     40
#define CELL_YC     30
#define CELL_XM     40
#define CELL_YM     40

#define PANEL_NAME  "images/bubblets.png"
#define PANEL_ROWS   2
#define IMAGE_W     64
#define IMAGE_H     64


typedef struct {
    Game *game;
    Ez_window win1;
    int cell_xb, cell_yb, cell_yc, cell_xm, cell_ym, win1_w, win1_h;
    Ez_uint32 color_inner[BUBBLES_NB+1], color_border[BUBBLES_NB+1],
              color_title;
    Ez_image  *images_panel;
    Ez_pixmap *pixmaps[BUBBLES_NB+1];
    int images_index[BUBBLES_NB+1];
} Gui;


void coords_init (Gui *gui)
{
    gui->cell_xb = CELL_XB;
    gui->cell_yb = CELL_YB;
    gui->cell_yc = CELL_YC;
    gui->cell_xm = CELL_XM;
    gui->cell_ym = CELL_YM;
    gui->win1_w = CELL_XB + CELL_XM*BOARD_W + CELL_XB;
    gui->win1_h = CELL_YB + CELL_YM*BOARD_H + CELL_YC;
}


int coords_adapt (Gui *gui)
{
    int cell_w, cell_h, k, rescale = 0;

    cell_w = (gui->win1_w - CELL_XB - CELL_XB) / BOARD_W;
    cell_h = (gui->win1_h - CELL_YB - CELL_YC) / BOARD_H;
    k = cell_w < cell_h ? cell_w : cell_h;
    if (k < 5) k = 5;
    if (gui->cell_xm != k || gui->cell_ym != k) {
        gui->cell_xm = k; gui->cell_ym = k;
        rescale = 1;
    }
    gui->cell_xb = (gui->win1_w - gui->cell_xm*BOARD_W)/2;
    gui->cell_yb = (gui->win1_h - gui->cell_ym*BOARD_H + CELL_YB - CELL_YC)/2;
    return rescale;
}


void colors_init (Gui *gui)
{
    double s = 0.70, v1 = 0.99, v2 = 0.8;
    double h[BUBBLES_NB+1] = { 0, 220, 290, 120, 0, 60};
    int i;

    for (i = 1; i <= BUBBLES_NB; i++) {
        gui->color_inner [i] = ez_get_HSV (h[i], s, v1);
        gui->color_border[i] = ez_get_HSV (h[i], s, v2);
    }

    gui->color_title = ez_get_RGB (239, 129, 0);
}


void set_image_bg (Ez_image *img, int R, int B, int G, int A)
{
    Ez_image *tmp;
    if (img == NULL) return;
    tmp = ez_image_dup (img);
    ez_image_fill_rgba (img, R, G, B, A);
    ez_image_blend (img, tmp, 0, 0);
    ez_image_destroy (tmp);
    if (A == 255) img->has_alpha = 0;
}


void images_init (Gui *gui)
{
    gui->images_panel = ez_image_load (PANEL_NAME);
    set_image_bg (gui->images_panel, 255, 255, 255, 255);
}


void pixmaps_init (Gui *gui)
{
    int i;
    for (i = 1; i <= BUBBLES_NB; i++)
        gui->pixmaps[i] = NULL;
}


void random_images_index (Gui *gui)
{
    int i;
    for (i = 1; i <= BUBBLES_NB; i++)
        gui->images_index[i] = ez_random(PANEL_ROWS);
}


void rescale_pixmaps (Gui *gui)
{
    Ez_image *tmp1, *tmp2;
    int i, j;
    double scale;

    if (gui->images_panel == NULL) return;

    for (i = 1; i <= BUBBLES_NB; i++) {
        if (gui->pixmaps[i] != NULL) ez_pixmap_destroy (gui->pixmaps[i]);

        scale = (double) (gui->cell_xm + gui->cell_ym - 4) / (IMAGE_W + IMAGE_H);

        j = gui->images_index[i];
        tmp1 = ez_image_extract (gui->images_panel, 
            (i-1)*IMAGE_W, j*IMAGE_H, IMAGE_W, IMAGE_H);
        tmp2 = ez_image_scale (tmp1, scale);
        gui->pixmaps[i] = ez_pixmap_create_from_image (tmp2);
        ez_image_destroy (tmp1);
        ez_image_destroy (tmp2);
    }
}


/* Coordonnes board j,i --> Coordinates pixels x,y */

int board_get_x (Gui *gui, int j)
{
    return gui->cell_xb + (j-1)*gui->cell_xm;
}

int board_get_y (Gui *gui, int i)
{
    return gui->cell_yb + (i-1)*gui->cell_ym;
}


/* Coordinates pixels x,y --> Coordinates board j,i */

int board_get_j (Gui *gui, int x)
{
    return 1 + (x - gui->cell_xb) / gui->cell_xm - (x < gui->cell_xb);
}

int board_get_i (Gui *gui, int y)
{
    return 1 + (y - gui->cell_yb) / gui->cell_ym - (y < gui->cell_yb);
}


void draw_block_boundary (Ez_window win, Gui *gui, int id_block)
{
    Game *game = gui->game;
    int i, j, x, y;

    ez_set_color (ez_black);

    /* Vertical lines */
    for (i = 1; i < BOARD_H+1; i++)
    for (j = 0; j < BOARD_W+1; j++)
        if ( (game->blocks[i][j] == id_block && game->blocks[i][j+1] != id_block) ||
             (game->blocks[i][j] != id_block && game->blocks[i][j+1] == id_block) )
        {
            x = board_get_x (gui, j+1);
            y = board_get_y (gui, i);
            ez_draw_line (win, x, y, x, y+gui->cell_ym);
        }

    /* Horizontal lines */
    for (i = 0; i < BOARD_H+1; i++)
    for (j = 1; j < BOARD_W+1; j++)
        if ( (game->blocks[i][j] == id_block && game->blocks[i+1][j] != id_block) ||
             (game->blocks[i][j] != id_block && game->blocks[i+1][j] == id_block) )
        {
            x = board_get_x (gui, j);
            y = board_get_y (gui, i+1);
            ez_draw_line (win, x, y, x+gui->cell_xm, y);
        }

}


void draw_bubble_xy (Ez_window win, Gui *gui, int x, int y, int val)
{
    if (val <= 0 || val > BUBBLES_NB) return;

    if (gui->pixmaps[val] != NULL) {
      ez_pixmap_paint (win, gui->pixmaps[val], x+1, y+1);
    } else {
       int x1 = x+2, y1 = y+2, x2 = x+gui->cell_xm-2, y2 = y+gui->cell_ym-2;

       ez_set_color (gui->color_inner[val]);
       ez_fill_circle (win, x1, y1, x2-1, y2-1);

       ez_set_color (gui->color_border[val]);
       ez_draw_circle (win, x1, y1, x2, y2);
    }
}


void draw_bang_xy (Ez_window win, Gui *gui, int x, int y)
{
    int d = (gui->cell_xm+gui->cell_ym)/10,
        x1 = x+d,              y1 = y+d, 
        x2 = x+gui->cell_xm/2, y2 = y+gui->cell_ym/2,
        x3 = x+gui->cell_xm-d, y3 = y+gui->cell_ym-d;

    ez_set_color (ez_yellow);
    ez_set_thick (3);
    ez_draw_line (win, x1, y2, x3, y2);
    ez_draw_line (win, x1, y1, x3, y3);
    ez_draw_line (win, x1, y3, x3, y1);
    ez_draw_line (win, x2, y1, x2, y3);
    ez_set_thick (1);

    ez_set_color (ez_white);
    ez_fill_rectangle (win, x2-3, y2-3, x2+3, y2+3);
}



void draw_bubbles (Ez_window win, Gui *gui)
{
    Game *game = gui->game;
    int i, j, x, y;

    for (i = 1; i < BOARD_H+1; i++)
    for (j = 1; j < BOARD_W+1; j++)
    {
        x = board_get_x (gui, j);
        y = board_get_y (gui, i);
        draw_bubble_xy (win, gui, x, y, game->bubbles[i][j]);
    }
}


void draw_bubbles_bang (Ez_window win, Gui *gui, int bang)
{
    Game *game = gui->game;
    int i, j, x, y;

    for (i = 1; i < BOARD_H+1; i++)
    for (j = 1; j < BOARD_W+1; j++)
    {
        x = board_get_x (gui, j);
        y = board_get_y (gui, i);
        if (game->bubbles[i][j] != 0 && game->blocks[i][j] == bang)
             draw_bang_xy   (win, gui, x, y);
        else draw_bubble_xy (win, gui, x, y, game->bubbles[i][j]);
    }
}


void draw_bubbles_drop (Ez_window win, Gui *gui, int bang, int step)
{
    Game *game = gui->game;
    int i, j, k, x, y, y1, y2;

    for (j = 1; j < BOARD_W+1; j++) {
        x = board_get_x (gui, j);
        for (i = k = BOARD_H; i >= 1; i--)
            if (game->bubbles[i][j] != 0 && game->blocks[i][j] != bang) {
                y1 = board_get_y (gui, i);
                y2 = board_get_y (gui, k);
                y = (y2*step + y1*(ANIM_SEL-step))/ANIM_SEL;
                draw_bubble_xy (win, gui, x, y, game->bubbles[i][j]);
                k--;
            }
    }
}


void draw_help (Ez_window win, Gui *gui)
{
    int text_w = 380, text_h = 220,
        x1 = (gui->win1_w - text_w)/2, y1 = (gui->win1_h - text_h)/2,
        x2 = (gui->win1_w + text_w)/2, y2 = (gui->win1_h + text_h)/2;

    ez_set_color (ez_white);
    ez_fill_rectangle (win, x1, y1, x2, y2);
    ez_set_color (gui->color_title);
    ez_draw_rectangle (win, x1, y1, x2, y2);
   
    ez_set_nfont (1);  ez_set_color (ez_black);
    ez_draw_text (win, EZ_TC, gui->win1_w/2, y1+20,
        "Click on a block (of at least two bubbles\n"
        "having same color) to see the gains.\n\n"
        "Click a second time on the block to\n"
        "delete the block and get the gains.\n\n"
        "The game is finished when there is no block.\n\n"
        "The larger the block, the more it pays ...\n"
        "Maximize your score !!"
    );
}


void draw_about (Ez_window win, Gui *gui)
{
    ez_set_color (ez_grey); ez_set_nfont (0);
    ez_draw_text (win, EZ_BC, gui->win1_w/2, gui->win1_h-50,
        "The program jeu-bubblet.c is part of EZ-Draw:\n"
        "http://pageperso.lif.univ-mrs.fr/~edouard.thiel/ez-draw");
}


void draw_final (Ez_window win, Gui *gui)
{
    Game *game = gui->game;
    int text_w = 300, text_h = 160,
        x1 = (gui->win1_w - text_w)/2, x2 = x1 + text_w,
        y1 = (gui->win1_h - text_h)/2, y2 = y1 + text_h;

    if (game->state == S_END_ANIM) {
        y1 = -text_h + (text_h+y1) * game->anim_step / ANIM_END;
        y2 = y1 + text_h;
    }

    ez_set_color (ez_white);
    ez_fill_rectangle (win, x1, y1, x2, y2);
    ez_set_color (gui->color_title);
    ez_draw_rectangle (win, x1, y1, x2, y2);

    ez_set_nfont (3); ez_set_color (ez_blue);
    ez_draw_text (win, EZ_TC, gui->win1_w/2, y1+30, 
        "GAME OVER !!\n\nBonus %d\nFinal score %d", game->bonus, game->score);
}


void step_sel_anim (Ez_window win, Gui *gui)
{
    Game *game = gui->game;

    if (game->anim_step == 0 && 
        ! bubbles_will_fall (game->bubbles, game->blocks, game->cur_block))
         game->anim_step = ANIM_SEL;  /* animation is useless */
    else game->anim_step ++;

    if (game->anim_step >= ANIM_SEL) {  /* animation is done; play the shot */
        int gains;

        store_for_undo (game);
        gains = find_block_gains (game, game->cur_block);
        game->score += gains;
        delete_block (game->bubbles, game->blocks, game->cur_block);
        pack_bubble_down (game->bubbles);
        pack_bubble_right (game->bubbles);
        fill_blocks (game);
        game->click_i = game->click_j = game->cur_block = -1;
        game->state = S_PLAY;
        if (game->max_count <= 1) {  /* no more block */
            game->bonus = compute_bonus (game);
            game->score += game->bonus;
            game->state = S_END_ANIM;
            game->anim_step = 0;
            ez_start_timer (win, DELAY_END);
        }
    }

    if (game->state == S_SEL_ANIM) ez_start_timer (win, DELAY_SEL);
    ez_send_expose (win);
}


void step_end_anim (Ez_window win, Gui *gui)
{
    Game *game = gui->game;

    if (game->anim_step < ANIM_END) {
        game->anim_step++;
        ez_start_timer (win, DELAY_END);
    }
    else {
        game->state = S_END;
    }
    ez_send_expose (win);
}


/*------------------------ E V E N T S   W I N   1 --------------------------*/


void win1_on_Expose (Ez_event *ev, Gui *gui)
{
    Game *game = gui->game;
    int sta = game->state;

    if (sta == S_SEL2)
         draw_bubbles_bang (ev->win, gui, game->cur_block);
    else if (sta == S_SEL_ANIM)
         draw_bubbles_drop (ev->win, gui, game->cur_block, game->anim_step);
    else if (sta != S_START)
         draw_bubbles (ev->win, gui);

    if (sta == S_SEL1)
        draw_block_boundary (ev->win, gui, game->cur_block);

    ez_set_nfont (3); ez_set_color (gui->color_title); 
    ez_draw_text (ev->win, EZ_TC, gui->win1_w/2, 10, "BUBBLET");

    if (sta == S_SEL1 || sta == S_SEL2 || sta == S_SEL_ANIM) {
        int gains = find_block_gains (game, game->cur_block);
        ez_set_nfont (2); ez_set_color (ez_red);
        ez_draw_text (ev->win, EZ_TR, gui->win1_w-10, 10, 
            gains > 0 ? "Gains: %d" : "Not good!", gains);
    }

    if (sta == S_PLAY || sta == S_SEL1 || sta == S_SEL2 || 
        sta == S_SEL_ANIM) {
        ez_set_nfont (2); ez_set_color (ez_blue);
        ez_draw_text (ev->win, EZ_TL, 10, 10, 
            "Score: %d", game->score);
    }

    ez_set_nfont (0);
    ez_set_color (ez_black);
    ez_draw_text (ev->win, EZ_BL, 10, gui->win1_h-10, 
        sta == S_START ? "n: new game" : "n: new game  a: aspect");
    ez_draw_text (ev->win, EZ_BR, gui->win1_w-10, gui->win1_h-10, 
        sta == S_START || sta == S_HELP || sta == S_SEL1  || sta == S_PLAY ? 
        "h: help  q: quit" : "q: quit");
    if ((sta == S_PLAY || sta == S_SEL1 || sta == S_SEL2) &&
         game->undo_possible)
        ez_draw_text (ev->win, EZ_BC, gui->win1_w/2, gui->win1_h-10, "u: undo");

    switch (sta) {
        case S_START    : draw_about (ev->win, gui); 
        case S_HELP     : draw_help  (ev->win, gui); break;
        case S_END_ANIM :
        case S_END      : draw_final (ev->win, gui); break;
        default         : break;
    }
}


void win1_on_ConfigureNotify (Ez_event *ev, Gui *gui)
{
    gui->win1_w = ev->width;
    gui->win1_h = ev->height;
    if (coords_adapt (gui)) rescale_pixmaps (gui);
}


void win1_on_ButtonPress (Ez_event *ev, Gui *gui)
{
    Game *game = gui->game;
    int id_block;

    if (game->state != S_PLAY && game->state != S_SEL1) return;

    game->click_i = board_get_i (gui, ev->my);
    game->click_j = board_get_j (gui, ev->mx);
    id_block = find_block_ij (game, game->click_i, game->click_j);

    switch (game->state) {
        case S_PLAY :
            if (id_block > 0) game->state = S_SEL1;
            break;
        case S_SEL1 :
            if (game->cur_block == id_block) game->state = S_SEL2;
            else if (id_block > 0) game->state = S_SEL1;
            else game->state = S_PLAY;
            break;
        default : break;
    }

    game->cur_block = id_block;
    ez_send_expose (ev->win);
}


void win1_on_ButtonRelease (Ez_event *ev, Gui *gui)
{
    Game *game = gui->game;

    switch (game->state) {
        case S_START :
        case S_HELP :
            game->state = S_PLAY;
            ez_send_expose (ev->win);
            break;
        case S_SEL1 : {
            int gains = find_block_gains (game, game->cur_block);

            if (gains == 0) {
                game->state = S_PLAY;
                game->cur_block = -1;
            }
            ez_send_expose (ev->win);
            break;
        }
        case S_SEL2 :
            game->state = S_SEL_ANIM;
            game->anim_step = 0;
            ez_start_timer (ev->win, DELAY_SEL);
            ez_send_expose (ev->win);
            break;
        default : break;
    }
}


void win1_on_KeyPress (Ez_event *ev, Gui *gui)
{
    Game *game = gui->game;

    switch (ev->key_sym) {

        case XK_q : 
            ez_quit (); 
            break;

        case XK_h : 
            if (game->state == S_START || game->state == S_HELP) 
                game->state = S_PLAY;
            else if (game->state == S_PLAY || game->state == S_SEL1) 
                game->state = S_HELP;
            else return;
            ez_send_expose (ev->win);
            break;

        case XK_n :
            if (game->state != S_END_ANIM && game->state != S_END)
                 store_for_undo (game);
            else game->undo_possible = 0;
            new_game (game);
            game->state = S_PLAY;
        case XK_a :
            random_images_index (gui);
            rescale_pixmaps (gui);
            ez_send_expose (ev->win);
            break;

        case XK_u :
            if (game->state == S_PLAY || game->state == S_SEL1 || game->state == S_SEL2)
                if (make_undo (game)) {
                    game->state = S_PLAY;
                    fill_blocks (game);
                    game->click_i = game->click_j = game->cur_block = -1;
                    ez_send_expose (ev->win);
                }
            break;
    }
}


void win1_on_TimerNotify (Ez_event *ev, Gui *gui)
{
    Game *game = gui->game;

    switch (game->state) {
        case S_SEL_ANIM : step_sel_anim (ev->win, gui); break;
        case S_END_ANIM : step_end_anim (ev->win, gui); break;
        default : break;
    }
}


void win1_on_event (Ez_event *ev)           /* Called for each event on win1 */
{
    Gui *gui = ez_get_data (ev->win);

    switch (ev->type) {
        case Expose          : win1_on_Expose          (ev, gui); break;
        case ConfigureNotify : win1_on_ConfigureNotify (ev, gui); break;
        case ButtonPress     : win1_on_ButtonPress     (ev, gui); break;
        case ButtonRelease   : win1_on_ButtonRelease   (ev, gui); break;
        case KeyPress        : win1_on_KeyPress        (ev, gui); break;
        case TimerNotify     : win1_on_TimerNotify     (ev, gui); break;
    }
}


/*-------------------------- G E N E R A L   I N I T ------------------------*/


void game_init (Game *game)
{
    new_game (game);
    game->state = S_START;
}


void gui_init (Gui *gui, Game *game)
{
    gui->game = game;

    coords_init (gui);
    colors_init (gui);
    images_init (gui);
    pixmaps_init (gui);
    random_images_index (gui);

    gui->win1 = ez_window_create (gui->win1_w, gui->win1_h, 
        "Bubblet game", win1_on_event);
    ez_window_dbuf (gui->win1, 1);
    ez_set_data (gui->win1, gui);

    coords_adapt (gui);
    rescale_pixmaps (gui);
}


/*--------------------------- M A I N   P R O G R A M -----------------------*/


int main ()
{
    Game game;
    Gui gui;
    
    if (ez_init() < 0) exit(1);

    game_init (&game);
    gui_init (&gui, &game);

    ez_main_loop ();
    exit(0);
}

