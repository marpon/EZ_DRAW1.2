/* jeu-sudoku.c: a game of sudoku
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 27/03/2014 - version 1.2
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


/* This program draws a sudoku grid and allows to play.
   The game_solve() function is to be completed.
*/


/*--------------------------------- G A M E ---------------------------------*/

typedef struct {
    int statement[81], attempt[81];     /* Cell x,y : [y*9+x] */
    int current;                        /* Current cell 0..80 or -1 for none */
    enum { S_EDIT, S_PLAY, S_WIN } state;
} Game;


void game_init (Game *game)
{
    int k;
    for (k = 0; k < 81; k++)
        game->statement[k] = game->attempt[k] = 0;
    game->state = S_EDIT;
    game->current = -1;
}


void game_set_statement (Game *game, char *s)
{
    int i;

    for (i = 0; i < 81 && s[i] != 0; i++)
        if (s[i] >= '0' && s[i] <= '9')
            game->statement[i] = game->attempt[i] = s[i] - '0';
}


void game_copy_statement (Game *game)
{
    int k;
    for (k = 0; k < 81; k++)
        game->attempt[k] = game->statement[k];
}


int game_value_is_possible (Game *game, int k, int val)
{
    int i = k/9, j = k%9, t, x, y;

    for (t = 0; t < 9; t++)
        if (t*9+j != k && game->attempt[t*9+j] == val) return 0;

    for (t = 0; t < 9; t++)
        if (i*9+t != k && game->attempt[i*9+t] == val) return 0;

    i = (i/3)*3; j = (j/3)*3;
    for (y = i; y < i+3; y++)
        for (x = j; x < j+3; x++)
            if (y*9+x != k && game->attempt[y*9+x] == val) return 0;

    return 1;
}


int game_is_solved (Game *game)
{
    int k;
    for (k = 0; k < 81; k++) {
         if (game->attempt[k] == 0)
             return 0;
         if (! game_value_is_possible (game, k, game->attempt[k]))
             return 0;
    }
    return 1;
}


void game_set_current_value (Game *game, int val)
{
    if (game->current < 0) return;

    if (game->state == S_EDIT) {
        game->statement[game->current] = game->attempt[game->current] = val;
    } else if (game->statement[game->current] == 0) {
        game->attempt[game->current] = val;
        game->state = S_PLAY;
        if (game_is_solved (game)) game->state = S_WIN;
    }
}


void game_move_current (Game *game, int dir)
{
    game->current = (game->current < 0) ? 0 : (game->current + dir + 81) % 81;
}


void game_deduce_current_value (Game *game)
{
    if (game->current != -1 && game->attempt[game->current] == 0) {

        int w, n = 0, k = -1;

        for (w = 1 ; w <= 9; w++)
            if (game_value_is_possible (game, game->current, w)) { k = w; n++; }

        if (n == 1) {
            game->attempt[game->current] = k;
            if (game_is_solved (game)) game->state = S_WIN;
        }
    }
}


/*---------------------- S O L V I N G   A L G O R I T H M ------------------*/


void game_solve (Game *game)
{
    /* Write here your own solving algorithm */


    /* Suppress this temporary code */
    int k;
    for (k = 0; k < 81; k++) {
        if (game->statement[k] == 0 && game->attempt[k] == 0)
           game->attempt[k] = 9;
    }
    printf ("The solving algorithm is not yet implemented !!\n");

}


/*---------------------------------- G U I ----------------------------------*/

#define CELL_SIZE   36
#define TITLE_TOP   25
#define LEGEND_TOP  10


typedef struct {
    Game *game;
    Ez_window win1;    /* Main window */
    Ez_window win2;    /* Help window */
} Gui;


int clicked_cell (int x, int y)
{
    int a = TITLE_TOP, b = CELL_SIZE/2, c = CELL_SIZE, i, j, k;
    if (x < b || x >= b+9*c || y < a+b || y >= a+b+9*c) return -1;
    i = (y-a-b) / c; j = (x-b) / c; k = i*9+j;
    if (k < 0 || k >= 81)
      { printf ("Internal error in clicked_cell\n"); return -1; }
    return k;
}


void draw_title (Ez_window win, Gui *gui)
{
    ez_set_color (ez_magenta); ez_set_nfont (3);
    ez_draw_text (win, EZ_MC, CELL_SIZE*10/2, TITLE_TOP-5,
        gui->game->state == S_WIN ? "WELL DONE !!" : "S u d o k u");
}


void draw_grid (Ez_window win, Gui *gui)
{
    int i, a = TITLE_TOP, b = CELL_SIZE/2, c = CELL_SIZE;
    (void) gui;
    for (i = 0; i <= 9; i++) {
        ez_set_color (i % 3 == 0 ? ez_blue : ez_grey);
        ez_draw_line (win, b+i*c, a+b, b+i*c, a+b+9*c);
        ez_draw_line (win, b, a+b+i*c, b+9*c, a+b+i*c);
    }
}


void draw_legend (Ez_window win, Gui *gui)
{
    int a = TITLE_TOP, b = CELL_SIZE/2, c = CELL_SIZE, d = LEGEND_TOP;
    ez_set_color (ez_black); ez_set_nfont (0);
    ez_draw_text (win, EZ_BL, b, a+c*10+d-4, "h: help");
    ez_draw_text (win, EZ_BR, b+9*c, a+c*10+d-4,
        gui->game->state == S_EDIT ? "editing mode" : "playing mode");
}


void draw_cell (Ez_window win, Gui *gui, int i, int j)
{
    Game *game = gui->game;
    int k = i*9+j, w, x = (j+1)*CELL_SIZE, y = TITLE_TOP + (i+1)*CELL_SIZE;

    ez_set_nfont (3);
    if (game->statement[k] != 0) {
        /* Statement of the game */
        ez_set_color (ez_blue);
        ez_draw_text (win, EZ_MC, x, y, "%d", game->statement[k]);

    } else if (game->attempt[k] != 0) {
        /* Played cells */
        if (k == game->current &&
            ! game_value_is_possible (game, game->current, game->attempt[game->current]))
             ez_set_color (ez_red);
        else ez_set_color (ez_black);
        ez_draw_text (win, EZ_MC, x, y, "%d", game->attempt[k]);

    } else if (k == game->current && game->state == S_PLAY) {

        /* Possibilities for this cell */
        ez_set_nfont (0);
        ez_set_color (ez_black);
        for (w = 1 ; w <= 9; w++)
            if (game_value_is_possible (game, game->current, w))
                ez_draw_text (win, EZ_MC,
                    x + ((w-1)%3)*11 - 11,
                    y + ((w-1)/3)*11 - 11,
                    "%d", w);
    }
}


void draw_cells (Ez_window win, Gui *gui)
{
    int i, j;

    for (i = 0; i < 9; i++)
    for (j = 0; j < 9; j++)
        draw_cell (win, gui, i, j);
}


void draw_current_cell (Ez_window win, Gui *gui)
{
    Game *game = gui->game;
    if (game->current >= 0) {
        int i, j, a = TITLE_TOP, b = CELL_SIZE/2, c = CELL_SIZE;
        ez_set_color (ez_magenta);
        i = game->current / 9; j = game->current % 9;
        ez_draw_rectangle (win, b+j*c+1, a+b+i*c+1, b+j*c+c-1, a+b+i*c+c-1);
    }
}


/*------------------------ E V E N T S   W I N   1 --------------------------*/


void win1_on_Expose (Ez_event *ev, Gui *gui)
{
    draw_title        (ev->win, gui);
    draw_grid         (ev->win, gui);
    draw_legend       (ev->win, gui);
    draw_cells        (ev->win, gui);
    draw_current_cell (ev->win, gui);
}


void win1_on_ButtonPress (Ez_event *ev, Gui *gui)
{
    Game *game = gui->game;
    game->current = clicked_cell (ev->mx, ev->my);
    ez_send_expose (ev->win);
}


void win1_on_KeyPress (Ez_event *ev, Gui *gui)
{
    Game *game = gui->game;

    switch (ev->key_sym) {

        case XK_e : game->state = S_EDIT; break;
        case XK_p : game->state = S_PLAY;  break;

        case XK_Up    : game_move_current (game, -9); break;
        case XK_Down  : game_move_current (game,  9); break;
        case XK_Left  : game_move_current (game, -1); break;
        case XK_Right : game_move_current (game,  1); break;

        case XK_Return :
        case XK_KP_Enter:
            if (game->state != S_PLAY) return;
            game_deduce_current_value (game);
            game_move_current (game, 1);
            break;

        case XK_s:
            if (game->state != S_PLAY) return;
            game_solve (game);
            if (game_is_solved (game)) game->state = S_WIN;
            break;

        case XK_Delete :
            game_set_current_value (game, 0);
            break;

        case XK_v :
            if (game->state == S_EDIT) game_init (game);
            else { game->state = S_PLAY ; game_copy_statement (game); }
            break;

        case XK_q : ez_quit (); return;
        case XK_h : ez_window_show (gui->win2, 1); return;

        default :
            if (ev->key_string[0] >= '1' && ev->key_string[0] <= '9')
                game_set_current_value (game, ev->key_string[0] - '0');
            else return;
    }

    ez_send_expose (ev->win);
}


void win1_on_WindowClose (void)
{
    ez_quit ();
}


void win1_on_event (Ez_event *ev)           /* Called for each event on win1 */
{
    Gui *gui = ez_get_data (ev->win);
    switch (ev->type) {
        case Expose        : win1_on_Expose       (ev, gui); break;
        case ButtonPress   : win1_on_ButtonPress  (ev, gui); break;
        case KeyPress      : win1_on_KeyPress     (ev, gui); break;
        case WindowClose   : win1_on_WindowClose  ();        break;
    }
}


/*------------------------ E V E N T S   W I N   2 --------------------------*/


void win2_on_Expose (Ez_event *ev)
{
    ez_set_color (ez_black);
    ez_draw_text (ev->win, EZ_TL, 10, 10,
        "h: help              Suppr: clear a cell\n"
        "e: editing mode          v: clear the grid\n"
        "p: playing mode      Enter: magic!\n"
        "q: quit                  s: solve the grid\n"
        "\n"
        "To modify a cell, click on the cell, then press\n"
        "key 1 to 9.\n"
        "\n"
        "If there is only one possibility, hit Enter.\n"
        "\n"
         "You can use arrows to move.\n"
        "\n"
        "\n"
        "The program jeu-sudoku.c is part of EZ-Draw:\n"
        "http://pageperso.lif.univ-mrs.fr/~edouard.thiel/ez-draw"
    );
}


void win2_on_event (Ez_event *ev)           /* Called for each event on win2 */
{
    switch (ev->type) {
        case Expose        : win2_on_Expose (ev); break;

        case ButtonPress   :
        case KeyPress      :
        case WindowClose   : ez_window_show (ev->win, 0); break;
    }
}


/*-------------------------- G E N E R A L   I N I T ------------------------*/


void gui_init (Gui *gui, Game *game)
{
    gui->game = game;

    gui->win1 = ez_window_create (
        CELL_SIZE*10, CELL_SIZE*10 + TITLE_TOP + LEGEND_TOP,
        "Sudoku Game", win1_on_event);
    ez_window_dbuf (gui->win1, 1);
    ez_set_data (gui->win1, gui);

    gui->win2 = ez_window_create (400, 300, "Sudoku Help", win2_on_event);
    ez_window_show (gui->win2, 0);
    ez_set_data (gui->win2, game);

    ez_auto_quit (0);
}


/*--------------------------- M A I N   P R O G R A M -----------------------*/


int main (int argc, char *argv[])
{
    Game game;
    Gui gui;

    if (ez_init() < 0) exit(1);

    game_init (&game);
    gui_init (&gui, &game);

    if (argc == 2) {
        game_set_statement (&game, argv[1]);
        game.state = S_PLAY;
    } else printf ("Usage: %s [game]\nExamples:\n"
        "%s 720000040300090060000080003007109680042000900800306005270000010004600290003710006\n"
        "%s 907100000405320001000070200000090003008507400500010000006030000100046907000001506\n"
        "%s 480000006001006050062500400000004000600098001000100800007039210050000603300000049\n",
        argv[0], argv[0], argv[0], argv[0]);

    ez_main_loop ();
    exit(0);
}

