#ifndef PLAYER_H
#define PLAYER_H

#include "defs.h"
#include "ui.h"

typedef enum {
    SINE,
    SQUARE,
    SAWTOOTH,
    TRIANGLE
} waveType;

typedef struct {
    char name[100];
//  u32 duration;
//  u64 frames; 
} mp3_t;

typedef struct {
    const char *name;
    const char *dir;
    mp3_t *mp3_list;
    size_t mp3_list_size;
    size_t current_mp3;
} playlist_t;

// Audio
void rec_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
void pb_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
void dft(float *in, cmplx *out, size_t n);
void fft(float in[], size_t stride, cmplx out[], size_t n);
i32 pb_input(SDL_Event event, ma_vars_t *ma_vars);
int remove_fft_zeros(cmplx *in, cmplx *out, size_t fft_size, size_t *new_fft_size);
f32 get_cmplx_frame_amp(cmplx frame, bool abs);
f32 get_zero_crossings(f32 samples[], size_t samples_size);
f32 max_abs_scale(f32 *x, size_t n);
i32 max(i32 x, i32 y);
f32 maxf(f32 x, f32 y);
i32 max_arr(i32 *arr, size_t n);
f32 maxf_arr(f32 *arr, size_t n);
f32 *auto_correlate(f32 *frames, size_t n);
f32 sine_wave(f32 phase);
f32 square_wave(f32 phase);
f32 sawtooth_wave(f32 phase);
f32 triangle_wave(f32 phase);
void draw_wave(SDL_Renderer *renderer);
void update_pb(ma_vars_t *ma_vars);
void render_pb(SDL_Renderer *renderer, mouse_t mouse, ma_vars_t *ma_vars);
void play_mp3(mp3_t mp3, ma_vars_t *ma_vars);
void print_playlist(playlist_t playlist);
void print_pb_info(pb_info pb_info);
void print_pb_state(pb_state pb_state);
void print_frame(f32 frame, size_t itr);
void print_frames(f32 frames[], size_t framesSize);
void print_fft_frame(cmplx fftFrame, size_t itr);
void print_fft_frames(cmplx fftFrames[], size_t framesSize);

void restart_pb();

bool check_file_mp3(const char* file);
bool check_file_wav(const char* file);
playlist_t create_playlist(const char *dir);

#endif // PLAYER_H
