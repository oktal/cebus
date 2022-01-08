#pragma once

#include "cebus/cebus_bool.h"

#include <pthread.h>

typedef pthread_t cb_thread_handle_t;
typedef void *(*cb_thread_start)(void *);

typedef struct cb_thread
{
    cb_thread_handle_t handle;

    cebus_bool is_running;
} cb_thread;

void cb_thread_init(cb_thread* thread);
cebus_bool cb_thread_spawn(cb_thread* thread, cb_thread_start start, void* arg);

cebus_bool cb_thread_joinable(cb_thread* thread);
void* cb_thread_join(cb_thread* thread);

void cb_thread_destroy(cb_thread* thread);
