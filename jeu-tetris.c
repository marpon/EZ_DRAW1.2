/* jeu-tetris.c: a tetris-like game
 *
 * by Julien Prudhomme <julien.prudhomme@etu.univ-amu.fr> - 02/06/2016
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "ez-draw.h"


/*------------------------- D E F I N I T I O N S ---------------------------*/

#define WIN_TITLE     "Tetrominos"
#define TIMER         50
#define LEVEL_MAX     20
#define HISTORY_SIZE   4
#define CHOOSE_TRY     4
#define BSIZE          5                         /* size of a tetromino */

#define GRID_W        10
#define GRID_H        22
#define CELLSIZE      30
#define WIN_WIDTH     ((GRID_W+6)*CELLSIZE)
#define WIN_HEIGHT    ((GRID_H-1)*CELLSIZE)

typedef enum {
  T_I, T_O, T_T, T_L, T_G, T_Z, T_S, T_N
} Type;

typedef enum {
  S_RUN, S_PAUSE, S_LOOSE, S_LINES
} State;

typedef enum {
  false, true
} Boolean;

typedef struct {
  Type type;
  int i, j;
  Ez_uint32 blocks[BSIZE][BSIZE];
} Tetromino;

typedef struct {
  Boolean downP, downR;
} Controls;

typedef struct {
  Tetromino next;
  Tetromino current;
  Type history[HISTORY_SIZE];
  Ez_uint32 board[GRID_W][GRID_H];
  int level;
  int step;
  int score;
  int total_lines;
  int current_lines;
  int anim_game_over;
  int anim_lines;
  Boolean lines[GRID_H];
  State state;
  State prev_state;
  Controls controls;
} Game;

typedef struct {
  Ez_window win;
  Game game;
} Data;


/*----------------------------- C O N T R O L S -----------------------------*/

void controls_pressDown (Controls *this)
{
  if (this->downR) {
    this->downR = false;
    this->downP = true;
  }
}


void controls_releaseDown (Controls *this)
{
  this->downR = true;
  this->downP = false;
}


Boolean controls_isDownPressed (Controls *this)
{
  return this->downP;
}


void controls_init (Controls *this)
{
  this->downP = false;
  this->downR = true;
}


/*----------------------------T E T R O M I N O -----------------------------*/

void block_copy (Ez_uint32 src[BSIZE][BSIZE], Ez_uint32 dst[BSIZE][BSIZE])
{
  int i, j;
  for (i = 0; i < BSIZE; i++) {
    for (j = 0; j < BSIZE; j++) dst[i][j] = src[i][j];
  }
}


void block_apply_color (Ez_uint32 src[BSIZE][BSIZE], Ez_uint32 dst[BSIZE][BSIZE],
                        Ez_uint32 color)
{
  int i, j;
  for (i = 0; i < BSIZE; i++) {
    for (j = 0; j < BSIZE; j++) dst[i][j] = src[i][j] == 1 ? color : 0;
  }
}


void block_rotate_90 (Ez_uint32 block[BSIZE][BSIZE])
{
  Ez_uint32 tmp[BSIZE][BSIZE];
  int i, j;
  block_copy (block, tmp);
  for (i = 0; i < BSIZE; i++) {
    for (j = 0; j < BSIZE; j++) block[i][j] = tmp[BSIZE-1-j][i];
  }
}


void tetromino_rotate (Tetromino *this)
{
  block_rotate_90 (this->blocks);
}


void tetromino_top_block (Tetromino *this)
{
  int i, j;
  for (j = 0; j < 2; j++) {
    for (i = 0; i < BSIZE; i++) {
      if (this->blocks[i][j] != 0) {
        this->j++;
        return;
      }
    }
  }
}


void tetromino_init_Z (Tetromino *this)
{
  Ez_uint32 tmp[BSIZE][BSIZE] = {
    { 0, 0, 0, 0, 0},
    { 0, 1, 1, 0, 0},
    { 0, 0, 1, 1, 0},
    { 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0}
  };
  block_apply_color (tmp, this->blocks, ez_red);
}


void tetromino_init_T (Tetromino *this)
{
  Ez_uint32 tmp[BSIZE][BSIZE] = {
    { 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0},
    { 0, 1, 1, 1, 0},
    { 0, 0, 1, 0, 0},
    { 0, 0, 0, 0, 0}
  };
  block_apply_color (tmp, this->blocks, ez_magenta);
}


void tetromino_init_S (Tetromino *this)
{
  Ez_uint32 tmp[BSIZE][BSIZE] = {
    { 0, 0, 0, 0, 0},
    { 0, 0, 1, 1, 0},
    { 0, 1, 1, 0, 0},
    { 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0}
   };
  block_apply_color (tmp, this->blocks, ez_green);
}


void tetromino_init_O (Tetromino *this)
{
  Ez_uint32 tmp[BSIZE][BSIZE] = {
    { 0, 0, 0, 0, 0},
    { 0, 1, 1, 0, 0},
    { 0, 1, 1, 0, 0},
    { 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0}
  };
  block_apply_color (tmp, this->blocks, ez_yellow);
}


void tetromino_init_L (Tetromino *this)
{
  Ez_uint32 color = ez_get_RGB (237, 127, 16);
  Ez_uint32 tmp[BSIZE][BSIZE] = {
    { 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0},
    { 0, 1, 1, 1, 0},
    { 0, 1, 0, 0, 0},
    { 0, 0, 0, 0, 0}
  };
  block_apply_color (tmp, this->blocks, color);
}


void tetromino_init_I (Tetromino *this)
{
  Ez_uint32 tmp[BSIZE][BSIZE] = {
    { 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0},
    { 0, 1, 1, 1, 1},
    { 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0}
  };
  block_apply_color (tmp, this->blocks, ez_cyan);
}


void tetromino_init_G (Tetromino *this)
{
  Ez_uint32 tmp[BSIZE][BSIZE] = {
    { 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 0},
    { 0, 1, 1, 1, 0},
    { 0, 0, 0, 1, 0},
    { 0, 0, 0, 0, 0}
  };
  block_apply_color (tmp, this->blocks, ez_blue);
}


void tetromino_init_blocks (Tetromino *this)
{
  switch (this->type) {
    case T_Z: tetromino_init_Z (this); break;
    case T_T: tetromino_init_T (this); break;
    case T_S: tetromino_init_S (this); break;
    case T_O: tetromino_init_O (this); break;
    case T_L: tetromino_init_L (this); break;
    case T_I: tetromino_init_I (this); break;
    case T_G: tetromino_init_G (this); break;
    default: break;
  }
  tetromino_rotate (this);
  tetromino_top_block (this);
}


void tetromino_init (Tetromino *this, Type t)
{
  this->type = t;
  this->i = GRID_W/4;                           /* Spawn at 1/4 of board */
  this->j = -2;
  tetromino_init_blocks (this);
}


/*----------------------------------- G A M E -------------------------------*/

void game_init_history (Game *this)
{
  int i;
  for (i = 0; i < HISTORY_SIZE; i++) {
    this->history[i] = ez_random (2) ? T_S:T_Z;
  }
}


Boolean game_is_in_history (Game *this, Type t)
{
  int i;
  for (i = 0; i < HISTORY_SIZE; i++) {
    if (this->history[i] == t) return true;
  }
  return false;
}


void game_choose_next (Game *this)
{
  int i;
  Type t;
  for (i = 0; i < CHOOSE_TRY; i++) {
    t = ez_random (T_N);
    if (!game_is_in_history (this, t)) break;
  }
  tetromino_init (&this->next, t);
  for (i = HISTORY_SIZE - 1; i > 0; i--) {
    this->history[i] = this->history[i-1];
  }
  this->history[0] = t;
}


void game_init_board (Game *this)
{
  int i, j;
  for (i = 0; i < GRID_W; i++) {
    for (j = 0; j < GRID_H; j++) {
      this->board[i][j] = 0;
    }
  }
}


Boolean game_current_can_move (Game *this, int x, int y)
{
  Tetromino *c = &this->current;
  int i, j;
  for (i = 0; i < BSIZE; i++) {
    for (j = 0; j < BSIZE; j++) {
      if (c->blocks[i][j] == 0) continue;
      if (c->i + i + x >= GRID_W || c->i + i + x < 0) return false;
      if (c->j + j + y >= GRID_H-1) return false;
      if (c->j + j + y < 0) continue;
      if (this->board[c->i + i + x][c->j + j + y] != 0) return false;
    }
  }
  return true;
}


void game_take_next (Game *this)
{
  this->current = this->next;
  game_choose_next (this);
  if (!game_current_can_move (this, 0, 0)) {
    this->state = S_LOOSE;
  }
}


Boolean game_is_level_up (Game *this)
{
  return
    this->total_lines > (3 + this->level*this->level)
    && this->level < 20;
}


void game_reset_lines (Game *this)
{
  int i;
  for (i = 0; i < GRID_H; i++) {
    this->lines[i] = false;
  }
}


Boolean game_is_line_full (Game *this, int i)
{
  int j;
  for (j = 0; j < GRID_W; j++) {
    if (this->board[j][i] == 0) return false;
  }
  return true;
}


void game_check_lines (Game *this)
{
  int i;
  int n = 0;

  game_reset_lines (this);
  for (i = 0; i < GRID_H; i++) {
    if (game_is_line_full (this, i)) {
      this->lines[i] = true;
      n++;
    }
  }
  this->current_lines = n;
  if (n > 0) this->state = S_LINES;
}


void game_current_put (Game *this)
{
  int i, j;
  Tetromino *c = &this->current;
  for (i = 0; i < BSIZE; i++) {
    for (j = 0; j < BSIZE; j++) {
      if (c->blocks[i][j] != 0) {
        this->board[c->i + i][c->j + j] = c->blocks[i][j];
      }
    }
  }
  game_check_lines (this);
  if (game_is_level_up (this)) this->level++;
  game_take_next (this);
  this->controls.downP = false;
}


void game_shift_board (Game *this, int k, int n)
{
  int i, j;
  for (i = k-1; i >= 0; i--) {
    for (j = 0; j < GRID_W; j++) {
      this->board[j][i + n] = this->board[j][i];
    }
  }
  for (j = 0; j < GRID_W; j++) {
    this->board[j][0] = 0;
  }
}


void game_add_lines_score (Game *this)
{
  switch (this->current_lines) {
    case 1: this->score += this->level*10; break;
    case 2: this->score += this->level*100; break;
    case 3: this->score += this->level*500; break;
    case 4: this->score += this->level*2000; break;
    default: return;
  }
}


void game_do_lines (Game *this)
{
  int i;
  game_add_lines_score (this);
  this->total_lines += this->current_lines;
  for (i = 0; i < GRID_H; i++) {
    if (this->lines[i]) {
      game_shift_board (this, i, 1);
    }
  }
  this->anim_lines = 0;
  this->state = S_RUN;
}


Boolean game_current_fall (Game *this)
{
  if (game_current_can_move (this, 0, 1)) {
    this->current.j++;
    if (this->current.j > 3) this->score += this->level;
    return true;
  } else {
    game_current_put (this);
    return false;
  }
}


void game_current_auto_fall (Game *this)
{
  this->step = (this->step + 1)% (LEVEL_MAX + 1 - this->level);

  if (controls_isDownPressed (&this->controls)) {
    game_current_fall (this);
  }
  else if (this->step == 0)  {
     game_current_fall (this);
  }
}


void game_current_full_fall (Game *this)
{
  while (game_current_fall (this));
}


Boolean game_is_line_clipping (Game *this, int j)
{
  return
    this->state == S_LINES &&
    this->lines[j] &&
    this->anim_lines%2;
}


void game_current_move (Game *this, int x)
{
  if (game_current_can_move (this, x, 0)) {
    this->current.i += x;
  }
}


void game_current_rotate (Game *this)
{
  int i;
  if (this->current.type == T_O) return;
  tetromino_rotate (&this->current);
  if (game_current_can_move (this, 0, 0)) return;

  for (i = 1; i < BSIZE; i++) {
    if (game_current_can_move (this, i, 0)) {
      this->current.i += i;
      return;
    }
    else if (game_current_can_move (this, -i, 0)) {
      this->current.i -= i;
      return;
    }
    else if (game_current_can_move (this, 0, -i)) {
      this->current.j -= i;
      return;
    }
  }
  /* can't rotate without conflicts -- abort */
  for (i = 0; i < 3; i++) {
    tetromino_rotate (&this->current);
  }
}


void game_put_grey_line (Game *this)
{
  int i, j;
  j = GRID_H - this->anim_game_over - 1;
  for (i = 0; i < GRID_W; i++) {
    this->board[i][j] = ez_grey;
  }
}


void game_anim_game_over (Game *this)
{
  if (this->anim_game_over <= GRID_H) {
    game_put_grey_line (this);
    this->anim_game_over++;
  }
}


void game_anim_lines (Game *this)
{
  this->anim_lines++;
  if (this->anim_lines >= 10) {
    game_do_lines (this);
  }
}


void game_toogle_pause (Game *this)
{
  State s;
  if (this->state == S_LOOSE) return;
  s = this->prev_state;
  this->prev_state = this->state;
  this->state = this->state != S_PAUSE ? S_PAUSE:s;
}


void game_init (Game *this)
{
  game_init_history (this);
  game_choose_next (this);
  game_init_board (this);
  game_take_next (this);
  this->step = 0;
  this->level = 1;
  this->score = 0;
  this->total_lines = 0;
  this->state = S_RUN;
  this->current_lines = 0;
  this->anim_game_over = 0;
  this->anim_lines = 0;
  controls_init (&this->controls);
}


/*---------------------------------- V I E W  -------------------------------*/

void cell_draw (Ez_window win, int x, int y, Ez_uint32 color)
{
  ez_set_color (color);
  ez_fill_rectangle (win, x, y, x + CELLSIZE, y + CELLSIZE);
  ez_set_thick (2);
  ez_set_color (ez_grey);
  ez_draw_rectangle (win, x+1, y+1, x + CELLSIZE -1, y + CELLSIZE-1);
  ez_set_thick (3);
  ez_set_color (ez_black);
  ez_draw_rectangle (win, x, y, x + CELLSIZE, y + CELLSIZE);
}


void shadow_draw (Ez_window win, int i, int j)
{
  int x = i*CELLSIZE;
  int y = j*CELLSIZE;
  ez_set_color (ez_grey);
  ez_fill_rectangle (win, x, y, x+CELLSIZE, y+CELLSIZE);
}


void game_cell_draw (Game *this, Ez_window win, int i, int j)
{
  int x = i*CELLSIZE;
  int y = j*CELLSIZE;
  cell_draw (win, x, y, this->board[i][j]);
}


void tetromino_draw (Tetromino *this, Ez_window win, int x, int y)
{
  int i, j;
  for (i = 0;  i < BSIZE; i++) {
    for (j = 0; j < BSIZE; j++) {
      if (this->blocks[i][j] != 0) {
        cell_draw (win, x + i*CELLSIZE, y + j*CELLSIZE, this->blocks[i][j]);
      }
    }
  }
}


void game_current_draw (Game *this, Ez_window win)
{
  int i, j;
  int y;
  Tetromino *c = &this->current;

  if (this->state == S_LINES || this->prev_state == S_LINES) return;
  y = 1;
  while (game_current_can_move (this, 0, y)) y++;

  for (i = 0; i < BSIZE; i++) {
    for (j = 0; j < BSIZE; j++) {
      if (c->blocks[i][j] != 0) {
        shadow_draw (win, c->i + i, c->j + j + y - 1);
      }
    }
  }

  tetromino_draw (c, win, c->i*CELLSIZE, c->j*CELLSIZE);
}


void game_board_draw (Game *this, Ez_window win)
{
  int i, j;
  for (i = 0; i < GRID_W; i++) {
    for (j = 0; j < GRID_H; j++) {
      if (game_is_line_clipping (this, j)) continue;
      if (this->board[i][j] != 0) {
        game_cell_draw (this, win, i, j);
      }
    }
  }
}


void game_controls_draw (Game *this, Ez_window win, int x, int y)
{
  (void) this;
  ez_set_color (ez_white);
  ez_set_nfont (2);
  ez_draw_text (win, EZ_ML, x, y, "CONTROLS");
  ez_set_nfont (1);
  ez_draw_text (win, EZ_ML, x, y + CELLSIZE, "<- ->: move");
  ez_draw_text (win, EZ_ML, x, y + CELLSIZE*2, "Down:  Fast fall");
  ez_draw_text (win, EZ_ML, x, y + CELLSIZE*3, "Up:    Full fall");
  ez_draw_text (win, EZ_ML, x, y + CELLSIZE*4, "Space: Rotate");
  ez_draw_text (win, EZ_ML, x, y + CELLSIZE*5, "P:     Pause");
}


void game_ui_draw (Game *this, Ez_window win)
{
  int i;
  int xtip = GRID_W*CELLSIZE;
  int ytip = 0;

  ez_set_color (ez_black);
  ez_fill_rectangle (win, xtip, ytip, WIN_WIDTH, WIN_HEIGHT);

  ez_set_nfont (3);
  ez_set_color (ez_white);
  ez_draw_text (win, EZ_ML, xtip + CELLSIZE, ytip + CELLSIZE, "SCORE");

  ez_draw_text (win, EZ_ML, xtip + CELLSIZE, ytip + CELLSIZE*2, "%010d", this->score);

  ez_draw_text (win, EZ_ML, xtip + CELLSIZE, ytip + CELLSIZE*4,"LEVEL");
  ez_draw_text (win, EZ_ML, xtip + CELLSIZE, ytip + CELLSIZE*5,"%02d", this->level);

  ez_draw_text (win, EZ_ML, xtip + CELLSIZE, ytip + CELLSIZE*7,"LINES");
  ez_draw_text (win, EZ_ML, xtip + CELLSIZE, ytip + CELLSIZE*8,"%04d", this->total_lines);

  ez_draw_text (win, EZ_ML, xtip + CELLSIZE, ytip + CELLSIZE*10, "NEXT");
  tetromino_draw (&this->next, win, xtip + 1, ytip + CELLSIZE*10);

  ez_set_thick (1);
  ez_set_color (ez_grey);
  for (i = 0; i <= GRID_W; i++) {
    ez_draw_line (win, i*CELLSIZE, 0, i*CELLSIZE, (GRID_H-1)*CELLSIZE);
  }
  game_controls_draw (this, win, xtip + CELLSIZE, ytip + CELLSIZE * 15);
}


void game_display_gameover (Game *this, Ez_window win)
{
  if (! (this->state == S_LOOSE && this->anim_game_over > 20)) return;
  ez_set_color (ez_white);
  ez_fill_rectangle (win, 0, (GRID_H-4)/2*CELLSIZE, GRID_W*CELLSIZE,
                    (GRID_H+2)/2*CELLSIZE);
  ez_set_color (ez_red);
  ez_set_nfont (3);
  ez_draw_text (win, EZ_MC, GRID_W/2 * CELLSIZE,
               ((GRID_H - 1)/2)*CELLSIZE, "GAME OVER");
  ez_set_nfont (2);
  ez_draw_text (win, EZ_MC, GRID_W/2 * CELLSIZE,
               ((GRID_H +1)/2)*CELLSIZE, "PRESS SPACE TO RESTART");
}


void game_display_pause (Game *this, Ez_window win)
{
  if (this->state != S_PAUSE) return;
  ez_set_color (ez_white);
  ez_fill_rectangle (win, 0, (GRID_H-4)/2*CELLSIZE, GRID_W*CELLSIZE,
                    (GRID_H+2)/2*CELLSIZE);
  ez_set_color (ez_red);
  ez_set_nfont (3);
  ez_draw_text (win, EZ_MC, GRID_W/2 * CELLSIZE,
               ((GRID_H - 1)/2)*CELLSIZE, "PAUSE");
  ez_set_nfont (2);
  ez_draw_text (win, EZ_MC, GRID_W/2 * CELLSIZE,
               ((GRID_H +1)/2)*CELLSIZE, "PRESS P TO RESTART");
}


void win_on_expose (Ez_event *ev)
{
  Data *data = ez_get_data (ev->win);
  Game *game = &data->game;

  game_ui_draw (game, data->win);
  game_current_draw (game, data->win);
  game_board_draw (game, data->win);

  game_display_gameover (game, data->win);
  game_display_pause (game, data->win);
}


/*----------------------------- C O N T R O L E R ---------------------------*/

void win_on_timerNotify (Ez_event *ev)
{
  Data *data = ez_get_data (ev->win);
  Game *game = &data->game;

  if (game->state == S_RUN) {
    game_current_auto_fall (game);
  }
  else if (game->state == S_LINES) {
    game_anim_lines (game);
  }
  else if (game->state == S_LOOSE) {
    game_anim_game_over (game);
  }

  ez_start_timer (data->win, TIMER);
  ez_send_expose (data->win);
}


void win_on_keyPress_run (Ez_event *ev)
{
  Data *data = ez_get_data (ev->win);
  Game *game = &data->game;

  switch (ev->key_sym) {
    case XK_Left: game_current_move (game, -1); break;
    case XK_Right: game_current_move (game, 1); break;
    case XK_space:  game_current_rotate (game); break;
    case XK_Down: controls_pressDown (&game->controls); break;
    case XK_Up: game_current_full_fall (game); break;
  }
}


void win_on_keyPress_loose (Ez_event *ev)
{
  Data *data = ez_get_data (ev->win);
  Game *game = &data->game;

  if (game->anim_game_over < GRID_H) return;

  switch (ev->key_sym) {
    case XK_space: game_init (game); break;
  }
}


void win_on_keyPress (Ez_event *ev)
{
  Data *data = ez_get_data (ev->win);
  Game *game = &data->game;

  switch (ev->key_sym) {
    case XK_p: game_toogle_pause (game); break;
  }

  switch (game->state) {
    case S_RUN: win_on_keyPress_run (ev); break;
    case S_LINES: break;
    case S_LOOSE: win_on_keyPress_loose (ev); break;
    default: break;
  }
}


void win_on_keyRelease (Ez_event *ev)
{
  Data *data = ez_get_data (ev->win);
  Game *game = &data->game;

  switch (ev->key_sym) {
    case XK_Down: controls_releaseDown (&game->controls); break;
  }
}


void win_on_event (Ez_event *ev)
{
  switch (ev->type) {
    case Expose: win_on_expose (ev); break;
    case KeyPress: win_on_keyPress (ev); break;
    case KeyRelease: win_on_keyRelease (ev); break;
    case TimerNotify: win_on_timerNotify (ev); break;
  }
}


/*----------------------------------- M A I N -------------------------------*/

void data_init (Data *data)
{
  data->win = ez_window_create (WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, win_on_event);
  game_init (&data->game);
  ez_window_dbuf (data->win, true);
  ez_set_data (data->win, data);
  ez_start_timer (data->win, TIMER);
}


int main ()
{
  Data data;
  
  if (ez_init() < 0) exit(1);

  data_init (&data);

  ez_main_loop ();
  exit (0);
}

