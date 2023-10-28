#pragma once
#include "utils.h"
#include <stddef.h>
#include <stdint.h>

extern struct key_input_event {
    int scancode;
    bool key_pressed;
} *key_input_events;
extern size_t key_input_n;
extern bool _input_event_guard;

/* Tracks keyboard input events. */
void *track_keyboard_input(void *_vargp);

/* Waits for a given input */
void wait_input_event(int scancode);
