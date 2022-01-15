#pragma once

#include "cebus/cebus_bool.h"

#if defined(CEBUS_THREAD_POSIX)
  #include <pthread.h>
  typedef pthread_t cb_thread_t;
  typedef pthread_mutex_t cb_mutex_t;
  typedef pthread_cond_t cb_cond_t;
#elif defined(CEBUS_THREAD_C11)
  #include <threads.h>
  typedef thrd_t cb_thread_t;
  typedef mtx_t cb_mutex_t;
  typedef cnd_t cb_cond_t;
#else
# error "Missing threading implementation"
#endif

//
// Threading
//

typedef void *(*cb_thread_start)(void *);

typedef struct cb_thread
{
    cb_thread_t id;
    void* data;
} cb_thread;

void cb_thread_init(cb_thread* thread);
cebus_bool cb_thread_spawn(cb_thread* thread, cb_thread_start start, void* arg);

cebus_bool cb_thread_joinable(cb_thread *thread);
void* cb_thread_join(cb_thread* thread);

void cb_thread_destroy(cb_thread* thread);

cb_thread_t cb_thread_current();

//
// Mutex
//

/// Initializes a new mutex
void cb_mutex_init(cb_mutex_t* mutex);

/// Blocks the current thread until the mutex pointed to by `mutex` is locked
/// Returns `cebus_true` if successfull or `cebus_false` otherwise
cebus_bool cb_mutex_lock(cb_mutex_t* mutex);

/// Unblocks the mutex pointed to by `mutex`
/// Returns `cebus_true` if successfull or `cebus_false` otherwise
cebus_bool cb_mutex_unlock(cb_mutex_t* mutex);

/// Destroys the mutex pointed to by `mutex`
void cb_mutex_destroy(cb_mutex_t* mutex);


//
// Condition variable
//

/// Initializes new condition variable.
void cb_cond_init(cb_cond_t *cond);

/// Unblocks one thread that currently waits on condition variable pointed to by `cond`
cebus_bool cb_cond_signal(cb_cond_t* cond);

/// Atomically unlocks the mutex pointed to by `mutex` and blocks on the condition variable pointed to by `cond` until the thread is signalled
cebus_bool cb_cond_wait(cb_cond_t* cond, cb_mutex_t* mutex);

/// Destroys the condition variable pointed to by cond.
void cb_cond_destroy(cb_cond_t* cond);
