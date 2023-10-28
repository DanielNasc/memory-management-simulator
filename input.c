#include "input.h"
#include "utils.h"
#include <inttypes.h>
#include <stdio.h>

struct key_input_event *key_input_events = NULL;
size_t key_input_n = 0;
bool _input_event_guard;

/* Tracks keyboard input events. */
void *track_keyboard_input(void *_vargp)
{
    for (;;) {
        int code = getchar();
        if (!_input_event_guard && key_input_events != NULL) {
            _input_event_guard = true;
            struct key_input_event *kie = key_input_events;

            for (int i = key_input_n; i != 0; i--)
                if (kie->scancode != code)
                    kie++;
                else
                    kie->key_pressed = true;

            _input_event_guard = false;
        }
    }
    return NULL;
}


void wait_input_event(int scancode)
{
    struct key_input_event kie = { .scancode=scancode, .key_pressed=false};

    // Set input event
    while (_input_event_guard);
    _input_event_guard = true;
    key_input_n = 1;
    key_input_events = &kie;
    _input_event_guard = false;

    while (!kie.key_pressed); // Wait

    // Unset event
    while (_input_event_guard);
    _input_event_guard = true;
    key_input_n = 0;
    key_input_events = NULL;
    _input_event_guard = false;
}
