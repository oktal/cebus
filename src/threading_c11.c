#include "cebus/threading.h"
#include "cebus/alloc.h"

#include <threads.h>

#define cebus_bool_from_thrd(val) cebus_bool_from_int((val) == thrd_success)

typedef struct cb_thread_main_arg
{
    // The current thread
    cb_thread* thread;

    // The entry point of the thread
    cb_thread_start start;

    // The argument of the entry point
    void* arg;

    // The result of the thread
    void* result;
} cb_thread_main_arg;

int cb_thread_main(void* arg)
{
    cb_thread_main_arg* main_arg = (cb_thread_main_arg *) arg;

    main_arg->thread->id = cb_thread_current();
    main_arg->result = main_arg->start(main_arg->arg);

    return 0;
}

cebus_bool cb_thread_spawn(cb_thread* thread, cb_thread_start start, void* arg)
{
    cb_thread_main_arg* main_arg = cb_new(cb_thread_main_arg, 1);

    main_arg->start = start;
    main_arg->arg = arg;
    main_arg->thread = thread;

    int rc = thrd_create(&thread->id, cb_thread_main, main_arg);
    if (rc == thrd_success)
    {
        thread->data = main_arg;
        return cebus_true;
    }

    free(main_arg);
    return cebus_false;
}

void* cb_thread_join(cb_thread* thread)
{
    cb_thread_main_arg* arg = (cb_thread_main_arg *) thread->data;

    if (arg == NULL)
            return NULL; 

    // Wait for the thread to finish execution
    thrd_join(thread->id, NULL);

    // Return back the result
    return arg->result;
}

thrd_t cb_thread_current()
{
    return thrd_current();
}

void cb_mutex_init(cb_mutex_t* mutex)
{
    mtx_init(mutex, mtx_plain);
}

cebus_bool cb_mutex_lock(cb_mutex_t* mutex)
{
    return cebus_bool_from_thrd(mtx_lock(mutex));
}

cebus_bool cb_mutex_unlock(cb_mutex_t* mutex)
{
    return cebus_bool_from_thrd(mtx_unlock(mutex));
}

void cb_mutex_destroy(cb_mutex_t* mutex)
{
    mtx_destroy(mutex);
}

void cb_cond_init(cb_cond_t* cond)
{
    cnd_init(cond);
}

cebus_bool cb_cond_signal(cb_cond_t* cond)
{
    return cebus_bool_from_thrd(cnd_signal(cond));
}

cebus_bool cb_cond_wait(cb_cond_t* cond, cb_mutex_t* mutex)
{
    return cebus_bool_from_thrd(cnd_wait(cond, mutex));
}

void cb_cond_destroy(cb_cond_t* cond)
{
    cnd_destroy(cond);
}
