#include "ui.h"
#include "defs.h"

box_t **box_arr = NULL;
size_t box_arr_size = 0;

box_t *create_box(SDL_Renderer *renderer, 
                  SDL_FRect rect, 
                  SDL_Color rect_color, 
                  SDL_Texture *texture, 
                  const char *text, 
                  SDL_Color text_color, 
                  TTF_Font *font, 
                  u32 flags) 
{
    box_arr = realloc(box_arr, sizeof(box_t) * (box_arr_size+1));      
    box_arr[box_arr_size] = malloc(sizeof(box_t));

    box_arr[box_arr_size]->rect = rect;
    box_arr[box_arr_size]->rect_color = rect_color;

    if (texture != NULL) {
        box_arr[box_arr_size]->texture = texture;
    } else {
        box_arr[box_arr_size]->texture = NULL;
    }

    if (text != NULL) {
        strcpy(box_arr[box_arr_size]->text, text);
        box_arr[box_arr_size]->text_color = text_color;
        if (font == NULL) {
            box_arr[box_arr_size]->font = DEFAULT_FONT;
        } else {
            box_arr[box_arr_size]->font = font;
        }
        box_arr[box_arr_size]->font_texture = __new_font_texture(renderer, 
                                                                 box_arr[box_arr_size], 
                                                                 box_arr[box_arr_size]->font, 
                                                                 box_arr[box_arr_size]->text, 
                                                                 box_arr[box_arr_size]->text_color);
//        result->font_texture = create_font_texture(renderer, result->font, result->text, result->text_color);
        box_arr[box_arr_size]->state |= BOX_TEXT_VISIBLE;
    } else {
//        result->text = NULL;
        box_arr[box_arr_size]->font = NULL;
        box_arr[box_arr_size]->font_texture = NULL;
    }

    if (flags == BOX_NONE) {
        box_arr[box_arr_size]->state |= BOX_VISIBLE;
    } else {
        box_arr[box_arr_size]->state |= flags;
    }
    
    box_arr[box_arr_size]->anim_clock = (Clock) {
        .state = CLK_STOPPED,
        .start_ms = 0,
        .current_ms = 0
    };

    box_arr[box_arr_size]->id = box_arr_size;
    box_arr_size++;

    return box_arr[box_arr_size-1];
}

void update_box_arr(mouse_t mouse) {
    for (size_t i = 0; i < box_arr_size; i++) {
        if (box_arr[i]->state & BOX_VISIBLE) {
            if (check_mouse_rect_collision(mouse, box_arr[i]->rect)) {
                box_arr[i]->state |= BOX_HOVERED;
            } else {
                box_arr[i]->state &= ~BOX_HOVERED;
            }
        } else {
            box_arr[i]->state &= ~BOX_HOVERED;
        }
    } 



}

// render boxes created by func create_box()
void render_box_arr(SDL_Renderer *renderer) {
    for (size_t i = 0; i < box_arr_size; i++) {
        if (box_arr[i]->state & BOX_VISIBLE) {
            if (box_arr[i]->state & BOX_TEXT_VISIBLE) {
                SDL_RenderTexture(renderer, 
                                  box_arr[i]->font_texture, 
                                  NULL, 
                                  &box_arr[i]->rect);
            }
            if (box_arr[i]->state & BOX_BORDER) {
                SDL_SetRenderDrawColor(renderer, 
                                       box_arr[i]->rect_color.r, 
                                       box_arr[i]->rect_color.g, 
                                       box_arr[i]->rect_color.b, 
                                       box_arr[i]->rect_color.a);
                SDL_RenderRect(renderer, &box_arr[i]->rect);
            }
            if (box_arr[i]->state & BOX_COLOR_FILL) {
                SDL_SetRenderDrawColor(renderer, 
                                       box_arr[i]->rect_color.r, 
                                       box_arr[i]->rect_color.g, 
                                       box_arr[i]->rect_color.b, 
                                       box_arr[i]->rect_color.a);
                SDL_RenderFillRect(renderer, &box_arr[i]->rect);
                SDL_RenderRect(renderer, &box_arr[i]->rect);
            }
            if (box_arr[i]->texture != NULL) {
                SDL_RenderTexture(renderer, box_arr[i]->texture, NULL, &box_arr[i]->rect);
            }
            if (box_arr[i]->new_text) {
                box_arr[i]->font_texture = __new_font_texture(renderer, box_arr[i], box_arr[i]->font, box_arr[i]->text, box_arr[i]->text_color);

                box_arr[i]->new_text = false;
            }
            
        }

    }

}

// check if mouse is inside passed rect
bool check_mouse_rect_collision(mouse_t mouse, SDL_FRect rect) {
    if ((mouse.pos.x >= rect.x && mouse.pos.x <= rect.x + rect.w) &&
        (mouse.pos.y >= rect.y && mouse.pos.y <= rect.y + rect.h)) 
    {        
        return true;
    } else {
        return false;
    }

}

void mouse_update(SDL_Event event, mouse_t *mouse) {
    if (mouse->state & M_RELEASED) {
        mouse->button = 0;
    }

    mouse->state = 0;
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        SDL_GetMouseState(&mouse->pos.x, &mouse->pos.y);
        mouse->state |= M_MOVED;
    }
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            mouse->button |= MB_LEFT_CLICK;
        }
        mouse->state = M_PRESSED;
//      mouse->last_pos.x = mouse->pos.x;
//      mouse->last_pos.y = mouse->pos.y;

    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        mouse->state = M_RELEASED;
    }
}

// check if mouse was clicked
bool mouse_clicked(mouse_t mouse) {
    if (mouse.state & M_RELEASED && mouse.button & MB_LEFT_CLICK) {
        return true;
    } else {
        return false;
    }
}

SDL_Texture *create_font_texture(SDL_Renderer *renderer, TTF_Font* font, const char* text, SDL_Color text_color) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, text_color);
    if (surface == NULL) {
        fprintf(stderr, "Error creating surface from font. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        fprintf(stderr, "Error creating texture from font surface. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_DestroySurface(surface);

    return texture;
}

SDL_Texture *__new_font_texture(SDL_Renderer *renderer, box_t *box, TTF_Font *font, const char *text, SDL_Color text_color) {
    if (box->font_texture != NULL) {
        SDL_DestroyTexture(box->font_texture);
        box->font_texture = NULL;
    }

    SDL_Surface *surface = TTF_RenderText_Solid(font, text, text_color);
    if (surface == NULL) {
        fprintf(stderr, "Error creating surface from font. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        fprintf(stderr, "Error creating texture from font surface. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_DestroySurface(surface);

    box->state |= BOX_TEXT_VISIBLE;

    return texture;
}

// pass NULL as font to use DEFAULT_FONT(Sans Serif)
void _new_font_texture(SDL_Renderer *renderer, box_t *box, TTF_Font *font, const char *new_text) {
    if (box->font_texture != NULL) {
        SDL_DestroyTexture(box->font_texture);
        box->font_texture = NULL;
    }

    SDL_Surface *surface = TTF_RenderText_Solid(font, new_text, box->text_color);
    if (surface == NULL) {
        fprintf(stderr, "Error creating surface from font. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        fprintf(stderr, "Error creating texture from font surface. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_DestroySurface(surface);

}

void new_font_texture(SDL_Renderer *renderer, box_t *box, char *new_text) {
    SDL_DestroyTexture(box->font_texture);
    box->font_texture = NULL;
    box->font_texture = create_font_texture(renderer, box->font, new_text, box->text_color); 
    if (box->font_texture == NULL) {
        fprintf(stderr, "Error resetting font_texture of box: %zu. SDL_Error: %s\n", box->id, SDL_GetError());
        exit(1);
    }
}

void anim_expand_box(box_t *box) {
    static SDL_FRect og_rect = {0};

    if (box->state & BOX_HOVERED) {
        if (box->anim_clock.state == CLK_STOPPED) {
            og_rect.w = box->rect.w;
            og_rect.h = box->rect.h;
            og_rect.x = box->rect.x;
            og_rect.y = box->rect.y;

            start_clock(&box->anim_clock);
        } else if (box->anim_clock.current_ms < 250) {
            box->rect.x -= 0.15f;
            box->rect.y -= 0.15f;
            box->rect.h += 0.30f;
            box->rect.w += 0.30f;

            update_clock(&box->anim_clock); 
        }
    } else if (box->anim_clock.state == CLK_RUNNING) {
        box->rect.w = og_rect.w;
        box->rect.h = og_rect.h;
        box->rect.x = og_rect.x;
        box->rect.y = og_rect.y;
        stop_clock(&box->anim_clock);
    }

}

void update_animations(SDL_Renderer *renderer) {

}

SDL_Texture *load_svg(SDL_Renderer *renderer, const char *svg_file) {
    SDL_RWops *rw = SDL_RWFromFile(svg_file, "r");
    if (rw == NULL) {
        fprintf(stderr, "Error reading SVG file: %s\n", svg_file);
        exit(1);
    }

    SDL_Surface *surface = IMG_LoadSVG_RW(rw);
    if (surface == NULL) {
        fprintf(stderr, "Error creating surface from SVG: %s. SDL_Error: %s\n", svg_file, SDL_GetError());
        exit(1);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        fprintf(stderr, "Error creating texture from surface. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_RWclose(rw);
    SDL_DestroySurface(surface);

    printf("Texture from SVG: %s. Created succesfully\n", svg_file);

    return texture;
}

SDL_Texture *load_png(SDL_Renderer *renderer, const char *png_file) {
    SDL_Surface *surface = IMG_Load(png_file);
    if (surface == NULL) {
        fprintf(stderr, "Error creating surface from SVG: %s. SDL_Error: %s\n", png_file, SDL_GetError());
        exit(1);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        fprintf(stderr, "Error creating texture from surface. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    printf("Texture from PNG: %s. Created succesfully\n", png_file);

    SDL_DestroySurface(surface);

    return texture;
}

void print_box_info() {
    if (box_arr_size > 0) {
        printf("[BOX INFO]\n");
        for (size_t i = 0; i < box_arr_size; i++) {
            printf("Box ID: %zu\n", box_arr[i]->id);
            printf("%p\n", box_arr[i]);
            printf("Pos: X | Y\n");
            printf("%.2f | %.2f\n", box_arr[i]->rect.x, box_arr[i]->rect.y);
            if (box_arr[i]->text != NULL) {
                printf("TEXT: %s\n", box_arr[i]->text);
            }
            printf("VISIBLE: %u\n", box_arr[i]->state&BOX_VISIBLE);
            printf("TEXT VISIBLE: %u\n", box_arr[i]->state&BOX_TEXT_VISIBLE);
            printf("HOVERED: %u\n", box_arr[i]->state&BOX_HOVERED);
            printf("COLOR FILL: %u\n\n", box_arr[i]->state&BOX_COLOR_FILL);
        }
    }

}
