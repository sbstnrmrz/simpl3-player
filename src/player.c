#include "player.h"
#include "defs.h"
#include "ui.h"
#include <dirent.h>
#include <string.h>

//tsoding video fft. min: 1:39:31
f32   *inSamples = NULL;
f32   *out = NULL;
f32   *in = NULL;
cmplx *fft_out = NULL;
cmplx *fft_out_woz = NULL;
size_t frames_size = 0;
SDL_Texture *button_textures[6] = {0};
SDL_FRect *button_rects[6] = {0};

box_t *prev_song_button = NULL;
box_t *next_song_button = NULL;
box_t *progress_bar = NULL;
box_t *play_button = NULL;
box_t *loop_button = NULL;
box_t *slider = NULL;

f32 progress_bar_h = 20;
f32 progress_bar_w = 300;

playlist_t test_pl = {0};

ma_result err;

void pb_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_vars_t *ma_vars = (ma_vars_t*)pDevice->pUserData;
    if (ma_vars->pb_state & PB_PLAYING) {
        err = ma_decoder_read_pcm_frames(&ma_vars->decoder, pOutput, frameCount, NULL);

        if (err == MA_AT_END) {
            if (ma_vars->pb_state & PB_LOOPING) {
                ma_decoder_seek_to_pcm_frame(&ma_vars->decoder, 0);
            } else {
                printf("Playback has finished!\n"); 
                exit(1);
            }
        }
        else if (err != MA_SUCCESS) {
            fprintf(stderr, "An error has ocurred. ma_result: %d\n", err);
            exit(1);
        }

//      out = (f32*)pOutput;
//      fft_out = (cmplx*)malloc(sizeof(cmplx) * frameCount);
//      fft(out, 1, fft_out, frameCount);
    } else if (ma_vars->pb_state & PB_PAUSED) {

    }
    ma_decoder_get_cursor_in_pcm_frames(&ma_vars->decoder, &ma_vars->pb_info.cursor);
    frames_size = frameCount;

    (void)pInput;
}

void rec_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_vars_t *ma_vars = (ma_vars_t*)pDevice->pUserData;
    if (ma_vars->rec_state == REC_RECORDING) {
        ma_encoder_write_pcm_frames(&ma_vars->encoder , pInput, frameCount, NULL);
        in = (f32*)pInput;
        fft_out = (cmplx*)malloc(sizeof(cmplx) * frameCount);
        fft(in, 1, fft_out, frameCount);
    }
    frames_size = frameCount;

    (void)pOutput;
}

void dft(float *in, cmplx *out, size_t n) {
    for (size_t f = 0; f < n; f++) {
        out[f] = 0;
        for (size_t i = 0; i < n; i++) {
            f32 t = (f32)i/n;
            out[f] += in[i] * cexp(I * 2.0f * PI * f * t);
        }
    }

}

void fft(float in[], size_t stride, cmplx out[], size_t n) {
    if (n == 1) {
        out[0] = in[0];
        return;
    }

    fft(in, stride*2, out, n/2);
    fft(in + stride, stride*2,  out + n/2, n/2);

    for (size_t k = 0; k < n/2; ++k) {
        float t = (float)k/n;
        cmplx v = cexp(-2*I*PI*t) * out[k + n/2];
        cmplx e = out[k];
        out[k]       = e + v; 
        out[k + n/2] = e - v;
    }
/*
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n/2; j++)) {
            float t = (float)k/n;
            cmplx v = cexp(-2*I*PI*t) * out[k + n/2];
            cmplx e = out[k];
            out[k]       = e + v; 
            out[k + n/2] = e - v;
        }

    }  
    out[0] = in[0];
*/
}

int remove_fft_zeros(cmplx *in, cmplx *out, size_t fft_size, size_t *new_fft_size) {
    size_t size = 0;
    out = 0;
    for (size_t i = 0; i < fft_size; i++) {
        if (get_cmplx_frame_amp(fft_out[i], true) <= 0) {
            continue; 
        } 
        out = realloc(out, (size+1) * sizeof(cmplx));
        out[size] = in[i];
        size++;
    }
    *new_fft_size = size;

    return size;
}

f32 get_cmplx_frame_amp(cmplx frame, bool abs) {
    f32 a = 0;
    f32 b = 0;

    if (abs) {
        a = fabsf(crealf(frame));
        b = fabsf(cimagf(frame));
    } else {
        a = crealf(frame);
        b = cimagf(frame);
    }

    return a > b ? a : b;
}

f32 get_zero_crossings(f32 samples[], size_t samples_size) {
    f32 result = 0;
    for (size_t i = 1; i < samples_size; i++) {
        if (samples[i] * samples[i-1] <= 0) {
            result++;
        }
    } 
    return result;
}

f32 maxf_arr(f32 *arr, size_t n) {
    f32 result = 0;
    for (size_t i = 0; i < n; i++) {
        if (arr[i] > result) {
            result = arr[i];
        }
    }

    return result;
}

f32 max_abs_scale(f32 *x, size_t n) {
    f32 result = maxf_arr(x, n);
    return result / fabs(result);
}

f32* auto_correlate(f32 *samples, size_t n) {
    f32 *result = (f32*)malloc(n * sizeof(f32));
    for (size_t i = 0; i < n; i++) {
        f32 sum = 0.0f;
        for (size_t j = 0; j < n-i-1; j++) {
            sum += (samples[j] * samples[j + i]); 
        }
        result[i] = sum;
    }

    return result;
}


f32 sine_wave(f32 phase) {
    return (f32)sin(2.0f * PI * phase);
}

f32 square_wave(f32 phase) {
    return (f32)sgn(sine_wave(phase)); 
}

f32 triangle_wave(f32 phase) {
    return (4.0f * fabs(phase - floorf(phase + 1.0f/2.0f)) - 1.0f);
}

f32 sawtooth_wave(f32 phase) {
    f32 freq = phase * SAMPLE_RATE;
    f32 t = phase / freq; 
    return (2.0f * fmod(t, 1.0f/freq) * freq - 1.0f); 
}

i32 pb_input(SDL_Event event, ma_vars_t *ma_vars) {
    f32 sec = ((f32)ma_vars->pb_info.cursor/SAMPLE_RATE);
    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.keysym.sym == SDLK_RIGHT) {
            ma_vars->pb_info.last_cursor = ma_vars->pb_info.cursor;
            printf("last: %llu\n", ma_vars->pb_info.last_cursor);
        }
        if (event.key.keysym.sym == SDLK_LEFT) {
            ma_vars->pb_info.last_cursor = ma_vars->pb_info.cursor;
        }

    } else if (event.type == SDL_EVENT_KEY_UP) {


    }

    return 0;
}

void init_ui(SDL_Renderer *renderer) {
    button_textures[BUTTON_PLAY] = IMG_LoadTexture(renderer, "assets/buttons/play.png");
    button_textures[BUTTON_PAUSE] = IMG_LoadTexture(renderer, "assets/buttons/pause.png");
    button_textures[BUTTON_NEXT_SONG] = IMG_LoadTexture(renderer, "assets/buttons/next.png");
    button_textures[BUTTON_PREV_SONG] = IMG_LoadTexture(renderer, "assets/buttons/prev.png");
    button_textures[BUTTON_SLIDER] = IMG_LoadTexture(renderer, "assets/buttons/slider.png");
    button_textures[BUTTON_LOOP] = IMG_LoadTexture(renderer, "assets/buttons/loop.png");

    progress_bar = create_box(renderer, 
                              (SDL_FRect) {
                                    .x = ((f32)WIN_WIDTH/2) - progress_bar_w/2,
                                    .y = ((f32)WIN_HEIGHT/2),
                                    .w = progress_bar_w,
                                    .h = 10, 
                              },
                              WHITE,
                              NULL,
                              NULL,
                              WHITE,
                              NULL,
                              BOX_VISIBLE | BOX_COLOR_FILL
                              ); 

    play_button = create_box(renderer, 
                              (SDL_FRect) {
                                .w = 32,
                                .h = 32,
                                .x = ((f32)WIN_WIDTH/2) - 16,
                                .y = (f32)WIN_HEIGHT/2 + 32,
                              },
                              WHITE,
                              button_textures[BUTTON_PLAY],
                              NULL,
                              WHITE,
                              NULL,
                              BOX_VISIBLE
                              ); 

    slider = create_box(renderer, 
                              (SDL_FRect) {
                                  .w = 32,
                                  .h = 32,
                                  .x = 0,
                                  .y = 0,
                              },
                              WHITE,
                              button_textures[BUTTON_SLIDER],
                              NULL,
                              WHITE,
                              NULL,
                              BOX_VISIBLE
                              ); 

    prev_song_button = create_box(renderer, 
                              (SDL_FRect) {
                                .x = play_button->rect.x - play_button->rect.w*2,
                                .y = (f32)WIN_HEIGHT/2 + 32,
                                .w = 32,
                                .h = 32,
                              },
                              WHITE,
                              button_textures[BUTTON_PREV_SONG],
                              NULL,
                              WHITE,
                              NULL,
                              BOX_VISIBLE
                              ); 
    next_song_button = create_box(renderer, 
                              (SDL_FRect) {
                                .x = play_button->rect.x+play_button->rect.w + 32,
                                .y = (f32)WIN_HEIGHT/2 + 32,
                                .w = 32,
                                .h = 32,
                              },
                              WHITE,
                              button_textures[BUTTON_NEXT_SONG],
                              NULL,
                              WHITE,
                              NULL,
                              BOX_VISIBLE
                              ); 

    loop_button = create_box(renderer, 
                              (SDL_FRect) {
                                .x = ((f32)WIN_WIDTH/2) - 16,
                                .y = (f32)WIN_HEIGHT/2 + 96,
                                .w = 32,
                                .h = 32,
                              },
                              WHITE,
                              button_textures[BUTTON_LOOP],
                              NULL,
                              WHITE,
                              NULL,
                              BOX_VISIBLE
                              ); 

    test_pl = create_playlist("assets/music/"); 
    print_playlist(test_pl);
}

void update_pb(ma_vars_t *ma_vars) {

}

void render_pb(SDL_Renderer *renderer, Mouse mouse, ma_vars_t *ma_vars) { 

    slider->rect.x = (progress_bar->rect.x + ((f32)progress_bar_w/ma_vars->pb_info.total_frames) * ma_vars->pb_info.cursor) - 16;
    slider->rect.y = progress_bar->rect.y - 10;
    if (progress_bar->state & BOX_HOVERED) {
        if (mouse_clicked(mouse)) {
            ma_vars->pb_state &= ~PB_PLAYING;
            ma_vars->pb_state |= PB_PAUSED;
            u64 pos = ((ma_vars->pb_info.total_frames * (mouse.pos.x - progress_bar->rect.x) )/progress_bar->rect.w);
            ma_decoder_seek_to_pcm_frame(&ma_vars->decoder, pos); 
            ma_vars->pb_state &= ~PB_PAUSED;
            ma_vars->pb_state |= PB_PLAYING;
        } 
    }

    if (slider->state & BOX_HOVERED) {
        SDL_SetTextureColorMod(button_textures[BUTTON_SLIDER], 150, 150, 150);
    } else {
        SDL_SetTextureColorMod(button_textures[BUTTON_SLIDER], 255, 255, 255);
    }

    if (next_song_button->state & BOX_HOVERED) {
        if (mouse_clicked(mouse)) {    
            if (test_pl.current_mp3 < 4) {
                test_pl.current_mp3++;
            } else {
                test_pl.current_mp3 = 0;
            }
            play_mp3(test_pl.mp3_list[test_pl.current_mp3], ma_vars);
        }


        SDL_SetTextureColorMod(button_textures[BUTTON_NEXT_SONG], 150, 150, 150);
    } else {
        SDL_SetTextureColorMod(button_textures[BUTTON_NEXT_SONG], 255, 255, 255);
    }
    if (prev_song_button->state & BOX_HOVERED) {
        if (mouse_clicked(mouse)) {    
            if (test_pl.current_mp3 > 0) {
                test_pl.current_mp3--;
            } else {
                test_pl.current_mp3 = test_pl.mp3_list_size-1;
            }
            play_mp3(test_pl.mp3_list[test_pl.current_mp3], ma_vars);
        }
        SDL_SetTextureColorMod(button_textures[BUTTON_PREV_SONG], 150, 150, 150);
    } else {
        SDL_SetTextureColorMod(button_textures[BUTTON_PREV_SONG], 255, 255, 255);
    }

    if (loop_button->state & BOX_HOVERED) {
        if (mouse_clicked(mouse)) {
            if (ma_vars->pb_state & PB_LOOPING) {
                ma_vars->pb_state &= ~PB_LOOPING;
            } else {
                ma_vars->pb_state |= PB_LOOPING;
            }
        }
        SDL_SetTextureColorMod(button_textures[BUTTON_LOOP], 150, 150, 150);
    } else {
        SDL_SetTextureColorMod(button_textures[BUTTON_LOOP], 255, 255, 255);
    }

    if (ma_vars->pb_state & PB_PLAYING) {
        play_button->texture = button_textures[BUTTON_PAUSE];
    }
    if (ma_vars->pb_state & PB_PAUSED) {
        play_button->texture = button_textures[BUTTON_PLAY];
    }

    if (play_button->state & BOX_HOVERED) {
        if (mouse_clicked(mouse)) {
            if (ma_vars->pb_state & PB_PLAYING) {
                ma_vars->pb_state &= ~PB_PLAYING;
                ma_vars->pb_state |= PB_PAUSED; 
            } else {
                ma_vars->pb_state &= ~PB_PAUSED;
                ma_vars->pb_state |= PB_PLAYING;
            }
        }
        SDL_SetTextureColorMod(play_button->texture, 150, 150, 150);
    } else {
        SDL_SetTextureColorMod(play_button->texture, 255, 255, 255);
    }

}

void draw_wave(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    f32 max_amp = 0;
    f32 min_amp = INT32_MAX;
    
    for (size_t i = 0; i < frames_size; i++) {
       if (get_cmplx_frame_amp(fft_out[i], true) > max_amp) {
           max_amp = get_cmplx_frame_amp(fft_out[i], true); 
       } 
       if (get_cmplx_frame_amp(fft_out[i], true) < min_amp) {
           min_amp = get_cmplx_frame_amp(fft_out[i], true); 
       } 
    }

    size_t fft_woz_size = 0;
    for (size_t i = 0; i < frames_size; i++) {
        if (get_cmplx_frame_amp(fft_out[i], true) <= 0) {
            continue; 
        } 
        fft_out_woz = realloc(fft_out_woz, (fft_woz_size+1) * sizeof(cmplx));
        fft_out_woz[fft_woz_size] = fft_out[i];
        fft_woz_size++;
    }
//  remove_fft_zeros(fft_out, fft_out_woz, frames_size, &fft_woz_size);

    size_t N = fft_woz_size/2;//frames_size;
    f32 cell_w = (f32)WIN_WIDTH / N; 
    for (size_t i = 0; i < N; i++) {
        f32 t = get_cmplx_frame_amp(fft_out_woz[i], 1);
        SDL_FRect rect = {
            .x = i * cell_w,
            .y = WIN_HEIGHT - (WIN_HEIGHT * t),
            .w = cell_w,
            .h = (WIN_HEIGHT) * t  
        };
        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderRect(renderer, &rect); 
    }

//  f32* cor = auto_correlate(in, frames_size);
//  f32 cell_w = (f32)WIN_WIDTH / frames_size;
//  for (size_t i = 0; i < frames_size; i++) {
//      f32 t = cor[i];
//      SDL_FRect rect = {
//          .x = i * cell_w,
//          .y = WIN_HEIGHT/2,
//          .w = cell_w,
//          .h = (WIN_HEIGHT) * t  
//      };
//      SDL_RenderFillRect(renderer, &rect);
//      SDL_RenderRect(renderer, &rect); 
//  }

//    f32 crossings = get_zero_crossings(cor, frames_size)-1;
//  f32 cycles = crossings / 2.0f;
//  f32 t = (f32)frames_size/SAMPLE_RATE;
//  printf("freq?: %f\n", cycles / t);
    
}

void play_mp3(mp3_t mp3, ma_vars_t *ma_vars) {
    ma_result err;
    ma_vars->pb_state &= ~PB_PLAYING;
    ma_vars->pb_state |= PB_PAUSED;

    err = ma_decoder_init_file(mp3.name, &ma_vars->decoderConfig, &ma_vars->decoder); 

    if (err != MA_SUCCESS) {
        fprintf(stderr, "Failed to open '%s' mp3 file. MA_ERROR: %d\n", mp3.name, err);
        exit(1);
    }

    ma_vars->deviceConfig.playback.format   = ma_vars->decoder.outputFormat;
    ma_vars->deviceConfig.playback.channels = ma_vars->decoder.outputChannels;
    ma_vars->deviceConfig.sampleRate        = ma_vars->decoder.outputSampleRate;
    ma_decoder_get_length_in_pcm_frames(&ma_vars->decoder, &ma_vars->pb_info.total_frames);
    ma_vars->pb_info.sample_rate = ma_vars->decoder.outputSampleRate;
    ma_vars->pb_info.channels = ma_vars->decoder.outputChannels;
    ma_vars->pb_info.last_cursor = 0;

//  if (ma_vars->decoder.outputFormat == ma_format_f32) {
//      ma_vars->pb_info.format = "float32";
//  }

    ma_vars->pb_state &= ~PB_PAUSED;
    ma_vars->pb_state |= PB_PLAYING;
}



bool check_file_mp3(const char *file) {
    size_t len = strlen(file);
    if (!strcasecmp(file+(len-3), "mp3")) {
        return true;
    }

    return false;
}

bool check_file_wav(const char *file) {
    size_t len = strlen(file);
    if (!strcasecmp(file+(len-3), "wav")) {
        return true;
    }

    return false;
}

playlist_t create_playlist(const char *dir_name) {
    playlist_t result = {
        .name = "playlist",
        .dir = dir_name,
        .mp3_list = NULL,
        .mp3_list_size = 0,
        .current_mp3 = 0,
    };
 
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        fprintf(stderr, "Could not open directory\n");
    }

    de de = {0};
    
    de = readdir(dir);
    while (de != NULL) {
        if (de->d_type == DT_REG) {
            if (check_file_mp3(de->d_name)) {
                char str1[100] = {0};
                strcpy(str1, dir_name);
                printf("str1: %s\n", str1);
                char str2[100] = {0};
                strcpy(str2, de->d_name);
                printf("str2: %s\n", str2);
                char str3[100] = {0};
                strcpy(str3, strcat(str1, str2));
                printf("str3: %s\n", str3);

                result.mp3_list = realloc(result.mp3_list, sizeof(mp3_t) * (result.mp3_list_size+1));
                strcpy(result.mp3_list[result.mp3_list_size].name, str3); 
                printf("result %zu: %s\n", result.mp3_list_size, result.mp3_list[result.mp3_list_size].name);
                result.mp3_list_size++;
            }
        }
        de = readdir(dir);
    }
    closedir(dir);
    printf("MEMORY: %p\n", result.mp3_list);

    return result;
}

void print_playlist(playlist_t playlist) {
    printf("[Playlist: %s]\n", playlist.name);
    for (size_t i = 0; i < playlist.mp3_list_size; i++) {
        printf("File %zu: %s\n", i, playlist.mp3_list[i].name);
    }
}

void print_frame(f32 frame, size_t itr) {
    printf("frame[L, %zu]: %f\n", itr, frame);
}

void print_frames(f32 frames[], size_t framesSize) {
    for (size_t i = 0; i < framesSize; i++) {
        printf("frame[L, %zu]: %f\n", i, frames[i]);
    }
}

void print_fft_frame(cmplx frame, size_t itr) {
    printf("frame[L, %zu]: REAL: %f | IMAG: %f\n", itr, crealf(frame), cimagf(frame));
}

void print_fft_frames(cmplx frames[], size_t framesSize) {
    for (size_t i = 0; i < framesSize; i++) {
        printf("frame[L, %zu]: REAL: %f, IMAG: %f\n", i, crealf(fft_out[i]), cimagf(fft_out[i]));
    }
}



void print_pb_info(pb_info pb_info) {
    u32 frames_to_sec = pb_info.cursor / SAMPLE_RATE;
    u32 sec =  frames_to_sec % 60;
    u32 min = frames_to_sec / 60;
    u32 hr = frames_to_sec / 3600;

    printf("[PLAYBACK INFO]\n");
    printf("  Format: %s\n", pb_info.format);
    printf("  Sample rate: %uhz\n", pb_info.sample_rate);
    printf("  Channels: %u\n", pb_info.channels);
    printf("  Duration: %f seconds\n", (f32)pb_info.total_frames/pb_info.sample_rate);
    printf("  Frames cursor: %llu\n", pb_info.cursor);
    printf("  Last frames cursor: %llu\n", pb_info.last_cursor);
    printf("  Progress: %02u:%02u:%02u\n", hr, min, sec);

    printf("\n");
}

void print_pb_state(pb_state pb_state) {
    printf("[PLAYBACK STATE]\n");
    printf("  Playing: %d\n", pb_state&PB_PLAYING);
    printf("  Paused: %d\n", pb_state&PB_PAUSED);
    printf("  Looping: %d\n", pb_state & (PB_LOOPING));

    printf("\n");
}
