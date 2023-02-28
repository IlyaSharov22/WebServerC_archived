#pragma once

#include "includes.h"
#include "buffer.h"

enum {
	EVENT_STATE_INVALID,
	EVENT_STATE_ACTIVE,
	EVENT_STATE_PENDING
};

struct event_loop;

typedef struct event_loop event_loop_t;
typedef void(*event_callback_t)(event_loop_t*, usize, void*);
typedef void(*event_system_error_callback_t)(event_loop_t*, void*);

typedef void* event_key_t;

event_loop_t* event_loop_new();
event_key_t event_add(fd_t fd, event_loop_t* loop);
void event_submit(event_key_t event_key, event_callback_t event_cb);
void event_close(event_loop_t* loop, event_key_t event_key);
void event_force_close(event_key_t event_key);
void event_set_user_data(event_key_t event_key, void* user_data);
void event_loop_run(event_loop_t* loop);
void event_loop_stop(event_loop_t* loop);
void event_loop_free(event_loop_t* loop);

void event_loop_default_error_handler(event_loop_t* loop, void* event_key);
void event_loop_set_error_handler(event_system_error_callback_t error_cb);