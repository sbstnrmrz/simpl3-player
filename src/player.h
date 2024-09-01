#ifndef PLAYER_H
#define PLAYER_H

#include "defs.h"
#include "ui.h"

void pb_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
void update_pb(SDL_Event event, SDL_Renderer *renderer, ma_vars_t *ma_vars, mouse_t mouse);
void render_pb(SDL_Renderer *renderer, mouse_t mouse, ma_vars_t *ma_vars);

void new_sidebar_item(SDL_Renderer *renderer, ma_vars_t *ma_vars, mp3_t mp3);
void open_sidebar();
void close_sidebar();
void update_sidebar(SDL_Renderer *renderer, mouse_t mouse);
void open_volume();
void close_volume();

// reposition of the buttons when opening and closing the sidebar
void repos_buttons();

// initializes mp3 player variables 
void init_player(SDL_Renderer *renderer, ma_vars_t *ma_vars);
void unpause_pb(pb_state *state);
void pause_pb(pb_state *state);

void restart_pb();

bool check_file_mp3(const char* file);
bool check_file_wav(const char* file);
bool check_directory(const char *directory);
mp3_t new_mp3(const char *mp3_file);
void play_mp3(mp3_t mp3, ma_vars_t *ma_vars);
void next_song(ma_vars_t *ma_vars);
void prev_song(ma_vars_t *ma_vars);
void add_mp3_to_playlist(SDL_Renderer *renderer, ma_vars_t *ma_vars, const mp3_t mp3);
playlist_t create_playlist(SDL_Renderer *renderer, ma_vars_t *ma_vars, const char *dir_name);

void print_playlist(playlist_t playlist);
void print_pb_info(pb_info_t pb_info);
void print_pb_state(pb_state pb_state);
void print_playlist_info(playlist_t playlist);

#endif // PLAYER_H
