#ifndef UI_H
#define UI_H

#include "defs.h"
#include "clock.h"

#define DEFAULT_FONT TTF_OpenFont("assets/fonts/FreeSans.ttf", 128)

typedef enum {
    MB_NONE         = 0,
    MB_LEFT_CLICK   = 1,
    MB_RIGHT_CLICK  = 2,
    MB_MIDDLE_CLICK = 3,
} mouse_button;

typedef enum {
    M_PRESSED = 1,
    M_RELEASED = 2,
    M_MOVED,
} mouse_state;

typedef struct {
    vecf2d_t     pos;
    mouse_button button;
    mouse_state  state;
} mouse_t;

typedef enum {
    BOX_NONE = 0,
    BOX_HOVERED = 1,
    BOX_VISIBLE = 2,
    BOX_TEXT_VISIBLE = 4,
    BOX_COLOR_FILL = 8,
    BOX_BORDER = 16,
} box_state;

typedef struct {
    SDL_FRect    rect;
    SDL_Color    rect_color;
    SDL_Texture *texture;
    SDL_FRect    text_rect;
    SDL_Color    text_color;
    char        *text;
    TTF_Font    *font;
    SDL_Texture *font_texture;
    Clock        anim_clock;
    box_state    state;
    size_t       id;
} box_t;

typedef enum {
    BUTTON_PLAY = 0,
    BUTTON_PAUSE = 1,
    BUTTON_NEXT_SONG = 2,
    BUTTON_PREV_SONG = 3,
    BUTTON_SLIDER = 4,
    BUTTON_ONCE = 5,
    BUTTON_LOOP = 6,
    BUTTON_SHUFFLE = 7,
    BUTTON_SIDEBAR = 8,
} button_id;

box_t *create_box(SDL_Renderer *renderer, 
                  SDL_FRect rect, 
                  SDL_Color rect_color, 
                  SDL_Texture *texture,
                  const char *text, 
                  SDL_Color text_color, 
                  TTF_Font *font, 
                  u32 flags);
void update_box_arr(mouse_t mouse);
void render_box_arr(SDL_Renderer *renderer);
void new_font_texture(SDL_Renderer *renderer, box_t *box, char *new_text);
SDL_Texture *create_font_texture(SDL_Renderer *renderer, TTF_Font* font, const char* text, SDL_Color text_color);
bool check_mouse_rect_collision(mouse_t mouse, SDL_FRect rect);
void mouse_update(SDL_Event event, mouse_t *mouse);
bool mouse_clicked(mouse_t mouse);
SDL_Texture *load_svg(SDL_Renderer *renderer, const char* svg_file);

void update_animations(SDL_Renderer *renderer);
void anim_expand_box(box_t *box);

void print_box_info();

#endif // UI_H
