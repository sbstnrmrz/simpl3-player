#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include <dirent.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_ttf.h>
#include <SDL3/SDL_render.h>
#include "miniaudio.h"

#ifdef _WIN32
#define CLEAR_SCREEN system("cls")
#elif __unix__
#define CLEAR_SCREEN system("clear")
#endif

#define array_len(x) sizeof(x)/sizeof(x[0])
#define PI atan2f(1, 1) * 4
#define sgn(x) (x < 0) ? -1 : (x > 0)// ? 1 : 0
#define SAMPLE_RATE 48000
#define BUFFER_SIZE 4096
#define WIN_WIDTH   800
#define WIN_HEIGHT  600

#define RED    (SDL_Color) {255, 0, 0, 255}
#define GREEN  (SDL_Color) {0, 255, 0, 255}
#define BLUE   (SDL_Color) {0, 0, 255, 255}
#define CYAN   (SDL_Color) {0, 255, 255, 255}
#define YELLOW (SDL_Color) {255, 255, 0, 255}
#define ORANGE (SDL_Color) {255, 80, 0, 255}
#define PURPLE (SDL_Color) {255, 0, 255, 255}
#define WHITE  (SDL_Color) {255, 255, 255, 255}
#define BLACK  (SDL_Color) {0, 0, 0, 255}

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef float _Complex cmplx; 
typedef struct dirent* de;

typedef struct {
    i32 x;
    i32 y;
} vec2d_t;

typedef struct {
    f32 x;
    f32 y;
} vecf2d_t;

typedef enum {
    PB_PLAYING = 1,
    PB_PAUSED = 2,
    PB_ONCE = 4,
    PB_LOOPING = 8,
    PB_SHUFFLE = 16,
    PB_ENDED,
} pb_state;

typedef enum {
    ERROR_NOCOMMAND = -2,
    ERROR_FILETYPE = -1,
    PLAYBACK_WAV = 1,
    PLAYBACK_MP3,
    PLAYBACK_FLAC,
    PRINT_HELP,
    PRINT_VERSION
} parse_result;

typedef struct {
    char filename[128];
    char dir[128];
    char *format;
    u32 sample_rate;
    u8 channels;
    u64 frames;
} mp3_t;

typedef struct {
    const char *name;
    const char *dir;
    mp3_t *mp3_list;
    size_t mp3_list_size;
    size_t current_mp3;
} playlist_t;

typedef struct {
    mp3_t current_mp3;
    u64 cursor;
    u64 last_cursor;
} pb_info;

typedef struct {
    ma_context        context;
    ma_decoder        decoder;
    ma_decoder_config decoder_config;
    ma_device         device;
    ma_device_config  device_config;
    pb_info           pb_info;
    pb_state          pb_state;
    playlist_t        playlist;
} ma_vars_t;

#endif // DEFS_H
