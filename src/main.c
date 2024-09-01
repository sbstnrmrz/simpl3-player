#define MINIAUDIO_IMPLEMENTATION
#include "defs.h"
#include "ui.h"
#include "player.h"

#define VERSION "1.0"

struct {
    bool          running;
    mouse_t       mouse;
    SDL_Window   *window;
    SDL_Renderer *renderer;
} engine;

ma_vars_t ma_vars = {0};

void print_args(int argc, char *argv[]) {
    printf("argc: %d\n", argc);
    for (i32 i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    printf("\n");

}

void print_pb_file_info(const char *filename, u64 total_frames, u8 channels, u32 sample_rate) {
    char *str = channels < 2 ? "Mono" : "Stereo";
    printf("Playback file info:\n");
    printf("  File: %s\n", filename);
    printf("  %s %d channels\n", str, channels);
    printf("  Sample rate: %dhz\n", sample_rate);
    printf("  Frames: %llu\n", total_frames);
    printf("  Duration: %f seconds\n\n", (f32)total_frames/sample_rate);

}

int parse_args(int argc, char *argv[]) {
    if (argc < 2) {
        return NO_FILE;
    } else if (argc > 2) {
        fprintf(stderr, "Only two commands can be used\n");
        exit(1);
    }

    size_t len = strlen(argv[1]);
    if (check_directory(argv[1]) && *(argv[1]+len-1) != '/') {
        argv[1] = strcat(argv[1], "/"); 
        return PLAYBACK_MP3;
    }
 
    return ERROR_NO_ARGS;
}

void print_devices(ma_context *context) {
    ma_device_info *output_devs = NULL;
    u32 output_devs_count = 0;
    ma_device_info *input_devs = NULL;
    u32 input_devs_count = 0;

    if (ma_context_get_devices(context, &output_devs, &output_devs_count, &input_devs, &input_devs_count)) {
        fprintf(stderr, "Failed to get devices.\n");
        exit(1);
    }
    
    printf("Output devices:\n");
    for (size_t i = 0; i < output_devs_count; i++) {
        printf("%zu) Name: %s ", i, output_devs[i].name);
        if (output_devs[i].isDefault) {
            printf("(Default)");
        }
        printf("\n");
    }
    printf("\nInput devices:\n");
    for (size_t i = 0; i < input_devs_count; i++) {
        printf("%zu) Name: %s ", i, input_devs[i].name);
        if (input_devs[i].isDefault) {
            printf("(Default)");
        }
        printf("\n");
    }

}

void init_sdl(const char *title, int win_width, int win_height, u32 flags) {
    engine.running = false;

    if (SDL_Init(SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "Failed to initialize SDL. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    printf("SDL initialized\n");

    engine.window = SDL_CreateWindow(title, win_width, win_height, 0);
    if (engine.window == NULL) {
        fprintf(stderr, "Failed to create SDL_Window. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    printf("SDL_Window created\n");

    engine.renderer = SDL_CreateRenderer(engine.window, NULL, SDL_RENDERER_PRESENTVSYNC);
    if (engine.renderer == NULL) {
        fprintf(stderr, "Failed to create SDL_Renderer. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    printf("SDL_Renderer created\n");

    if (TTF_Init() < 0) {
        fprintf(stderr, "Failed to initialize TTF. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    printf("TTF initialized\n");
    engine.running = true;

}

void update() {
    SDL_Event event;
    SDL_PollEvent(&event);

    if (event.type == SDL_EVENT_QUIT) {
        engine.running = false;
    }
    mouse_update(event, &engine.mouse);

    update_box_arr(engine.mouse);
    update_sidebar(engine.renderer, engine.mouse);
    update_pb(event, engine.renderer, &ma_vars, engine.mouse);
}

void render() {
    SDL_SetRenderDrawColor(engine.renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderClear(engine.renderer);

    render_box_arr(engine.renderer);
    render_pb(engine.renderer, engine.mouse, &ma_vars);

    SDL_RenderPresent(engine.renderer);

}

void uninit_sdl() {
    SDL_DestroyRenderer(engine.renderer);
    SDL_DestroyWindow(engine.window); 
    SDL_Quit();

}

void uninit_ma() {
    ma_device_uninit(&ma_vars.device);
    ma_context_uninit(&ma_vars.context);
    ma_decoder_uninit(&ma_vars.decoder);
}

void debug() {
//  print_mouse_info(engine.mouse);
//  print_playlist_info(ma_vars.playlist);
//  print_pb_info(ma_vars.pb_info);
//  print_pb_state(ma_vars.pb_state);

}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    print_args(argc, argv);
    init_sdl("siMPl3 PLAYER", WIN_WIDTH, WIN_HEIGHT, 0);
    int parse = parse_args(argc, argv); 


    printf("parse_result: %d\n\n", parse);

    init_player(engine.renderer, &ma_vars);

    ma_vars.decoder_config             = ma_decoder_config_init(ma_format_f32, 2, 48000); 
    ma_vars.device_config              = ma_device_config_init(ma_device_type_playback);
    ma_vars.device_config.dataCallback = pb_callback;
    ma_vars.device_config.pUserData    = &ma_vars;
    ma_vars.device_config.sampleRate   = SAMPLE_RATE;

    if (argc > 1) {
        if (check_directory(argv[1])) {
            ma_vars.pb_info.playlist = create_playlist(engine.renderer, &ma_vars, argv[1]);
            play_mp3(ma_vars.pb_info.playlist.curr_mp3, &ma_vars);
        } else if (check_file_mp3(argv[1])) {
            add_mp3_to_playlist(engine.renderer, &ma_vars, new_mp3(argv[1]));
            play_mp3(ma_vars.pb_info.playlist.curr_mp3, &ma_vars);
        }
    } else {
        printf("NO COMMAND ARG\n");
        ma_vars.pb_info.playlist = (playlist_t) {
            .name = "Playlist 1",
            .dir = NULL,
            .mp3_list_size = 0,
            .mp3_list = NULL,
            .curr_mp3 = 0,
        };
        ma_vars.pb_info.state |= PB_PAUSED;
    }

    if (ma_context_init(NULL, 0, NULL, &ma_vars.context) != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize miniaudio context!\n");
        exit(1);
    }
    printf("Context initialized\n");

    if (ma_device_init(NULL, &ma_vars.device_config, &ma_vars.device) != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize device!\n");
        exit(1);
    }
    ma_device_set_master_volume(&ma_vars.device, 1);
    printf("Device initialized\n");

    if (ma_device_start(&ma_vars.device) != MA_SUCCESS) {
        fprintf(stderr, "Failed to start device\n");
        exit(1);
    }
    printf("Device started\n");

	while(engine.running) {
        update();
        render();
        debug();
	}

    uninit_sdl();    
    uninit_ma();

    return 0;
}
