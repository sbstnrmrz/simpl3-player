#ifndef UI_H
#define UI_H

#include "defs.h"
#include "clock.h"

#define DEFAULT_FONT TTF_OpenFont("UbuntuMono Nerd Font.ttf", 48)

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

box_t *create_box(SDL_Renderer *renderer, 
                  SDL_FRect rect, 
                  SDL_Color rect_color, 
                  SDL_Texture *texture,
                  const char *text, 
                  SDL_Color text_color, 
                  TTF_Font *font, 
                  u32 flags);
void init_ui(SDL_Renderer *renderer);
void update_box_arr(mouse_t mouse);
void render_box_arr(SDL_Renderer *renderer);
void new_font_texture(SDL_Renderer *renderer, box_t *box, char *new_text);
SDL_Texture *create_font_texture(SDL_Renderer *renderer, TTF_Font* font, const char* text, SDL_Color text_color);
bool check_mouse_rect_collision(mouse_t mouse, SDL_FRect rect);
void mouse_update(SDL_Event event, mouse_t *mouse);
bool mouse_clicked(mouse_t mouse);
SDL_Texture *load_svg(SDL_Renderer *renderer, const char* svg_file);
SDL_Texture *load_png(SDL_Renderer *renderer, const char *png_file);

void update_animations(SDL_Renderer *renderer);
void anim_expand_box(box_t *box);

void print_box_info();

#endif // UI_H
