#include "clock.h"

void start_clock(Clock *clock) {
    if (clock->state & CLK_PAUSED) {
        clock->start_ms = clock->current_ms + SDL_GetTicks();
    } else {
        clock->start_ms = SDL_GetTicks();
    }
    clock->state = CLK_RUNNING;

}

void pause_clock(Clock *clock) {
    clock->state = CLK_PAUSED;

}

void stop_clock(Clock *clock) {
    clock->start_ms = 0;
    clock->current_ms = 0;
    clock->state = CLK_STOPPED;

}

void restart_clock(Clock *clock) {
    stop_clock(clock);
    start_clock(clock);

}

void update_clock(Clock *clock) {
    if (clock->state == CLK_RUNNING) {
        clock->current_ms = SDL_GetTicks() - clock->start_ms; 
    }
}

void print_time_24hrs(u32 ms) {
    u32 sec = ms % 60;
    u32 min = ms / 60;
    u32 hr  = ms / 3600;
    printf("%02u:%02u:%02u", hr, min, sec);
}

void time_24hrs(char *str, u32 ms) {
    u32 sec = ms % 60;
    u32 min = ms / 60;
    u32 hr  = ms / 3600;
    sprintf(str, "%02u:%02u:%02u", hr, min, sec);
}

u64 get_clock_time_min(Clock clock) {
    return clock.current_ms / 1000;
}

u64 get_clock_time_hr(Clock clock) {
    return clock.current_ms / 3.6e+6;
}
