/* jeu-doodle.c: a platform game inspired by Doodle Jump
 *
 * Julien Prudhomme <Julien.Prudhomme@etu.univ-amu.fr> - 10/07/2013
 *
 * Graphic arts: Samantha Thiel <samantha83var@gmail.com>
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/


#include "ez-draw.h"
#include "ez-image.h"

/* --------------------------- D E F I N E S ------------------------------- */

#define WIN_TITLE "Jeu doodle"
#define WIN_WIDTH 640
#define WIN_HEIGHT 800

#define SCORE_BUFFER 50
#define NAME_BUFFER 200

#define TIMER1 100
#define TIMER2 10

#define FALSE 0
#define TRUE 1

#define PI 3.14159265359

#define DISTANCE_HOR(x1,x2) ((sqrt(((x1) - (x2))*((x1) - (x2)))))
#define DISTANCE_VER(y1,y2) ((sqrt(((y1) - (y2))*((y1) - (y2)))))
#define U __attribute__((unused))


/* ------------------------- S T R U C T U R E S --------------------------- */

/* Queue & list */

typedef unsigned char boolean;

typedef struct List
{
  void *data;
  struct List *prev;
  struct List *next;
} List;

typedef struct Queue
{
  List *head;
  List *tail;
  int lenght;
} Queue;

typedef struct Point
{
  double x, y;
} Point;

typedef struct Point Vector;

enum {DOODLER_LEFT, DOODLER_RIGHT, DOODLER_JUMP_LEFT, DOODLER_JUMP_RIGHT, DOODLER_SHOOT, DOODLER_JUMP_SHOOT, DOODLER_N };

#define DOODLER_HIT_WIDTH 50
#define DOODLER_HIT_HEIGHT 80

typedef struct HitBoxAB
{
  Point a;
  Point b;
} HitBoxAB;

typedef enum { B_NO_BONUS, B_CHAPEAU, B_SHOES, B_JETPACK, B_RESSORT, B_SHIELD, B_TRAMP, B_NB} Bonus_type;

enum { BS_CASQUETTE, BS_SHOES, BS_JETPACK,
       BS_RESSORT1, BS_RESSORT2,
       BS_SHIELD,
       BS_TRAMP1, BS_TRAMP2, BS_TRAMP3,
       BS_N };

enum {BES_CASQUETTE1, BES_CASQUETTE2, BES_CASQUETTE3, BES_CASQUETTE4,
      BES_JETPACK1, BES_JETPACK2, BES_JETPACK3, BES_JETPACK4, BES_JETPACK5, BES_JETPACK6,
      BES_JETPACK7, BES_JETPACK8, BES_JETPACK9, BES_JETPACK10,
      BES_SHIELD1, BES_SHIELD2, BES_SHIELD3,
      BES_SHOES1, BES_SHOES2, BES_SHOES3, BES_SHOES4, BES_SHOES5,
      BES_N};

enum { STARS1, STARS2, STARS3, STARSN };

typedef struct Bonus
{
  Bonus_type type;
  Point pos;
  boolean taken;
  int step;
  double width, height;
} Bonus;

typedef struct Bonus_Effect
{
  Bonus_type type;
  Point pos;
  Point speed;
  double angle;
  int time_left;
  int bound_left;
  boolean can_fall;
  boolean can_grab;
  boolean can_shoot;
  boolean can_die;
  int step;
  boolean left;
} Bonus_Effect;

typedef struct Tir
{
  Point pos;
  Vector speed;
} Tir;

typedef struct Tirs
{
  List *tirs;
  Ez_pixmap *image;
  int time_since_last_shoot;
  double angle;
} Tirs;

typedef struct Doodler
{
  Point pos;
  Vector speed;
  double hit_width, hit_height;
  HitBoxAB hitbox;         /* relative to the image center for the platforms */
  HitBoxAB hit_object;     /* for the objects (monsters, bonus) */
  Ez_image *image[DOODLER_N];
  Ez_image *bonus_image[BES_N];
  Ez_image *image_trompette;
  Ez_image *stars[STARSN];
  int anim_stars;
  boolean left;
  List *bonus;
  Tirs shoots;
  double angle;
  double scale;
  boolean is_rotating;
  boolean is_dead;
  boolean sound_fall_played;
} Doodler;

typedef enum { M_SERPENT, M_CACTUS, M_PIEUVRE, M_PROUT, M_ROUGE, M_SOUCOUPE, M_TROU_NOIR, N_MONSTER } MonsterType;

enum { MS_SERPENT1, MS_SERPENT2,
       MS_CACTUS1, MS_CACTUS2,
       MS_PIEUVRE,
       MS_PROUT1, MS_PROUT2, MS_PROUT3, MS_PROUT4,
       MS_ROUGE,
       MS_SOUCOUPE1, MS_SOUCOUPE2,
       MS_TROU_NOIR,
       MS_N };

typedef struct Monster
{
  MonsterType type;
  Point pos;
  Vector speed;
  double width, height;
  int hp;
  int x1, x2;
  int step;
  boolean jumped;
  int anim;
} Monster;

typedef struct Monsters
{
  List *monsters;
  Ez_image *images[MS_N];
} Monsters;


typedef enum { P_STATIC, P_HORIZONTAL, P_VERTICAL, P_EXPLODE, P_VANISH, P_BREAK, P_MOVE } Platform_type;

enum { SP_STATIC, SP_HORIZONTAL, SP_VERTICAL,
       SP_EXPLODE1, SP_EXPLODE2, SP_EXPLODE3, SP_EXPLODE4, SP_EXPLODE5, SP_EXPLODE6, SP_EXPLODE7, SP_EXPLODE8,
       SP_VANISH,
       SP_BREAK1, SP_BREAK2, SP_BREAK3, SP_BREAK4,
       SP_MOVE,
       SP_N };

#define PLATFORM_WIDTH 114
#define PLATFORM_HEIGHT 30

typedef struct Platform
{
  Platform_type type;
  Point pos;
  Vector speed;
  double width, height;

  /* vertical */
  double ymin, ymax;

  /* explosion, breaking ... */
  int time_left;         /* 1/100 sec */

  /* vanish */
  boolean visible;
  double alpha;

  /* move */
  double xl, xr, x2;
  struct Platform *link;

  /*  */
  int step;

  /* bonus */
  Bonus *bonus;

} Platform;

typedef struct Platforms
{
  Ez_pixmap *images[SP_N];
  Ez_pixmap *bonus_images[BS_N];
  Queue *queue;
} Platforms;

typedef struct Game
{
  Doodler doodler;
  Ez_pixmap *background;
  Ez_image *background_image;     /* for blender */
  Ez_pixmap *gameover;
  Ez_pixmap *doodlejump;
  Platforms platforms;
  Monsters monsters;
  Monster *soucoupe;
} Game;

typedef enum { GAME_BEGIN, GAME_PLAYING, GAME_PAUSED, GAME_LOST } State;

typedef struct Keystates
{
  boolean left, right, up, down;
  boolean space;
  double mx, my;
  boolean button1;
} Keystates;

typedef struct Highscore
{
  char name[NAME_BUFFER];
  int score;
} Highscore;

enum { SOUND_EXPLODE1, SOUND_EXPLODE2,
       SOUND_JUMP_MONSTER, SOUND_JUMP,
       SOUND_BREAK,
       SOUND_CASQUETTE,
       SOUND_TIR1, SOUND_TIR2,
       SOUND_JETPACK,
       SOUND_SHOES,
       SOUND_RESSORT,
       SOUND_TRAMP,
       SOUND_CHUTE,
       SOUND_HIT,
       SOUND_ASP,
       SOUND_N };

typedef struct Info
{
  Keystates keystates;
  State state;
  int timer1, timer2;
  int score;
  unsigned scroll;
  Game game;
  List *highscores;
  int area_width, area_height;
  Ez_window win;
  boolean show_fps;
} Info;

typedef void (*destroyFunc)(void* data);

/* Bounding */
int WINDOW_WIDTH;
int WINDOW_HEIGHT;
int AREA_WIDTH;
int AREA_HEIGHT;


/* ------------------------ P R O T O T Y P E S ---------------------------- */

List *list_new(void *data, List *prev, List *next);
List *list_append(List *list, void *data);
List *list_prepend(List *list, void *data);
List *list_remove_link(List *list, List *link);
List *list_remove(List *list, void *data);
List *list_free_full(List *list, destroyFunc func);
List *list_next(List *list);
List *list_prev(List *list);
Queue *queue_new(void);
void queue_push_head(Queue *queue, void *data);
void queue_push_tail(Queue *queue, void *data);
void *queue_pop_head(Queue *queue);
void *queue_pop_tail(Queue *queue);
void *queue_peek_tail(Queue *queue);
void *queue_peek_head(Queue *queue);
int get_random_with_range(int a, int b);
Ez_pixmap *ez_pixmap_create_from_file(const char *filename);
void info_init_default(Info *info);
void info_timer_init(Info *info);
void info_timer_stop(Info *info);
boolean timer1_on_timeout(void* data);
boolean timer2_on_timeout(void* data);
void info_timer_start(Info *info, void* data);
char* info_get_state_button_label(Info *info);
void info_set_state(Info *info, State state);
void info_restart(Info *info);
void info_set_state_begin(Info *info);
void info_set_state_playing(Info *info, void* data);
void info_set_state_paused(Info *info);
void info_set_state_lost(Info *info);
void info_keystates_init(Info *info);
void info_keystates_set_left(Info *info, boolean pressed);
void info_keystates_set_right(Info *info, boolean pressed);
void info_keystates_set_up(Info *info, boolean pressed);
void info_keystates_set_down(Info *info, boolean pressed);
void info_keystates_set_space(Info *info, boolean pressed);
char* info_get_score_string(Info *info);
void info_set_score(Info *info, unsigned score);
void info_score_update(Info *info, Doodler *d);
void info_game_init(Info *info);
void info_game_init_background(Info *info);
void info_game_doodlejump_draw(Info *info, Ez_window win);
void info_game_gameover_draw(Info *info, Ez_window win);
void info_game_background_draw(Info *info, Ez_window win);
void doodler_init_bonus(Doodler *d);
void info_game_doodler_init(Info *info);
void info_game_doodler_load_images(Info *info);
void doodler_start_jump_platform(Doodler *d, Platform U *p);
void info_game_doodler_destroy(Info *info);
unsigned info_game_doodler_get_image(Info *info);
void doodler_draw_trompette(Info *info, Ez_window win);
void doodler_stars_draw(Info *info, Ez_window win);
void info_game_doodler_draw(Info *info, Ez_window win);
void info_game_doodler_check_input(Info *info);
void info_game_doodler_move(Info *info);
boolean doodler_can_fall(Info *info);
void info_game_doodler_fall(Info *info);
void info_game_doodler_rotate(Info *info);
void doodler_stop_rotating(Info *info);
void info_game_doodler_scale(Info *info);
void info_game_doodler_next_step(Info *info);
Monster *monster_new(double x, double y, double dx, double dy, int hp, MonsterType type);
Monster *monster_trou_noir_new(double x, double y);
Monster *monster_serpent_new(double x, double y);
Monster *monster_cactus_new(double x, double y);
Monster *monster_pieuvre_new(double x, double y);
Monster *monster_prout_new(double x, double y);
Monster *monster_rouge_new(double x, double y);
Monster *monster_soucoupe_new(double x, double y);
void info_game_monsters_init(Info *info);
void free_monster(void* data);
void info_game_monsters_restart(Info *info);
void info_game_monsters_add(Monster *m, Info *info);
void info_game_monsters_delete(Monster *m, Info *info);
boolean monster_is_out_bottom_screen(Monster *m, Info *info);
boolean shoot_is_on_monster(Tir *tir, Monster *m);
int shoots_is_on_monster(Info *info, Monster *m);
int doodler_in_monster(Doodler *d, Monster *m);
boolean doodler_can_die(Doodler *d);
void monster_trou_noir_next_step(Info *info, Monster *m);
void monster_serpent_next_step(Info *info, Monster *m);
void monster_cactus_next_step(Info *info, Monster *m);
void monster_pieuvre_next_step(Info *info, Monster *m);
void monster_prout_next_step(Info *info, Monster *m);
void monster_rouge_next_step(Info *info, Monster *m);
void monster_soucoupe_next_step(Info *info, Monster *m);
void info_game_monsters_next_step(Info *info);
void monster_serpent_draw(Info *info, Ez_window win, Monster *m);
void monster_trou_noir_draw(Info *info, Ez_window win, Monster *m);
void monster_cactus_draw(Info *info, Ez_window win, Monster *m);
void monster_pieuvre_draw(Info *info, Ez_window win, Monster *m);
void monster_prout_draw(Info *info, Ez_window win, Monster *m);
void monster_rouge_draw(Info *info, Ez_window win, Monster *m);
void monster_soucoupe_draw(Info *info, Ez_window win, Monster *m);
void info_game_monsters_draw(Info *info, Ez_window win);
void info_game_doodler_tirs_init(Info *info);
void info_game_doodler_tirs_load_images(Info *info);
Vector calculer_vecteur_doodler_mouse(Point *doodler, int scroll, Point *mouse);
double calculer_norme(Vector *v);
double produit_scalaire(Vector *v1, Vector *v2);
void transform_vecteur_unitaire(Vector *v);
void scale_vecteur(Vector *v, double k);
void info_game_doodler_tirs_add_shoot(Info *info);
void tirs_delete_tir(Tirs *tirs, Tir *tir);
boolean tir_in_screen(Info *info, Tir *tir);
void info_game_doodler_tir_next_step(Info *info, Tir *tir);
boolean doodler_can_shoot(Info *info);
void info_game_doodler_tirs_next_step(Info *info);
void info_game_doodler_tir_draw(Info *info, Ez_window win, Tir *t);
void info_game_doodler_tirs_draw(Info *info, Ez_window win);
Bonus_Effect *bonus_effect_new(void);
Bonus_Effect *bonus_effect_chapeau_new(void);
Bonus_Effect *bonus_effect_shield_new(void);
Bonus_Effect *bonus_effect_jetpack_new(void);
Bonus_Effect *bonus_effect_shoes_new(void);
void doodler_set_bonus_chapeau(Doodler *d);
void doodler_set_bonus_shield(Doodler *d);
void doodler_set_bonus_jetpack(Doodler *d);
void doodler_set_bonus_shoes(Doodler *d);
void bonus_decrease_bounds(Doodler *d);
Bonus *bonus_new(double x, double y, Bonus_type type);
Bonus *bonus_chapeau_new(double x, double y);
Bonus *bonus_shoes_new(double x, double y);
Bonus *bonus_jetpack_new(double x, double y);
Bonus *bonus_ressort_new(double x, double y);
Bonus *bonus_shield_new(double x, double y);
Bonus *bonus_tramp_new(double x, double y);
void doodler_bonus_chapeau_next_step(Doodler *d, Bonus_Effect *b);
void doodler_bonus_shield_next_step(Doodler *d, Bonus_Effect *b);
void doodler_bonus_shoes_next_step(Doodler *d, Bonus_Effect *b);
void doodler_bonus_jetpack_next_step(Doodler *d, Bonus_Effect *b);
void info_game_doodler_bonus_next_step(Info *info);
void info_game_platforms_bonus_load_images(Info *info);
void info_game_doodler_bonus_load_images(Info *info);
void bonus_chapeau_draw(Info *info, Ez_window win, Bonus *b);
void bonus_shoes_draw(Info *info, Ez_window win, Bonus *b);
void bonus_jetpack_draw(Info *info, Ez_window win, Bonus *b);
void bonus_ressort_draw(Info *info, Ez_window win, Bonus *b);
void bonus_shield_draw(Info *info, Ez_window win, Bonus *b);
void bonus_tramp_draw(Info *info, Ez_window win, Bonus *b);
void bonus_draw(Info *info, Ez_window win, Bonus *b);
void doodler_bonus_chapeau_draw(Info *info, Ez_window win, Bonus_Effect *b);
void doodler_bonus_shield_draw(Info *info, Ez_window win, Bonus_Effect *b);
void doodler_bonus_jetpack_draw(Info *info, Ez_window win, Bonus_Effect *b);
void doodler_bonus_shoes_draw(Info *info, Ez_window win, Bonus_Effect *b);
boolean doodler_on_bonus(Doodler *doodler, Bonus *b);
boolean can_grab_bonus(Info *info);
void bonus_chapeau_next_step(Bonus *b, Info *info);
void bonus_jetpack_next_step(Bonus *b, Info *info);
boolean can_jump_bonus(Bonus *b, Info *info);
void bonus_shoes_next_step(Bonus *b, Info *info);
void bonus_shield_next_step(Bonus *b, Info *info);
void bonus_ressort_next_step(Bonus *b, Info *info);
void bonus_tramp_next_step(Bonus *b, Info *info);
void info_game_platforms_bonus_next_step(Platform *p, Info *info);
void info_game_platforms_init(Info *info);
void info_game_platforms_restart(Info *info);
Platform *platform_new(double x, double y, Platform_type type);
Platform *platform_static_new(double x, double y);
Platform *platform_horizontal_new(double x, double y, double xspeed);
Platform *platform_vertical_new(double x, double y, double yspeed, double amplitude);
Platform *platform_break_new(double x, double y, double xspeed);
Platform *platform_explode_new(double x, double y, int time);
Platform *platform_vanish_new(double x, double y);
Platform *platform_move_new(double x, double y);
void free_platform(void* data);
void platform_destroy_over_bot_screen(Info *info);
void info_game_platforms_load_images(Info *info);
void platform_static_draw(Info *info, Ez_window win, Platform *p);
void platform_horizontal_draw(Info *info, Ez_window win, Platform *p);
void platform_vertical_draw(Info *info, Ez_window win, Platform *p);
void platform_explode_draw(Info *info, Ez_window win, Platform *p);
void platform_vanish_draw(Info *info, Ez_window win, Platform *p);
void platform_break_draw(Info *info, Ez_window win, Platform *p);
void platform_move_draw(Info *info, Ez_window win, Platform *p);
void info_game_platforms_draw(Info *info, Ez_window win);
boolean doodler_has_bonus_shoes(Doodler *d);
boolean platform_contain_doodler(Doodler *d, Platform *p);
void info_game_platforms_next_step(Info *info);
boolean platform_regular_bound(Platform *p, Info *info);
void platform_static_next_step(Platform *p, Info *info);
void platform_horizontal_next_step(Platform *p, Info *info);
void platform_vertical_next_step(Platform *p, Info *info);
void platform_explode_next_step(Platform *p, Info *info);
void platform_break_next_step(Platform *p, Info *info);
void platform_vanish_next_step(Platform *p, Info *info);
void platform_move_next_step(Platform *p, Info *info);
Platform *platform_find_no_breakable(List *list);
Point get_next_platform_pos(Info *info);
Bonus *get_platform_bonus(Info U *info, Platform *p);
void monsters_genere(Info *info);
void platforms_genere_statics(Info *info);
void platforms_genere_horizontal(Info *info);
void platforms_genere_vertical(Info *info);
void platforms_genere_explode(Info *info);
void platforms_genere_vanish(Info *info);
void platforms_genere_move(Info *info);
void platforms_genere_others(Info *info);
void info_game_platforms_genere(Info *info);
void win_on_expose(Ez_event *ev);
void quit(Info *info);


/* -------------------------------- L I S T -------------------------------- */

List *list_new(void *data, List *prev, List *next)
{
  List *l = malloc(sizeof *l);
  l->data = data;
  l->prev = prev;
  l->next = next;
  return l;
}


List *list_append(List *list, void *data)
{
  List *i;

  if(list == NULL) return list_new(data, NULL, NULL);

  i = list;

  while(i->next != NULL)
    i = i->next;

  i->next = list_new(data, i, NULL);

  return list;
}


List *list_prepend(List *list, void *data)
{
  if(list == NULL) return list_new(data, NULL, NULL);

  list->prev = list_new(data, NULL, list);
  return list->prev;
}


List *list_remove_link(List *list, List *link)
{
  if(link == NULL) return list;

  if(link->prev != NULL)
    link->prev->next = link->next;

  if(link->next != NULL)
    link->next->prev = link->prev;

  if(link == list)
    list = link->next;

  free(link);

  return list;
}


List *list_remove(List *list, void *data)
{
  List *i;
  if(list == NULL) return NULL;

  for(i = list; i != NULL; i = i->next)
  {
    if(i->data == data) return list_remove_link(list, i);
  }

  return list;
}


List *list_free_full(List *list, destroyFunc func)
{
  List *i, *n;

  for(i = list; i != NULL; )
  {
    func(i->data);
    n = i->next;
    free(i);
    i = n;
  }

  return NULL;
}


List *list_next(List *list)
{
  return list->next;
}


List *list_prev(List *list)
{
  return list->prev;
}


/* ------------------------------ Q U E U E -------------------------------- */

Queue *queue_new(void)
{
  Queue *q = malloc(sizeof *q);
  q->head = NULL;
  q->tail = NULL;
  q->lenght = 0;
  return q;
}


void queue_push_head(Queue *queue, void *data)
{
  queue->head = list_prepend(queue->head, data);
  if(queue->tail == NULL) queue->tail = queue->head;
  queue->lenght++;
}


void queue_push_tail(Queue *queue, void *data)
{
  queue->tail = list_append(queue->tail, data);
  if(queue->tail->next != NULL) queue->tail = queue->tail->next;
  if(queue->head == NULL) queue->head = queue->tail;
  queue->lenght++;
}


void *queue_pop_head(Queue *queue)
{
  void *data;

  if(queue->lenght == 0) return NULL;
  data = queue->head->data;
  queue->head = list_remove_link(queue->head, queue->head);
  queue->lenght--;

  if(queue->lenght == 0) queue->tail = NULL;

  return data;
}


void *queue_pop_tail(Queue *queue)
{
  void *data;
  if(queue->lenght == 0) return NULL;
  data = queue->tail->data;
  if(queue->tail->prev != NULL)
    queue->tail = list_remove_link(queue->tail->prev, queue->tail);
  else queue->tail = list_remove_link(queue->tail, queue->tail);

  queue->lenght--;
  if(queue->lenght == 0)
    queue->head = NULL;

  return data;
}


void *queue_peek_tail(Queue *queue)
{
  if(queue->lenght == 0) return NULL;
  return queue->tail->data;
}


void *queue_peek_head(Queue *queue)
{
  if(queue->lenght == 0) return NULL;

  return queue->head->data;
}


void queue_free_full(Queue *queue, destroyFunc func)
{
  list_free_full(queue->head, func);
  free(queue);
}


/* ------------------------------ U T I L S -------------------------------- */

int get_random_with_range(int a, int b)
{
  if(b <= a || b == 0) return 0;

  return a + rand()%(b - a);
}


void image_paint_extended(Ez_window win, Ez_image *img, double x, double y, double scale, double rotate, boolean vsym, boolean hsym)
{
  Ez_image *rez;
  Ez_image *del;

  x += WINDOW_WIDTH/2 - AREA_WIDTH/2;
  y += WINDOW_HEIGHT/2 - AREA_HEIGHT/2;

  if(scale == 1.0 && rotate == 0 && !hsym && !vsym)
  {
    ez_image_paint(win, img, (int)(x - img->width/2), (int)(y - img->height/2));
    return;
  }

  rez = ez_image_dup(img);

  if(scale != 1.0)
  {
    if(scale <= 0) scale = 0.01;
    del = rez;
    rez = ez_image_scale(rez, scale);
    ez_image_destroy(del);
  }
  if(rotate != 0.0)
  {
    del = rez;
    rez = ez_image_rotate(rez, rotate, 0);
    ez_image_destroy(del);
  }
  if(hsym)
  {
    del = rez;
    rez = ez_image_sym_hor(rez);
    ez_image_destroy(del);
  }
  if(vsym)
  {
    del = rez;
    rez = ez_image_sym_ver(rez);
    ez_image_destroy(del);
  }

  ez_image_paint(win, rez, (int)(x - rez->width/2), (int)(y - rez->height/2));
  ez_image_destroy(rez);
}


void pixmap_paint_extended(Ez_window win, Ez_pixmap *pix, double x, double y)
{
  x += WINDOW_WIDTH/2 - AREA_WIDTH/2;
  y += WINDOW_HEIGHT/2 - AREA_HEIGHT/2;
  ez_pixmap_paint(win, pix, (int)(x - pix->width/2), (int)(y - pix->height/2));
}


/* ------------------------------- P I X M A P ----------------------------- */

Ez_pixmap *ez_pixmap_create_from_file(const char *filename)
{
  Ez_image *image = ez_image_load(filename);
  Ez_pixmap *pixmap = ez_pixmap_create_from_image(image);

  ez_image_destroy(image);
  return pixmap;
}


/* --------------------------------- I N F O ------------------------------- */

void info_init_default(Info *info)
{
  info_game_init(info);
  info_timer_init(info);
  info_set_state_begin(info);
  info->show_fps = FALSE;
}


/* ------------------------------- T I M E R S ----------------------------- */

void info_timer_init(Info *info)
{
  info->timer1 = info->timer2 = 0;
}


void info_timer_stop(Info *info)
{
  ez_start_timer (info->win, -1);
}


boolean timer1_on_timeout(void U *data)
{
  /* Gui *gui = gui_check(data); */

  /* gui_area_redraw(gui); */
  return TRUE;
}


boolean timer2_on_timeout(void* data)
{
  Info *info = data;

  if(info->game.doodler.is_dead && info->scroll < (unsigned)info->area_height*2)
  {
    info->scroll += 20;
  }

  if(info->state == GAME_PLAYING)
    info_game_platforms_genere(info);
  info_game_monsters_next_step(info);
  info_game_doodler_next_step(info);
  info_game_platforms_next_step(info);

  ez_start_timer(info->win, TIMER2);
  ez_send_expose(info->win);

  return TRUE;
}


void info_timer_start(Info *info, void U *data)
{
  ez_start_timer(info->win, TIMER2);
}


/* ----------------------------- S T A T E --------------------------------- */

char* info_get_state_button_label(Info *info)
{
  static char *label[] = { "Start", "Pause", "Resume", "Restart" };

  return label[info->state];
}


void info_set_state(Info *info, State state)
{
  if(info->state != state)
  {
    info->state = state;
    /* listeners_notify(&info->ls, "state-changed"); */
  }
}


double get_scrolling(double y, Info *info)
{
  return y + (info->score - info->scroll) - (WIN_HEIGHT - info->area_height);
}


void info_restart(Info *info)
{
  info_keystates_init(info);
  info_set_score(info, 2000);
  info->scroll = 0;
  info_game_doodler_init(info);
  info_game_platforms_restart(info);
  info_game_monsters_restart(info);
  info_game_platforms_restart(info);
}


void info_set_state_begin(Info *info)
{
  info_set_state(info, GAME_BEGIN);
  info_restart(info);
  info_keystates_init(info);
}



void info_set_state_playing(Info *info, void *data)
{
  info_timer_start(info, data);
  info_set_state(info, GAME_PLAYING);

  if(info->game.soucoupe != NULL)
  {
    info_game_monsters_delete(info->game.soucoupe, info);
    info->game.soucoupe = NULL;
  }
}


void info_set_state_paused(Info *info)
{
  info_set_state(info, GAME_PAUSED);
  info_timer_stop(info);
}


void info_set_state_lost(Info *info)
{
  if(info->state == GAME_LOST) return;
  info_set_state(info, GAME_LOST);
}


/* ---------------------------- K E Y S T A T E S -------------------------- */

void info_keystates_init(Info *info)
{
  Keystates *keys = &info->keystates;

  keys->left = keys->right = keys->up = keys->down = FALSE;
  keys->space = FALSE;
  keys->button1 = FALSE;
}


void info_keystates_set_left(Info *info, boolean pressed)
{
  info->keystates.left = pressed;
}


void info_keystates_set_right(Info *info, boolean pressed)
{
  info->keystates.right = pressed;
}


void info_keystates_set_up(Info *info, boolean pressed)
{
  info->keystates.up = pressed;
}


void info_keystates_set_down(Info *info, boolean pressed)
{
  info->keystates.down = pressed;
}


void info_keystates_set_space(Info *info, boolean pressed)
{
  info->keystates.space = pressed;
}


/* -------------------------------- S C O R E ------------------------------ */

char* info_get_score_string(Info *info)
{
  static char score[SCORE_BUFFER];
  sprintf(score, "Score: %08u", info->score);

  return score;
}


void info_set_score(Info *info, unsigned score)
{
  info->score = score;
  /* listeners_notify(&info->ls, "score-changed"); */
}


void info_score_update(Info *info, Doodler *d)
{
  int dif = (1.8/3.0)*info->area_height - get_scrolling(d->pos.y, info);

  if(info->state != GAME_PLAYING) return;

  if(dif > 0)
    info_set_score(info, info->score + dif);

}


void info_score_draw(Info *info, Ez_window win)
{
  ez_set_nfont(3);
  ez_draw_text(win, EZ_TL, WINDOW_WIDTH/2 - AREA_WIDTH/2 + 2, 
      WINDOW_HEIGHT/2 - AREA_HEIGHT/2 + 1, "%08d", info->score - 2000);
}


/* --------------------------------- G A M E --------------------------------*/

void info_game_init(Info *info)
{
  info_game_doodler_init(info);
  info_game_monsters_init(info);
  info_game_platforms_init(info);
  info_game_init_background(info);

  info_game_doodler_load_images(info);
  info->game.gameover = ez_pixmap_create_from_file("images-doodle/game_over.png");
  info->game.doodlejump = ez_pixmap_create_from_file("images-doodle/doodle_titre.png");
}


void info_game_destroy(Info *info)
{
  ez_pixmap_destroy(info->game.gameover);
  ez_pixmap_destroy(info->game.doodlejump);
}


void info_game_init_background(Info *info)
{
  Game *game = &info->game;

  game->background = ez_pixmap_create_from_file("images-doodle/carreaux.png");
  game->background_image = ez_image_load("images-doodle/carreaux.png");
}


void info_game_background_destroy(Info *info)
{
  Game *game = &info->game;
  ez_pixmap_destroy(game->background);
  ez_image_destroy(game->background_image);
}


void info_game_doodlejump_draw(Info *info, Ez_window win)
{
  Game *game = &info->game;

  if(info->state == GAME_BEGIN)
  {
    pixmap_paint_extended(win, game->doodlejump, 
        info->area_width/2 - 30, info->area_height/2 - 90);
    ez_set_nfont(3);
    ez_draw_text(win, EZ_TC, WINDOW_WIDTH/2, WINDOW_HEIGHT/2 + 40, "Press space");
    ez_draw_text(win, EZ_TC, WINDOW_WIDTH/2, WINDOW_HEIGHT/2 + 80, "Arrows to move, mouse button to shoot !");
  }
}


void info_game_gameover_draw(Info *info, Ez_window win)
{
  Game *game = &info->game;

  if(info->game.doodler.is_dead || info->state == GAME_LOST)
  {
    ez_set_nfont(3);
    ez_draw_text(win, EZ_TC, WINDOW_WIDTH/2, 
      info->area_height*2.5 - 310 - info->scroll, "Credits");
    ez_set_nfont(2);
    ez_draw_text(win, EZ_TC, WINDOW_WIDTH/2, 
      info->area_height*2.5 - 250 - info->scroll,
      "Program: Julien Prudhomme\n\nGraphic arts: Samantha Thiel\n\n"
      "Powered by EZ-Draw");
    pixmap_paint_extended(win, game->gameover, info->area_width/2 , 
      info->area_height*2.5  - info->scroll);
    ez_set_nfont(3);
    ez_draw_text(win, EZ_TC, WINDOW_WIDTH/2, 
      info->area_height*2.5 + 200 - info->scroll,   "Press space");
  }
}


void info_game_background_draw(Info *info, Ez_window win)
{
  Game *game = &info->game;

  ez_pixmap_tile(win, game->background, WINDOW_WIDTH/2 - AREA_WIDTH/2, WINDOW_HEIGHT/2 - AREA_HEIGHT/2 - 40 + (info->score%40), info->area_width, info->area_height + 40);
}


/* ----------------------------- D O O D L E R ------------------------------*/

void doodler_init_bonus(Doodler *d)
{
  d->bonus = NULL;
}


void info_game_doodler_init(Info *info)
{
  Doodler *d = &info->game.doodler;

  d->pos.x = 200;
  d->pos.y = WIN_HEIGHT-100 - 2000;

  d->angle = 0;
  d->scale = 1.0;
  d->is_rotating = FALSE;

  d->speed.x = 0;
  d->speed.y = -8;

  d->left = FALSE;
  d->is_dead = FALSE;
  d->sound_fall_played = FALSE;

  d->anim_stars = 0;

  d->hit_width = DOODLER_HIT_WIDTH;
  d->hit_height = DOODLER_HIT_HEIGHT;

  d->hitbox.a.x = -30.0;
  d->hitbox.a.y = 50.0;
  d->hitbox.b.x = 24.0;
  d->hitbox.b.y = 59.0;

  d->hit_object.a.x = -40.0;
  d->hit_object.a.y = -35.0;
  d->hit_object.b.x = 30.0;
  d->hit_object.b.y = 60.0;

  doodler_init_bonus(d);
  info_game_doodler_tirs_init(info);

  /* d->width = cairo_image_surface_get_width(d->image[DOODLER_LEFT]); */
  /* d->height = cairo_image_surface_get_height(d->image[DOODLER_LEFT]); */
}


void info_game_doodler_load_images(Info *info)
{
  Doodler *d = &info->game.doodler;

  d->image[DOODLER_LEFT]       = ez_image_load("images-doodle/bob_left.png");
  d->image[DOODLER_RIGHT]      = ez_image_load("images-doodle/bob_right.png");
  d->image[DOODLER_JUMP_LEFT]  = ez_image_load("images-doodle/bob_left_jump.png");
  d->image[DOODLER_JUMP_RIGHT] = ez_image_load("images-doodle/bob_right_jump.png");
  d->image[DOODLER_SHOOT]      = ez_image_load("images-doodle/bob_face.png");
  d->image[DOODLER_JUMP_SHOOT] = ez_image_load("images-doodle/bob_face_jump.png");

  d->stars[STARS1] = ez_image_load("images-doodle/stars1.png");
  d->stars[STARS2] = ez_image_load("images-doodle/stars2.png");
  d->stars[STARS3] = ez_image_load("images-doodle/stars3.png");

  d->image_trompette = ez_image_load("images-doodle/bob_shoot.png");

  info_game_doodler_bonus_load_images(info);
  info_game_doodler_tirs_load_images(info);
}


void info_game_doodler_destroy(Info *info)
{
  Doodler *d = &info->game.doodler;
  int i;

  for(i = 0; i < DOODLER_N; i++)
    ez_image_destroy(d->image[i]);
}


void doodler_start_jump_platform(Doodler *d, Platform U *p)
{
  if(d->is_dead) return;
  d->speed.y = -9.3;
  bonus_decrease_bounds(d);
}


unsigned info_game_doodler_get_image(Info *info)
{
  Doodler *d = &info->game.doodler;

  if(d->is_rotating)
    return (d->left ? DOODLER_JUMP_LEFT : DOODLER_JUMP_RIGHT);

  if(d->speed.y < -5)
  {
    if(d->shoots.time_since_last_shoot < 40) return DOODLER_JUMP_SHOOT;

    if(d->speed.x > 0)
    {
      d->left = FALSE;
      return DOODLER_JUMP_RIGHT;
    }
    else if(d->speed.x < 0)
    {
      d->left = TRUE;
      return DOODLER_JUMP_LEFT;
    }
    else return d->left ? DOODLER_JUMP_LEFT : DOODLER_JUMP_RIGHT;
  }
  else
  {
    if(d->shoots.time_since_last_shoot < 40) return DOODLER_SHOOT;
    if(d->speed.x > 0)
    {
      d->left = FALSE;
      return DOODLER_RIGHT;
    }
    else if(d->speed.x < 0)
    {
      d->left = TRUE;
      return DOODLER_LEFT;
    }
    else return d->left ? DOODLER_LEFT : DOODLER_RIGHT;
  }
}


void doodler_draw_trompette(Info *info, Ez_window win)
{
  Doodler *d = &info->game.doodler;
  Ez_image *surface = d->image_trompette;
  double x = 0, y = 0;

  if(d->shoots.angle > -10 && d->shoots.angle < 10)
  {
      y = -5;
  }
  if(d->shoots.angle < -30)
    x = -3;

  if(d->shoots.angle > 30)
    x = -1;

  if(d->shoots.time_since_last_shoot < 40)
    image_paint_extended(win, surface, d->pos.x + x, get_scrolling(d->pos.y, info) + 10 + y, 1.0, d->shoots.angle, FALSE, FALSE);
}


void doodler_stars_draw(Info *info, Ez_window win)
{
  Ez_image *surface;
  Doodler *d = &info->game.doodler;

  if(d->anim_stars >= 0 && d->anim_stars < 10)
    surface = d->stars[STARS1];
  else if(d->anim_stars >= 10 && d->anim_stars < 20)
    surface = d->stars[STARS2];
  else
    surface = d->stars[STARS3];

  image_paint_extended(win, surface, d->pos.x, get_scrolling(d->pos.y, info) - 20, 1.0, 0.0, FALSE, FALSE);
}


void info_game_doodler_draw(Info *info, Ez_window win)
{
  Doodler *d = &info->game.doodler;
  List *list;
  Ez_image *surface = d->image[info_game_doodler_get_image(info)];
  double y = 0;

  if(d->speed.y > -5) y = 5;
  info_game_doodler_tirs_draw(info, win);

  image_paint_extended(win, surface, d->pos.x, get_scrolling(d->pos.y, info) + y, d->scale, d->angle, FALSE, FALSE);
  doodler_draw_trompette(info, win);

  if(d->is_dead)
    doodler_stars_draw(info, win);

  for(list = d->bonus; list != NULL; list = list->next)
  {
    Bonus_Effect *b = list->data;
    switch(b->type)
    {
    case B_CHAPEAU: doodler_bonus_chapeau_draw(info, win, b); break;
    case B_JETPACK: doodler_bonus_jetpack_draw(info, win, b); break;
    case B_SHOES: doodler_bonus_shoes_draw(info, win, b); break;
    case B_SHIELD: doodler_bonus_shield_draw(info, win, b); break;
    default: break;
    }
  }
}


void info_game_doodler_check_input(Info *info)
{
  Doodler *d = &info->game.doodler;
  Keystates *in = &info->keystates;

  if(in->left)
  {
    if(d->speed.x > 0) d->speed.x -= 0.5;
    else d->speed.x -= 0.25;
  }
  if(in->right)
  {
    if(d->speed.x < 0)
      d->speed.x += 0.5;
    else
      d->speed.x+= 0.25;
  }

  if(!in->left && !in->right)
  {
    if(d->speed.x > 0) d->speed.x -= 0.25;
    if(d->speed.x < 0) d->speed.x += 0.25;
  }

  if(d->speed.x > 9) d->speed.x = 9;
  if(d->speed.x < -9) d->speed.x = -9;
}


void info_game_doodler_move(Info *info)
{
  Doodler *d = &info->game.doodler;

  d->pos.x += d->speed.x;

  if(d->pos.y <= 9999)
  d->pos.y += d->speed.y;


  /* teleportation left or right */
  if(d->pos.x > info->area_width)
    d->pos.x = 0;

  if(d->pos.x < 0)
    d->pos.x = info->area_width;

  /* ------------------------------ */

  if(get_scrolling(d->pos.y, info) + d->hitbox.b.y - 20 > info->area_height)
  {
    if(!d->sound_fall_played)
    {
      d->sound_fall_played = TRUE;
    }
    d->is_dead = TRUE;
  }

  if(get_scrolling(d->pos.y, info) > info->area_height*3)
    info_set_state_lost(info);

  info_score_update(info, d);
}


boolean doodler_can_fall(Info *info)
{
  Doodler *d = &info->game.doodler;
  List *list;

  for(list = d->bonus; list != NULL; list = list->next)
  {
    Bonus_Effect *b = list->data;

    if(!b->can_fall)
      return FALSE;
  }

  return TRUE;
}


void info_game_doodler_fall(Info *info)
{
  Doodler *d = &info->game.doodler;

  if(doodler_can_fall(info))
    d->speed.y += 0.18;
}


void info_game_doodler_rotate(Info *info)
{
  Doodler *d = &info->game.doodler;
  if(d->is_rotating) d->angle+=6;
  if(d->angle >= 360)
  {
    d->is_rotating = FALSE;
    d->angle = 0;
  }
}


void doodler_stop_rotating(Info *info)
{
  Doodler *d = &info->game.doodler;

  d->angle = 0;
  d->is_rotating = FALSE;
}


void info_game_doodler_scale(Info *info)
{
  Doodler *d = &info->game.doodler;

  if(d->scale < 1.0 && d->scale > 0)
    d->scale -= 0.01;

  if(d->scale <= 0)
    info_set_state_lost(info);
}


void info_game_doodler_next_step(Info *info)
{
  Doodler *d = &info->game.doodler;

  if(info->state == GAME_PLAYING)
    info_game_doodler_check_input(info);
  info_game_doodler_move(info);
  info_game_doodler_fall(info);
  info_game_doodler_bonus_next_step(info);
  info_game_doodler_rotate(info);
  info_game_doodler_scale(info);
  info_game_doodler_tirs_next_step(info);

  if(d->is_dead)
  {
    d->anim_stars++;
    if(d->anim_stars > 30) d->anim_stars = 0;
  }

}


/* ---------------------------- M O N S T E R S ---------------------------- */

Monster *monster_new(double x, double y, double dx, double dy, int hp, MonsterType type)
{
  Monster *m = malloc(sizeof *m);
  m->pos.x = x;
  m->pos.y = y;
  m->speed.x = dx;
  m->speed.y = dy;
  m->type = type;
  m->hp = hp;
  m->step = 0;
  m->anim = 0;
  m->jumped = FALSE;
  return m;
}


Monster *monster_trou_noir_new(double x, double y)
{
  Monster *m = monster_new(x, y, 0, 0, 1, M_TROU_NOIR);
  m->width = m->height = 140;
  return m;
}


Monster *monster_serpent_new(double x, double y)
{
  Monster *m = monster_new(x, y, 2, 0, 1, M_SERPENT);
  m->width = 73;
  m->height = 130;
  return m;
}


Monster *monster_cactus_new(double x, double y)
{
  Monster *m = monster_new(x, y, 0, 0, 2, M_CACTUS);
  m->width = 125;
  m->height = 165;
  return m;
}


Monster *monster_pieuvre_new(double x, double y)
{
  Monster *m = monster_new(x, y, 2, 0, 1, M_PIEUVRE);
  m->width = 107;
  m->height = 96;
  m->x1 = x - 10;
  m->x2 = x + 10;
  return m;
}


Monster *monster_prout_new(double x, double y)
{
  Monster *m = monster_new(x, y, 0, 1, 3, M_PROUT);
  m->width = 158;
  m->height = 158;
  return m;
}


Monster *monster_rouge_new(double x, double y)
{
  Monster *m = monster_new(x, y, 1, 0, 1, M_ROUGE);
  m->width = 89;
  m->height = 100;
  m->x1 = x - 10;
  m->x2 = x + 10;
  return m;
}


Monster *monster_soucoupe_new(double x, double y)
{
  Monster *m = monster_new(x, y, 0.25, 0, 1, M_SOUCOUPE);
  m->width = 157;
  m->height = 228;
  m->x1 = x - 5;
  m->x2 = x + 5;
  return m;
}


void info_game_monsters_init(Info *info)
{
  Monsters *monsters = &info->game.monsters;
  monsters->monsters = NULL;

  monsters->images[MS_SERPENT1] = ez_image_load("images-doodle/monstre_serpent1.png");
  monsters->images[MS_SERPENT2] = ez_image_load("images-doodle/monstre_serpent2.png");

  monsters->images[MS_CACTUS1] = ez_image_load("images-doodle/monstre_cactus1.png");
  monsters->images[MS_CACTUS2] = ez_image_load("images-doodle/monstre_cactus2.png");

  monsters->images[MS_PIEUVRE] = ez_image_load("images-doodle/monstre_pieuvre.png");

  monsters->images[MS_PROUT1] = ez_image_load("images-doodle/monstre_prout1.png");
  monsters->images[MS_PROUT2] = ez_image_load("images-doodle/monstre_prout2.png");
  monsters->images[MS_PROUT3] = ez_image_load("images-doodle/monstre_prout3.png");
  monsters->images[MS_PROUT4] = ez_image_load("images-doodle/monstre_prout4.png");

  monsters->images[MS_ROUGE] = ez_image_load("images-doodle/monstre_rouge.png");

  monsters->images[MS_SOUCOUPE1] = ez_image_load("images-doodle/monstre_soucoupe1.png");
  monsters->images[MS_SOUCOUPE2] = ez_image_load("images-doodle/monstre_soucoupe2.png");

  monsters->images[MS_TROU_NOIR] = ez_image_load("images-doodle/trou_noir.png");
}


void info_game_monsters_destroy(Info *info)
{
  Monsters *monsters = &info->game.monsters;
  int i;

  for(i = 0; i < MS_N; i++)
    ez_image_destroy(monsters->images[i]);
}


void free_monster(void* data)
{
  Monster *m = data;
  free(m);
}


void info_game_monsters_restart(Info *info)
{
  Monsters *monsters = &info->game.monsters;
  list_free_full(monsters->monsters, free_monster);
  monsters->monsters = NULL;
}


void info_game_monsters_add(Monster *m, Info *info)
{
  Monsters *monsters = &info->game.monsters;

  monsters->monsters = list_prepend(monsters->monsters, m);
}


void info_game_monsters_delete(Monster *m, Info *info)
{
  Monsters *monsters = &info->game.monsters;
  monsters->monsters = list_remove(monsters->monsters, m);
  free(m);
}


boolean monster_is_out_bottom_screen(Monster *m, Info *info)
{
  return get_scrolling(m->pos.y - m->height/2, info) > info->area_height + 20;
}


boolean shoot_is_on_monster(Tir *tir, Monster *m)
{
  if(tir->pos.x < m->pos.x - m->width/2) return FALSE;
  if(tir->pos.x > m->pos.x + m->width/2) return FALSE;
  if(tir->pos.y < m->pos.y - m->height/2) return FALSE;
  if(tir->pos.y > m->pos.y + m->height/2) return FALSE;

  return TRUE;
}


int shoots_is_on_monster(Info *info, Monster *m)
{
  Doodler *d = &info->game.doodler;
  List *list;
  int res = 0;

  for(list = d->shoots.tirs; list != NULL; )
  {
    Tir *tir = list->data;
    list = list->next;
    if(shoot_is_on_monster(tir, m))
    {
      res++;
      tirs_delete_tir(&info->game.doodler.shoots, tir);
    }
  }

  return res;
}


int doodler_in_monster(Doodler *d, Monster *m) /* 0:no 1:jump 2:die */
{
  if(d->is_dead) return 0;
  if(d->pos.x + d->hit_object.a.x > m->pos.x + m->width/2) return 0;
  if(d->pos.x + d->hit_object.b.x < m->pos.x - m->width/2) return 0;
  if(d->pos.y + d->hit_object.b.y < m->pos.y - m->width/2) return 0;
  if(d->pos.y + d->hit_object.a.y > m->pos.y + m->width/2) return 0;

  if(d->speed.y > -1 && d->pos.y < m->pos.y - m->height/4) return 1;

  else return 2;
}


boolean doodler_can_die(Doodler *d)
{
  List *list;

  for(list = d->bonus; list != NULL; list = list->next)
  {
    Bonus_Effect *b = list->data;
    if(!b->can_die) return FALSE;
  }

  return TRUE;
}


void monster_trou_noir_next_step(Info *info, Monster *m)
{
  int state;
  Doodler *d = &info->game.doodler;

  state = doodler_in_monster(d, m);

  if(state == 2 && doodler_can_die(d) && m->step == 0 && m->jumped == FALSE)
  {
    d->speed.y = 0;
    d->scale = 0.99;
    d->speed.x = 0;
    m->step = 1;
  }

  if(m->step == 1)
  {
    d->speed.x = 0;
    d->speed.y = 0;
    if(d->pos.y > m->pos.y)
      d->pos.y--;
    if(d->pos.x < m->pos.x) d->pos.x++;
    else d->pos.x--;
  }
}


void monster_serpent_next_step(Info *info, Monster *m)
{
  int state;
  Doodler *d = &info->game.doodler;
  if(m->hp < 1) return;
  m->pos.x += m->speed.x;
  m->pos.y += m->speed.y;
  if(m->pos.x - m->width/2 < 0)
  {
    m->pos.x = m->width/2;
    m->speed.x = -m->speed.x;
  }
  if(m->pos.x + m->width/2 > info->area_width)
  {
    m->pos.x = info->area_width - m->width/2;
    m->speed.x = -m->speed.x;
  }

  if(shoots_is_on_monster(info, m))
    m->hp--;

  state = doodler_in_monster(d, m);

  if(state == 1)
  {
    m->speed.y = 5;
    d->speed.y = -15;
    m->jumped = TRUE;
  }
  if(state == 2 && doodler_can_die(d) && m->jumped == FALSE)
  {
    d->speed.y = 0;
    d->is_dead = TRUE;
  }
}


void monster_cactus_next_step(Info *info, Monster *m)
{
  int state;
  Doodler *d = &info->game.doodler;
  if(m->hp < 1) return;

  m->pos.y += m->speed.y;
  m->speed.y += 0.25;
  if(m->speed.y > 2 && m->step == 0) m->speed.y = -2;


  if(m->anim > 0) m->anim++;
  if(m->anim > 10) m->anim = 0;

  if(shoots_is_on_monster(info, m))
  {
    m->anim = 1;
    m->hp--;
  }

  state = doodler_in_monster(d, m);

  if(state == 1)
  {
    m->speed.y = 5;
    d->speed.y = -15;
    m->step = 1;
    m->jumped = TRUE;
  }
  if(state == 2 && doodler_can_die(d) && m->jumped == FALSE)
  {
    d->speed.y = 0;
    d->is_dead = TRUE;
  }
}


void monster_pieuvre_next_step(Info *info, Monster *m)
{
  int state;
  Doodler *d = &info->game.doodler;
  if(m->hp < 1) return;
  m->pos.x += m->speed.x;
  m->pos.y += m->speed.y;
  if(m->pos.x < m->x1)
    m->speed.x = -m->speed.x;
  if(m->pos.x > m->x2)
    m->speed.x = -m->speed.x;

  if(shoots_is_on_monster(info, m))
    m->hp--;


  state = doodler_in_monster(d, m);

  if(state == 1)
  {
    m->speed.y = 5;
    d->speed.y = -15;
    m->jumped = TRUE;
  }
  if(state == 2 && doodler_can_die(d) && m->jumped == FALSE)
  {
    d->speed.y = 0;
    d->is_dead = TRUE;
  }
}


void monster_prout_next_step(Info *info, Monster *m)
{
  int state;
  Doodler *d = &info->game.doodler;

  if(m->hp < 1) return;
  m->pos.y += m->speed.y;
  m->speed.y += 0.25;
  if(m->speed.y > 2 && m->step == 0) m->speed.y = -2;

  if(m->anim > 0) m->anim++;
  if(m->anim >= 30) m->anim = 0;

  if(shoots_is_on_monster(info, m))
  {
    m->anim = 1;
    m->hp--;
  }

  state = doodler_in_monster(d, m);

  if(state == 1)
  {
    m->speed.y = 5;
    d->speed.y = -15;
    m->step = 1;
    m->jumped = TRUE;
  }
  if(state == 2 && doodler_can_die(d) && m->jumped == FALSE)
  {
    d->speed.y = 0;
    d->is_dead = TRUE;
  }
}


void monster_rouge_next_step(Info *info, Monster *m)
{
  int state;
  Doodler *d = &info->game.doodler;
  if(m->hp < 1) return;
  m->pos.x += m->speed.x;
  m->pos.y += m->speed.y;
  if(m->pos.x < m->x1)
    m->speed.x = -m->speed.x;
  if(m->pos.x > m->x2)
    m->speed.x = -m->speed.x;

  if(shoots_is_on_monster(info, m))
    m->hp--;

  state = doodler_in_monster(d, m);

  if(state == 1)
  {
    m->speed.y = 5;
    m->jumped = TRUE;
    d->speed.y = -15;
  }
  if(state == 2 && doodler_can_die(d) && m->jumped == FALSE)
  {
    d->speed.y = 0;
    d->is_dead = TRUE;
  }
}


void monster_soucoupe_next_step(Info *info, Monster *m)
{
  int state;
  Doodler *d = &info->game.doodler;
  Monster tmp = (*m);
  if(m->hp < 1) return;
  m->pos.x += m->speed.x;
  m->pos.y += m->speed.y;
  if(m->pos.x < m->x1)
    m->speed.x = -m->speed.x;
  if(m->pos.x > m->x2)
    m->speed.x = -m->speed.x;

  tmp.height = m->height/4;
  tmp.pos.y = m->pos.y - m->height/4;

  if(shoots_is_on_monster(info, &tmp) && m->step != 1)
  {
    m->hp--;
    return;
  }

  state = doodler_in_monster(d, m);

  if(state == 1 && d->scale == 1.0)
  {
    m->speed.y = 5;
    d->speed.y = -15;
    m->jumped = TRUE;
  }
  if(state == 2 && doodler_can_die(d) && m->step == 0 && m->jumped == FALSE)
  {
    d->speed.y = 0;
    d->scale = 0.99;
    d->speed.x = 0;
    m->step = 1;
  }

  if(m->step == 1)
  {
    d->speed.x = 0;
    d->speed.y = 0;
    if(d->pos.y > m->pos.y)
      d->pos.y--;
    if(d->pos.x < m->pos.x) d->pos.x++;
    else d->pos.x--;
  }
}


void info_game_monsters_next_step(Info *info)
{
  Monsters *monsters = &info->game.monsters;
  List *list;

  for(list = monsters->monsters; list != NULL; )
  {
    Monster *m = list->data;
    list = list->next;

    switch(m->type)
    {
    case M_SERPENT: monster_serpent_next_step(info, m); break;
    case M_CACTUS: monster_cactus_next_step(info, m); break;
    case M_PIEUVRE: monster_pieuvre_next_step(info, m); break;
    case M_PROUT: monster_prout_next_step(info, m); break;
    case M_ROUGE: monster_rouge_next_step(info, m); break;
    case M_SOUCOUPE: monster_soucoupe_next_step(info, m); break;
    case M_TROU_NOIR: monster_trou_noir_next_step(info, m); break;
    default: break;
    }

    if(monster_is_out_bottom_screen(m, info))
      info_game_monsters_delete(m, info);
  }
}


void monster_serpent_draw(Info *info, Ez_window win, Monster *m)
{
  Monsters *monsters = &info->game.monsters;
  Ez_image *surface = monsters->images[m->speed.x < 0 ? MS_SERPENT2:MS_SERPENT1];

  if(m->hp > 0)
    image_paint_extended(win, surface, m->pos.x, get_scrolling(m->pos.y, info), 1.0, 0.0, FALSE, FALSE);
}


void monster_trou_noir_draw(Info *info, Ez_window win, Monster *m)
{
  Monsters *monsters = &info->game.monsters;
  Ez_image *surface = monsters->images[MS_TROU_NOIR];

  image_paint_extended(win, surface, m->pos.x, get_scrolling(m->pos.y, info), 1.0, 0.0, FALSE, FALSE);
}


void monster_cactus_draw(Info *info, Ez_window win, Monster *m)
{
  Monsters *monsters = &info->game.monsters;
  Ez_image *surface = monsters->images[m->anim == 0 ? MS_CACTUS1:MS_CACTUS2];

  if(m->hp > 0)
    image_paint_extended(win, surface, m->pos.x, get_scrolling(m->pos.y, info), 1.0, 0.0, FALSE, FALSE);
}


void monster_pieuvre_draw(Info *info, Ez_window win, Monster *m)
{
  Monsters *monsters = &info->game.monsters;
  Ez_image *surface = monsters->images[MS_PIEUVRE];

  if(m->hp > 0)
    image_paint_extended(win, surface, m->pos.x, get_scrolling(m->pos.y, info), 1.0, 0.0, FALSE, FALSE);
}


void monster_prout_draw(Info *info, Ez_window win, Monster *m)
{
  Monsters *monsters = &info->game.monsters;
  Ez_image *surface = monsters->images[MS_PROUT1];


  if(m->anim == 0) surface = monsters->images[MS_PROUT1];
  else if(m->anim > 0 && m->anim < 10) surface = monsters->images[MS_PROUT2];
  else if(m->anim >= 10 && m->anim < 20) surface = monsters->images[MS_PROUT3];
  else surface = monsters->images[MS_PROUT4];

  if(m->hp > 0)
    image_paint_extended(win, surface, m->pos.x, get_scrolling(m->pos.y, info), 1.0, 0.0, FALSE, FALSE);
}


void monster_rouge_draw(Info *info, Ez_window win, Monster *m)
{
  Monsters *monsters = &info->game.monsters;
  Ez_image *surface = monsters->images[MS_ROUGE];

  if(m->hp > 0)
    image_paint_extended(win, surface, m->pos.x, get_scrolling(m->pos.y, info), 1.0, 0.0, FALSE, FALSE);
}


void monster_soucoupe_draw(Info *info, Ez_window win, Monster *m)
{
  Monsters *monsters = &info->game.monsters;
  Ez_image *surface = monsters->images[rand()%3 == 0 ? MS_SOUCOUPE2:MS_SOUCOUPE1];

  if(m->hp > 0)
    image_paint_extended(win, surface, m->pos.x, get_scrolling(m->pos.y, info), 1.0, 0.0, FALSE, FALSE);

}


void info_game_monsters_draw(Info *info, Ez_window win)
{
  Monsters *monsters = &info->game.monsters;
  List *list;

  for(list = monsters->monsters; list != NULL; list = list->next)
  {
    Monster *m = list->data;

    switch(m->type)
    {
    case M_SERPENT: monster_serpent_draw(info, win, m); break;
    case M_CACTUS: monster_cactus_draw(info, win, m); break;
    case M_PIEUVRE: monster_pieuvre_draw(info, win, m); break;
    case M_PROUT: monster_prout_draw(info, win, m); break;
    case M_ROUGE: monster_rouge_draw(info, win, m); break;
    case M_SOUCOUPE: monster_soucoupe_draw(info, win, m); break;
    case M_TROU_NOIR: monster_trou_noir_draw(info, win, m); break;
    default: break;
    }
  }
}


/* ------------------------------ S H O O T S -------------------------------*/

void info_game_doodler_tirs_init(Info *info)
{
  Tirs *tirs = &info->game.doodler.shoots;
  tirs->tirs = NULL;
  tirs->time_since_last_shoot = 150;
  tirs->angle = 0;
}


void info_game_doodler_tirs_load_images(Info *info)
{
  Tirs *tirs = &info->game.doodler.shoots;
  tirs->image = ez_pixmap_create_from_file("images-doodle/shoot.png");
}


void info_game_doodler_tirs_destroy(Info *info)
{
  Tirs *tirs = &info->game.doodler.shoots;
  ez_pixmap_destroy(tirs->image);
}


Tir *tir_new(Point *d, Vector *v)
{
  Tir *tir = malloc(sizeof *tir);
  tir->pos.x = d->x;
  tir->pos.y = d->y;
  tir->speed.x = v->x;
  tir->speed.y = v->y;

  return tir;
}


Vector calculer_vecteur_doodler_mouse(Point *doodler, int scroll, Point *mouse)
{
  Vector v;

  mouse->x -= WINDOW_WIDTH/2 - AREA_WIDTH/2;
  mouse->y -= WINDOW_HEIGHT/2 - AREA_HEIGHT/2;

  v.x = (mouse->x - doodler->x);
  v.y = (mouse->y - (doodler->y + scroll));

  return v;
}


double calculer_norme(Vector *v)
{
  return sqrt(v->x*v->x + v->y*v->y);
}


double produit_scalaire(Vector *v1, Vector *v2)
{
  return v1->x*v2->y + v1->y*v2->x;
}


void transform_vecteur_unitaire(Vector *v)
{
  double norme = calculer_norme(v);
  v->x = v->x/norme;
  v->y = v->y/norme;
}


void scale_vecteur(Vector *v, double k)
{
  v->x = v->x*k;
  v->y = v->y*k;
}


void info_game_doodler_tirs_add_shoot(Info *info)
{
  Doodler *d = &info->game.doodler;
  Tirs *shoots = &d->shoots;
  Point mouse;
  Vector v;
  Vector v1, v2;
  double sin;
  double k;

  shoots->time_since_last_shoot = 0;
  info->keystates.button1 = FALSE;
  mouse.x = info->keystates.mx;
  mouse.y = info->keystates.my;

  v = calculer_vecteur_doodler_mouse(&d->pos, info->score - (WIN_HEIGHT - info->area_height), &mouse);

  v1 = v;
  v2.x = 0;
  v2.y = 10;

  k = produit_scalaire(&v1, &v2);
  sin = k/(calculer_norme(&v1)*calculer_norme(&v2));
  shoots->angle = 180.0 * ((asin(sin)/PI));
  transform_vecteur_unitaire(&v);
  scale_vecteur(&v, 20);
  if(v.y > 0) v.y = -v.y;

  shoots->tirs = list_prepend(shoots->tirs, tir_new(&d->pos, &v));
}


void tirs_delete_tir(Tirs *tirs, Tir *tir)
{
  tirs->tirs = list_remove(tirs->tirs, tir);
  free(tir);
}


boolean tir_in_screen(Info *info, Tir *tir)
{
  if(tir->pos.x < -5) return FALSE;
  if(tir->pos.x > info->area_width + 5) return FALSE;
  if(get_scrolling(tir->pos.y, info) > info->area_height + 5) return FALSE;
  if(get_scrolling(tir->pos.y, info) < -5) return FALSE;

  return TRUE;
}


void info_game_doodler_tir_next_step(Info *info, Tir *tir)
{
  tir->pos.x += tir->speed.x;
  tir->pos.y += tir->speed.y;

  if(!tir_in_screen(info, tir))
    tirs_delete_tir(&info->game.doodler.shoots, tir);
}


boolean doodler_can_shoot(Info *info)
{
  Doodler *d = &info->game.doodler;
  List *list;

  if(d->is_dead) return FALSE;
  if(d->scale != 1.0) return FALSE;

  if(!info->keystates.button1) return FALSE;
  if(d->angle != 0) return FALSE;
  for(list = d->bonus; list != NULL; list = list->next)
  {
    Bonus_Effect *b = list->data;

    if(!b->can_shoot) return FALSE;
  }

  return TRUE;
}


void info_game_doodler_tirs_next_step(Info *info)
{
  Doodler *d = &info->game.doodler;
  Tirs *shoots = &d->shoots;
  List *list;
  Tir *t;

  if(doodler_can_shoot(info))
    info_game_doodler_tirs_add_shoot(info);

  for(list = shoots->tirs; list != NULL; )
  {
    t = list->data;
    list = list->next;
    info_game_doodler_tir_next_step(info, t);
  }

  d->shoots.time_since_last_shoot++;
}


void info_game_doodler_tir_draw(Info *info, Ez_window win, Tir *t)
{
  Tirs *tirs = &info->game.doodler.shoots;
  Ez_pixmap *surface = tirs->image;

  pixmap_paint_extended(win, surface, t->pos.x, get_scrolling(t->pos.y, info));
}


void info_game_doodler_tirs_draw(Info *info, Ez_window win)
{
  Doodler *d = &info->game.doodler;
  Tirs *shoots = &d->shoots;
  List *list;
  Tir *t;

  for(list = shoots->tirs; list != NULL; list = list->next)
  {
    t = list->data;
    info_game_doodler_tir_draw(info, win, t);
  }
}


/* ------------------------------- B O N U S ------------------------------- */

Bonus_Effect *bonus_effect_new(void)
{
  Bonus_Effect *b = malloc(sizeof *b);
  b->angle = 0;
  b->step = 0;
  b->pos.x = -500;
  b->pos.y = 0;
  return b;
}


Bonus_Effect *bonus_effect_chapeau_new(void)
{
  Bonus_Effect *b = bonus_effect_new();
  b->type = B_CHAPEAU;
  b->time_left = 300;
  b->can_fall = FALSE;
  b->can_grab = FALSE;
  b->can_shoot = FALSE;
  b->can_die = FALSE;
  return b;
}


Bonus_Effect *bonus_effect_shield_new(void)
{
  Bonus_Effect *b = bonus_effect_new();
  b->type = B_SHIELD;
  b->time_left = 1000;
  b->can_fall = TRUE;
  b->can_grab = TRUE;
  b->can_shoot = TRUE;
  b->can_die = FALSE;
  return b;
}


Bonus_Effect *bonus_effect_jetpack_new(void)
{
  Bonus_Effect *b = bonus_effect_new();
  b->type = B_JETPACK;
  b->time_left = 500;
  b->can_fall = FALSE;
  b->can_grab = FALSE;
  b->can_shoot = FALSE;
  b->can_die = FALSE;
  return b;
}


Bonus_Effect *bonus_effect_shoes_new(void)
{
  Bonus_Effect *b = bonus_effect_new();
  b->type = B_SHOES;
  b->bound_left = 5;
  b->can_fall = TRUE;
  b->can_grab = FALSE;
  b->can_shoot = TRUE;
  b->can_die = TRUE;
  return b;
}


void doodler_set_bonus_chapeau(Doodler *d)
{
  Bonus_Effect *b = bonus_effect_chapeau_new();
  d->bonus = list_prepend(d->bonus, b);
}


void doodler_set_bonus_shield(Doodler *d)
{
  Bonus_Effect *b = bonus_effect_shield_new();
  d->bonus = list_prepend(d->bonus, b);
}


void doodler_set_bonus_jetpack(Doodler *d)
{
  Bonus_Effect *b = bonus_effect_jetpack_new();
  d->bonus = list_prepend(d->bonus, b);
}


void doodler_set_bonus_shoes(Doodler *d)
{
  Bonus_Effect *b = bonus_effect_shoes_new();
  d->bonus = list_prepend(d->bonus, b);
}


void bonus_decrease_bounds(Doodler *d)
{
  List *list;

  for(list = d->bonus; list != NULL; list = list->next)
  {
    Bonus_Effect *b = list->data;
    b->bound_left--;
    if(b->type == B_SHOES && b->bound_left >= 0)
      b->step = 1;
  }
}


Bonus *bonus_new(double x, double y, Bonus_type type)
{
  Bonus *b = malloc(sizeof *b);
  b->type = type;
  b->pos.x = x;
  b->pos.y = y;
  b->taken = FALSE;
  b->step = 0;
  return b;
}


Bonus *bonus_chapeau_new(double x, double y)
{
  Bonus *b = bonus_new(x, y, B_CHAPEAU);
  b->width = 61;
  b->height = 39;
  return b;
}


Bonus *bonus_shoes_new(double x, double y)
{
  Bonus *b = bonus_new(x, y, B_SHOES);
  b->width = 57;
  b->height = 43;
  return b;
}


Bonus *bonus_jetpack_new(double x, double y)
{
  Bonus *b = bonus_new(x, y, B_JETPACK);
  b->width = 48;
  b->height = 74;
  return b;
}


Bonus *bonus_ressort_new(double x, double y)
{
  Bonus *b = bonus_new(x, y, B_RESSORT);
  b->width = 34;
  b->height = 34;
  return b;
}


Bonus *bonus_shield_new(double x, double y)
{
  Bonus *b = bonus_new(x, y, B_SHIELD);
  b->width = b->height = 66;
  return b;
}


Bonus *bonus_tramp_new(double x, double y)
{
  Bonus *b = bonus_new(x, y, B_TRAMP);
  b->width = 71;
  b->height = 27;
  return b;
}


void doodler_bonus_chapeau_next_step(Doodler *d, Bonus_Effect *b)
{
  if(b->time_left > 200 && d->speed.y > -10)
    d->speed.y -= 0.25;

  if(d->speed.y < -10 && b->time_left > 200)
    d->speed.y = -10;

  if(b->time_left > 0)
  {
    b->pos.x = d->pos.x;
    b->pos.y = d->pos.y -29;
    b->left = d->left;
  }

  else if(b->time_left == 0)
  {
    b->can_fall = TRUE;
    b->can_grab = TRUE;
    b->can_shoot = TRUE;
    b->speed.x = d->speed.x;
    b->pos.y -= 5;
    if(b->speed.x > 0) b->speed.x = 1;
    else b->speed.x = -1;
    b->speed.y = d->speed.y - 0.5;
  }
  else
  {
    b->speed.y += 0.15;
    b->pos.y += b->speed.y;
    b->pos.x += b->speed.x;
    if(b->speed.x > 0) b->angle+=5;
    else b->angle-=5;
  }

  if(b->time_left%3 == 0) b->step++;
  if(b->step > 3) b->step = 0;
}


void doodler_bonus_shield_next_step(Doodler *d, Bonus_Effect *b)
{
  if(b->time_left > 0)
  {
    b->pos.x = d->pos.x;
    b->pos.y = d->pos.y + 20;
  }

  if(b->time_left == 0)
    b->can_die = TRUE;

  if(b->time_left%3 == 0) b->step++;
  if(b->step > 3) b->step = 0;
}


void doodler_bonus_shoes_next_step(Doodler *d, Bonus_Effect *b)
{

  if((b->bound_left > 0 || d->speed.y < -8) && b->step != 8)
  {
    if(d->left)
      b->pos.x = d->pos.x - 5;
    else
      b->pos.x = d->pos.x + 5;

    b->pos.y = d->pos.y + 73;
    if(d->speed.y > -5) b->pos.y += 4;
    b->speed.y = d->speed.y + 1;
    b->speed.x = d->speed.x;
    b->left = d->left;
  }
  else if( !((b->bound_left > 0 || d->speed.y < -8) && b->step != 8) || d->scale != 1.0)
  {
    b->can_grab = TRUE;
    b->can_shoot = TRUE;
    b->step = 8;
    b->pos.x += b->speed.x;
    b->pos.y += b->speed.y;
    b->speed.y += 0.15;
    b->angle++;
    return;
  }

  if(b->step == 1)
  {
    d->speed.y = -15;
    b->step = 2;
  }
  else if(b->step > 1 && b->time_left%4 == 0)
    b->step++;

  if(b->step > 7)
    b->step = 0;
}


void doodler_bonus_jetpack_next_step(Doodler *d, Bonus_Effect *b)
{
  if(b->time_left > 400 && d->speed.y > -20)
    d->speed.y -= 0.5;

  if(b->time_left > 0)
  {
    b->pos.x = d->left ? d->pos.x + 40: d->pos.x -40;
    b->pos.y = d->pos.y + 30;
    b->left = d->left;
  }
  else if(b->time_left == 0)
  {
    b->can_fall = TRUE;
    b->can_grab = TRUE;
    b->can_shoot = TRUE;
    b->speed.x = d->speed.x;
    b->pos.y -= 5;
    if(b->left) b->speed.x = 1;
    else b->speed.x = -1;
    b->speed.y = d->speed.y - 0.5;
  }
  else
  {
    b->speed.y += 0.15;
    b->pos.y += b->speed.y;
    b->pos.x += b->speed.x;
    if(b->speed.x > 0) b->angle+=1;
    else b->angle-=1;
  }

  if(b->time_left%3 == 0)
    b->step++;

  if(b->step > 5 && b->time_left > 100) b->step = 3;

  if(b->step > 8) b->step = 6;
}


void info_game_doodler_bonus_next_step(Info *info)
{
  Doodler *d = &info->game.doodler;
  List *list;

  for(list = d->bonus; list != NULL; )
  {
    Bonus_Effect *b = list->data;
    b->time_left--;
    list = list->next;

    switch(b->type)
    {
    case B_CHAPEAU: doodler_bonus_chapeau_next_step(d, b); break;
    case B_JETPACK: doodler_bonus_jetpack_next_step(d, b); break;
    case B_SHOES : doodler_bonus_shoes_next_step(d, b); break;
    case B_SHIELD: doodler_bonus_shield_next_step(d, b); break;
    default: break;
    }

    if(get_scrolling(b->pos.y, info) - 50 > info->area_height)
    {
      d->bonus = list_remove(d->bonus, b);
      free(b);
    }
  }
}


void info_game_platforms_bonus_load_images(Info *info)
{
  Platforms *platforms = &info->game.platforms;

  platforms->bonus_images[BS_CASQUETTE] = ez_pixmap_create_from_file("images-doodle/bonus_chapeau.png");
  platforms->bonus_images[BS_SHOES] = ez_pixmap_create_from_file("images-doodle/bonus_shoes.png");
  platforms->bonus_images[BS_JETPACK] = ez_pixmap_create_from_file("images-doodle/bonus_jetpack.png");

  platforms->bonus_images[BS_RESSORT1] = ez_pixmap_create_from_file("images-doodle/bonus_ressort1.png");
  platforms->bonus_images[BS_RESSORT2] = ez_pixmap_create_from_file("images-doodle/bonus_ressort2.png");

  platforms->bonus_images[BS_SHIELD] = ez_pixmap_create_from_file("images-doodle/bonus_shield.png");

  platforms->bonus_images[BS_TRAMP1] = ez_pixmap_create_from_file("images-doodle/bonus_tramp1.png");
  platforms->bonus_images[BS_TRAMP2] = ez_pixmap_create_from_file("images-doodle/bonus_tramp2.png");
  platforms->bonus_images[BS_TRAMP3] = ez_pixmap_create_from_file("images-doodle/bonus_tramp3.png");
}


void info_game_platforms_bonus_destroy(Info *info)
{
  Platforms *platforms = &info->game.platforms;
  int i;

  for(i = 0; i < BS_N; i++)
    ez_pixmap_destroy(platforms->bonus_images[i]);
}


void info_game_doodler_bonus_load_images(Info *info)
{
  Doodler *d = &info->game.doodler;

  d->bonus_image[BES_CASQUETTE1] = ez_image_load("images-doodle/bonus_anim_chapeau1.png");
  d->bonus_image[BES_CASQUETTE2] = ez_image_load("images-doodle/bonus_anim_chapeau2.png");
  d->bonus_image[BES_CASQUETTE3] = ez_image_load("images-doodle/bonus_anim_chapeau3.png");
  d->bonus_image[BES_CASQUETTE4] = ez_image_load("images-doodle/bonus_anim_chapeau4.png");

  d->bonus_image[BES_JETPACK1] = ez_image_load("images-doodle/bonus_anim_jetpack1.png");
  d->bonus_image[BES_JETPACK2] = ez_image_load("images-doodle/bonus_anim_jetpack2.png");
  d->bonus_image[BES_JETPACK3] = ez_image_load("images-doodle/bonus_anim_jetpack3.png");
  d->bonus_image[BES_JETPACK4] = ez_image_load("images-doodle/bonus_anim_jetpack4.png");
  d->bonus_image[BES_JETPACK5] = ez_image_load("images-doodle/bonus_anim_jetpack5.png");
  d->bonus_image[BES_JETPACK6] = ez_image_load("images-doodle/bonus_anim_jetpack6.png");
  d->bonus_image[BES_JETPACK7] = ez_image_load("images-doodle/bonus_anim_jetpack7.png");
  d->bonus_image[BES_JETPACK8] = ez_image_load("images-doodle/bonus_anim_jetpack8.png");
  d->bonus_image[BES_JETPACK9] = ez_image_load("images-doodle/bonus_anim_jetpack9.png");
  d->bonus_image[BES_JETPACK10] = ez_image_load("images-doodle/bonus_anim_jetpack10.png");

  d->bonus_image[BES_SHIELD1] = ez_image_load("images-doodle/bonus_anim_shield1.png");
  d->bonus_image[BES_SHIELD2] = ez_image_load("images-doodle/bonus_anim_shield2.png");
  d->bonus_image[BES_SHIELD3] = ez_image_load("images-doodle/bonus_anim_shield3.png");

  d->bonus_image[BES_SHOES1] = ez_image_load("images-doodle/bonus_anim_shoes1.png");
  d->bonus_image[BES_SHOES2] = ez_image_load("images-doodle/bonus_anim_shoes2.png");
  d->bonus_image[BES_SHOES3] = ez_image_load("images-doodle/bonus_anim_shoes3.png");
  d->bonus_image[BES_SHOES4] = ez_image_load("images-doodle/bonus_anim_shoes4.png");
  d->bonus_image[BES_SHOES5] = ez_image_load("images-doodle/bonus_anim_shoes5.png");
}


void info_game_doodle_bonus_destroy(Info *info)
{
  Doodler *d = &info->game.doodler;
  int i;

  for(i = 0; i < BES_N; i++)
    ez_image_destroy(d->bonus_image[i]);
}


void bonus_chapeau_draw(Info *info, Ez_window win, Bonus *b)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface = platforms->bonus_images[BS_CASQUETTE];

  if(!b->taken)
    pixmap_paint_extended(win, surface, b->pos.x, get_scrolling(b->pos.y, info));
}


void bonus_shoes_draw(Info *info, Ez_window win, Bonus *b)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface = platforms->bonus_images[BS_SHOES];

  if(!b->taken)
    pixmap_paint_extended(win, surface, b->pos.x, get_scrolling(b->pos.y, info));
}


void bonus_jetpack_draw(Info *info, Ez_window win, Bonus *b)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface = platforms->bonus_images[BS_JETPACK];

  if(!b->taken)
    pixmap_paint_extended(win, surface, b->pos.x, get_scrolling(b->pos.y, info));
}


void bonus_ressort_draw(Info *info, Ez_window win, Bonus *b)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface;

  if(b->step == 0)
    surface = platforms->bonus_images[BS_RESSORT1];
  else
    surface = platforms->bonus_images[BS_RESSORT2];

  pixmap_paint_extended(win, surface, b->pos.x, get_scrolling(b->pos.y, info));
}


void bonus_shield_draw(Info *info, Ez_window win, Bonus *b)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface = platforms->bonus_images[BS_SHIELD];

  if(!b->taken)
    pixmap_paint_extended(win, surface, b->pos.x, get_scrolling(b->pos.y, info));
}


void bonus_tramp_draw(Info *info, Ez_window win, Bonus *b)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface;

  if(b->step == 0)
    surface = platforms->bonus_images[BS_TRAMP1];
  else if(b->step == 1)
    surface = platforms->bonus_images[BS_TRAMP2];
  else
    surface = platforms->bonus_images[BS_TRAMP3];

  pixmap_paint_extended(win, surface, b->pos.x, get_scrolling(b->pos.y, info));
}


void bonus_draw(Info *info, Ez_window win, Bonus *b)
{
  switch(b->type)
  {
  case B_CHAPEAU: bonus_chapeau_draw(info, win, b); break;
  case B_SHOES: bonus_shoes_draw(info, win, b); break;
  case B_JETPACK: bonus_jetpack_draw(info, win, b); break;
  case B_RESSORT: bonus_ressort_draw(info, win, b); break;
  case B_SHIELD: bonus_shield_draw(info, win, b); break;
  case B_TRAMP: bonus_tramp_draw(info, win, b); break;
  default: break;
  }
}


void doodler_bonus_chapeau_draw(Info *info, Ez_window win, Bonus_Effect *b)
{
  Ez_image *surface;
  Doodler *d = &info->game.doodler;
  double y = 0;

  if(d->speed.y > -2)  y = -2;

  if(b->time_left < 0) surface = d->bonus_image[BES_CASQUETTE1];
  else
  {
    switch(b->step)
    {
    case 0: surface = d->bonus_image[BES_CASQUETTE3]; break;
    case 1: surface = d->bonus_image[BES_CASQUETTE2]; break;
    case 2 : surface = d->bonus_image[BES_CASQUETTE4]; break;
    case 3 : surface = d->bonus_image[BES_CASQUETTE3]; break;
    default: surface = d->bonus_image[BES_CASQUETTE3]; break;
    }
  }

  image_paint_extended(win, surface, b->pos.x, get_scrolling(b->pos.y, info) + y, 1.0, b->angle, b->left, FALSE);
}


void doodler_bonus_shield_draw(Info *info, Ez_window win, Bonus_Effect *b)
{
  Ez_image *surface;
  Doodler *d = &info->game.doodler;

  switch(b->step)
  {
  case 0: surface = d->bonus_image[BES_SHIELD1]; break;
  case 1: surface = d->bonus_image[BES_SHIELD2]; break;
  case 2 : surface = d->bonus_image[BES_SHIELD3]; break;
  default: surface = d->bonus_image[BES_SHIELD1]; break;
  }

  if(b->time_left > 0)
    image_paint_extended(win, surface, b->pos.x, get_scrolling(b->pos.y, info), 1.0, b->angle, b->left, FALSE);
}


void doodler_bonus_jetpack_draw(Info *info, Ez_window win, Bonus_Effect *b)
{
  Ez_image *surface;
  Doodler *d = &info->game.doodler;

  if(b->time_left < 0) surface = d->bonus_image[BES_JETPACK10];
  else
  {
    switch(b->step)
    {
    case 0: surface = d->bonus_image[BES_JETPACK1]; break;
    case 1: surface = d->bonus_image[BES_JETPACK2]; break;
    case 2: surface = d->bonus_image[BES_JETPACK3]; break;
    case 3: surface = d->bonus_image[BES_JETPACK4]; break;
    case 4: surface = d->bonus_image[BES_JETPACK5]; break;
    case 5: surface = d->bonus_image[BES_JETPACK6]; break;
    case 6: surface = d->bonus_image[BES_JETPACK7]; break;
    case 7: surface = d->bonus_image[BES_JETPACK8]; break;
    case 8: surface = d->bonus_image[BES_JETPACK9]; break;
    default: surface = d->bonus_image[BES_JETPACK1]; break;
    }
  }

  image_paint_extended(win, surface, b->pos.x, get_scrolling(b->pos.y, info), 1.0, b->angle, !b->left, FALSE);
}


void doodler_bonus_shoes_draw(Info *info, Ez_window win, Bonus_Effect *b)
{
  Ez_image *surface;
  Doodler *d = &info->game.doodler;

  switch(b->step)
  {
  case 0: surface = d->bonus_image[BES_SHOES5]; break;
  case 1: surface = d->bonus_image[BES_SHOES4]; break;
  case 2: surface = d->bonus_image[BES_SHOES3]; break;
  case 3: surface = d->bonus_image[BES_SHOES2]; break;
  case 4: surface = d->bonus_image[BES_SHOES1]; break;
  case 5: surface = d->bonus_image[BES_SHOES2]; break;
  case 6: surface = d->bonus_image[BES_SHOES3]; break;
  case 7: surface = d->bonus_image[BES_SHOES4]; break;
  case 8: surface = d->bonus_image[BES_SHOES5]; break;
  default: surface = d->bonus_image[BES_SHOES5]; break;
  }

  image_paint_extended(win, surface, b->pos.x, get_scrolling(b->pos.y, info), 1.0, b->angle, !b->left, FALSE);
}


boolean doodler_on_bonus(Doodler *doodler, Bonus *b)
{
  if(doodler->is_dead) return FALSE;

  if(doodler->pos.y + doodler->hit_object.a.y > b->pos.y + b->height/2)
    return FALSE;

  if(doodler->pos.y + doodler->hit_object.b.y < b->pos.y - b->height/2)
    return FALSE;

  if(doodler->pos.x + doodler->hit_object.a.x > b->pos.x + b->width/2)
    return FALSE;

  if(doodler->pos.x + doodler->hit_object.b.x < b->pos.x - b->width/2)
    return FALSE;

  return TRUE;
}


boolean can_grab_bonus(Info *info)
{
  Doodler *d = &info->game.doodler;
  List *list;

  if(d->is_dead) return FALSE;

  for(list = d->bonus; list != NULL; list = list->next)
  {
    Bonus_Effect *b = list->data;
    if(!b->can_grab)
      return FALSE;
  }
  return TRUE;
}


void bonus_chapeau_next_step(Bonus *b, Info *info)
{
  if(doodler_on_bonus(&info->game.doodler, b) && b->taken == FALSE)
  {
    info->game.doodler.shoots.time_since_last_shoot = 150;
    b->taken = TRUE;
    doodler_stop_rotating(info);
    doodler_set_bonus_chapeau(&info->game.doodler);
  }
}


void bonus_jetpack_next_step(Bonus *b, Info *info)
{
  if(doodler_on_bonus(&info->game.doodler, b) && b->taken == FALSE)
  {
    info->game.doodler.shoots.time_since_last_shoot = 150;
    b->taken = TRUE;
    doodler_stop_rotating(info);
    doodler_set_bonus_jetpack(&info->game.doodler);
  }
}


boolean can_jump_bonus(Bonus *b, Info *info)
{
  Doodler *d = &info->game.doodler;

  if(d->speed.y < 0) return FALSE;

  if(d->pos.x + d->hitbox.b.x < b->pos.x - b->width/2)
    return FALSE;
  if(d->pos.x + d->hitbox.a.x > b->pos.x + b->width/2)
    return FALSE;
  if(d->pos.y + d->hitbox.a.y > b->pos.y + b->height/2)
    return FALSE;
  if(d->pos.y + d->hitbox.b.y < b->pos.y - b->height/2)
    return FALSE;

  return TRUE;
}


void bonus_shoes_next_step(Bonus *b, Info *info)
{
  Doodler *d = &info->game.doodler;

  if(doodler_on_bonus(d, b) && b->taken == FALSE)
  {
    b->taken = TRUE;
    doodler_stop_rotating(info);
    doodler_set_bonus_shoes(d);
  }
}


void bonus_shield_next_step(Bonus *b, Info *info)
{
  Doodler *d = &info->game.doodler;

  if(doodler_on_bonus(d, b) && b->taken == FALSE)
  {
    b->taken = TRUE;
    doodler_set_bonus_shield(d);
  }
}


void bonus_ressort_next_step(Bonus *b, Info *info)
{
  Doodler *d = &info->game.doodler;

  if(can_jump_bonus(b, info))
  {
    b->step = 1;
    d->speed.y = -15.0;
  }
}


void bonus_tramp_next_step(Bonus *b, Info *info)
{
  Doodler *d = &info->game.doodler;

  if(can_jump_bonus(b, info))
  {
    d->shoots.time_since_last_shoot = 150;
    b->step = 1;
    d->speed.y = -20.0;
    d->is_rotating = TRUE;
  }

  if(b->step == 1) b->step = 2;
  if(b->step == 2) b->step = 3;
}


void info_game_platforms_bonus_next_step(Platform *p, Info *info)
{
  Bonus *b = p->bonus;
  b->pos.x = p->pos.x;
  if(!can_grab_bonus(info)) return;

  switch(b->type)
  {
  case B_CHAPEAU: bonus_chapeau_next_step(b, info); break;
  case B_JETPACK: bonus_jetpack_next_step(b, info); break;
  case B_RESSORT: bonus_ressort_next_step(b, info); break;
  case B_TRAMP: bonus_tramp_next_step(b, info); break;
  case B_SHOES: bonus_shoes_next_step(b, info); break;
  case B_SHIELD: bonus_shield_next_step(b, info); break;
  default: break;
  }
}


/* ---------------------------- P L A T F O R M S -------------------------- */

void info_game_platforms_init(Info *info)
{
  Platforms *platforms = &info->game.platforms;

  platforms->queue = queue_new();
  info_game_platforms_load_images(info);
  queue_push_tail(platforms->queue, platform_static_new(200, WIN_HEIGHT - 80 - 1000));
}


void info_game_platforms_restart(Info *info)
{
  Platforms *platforms = &info->game.platforms;
  Game *game = &info->game;

  queue_free_full(platforms->queue, free_platform);
  platforms->queue = queue_new();
  queue_push_tail(platforms->queue, platform_static_new(200, WIN_HEIGHT - 2070));
  game->soucoupe = monster_soucoupe_new(WIN_WIDTH/1.2, -1800-30);
  info_game_monsters_add(game->soucoupe, info);
}


Platform *platform_new(double x, double y, Platform_type type)
{
  Platform *p = malloc(sizeof *p);
  p->type = type;
  p->pos.x = x;
  p->pos.y = y;
  p->width = PLATFORM_WIDTH;
  p->height = PLATFORM_HEIGHT;
  p->bonus = NULL;

  return p;
}


Platform *platform_static_new(double x, double y)
{
  return platform_new(x, y, P_STATIC);
}


Platform *platform_horizontal_new(double x, double y, double xspeed)
{
  Platform *p = platform_new(x, y, P_HORIZONTAL);
  p->speed.x = xspeed;
  return p;
}


Platform *platform_vertical_new(double x, double y, double yspeed, double amplitude)
{
  Platform *p = platform_new(x, y, P_VERTICAL);
  p->speed.y = yspeed;
  p->ymin = y - amplitude;
  p->ymax = y + amplitude;
  return p;
}


Platform *platform_break_new(double x, double y, double xspeed)
{
  Platform *p = platform_new(x, y, P_BREAK);
  p->speed.x = xspeed;
  p->step = 0;
  return p;
}


Platform *platform_explode_new(double x, double y, int time)
{
  Platform *p = platform_new(x, y, P_EXPLODE);
  p->time_left = time;
  return p;
}


Platform *platform_vanish_new(double x, double y)
{
  Platform *p = platform_new(x, y, P_VANISH);
  p->visible = TRUE;
  p->alpha = 1.0;
  return p;
}


Platform *platform_move_new(double x, double y)
{
  Platform *p = platform_new(x, y, P_MOVE);
  p->speed.x = rand()%2 ?1: -1;
  p->xl = x-5;
  p->xr = x+5;
  p->step = 0;
  p->link = NULL;

  return p;
}


void free_platform(void* data)
{
  Platform *p = data;
  if(data == NULL) return;

  if(p->bonus != NULL)
    free(p->bonus);

  free(p);
}


void platform_destroy_over_bot_screen(Info *info)
{
  Queue *q = info->game.platforms.queue;
  Platform *p = queue_peek_head(q);

  while(p != NULL && get_scrolling(p->pos.y - p->height/2, info) - 50 > info->area_height)
  {
    p = queue_pop_head(q);
    if(p->bonus != NULL)
      free(p->bonus);

     free(p);
     p = queue_peek_head(q);
  }
}


void info_game_platforms_load_images(Info *info)
{
  Platforms *platforms = &info->game.platforms;

  info_game_platforms_bonus_load_images(info);

  platforms->images[SP_STATIC] = ez_pixmap_create_from_file("images-doodle/plat_static.png");
  platforms->images[SP_HORIZONTAL] = ez_pixmap_create_from_file("images-doodle/plat_hor.png");
  platforms->images[SP_VERTICAL] = ez_pixmap_create_from_file("images-doodle/plat_ver.png");
  platforms->images[SP_HORIZONTAL] = ez_pixmap_create_from_file("images-doodle/plat_hor.png");

  platforms->images[SP_EXPLODE1] = ez_pixmap_create_from_file("images-doodle/plat_exp1.png");
  platforms->images[SP_EXPLODE2] = ez_pixmap_create_from_file("images-doodle/plat_exp2.png");
  platforms->images[SP_EXPLODE3] = ez_pixmap_create_from_file("images-doodle/plat_exp3.png");
  platforms->images[SP_EXPLODE4] = ez_pixmap_create_from_file("images-doodle/plat_exp4.png");
  platforms->images[SP_EXPLODE5] = ez_pixmap_create_from_file("images-doodle/plat_exp5.png");
  platforms->images[SP_EXPLODE6] = ez_pixmap_create_from_file("images-doodle/plat_exp6.png");
  platforms->images[SP_EXPLODE7] = ez_pixmap_create_from_file("images-doodle/plat_exp7.png");
  platforms->images[SP_EXPLODE8] = ez_pixmap_create_from_file("images-doodle/plat_exp8.png");

  platforms->images[SP_VANISH] = ez_pixmap_create_from_file("images-doodle/plat_vanish.png");

  platforms->images[SP_BREAK1] = ez_pixmap_create_from_file("images-doodle/plat_break1.png");
  platforms->images[SP_BREAK2] = ez_pixmap_create_from_file("images-doodle/plat_break2.png");
  platforms->images[SP_BREAK3] = ez_pixmap_create_from_file("images-doodle/plat_break3.png");
  platforms->images[SP_BREAK4] = ez_pixmap_create_from_file("images-doodle/plat_break4.png");

  platforms->images[SP_MOVE] = ez_pixmap_create_from_file("images-doodle/plat_move.png");
}


void info_game_platforms_destroy(Info *info)
{
  Platforms *platforms = &info->game.platforms;
  int i;

  for(i = 0; i < SP_N; i++)
    ez_pixmap_destroy(platforms->images[i]);
}


void platform_static_draw(Info *info, Ez_window win, Platform *p)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface = platforms->images[SP_STATIC];

  pixmap_paint_extended(win, surface, p->pos.x, get_scrolling(p->pos.y, info));
}


void platform_horizontal_draw(Info *info, Ez_window win, Platform *p)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface = platforms->images[SP_HORIZONTAL];

  pixmap_paint_extended(win, surface, p->pos.x, get_scrolling(p->pos.y, info));
}


void platform_vertical_draw(Info *info, Ez_window win, Platform *p)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface = platforms->images[SP_VERTICAL];

  pixmap_paint_extended(win, surface, p->pos.x, get_scrolling(p->pos.y, info));
}


void platform_explode_draw(Info *info, Ez_window win, Platform *p)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface = platforms->images[SP_EXPLODE1];

  if(p->time_left > 200) surface = platforms->images[SP_EXPLODE1];
  else if(p->time_left > 190) surface = platforms->images[SP_EXPLODE2];
  else if(p->time_left > 180) surface = platforms->images[SP_EXPLODE3];
  else if(p->time_left > 170) surface = platforms->images[SP_EXPLODE4];
  else if(p->time_left > 160) surface = platforms->images[SP_EXPLODE5];
  else if(p->time_left > 0 ) surface = platforms->images[SP_EXPLODE5];
  else if(p->time_left > -10 ) surface = platforms->images[SP_EXPLODE6];
  else if(p->time_left > -20 ) surface = platforms->images[SP_EXPLODE7];

  else if(p->time_left > -30 ) surface = platforms->images[SP_EXPLODE8];

  if(p->time_left > - 30)
     pixmap_paint_extended(win, surface, p->pos.x, get_scrolling(p->pos.y, info));
}


void platform_vanish_draw(Info *info, Ez_window win, Platform *p)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface = platforms->images[SP_VANISH];

  if(p->alpha > 0.0)
    pixmap_paint_extended(win, surface, p->pos.x, get_scrolling(p->pos.y, info));
}


void platform_break_draw(Info *info, Ez_window win, Platform *p)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface = platforms->images[SP_BREAK1];

  if(p->step == 0) surface = platforms->images[SP_BREAK1];
  else if(p->time_left > 10) surface = platforms->images[SP_BREAK2];
  else if(p->time_left > 5) surface = platforms->images[SP_BREAK3];
  else if(p->time_left > 0) surface = platforms->images[SP_BREAK4];
  else if(p->time_left <= 0) surface = platforms->images[SP_BREAK4];

  pixmap_paint_extended(win, surface, p->pos.x, get_scrolling(p->pos.y, info));
}


void platform_move_draw(Info *info, Ez_window win, Platform *p)
{
  Platforms *platforms = &info->game.platforms;
  Ez_pixmap *surface = platforms->images[SP_MOVE];

  pixmap_paint_extended(win, surface, p->pos.x, get_scrolling(p->pos.y, info));
}


void info_game_platforms_draw(Info *info, Ez_window win)
{
  Platforms *platforms = &info->game.platforms;
  List *list;

  for(list = platforms->queue->head; list != NULL; list = list_next(list))
  {
    Platform *p = list->data;
    switch(p->type)
    {
    case P_STATIC: platform_static_draw(info, win, p); break;
    case P_HORIZONTAL: platform_horizontal_draw(info, win, p); break;
    case P_VERTICAL: platform_vertical_draw(info, win, p); break;
    case P_EXPLODE: platform_explode_draw(info, win, p); break;
    case P_VANISH: platform_vanish_draw(info, win, p); break;
    case P_BREAK: platform_break_draw(info, win, p); break;
    case P_MOVE: platform_move_draw(info, win, p); break;
    }

    if(p->bonus != NULL)
      bonus_draw(info, win, p->bonus);
  }
}


boolean doodler_has_bonus_shoes(Doodler *d)
{
  List *list;

  for(list = d->bonus; list != NULL; list = list->next)
  {
    Bonus_Effect *b = list->data;
    if(b->type == B_SHOES) return TRUE;
  }

  return FALSE;
}


boolean platform_contain_doodler(Doodler *d, Platform *p)
{
  int addy = 0;

  if(doodler_has_bonus_shoes(d))
  {
    addy = 20;
  }

# ifdef NEVER_FALL
  return TRUE;
# endif
 
  if(d->pos.x + d->hitbox.a.x > p->pos.x + p->width/2) return FALSE;
  if(d->pos.x + d->hitbox.b.x < p->pos.x - p->width/2) return FALSE;
  if(d->pos.y + d->hitbox.a.y > p->pos.y + p->height/2) return FALSE;
  if(d->pos.y + d->hitbox.b.y - 10  + addy < p->pos.y - p->height/2) return FALSE;

  return TRUE;
}


void info_game_platforms_next_step(Info *info)
{
  Platforms *p = &info->game.platforms;
  List *list;

  for(list = p->queue->head; list != NULL; list = list_next(list))
  {
    Platform *platform = list->data;

    switch(platform->type)
    {
    case P_STATIC: platform_static_next_step(platform, info); break;
    case P_HORIZONTAL: platform_horizontal_next_step(platform, info); break;
    case P_VERTICAL: platform_vertical_next_step(platform, info); break;
    case P_EXPLODE: platform_explode_next_step(platform, info); break;
    case P_BREAK : platform_break_next_step(platform, info); break;
    case P_VANISH : platform_vanish_next_step(platform, info); break;
    case P_MOVE: platform_move_next_step(platform, info); break;
    }
    if(platform->bonus != NULL)
      info_game_platforms_bonus_next_step(platform, info);
  }

  platform_destroy_over_bot_screen(info);

}


boolean platform_regular_bound(Platform *p, Info *info)
{
  Doodler *d = &info->game.doodler;
  if(d->speed.y > 0 && platform_contain_doodler(d, p))
  {
    doodler_start_jump_platform(d, p);
    return TRUE;
  }
  return FALSE;
}


void platform_static_next_step(Platform *p, Info *info)
{
  platform_regular_bound(p, info);
}


void platform_horizontal_next_step(Platform *p, Info *info)
{
  p->pos.x += p->speed.x;
  if(p->pos.x - p->width/2 < 0)
  {
    p->pos.x = p->width/2;
    p->speed.x = -p->speed.x;
  }
  else if(p->pos.x + p->width/2 > info->area_width)
  {
    p->pos.x = info->area_width - p->width/2;
    p->speed.x = -p->speed.x;
  }

  platform_regular_bound(p, info);
}


void platform_vertical_next_step(Platform *p, Info *info)
{
  p->pos.y += p->speed.y;

  if(p->pos.y < p->ymin)
  {
    p->pos.y = p->ymin;
    p->speed.y = -p->speed.y;
  }
  else if(p->pos.y > p->ymax)
  {
    p->pos.y = p->ymax;
    p->speed.y = -p->speed.y;
  }

  platform_regular_bound(p, info);
}


void platform_explode_next_step(Platform *p, Info *info)
{
  if(get_scrolling(p->pos.y, info) > 0) p->time_left--;

  if(p->time_left >= 0)
    platform_regular_bound(p, info);

}


void platform_break_next_step(Platform *p, Info *info)
{
  Doodler *d = &info->game.doodler;

  p->pos.x += p->speed.x;
  if(p->step == 0 && (p->pos.x < p->width/2 || p->pos.x > info->area_width - p->width/2))
     p->speed.x = -p->speed.x;

  if(d->speed.y > 0 && platform_contain_doodler(d, p) && p->step == 0)
  {
    p->time_left = 15;
    p->step = 1;
  }

  if(p->step == 1)
  {
    p->time_left--;
    p->pos.y+=4;
  }

}


void platform_vanish_next_step(Platform *p, Info *info)
{
  if(p->visible)
  {
    if(platform_regular_bound(p, info))
      p->visible = FALSE;
  }
  else p->alpha -= 0.05;
}


void platform_move_next_step(Platform *p, Info *info)
{

  p->pos.x += p->speed.x;

  if(p->step == 0)
  {
    if(p->pos.x > p->xr)
      p->speed.x = -p->speed.x;
    if(p->pos.x < p->xl)
      p->speed.x = -p->speed.x;
  }
  if(p->step == 1)
  {
    if(p->speed.x > 0 && p->pos.x >= p->x2)
    {
      p->speed.x = 1.0;
      p->xl = p->pos.x - 5;
      p->xr = p->pos.x + 5;
      p->step = 0;
    }
    if(p->speed.x < 0 && p->pos.x <= p->x2)
    {
      p->speed.x = 1.0;
      p->xl = p->pos.x - 5;
      p->xr = p->pos.x + 5;
      p->step = 0;
    }
  }

  if(platform_regular_bound(p, info) && p->step == 0)
  {
    while(p)
    {
      p->step = 1;
      do
      {
        p->x2 = 5 + p->width/2 + rand()%(int)(info->area_width - p->width - 5);
      } while(DISTANCE_HOR(p->x2, p->pos.x) < 100);
      if(p->x2 < p->pos.x)
        p->speed.x = -15;
      else
        p->speed.x = 15;

      p = p->link;
    }
  }

}


/* ---------------------------- G E N E R A T O R -------------------------- */

Platform *platform_find_no_breakable(List *list)
{
  Platform *p = list->data;

  while(p->type == P_BREAK)
  {
    list = list->prev;
    p = list->data;
  }

  return p;
}


#define SAUT_MAX 210

Point get_next_platform_pos(Info *info)
{
  Platforms *platforms = &info->game.platforms;
  Platform *valid = platform_find_no_breakable(platforms->queue->tail);
  Platform *last = queue_peek_tail(platforms->queue);
  double x = last->width/2 + 5 + rand()%(int)(info->area_width - last->width - 5);
  double y = last->pos.y - last->height - 20;
  double yvalid = valid->pos.y;
  Point res;

  if(last->bonus != NULL) y -= 100;

  y -= rand()%(info->score/100 + 1);

  while(yvalid - y > SAUT_MAX) y+= 1 + rand()%50;

  res.x = x;
  res.y = y;

  return res;
}


Bonus *get_platform_bonus(Info U *info, Platform *p)
{
  int r;
  if(rand()%10 != 0) return NULL;

  r = rand()%100;

  if(r >= 0 && r < 10) return bonus_shield_new(p->pos.x, p->pos.y - p->height);
  if(r >= 10 && r < 70) return bonus_ressort_new(p->pos.x, p->pos.y - p->height);
  if(r >= 70 && r < 80) return bonus_chapeau_new(p->pos.x, p->pos.y - p->height);
  if(r >= 80 && r < 90) return bonus_tramp_new(p->pos.x, p->pos.y - p->height + 5);
  if(r >= 90 && r < 95) return bonus_shoes_new(p->pos.x, p->pos.y - p->height + 7);
  if(r >= 95 && r < 100) return bonus_jetpack_new(p->pos.x, p->pos.y - p->height - 10);

  return NULL;
}


void monsters_genere(Info *info)
{
  int r;
  Platforms *platforms = &info->game.platforms;
  Platform *last;
  double x, y;
  Monster *m;
  Point pos;
  r = rand()%150;
  if(info->score < 10000) return;
  if(rand()%20 != 0) return;

  pos = get_next_platform_pos(info);
  last = platform_static_new(pos.x, pos.y);
  queue_push_tail(platforms->queue, last);

  x = last->width/2 + rand()%(int)(info->area_width - last->width);
  y = last->pos.y - (100 + rand()%50);

  if(r >= 0 && r < 20) m = monster_serpent_new(x, y);
  else if(r >= 20 && r < 40)
  {
    last = platform_static_new(x, y);
    queue_push_tail(platforms->queue, last);
    m = monster_cactus_new(x, y - 100);
  }
  else if(r >= 40 && r < 60)
    m = monster_pieuvre_new(x, y);
  else if(r >= 60 && r < 80)
  {
    x = 300 + rand()%(int)(info->area_width - last->width - 300);
    last = platform_static_new(x - 120, y);
    queue_push_tail(platforms->queue, last);
    last = platform_static_new(x, y);
    queue_push_tail(platforms->queue, last);
    m = monster_prout_new(x - 70, y - 78);
  }
  else if(r >= 80 && r < 100)
    m = monster_rouge_new(x, y);
  else if(r >= 100 && r < 120)
  {
    if(x > info->area_width/2)
      last = platform_static_new(last->width/2 + rand()%(int)(info->area_width/2 - last->width), y - 50 - rand()%30);
    else
       last = platform_static_new(last->width/2 + info->area_width/2 + rand()%(int)(info->area_width/2 - last->width), y - 50 - rand()%30);

    queue_push_tail(platforms->queue, last);
    m = monster_soucoupe_new(x, y);
  }
  else m = monster_trou_noir_new(x, y);

  info_game_monsters_add(m, info);
  last = platform_static_new(last->width/2 + rand()%(int)(info->area_width - last->width), m->pos.y - m->height/2 - rand()%50);
  queue_push_tail(platforms->queue, last);
}


void platforms_genere_statics(Info *info)
{
  Platforms *platforms = &info->game.platforms;
  int r = get_random_with_range(1, 30 - info->score/5000);
  Platform *p2;
  int rbreak = info->score > 8000 ? 3:20;
  while(r > 0)
  {
    Platform *valid = platform_find_no_breakable(platforms->queue->tail);
    Point pos = get_next_platform_pos(info);
    if(rand()%rbreak == 0 && valid->pos.y - pos.y < SAUT_MAX - 80)
    {
      p2 = platform_break_new(pos.x, pos.y, 0);
    }
    else
    {
      p2 = platform_static_new(pos.x, pos.y);

      p2->bonus = get_platform_bonus(info, p2);
    }
    queue_push_tail(platforms->queue, p2);

    r--;
    monsters_genere(info);
  }
}


void platforms_genere_horizontal(Info *info)
{
  Platforms *platforms = &info->game.platforms;
  int r = 5 + rand()%(info->score/5000 + 1);

  double speed = 1 + rand()%(info->score/10000 + 1);

  if(speed > 6) speed = 5 + rand()%2;

  if(info->score < 5000) return;

  while(r > 0)
  {
    Platform *p;
    Point pos = get_next_platform_pos(info);
    Platform *valid = platform_find_no_breakable(platforms->queue->tail);
    if(rand()%10 == 0 && valid->pos.y - pos.y < SAUT_MAX - 40)
      p = platform_break_new(pos.x, pos.y, speed);
    else
    {
      p = platform_horizontal_new(pos.x, pos.y, speed);
      p->bonus = get_platform_bonus(info, p);
    }

    speed = -speed;
    queue_push_tail(platforms->queue, p);
    r--;
    monsters_genere(info);
  }
}


void platforms_genere_vertical(Info *info)
{
  Platforms *platforms = &info->game.platforms;
  Platform *p;
  Point pos;
  int r = rand()%2;
  double speed = 1 + rand()%(info->score/20000 + 1);

  if(info->score < 20000) return;

  if(speed > 3) speed = 3;

  pos = get_next_platform_pos(info);
  p = platform_static_new(pos.x, pos.y);
  queue_push_tail(platforms->queue, p);

  while(r > 0)
  {
    Platform *last = queue_peek_tail(platforms->queue);
    Point pos1 = get_next_platform_pos(info);
    Point pos2 = get_next_platform_pos(info);

    Platform *p1, *p2;
    double amp = 75 + rand()%75;

    if(pos1.y - last->pos.y > 75) pos1.y = last->pos.y - 75;
    if(pos2.y - last->pos.y > 75) pos2.y = last->pos.y - 75;

    pos1.y -= amp;
    pos2.y -= amp;

    if(rand()%2)
      pos1.y -= amp/2 + rand()%(int)(amp/2);
    else
      pos2.y -= amp/2 + rand()%(int)(amp/2);

    if(pos1.x < info->area_width/2)
      pos2.x = info->area_width/2 + last->width/2 + rand()%(int)(info->area_width/2 - last->width);
    else
      pos2.x = last->width/2 + rand()%(int)(info->area_width/2 - last->width);

    speed = rand()%2 ? speed: -speed;
    p1 = platform_vertical_new(pos1.x, pos1.y, speed, amp);
    p2 = platform_vertical_new(pos2.x, pos2.y, -speed, amp);

    queue_push_tail(platforms->queue, p1);
    queue_push_tail(platforms->queue, p2);

    pos1.y = pos1.y < pos2.y ? (pos1.y - amp - 20):(pos2.y - amp - 20);
    pos1.x = last->width/2 + 5 + rand()%(int)(info->area_width/2 - last->width - 5);

    p1 = platform_static_new(pos1.x, pos1.y);
    queue_push_tail(platforms->queue, p1);
    r--;
  }
}


void platforms_genere_explode(Info *info)
{
  Platforms *platforms = &info->game.platforms;
  int r = 10 + rand()%(info->score/20000 + 1);

  if(info->score < 12000) return;

  platforms_genere_statics(info);

  while(r > 0)
  {
    Platform *p;
    Point pos = get_next_platform_pos(info);
    double time_left = 380 + rand()%100;

    p = platform_explode_new(pos.x, pos.y, time_left);
    queue_push_tail(platforms->queue, p);
    r--;
  }
}


void platforms_genere_vanish(Info *info)
{
  Platforms *platforms = &info->game.platforms;
  int r = 10 + rand()%(info->score/20000 + 1);

  if(info->score < 9000) return;

  while(r)
  {
    Platform *p;
    Point pos = get_next_platform_pos(info);

    p = platform_vanish_new(pos.x, pos.y);
    queue_push_tail(platforms->queue, p);

    r--;
    monsters_genere(info);
  }
}


void platforms_genere_move(Info *info)
{
  Platforms *platforms = &info->game.platforms;
  int r = 10 + rand()%(info->score/20000 + 1);

  if(info->score < 12000) return;


  while(r > 0)
  {
    Platform *last = queue_peek_tail(platforms->queue);
    Point pos = get_next_platform_pos(info);
    Platform *p;

    p = platform_move_new(pos.x, pos.y);
    if(last->type == P_MOVE)
      last->link = p;

    queue_push_tail(platforms->queue, p);
    last = p;
    r--;
  }
}


void platforms_genere_others(Info *info)
{
  int s =  80 - info->score/10000;
  int r;

  if(s < 40) s = 40;

  r = get_random_with_range(0, s);


  if(r > 30) platforms_genere_horizontal(info);
  else if(r > 25) platforms_genere_vertical(info);
  else if(r > 18) platforms_genere_explode(info);
  else if(r > 9) platforms_genere_vanish(info);
  else platforms_genere_move(info);
}


void info_game_platforms_genere(Info *info)
{
  Platforms *platforms = &info->game.platforms;
  Platform *p = queue_peek_tail(platforms->queue);

  int r, s = 20 - info->score/5000;

  if(s < 13) s = 13;

  while(p && p->pos.y + info->score > - 200)
  {
    r = get_random_with_range(0, 20 - info->score/5000);

    if(r > 10)
      platforms_genere_statics(info);
    else
      platforms_genere_others(info);

    p = queue_peek_tail(platforms->queue);
  }
}


void dessiner_cadre(Ez_window win, Info *info)
{
  int w, h;

  ez_window_get_size(win, &w, &h);

  WINDOW_WIDTH = w;
  WINDOW_HEIGHT = h;
  AREA_WIDTH = info->area_width;
  AREA_HEIGHT = info->area_height;

  ez_set_color(ez_black);

  ez_fill_rectangle(win, 0, 0, w/2 - info->area_width/2, h);
  ez_fill_rectangle(win, w/2 + info->area_width/2, 0, w, h);
  ez_fill_rectangle(win, 0, 0, w, h/2 - info->area_height/2);
  ez_fill_rectangle(win, 0, h/2 + info->area_height/2, w, h);
}


void win_on_expose(Ez_event *ev)
{
  Info *info = ez_get_data(ev->win);
  static int nbexpose = 0;
  static int fps = 0;
  static double now = 0;
  static double before = 0;
  info_game_background_draw(info, info->win);
  info_game_platforms_draw(info, info->win);
  info_game_monsters_draw(info, info->win);
  info_game_gameover_draw(info, info->win);
  info_game_doodlejump_draw(info, info->win);
  info_score_draw(info, info->win);
  info_game_doodler_draw(info, info->win);

  if(info->show_fps)
    ez_draw_text(ev->win, 
        EZ_TR, WINDOW_WIDTH/2 + AREA_WIDTH/2 - 2,
        WINDOW_HEIGHT/2 - AREA_HEIGHT/2 + 1, "%d fps", fps);

  now = ez_get_time();
  nbexpose++;

  if(now - before >= 1.0)
  {
    before = now;
    fps = nbexpose;
    nbexpose = 0;
  }

  dessiner_cadre(ev->win, info);
}


void win_on_button_press(Ez_event *ev)
{
  Info *info = ez_get_data(ev->win);
  Keystates *ks = &info->keystates;
  ks->button1 = TRUE;
}


void win_on_button_release(Ez_event *ev)
{
  Info *info = ez_get_data(ev->win);
  Keystates *ks = &info->keystates;
  ks->button1 = FALSE;
}


void win_on_motion(Ez_event *ev)
{
  Info *info = ez_get_data(ev->win);
  Keystates *ks = &info->keystates;

  ks->mx = ev->mx;
  ks->my = ev->my;
}


void check_state(Info *info, Ez_event *ev)
{
  if(ev->key_sym != XK_space ) return;

  switch(info->state)
  {
  case GAME_BEGIN: info_set_state_playing(info, &info->win); break;
  case GAME_PLAYING: if(!info->game.doodler.is_dead) info_set_state_paused(info); break;
  case GAME_PAUSED: info_set_state_playing(info, &info->win); break;
  case GAME_LOST: info_set_state_begin(info); break;
  }
}


void win_on_key_press(Ez_event *ev)
{
  Info *info = ez_get_data(ev->win);

  check_state(info, ev);

  switch(ev->key_sym)
  {
    case XK_Left: 
    case XK_KP_Left:
    case XK_a:
    case XK_q:         info_keystates_set_left(info, TRUE); break;
    case XK_Right: 
    case XK_KP_Right: 
    case XK_p:
    case XK_d:         info_keystates_set_right(info, TRUE); break;
    case XK_Up: 
    case XK_KP_Up:     info_keystates_set_up(info, TRUE); break;
    case XK_Down: 
    case XK_KP_Down:   info_keystates_set_down(info, TRUE); break;
    case XK_space:     info_keystates_set_space(info, TRUE); break;
    case XK_f:         info->show_fps = !info->show_fps; break;
  }
}


void win_on_key_release(Ez_event *ev)
{
  Info *info = ez_get_data(ev->win);

 switch(ev->key_sym)
  {
    case XK_Left: 
    case XK_KP_Left:
    case XK_a:
    case XK_q:         info_keystates_set_left(info, FALSE); break;
    case XK_Right: 
    case XK_KP_Right: 
    case XK_p:
    case XK_d:         info_keystates_set_right(info, FALSE); break;
    case XK_Up: 
    case XK_KP_Up:     info_keystates_set_up(info, FALSE); break;
    case XK_Down: 
    case XK_KP_Down:   info_keystates_set_down(info, FALSE); break;
    case XK_space:     info_keystates_set_space(info, FALSE); break;
    case XK_Escape:    quit(info); break;
  }
}


void win_on_configure(Ez_event *ev)
{
  Info *info = ez_get_data(ev->win);

  info->area_height = ev->height;
  info->area_width = ev->height/1.25;

  if(info->area_width > ev->width)
  {
    info->area_height = ev->width * 1.25;
    info->area_width = ev->width;
  }
}


/* ----------------------------- E V E N T S ------------------------------- */

void quit(Info *info)
{
  info_game_monsters_destroy(info);
  info_game_doodle_bonus_destroy(info);
  info_game_background_destroy(info);
  info_game_doodler_destroy(info);
  info_game_doodler_tirs_destroy(info);
  info_game_platforms_bonus_destroy(info);
  info_game_destroy(info);
  info_game_platforms_destroy(info);
  ez_quit();
}


void win_on_event(Ez_event *ev)
{
  Info *info = ez_get_data(ev->win);
  switch(ev->type)
  {
  case Expose: win_on_expose(ev); break;
  case ButtonPress: win_on_button_press(ev); break;
  case ButtonRelease: win_on_button_release(ev); break;
  case MotionNotify: win_on_motion(ev); break;
  case KeyPress: win_on_key_press(ev); break;
  case KeyRelease: win_on_key_release(ev); break;
  case ConfigureNotify: win_on_configure(ev); break;
  case TimerNotify: timer2_on_timeout(ez_get_data(ev->win)); break;
  case WindowClose : quit(info); break;
  }
}


int main (void)
{
  Info info;
  if (ez_init() < 0) exit(1);

  info_init_default(&info);

  info.win = ez_window_create (WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, win_on_event);
  ez_window_dbuf (info.win, 1);
  ez_set_data (info.win, &info);

  ez_start_timer(info.win, TIMER1);
  ez_auto_quit (0);
  ez_main_loop ();
  exit(0);
}


