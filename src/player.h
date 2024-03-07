#ifndef PLAYER_H
#define PLAYER_H

#include "defs.h"
#include "ui.h"

void pb_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
void update_pb(SDL_Event event, SDL_Renderer *renderer, ma_vars_t *ma_vars, mouse_t mouse);
void render_pb(SDL_Renderer *renderer, mouse_t mouse, ma_vars_t *ma_vars);

void new_sidebar_item(SDL_Renderer *renderer, ma_vars_t *ma_vars);
void open_sidebar(SDL_Renderer *renderer);
void close_sidebar(SDL_Renderer *renderer);
void update_sidebar(SDL_Renderer *renderer, mouse_t mouse);
void render_sidebar(SDL_Renderer *renderer, mouse_t mouse);

// reposition of the buttons when opening and closing the sidebar
void repos_buttons();

// initializes mp3 player variables 
void init_player(SDL_Renderer *renderer, ma_vars_t *ma_vars);
void unpause_pb(pb_state *state);
void pause_pb(pb_state *state);

void restart_pb();

bool check_file_mp3(const char* file);
bool check_file_wav(const char* file);
void play_mp3(mp3_t mp3, ma_vars_t *ma_vars);
void add_mp3_to_playlist(SDL_Renderer *renderer, ma_vars_t *ma_vars, const char *filename);
playlist_t create_playlist(const char *dir);

void print_playlist(playlist_t playlist);
void print_pb_info(pb_info pb_info);
void print_pb_state(pb_state pb_state);

#endif // PLAYER_H
