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

Clock pressClock = {
    .start_ms = 0,
    .current_ms = 0,
    .state = CLK_PAUSED
};
ma_vars_t ma_vars = {0};

void print_args(int argc, char *argv[]) {
    printf("argc: %d\n", argc);
    for (i32 i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    printf("\n");

}

void print_help() {
    printf("-p [file]\tStarts the program with miniaudio playback mode. Formats supported: WAV, MP3 and FLAC\n");
    printf("-r [file]\tStarts the program with miniaudio capture mode, saves the record with the name specified in [file], if name is not specified, the file will have a default name. Formats supported: WAV\n");

}

void print_version() {
    printf("Version: %s\n", VERSION);

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
        return ERROR_NOCOMMAND;
    } else if (argc > 3) {
        fprintf(stderr, "Only two commands can be used\n");
        exit(1);
    }

    if (*argv[1] == '-') {
        if (*(argv[1]+1) == 'p') {
            char *file = strchr(argv[2], '.');
            if (file == NULL) {
                return PLAYBACK_MP3;
            }
            while(file != NULL) {
//              if (strcasecmp(file+1, "wav") || strcasecmp(file+1, "mp3") || strcasecmp(file+1, "flac")) {
//                  break;
//              }
                if (strcasecmp(file+1, "wav")) {
                    return PLAYBACK_WAV;
                }
                if (strcasecmp(file+1, "mp3")) {
                    return PLAYBACK_MP3;
                }
                if (strcasecmp(file+1, "flac")) {
                    return PLAYBACK_FLAC;
                }
                file = strchr(argv[2]+1, '.');
            } 
            return ERROR_FILETYPE; 
        }
        if (*(argv[1]+1) == 'h') {
            return PRINT_HELP;
        }

    }
 
    return ERROR_NOCOMMAND;
}

void print_parse_info(parse_result result) {


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

    engine.window = SDL_CreateWindow(title, win_width, win_height, 0);
    if (engine.window == NULL) {
        fprintf(stderr, "Failed to create SDL_Window. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    engine.renderer = SDL_CreateRenderer(engine.window, NULL, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (engine.renderer == NULL) {
        fprintf(stderr, "Failed to create SDL_Renderer. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    if (TTF_Init() < 0) {
        fprintf(stderr, "Failed to initialize TTF. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    printf("TTF initialized\n");
    engine.running = true;

}

void handle_events() {
    SDL_Event event;
    SDL_PollEvent(&event);

    if (event.type == SDL_EVENT_QUIT) {
        engine.running = false;
    }
    mouse_update(event, &engine.mouse);
    pb_input(event, &ma_vars);

    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.keysym.sym == SDLK_SPACE) {
            if (ma_vars.pb_state & PB_PLAYING) {
                ma_vars.pb_state &= ~PB_PLAYING;
                ma_vars.pb_state |= PB_PAUSED;
            } else {
                ma_vars.pb_state &= ~PB_PAUSED;
                ma_vars.pb_state |= PB_PLAYING;
            }
        }
    } else if (event.type == SDL_EVENT_KEY_UP) {

    } 

}

void update() {
    update_box_arr(engine.mouse);
    update_sidebar(engine.renderer, engine.mouse);
    update_pb(&ma_vars, engine.mouse);
}

void render() {
    SDL_SetRenderDrawColor(engine.renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderClear(engine.renderer);
 
    if (ma_vars.pb_state == PB_PLAYING) {

    }

    render_box_arr(engine.renderer);
    render_sidebar(engine.renderer, engine.mouse);
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
    print_pb_info(ma_vars.pb_info);
    print_pb_state(ma_vars.pb_state);

}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int parse = parse_args(argc, argv); 

    print_args(argc, argv);
    printf("parse_result: %d\n\n", parse);

    if (parse == PLAYBACK_WAV || parse == PLAYBACK_MP3 || parse == PLAYBACK_FLAC) {
        printf("Playback mode selected\n");

        ma_vars.decoder_config = ma_decoder_config_init(ma_format_f32, 2, 48000); 

        ma_vars.device_config = ma_device_config_init(ma_device_type_playback);
        ma_vars.device_config.dataCallback      = pb_callback;
        ma_vars.device_config.pUserData         = &ma_vars;
        
        ma_vars.playlist = create_playlist(argv[2]);
//      mp3_t test = {0};
//      strcpy(test.filename, argv[2]);
//      play_mp3(test, &ma_vars);

    }
    if (parse == PRINT_HELP) {
        print_help();
        exit(1);
    }
    if (parse == PRINT_VERSION) {
        print_version();
        exit(1);
    }
    if (parse == ERROR_NOCOMMAND) {
        fprintf(stderr, "No command provided! use -h to see commands\n");
        exit(1);
    }
    if (parse == ERROR_FILETYPE) {
        fprintf(stderr, "Unknown file type! only WAV, MP3 and FLAC suported\n");
        exit(1);
    }

    if (ma_context_init(NULL, 0, NULL, &ma_vars.context) != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize context.\n");
        exit(1);
    }
    printf("Context initialized\n");

    init_sdl("siMPl3 PLAYER", WIN_WIDTH, WIN_HEIGHT, 0);
    init_player(engine.renderer, &ma_vars);

    if (ma_device_init(NULL, &ma_vars.device_config, &ma_vars.device) != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize device\n");
        exit(1);
    }
    printf("Device initialized\n");

    if (ma_device_start(&ma_vars.device) != MA_SUCCESS) {
        fprintf(stderr, "Failed to start device\n");
        exit(1);
    }
    printf("Device started\n");
    
	while(engine.running) {
		handle_events();
        update();
        render();
        debug();
	}

    uninit_sdl();    
    uninit_ma();

    return 0;
}
