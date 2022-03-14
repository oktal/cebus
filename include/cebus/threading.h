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


//
// Future
//

typedef enum cb_future_state
{
    cb_future_pending,

    cb_future_ready,
} cb_future_state;

/// The function to call back when the future has been resolved
typedef void (*cb_future_continuation)(void* user, void* data);

/// The destructor function to call when destroying the future
typedef void (*cb_future_destructor)(void *user, void *data);

/// A `future` provides a mechanism to access the result of asynchronous operations:
typedef struct cb_future
{
    void* inner;
} cb_future;

/// Initializes a new `future`
void cb_future_init(cb_future* future, size_t value_size);

cb_future_state cb_future_poll(cb_future* future, void** data_out);

void cb_future_then(cb_future* future, cb_future_continuation continuation, void* user);

void cb_future_set(cb_future* future, void* data);

void* cb_future_get(cb_future* future);

void cb_future_destroy(cb_future* future, cb_future_destructor dtor);
