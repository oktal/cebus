#include "cebus/threading.h"

#include <stdlib.h>

void cb_thread_init(cb_thread* thread)
{
    thread->data = NULL;
}

cebus_bool cb_thread_joinable(cb_thread* thread)
{
    return cebus_bool_from_int(thread->data != NULL);
}

void cb_thread_destroy(cb_thread* thread)
{
    free(thread->data);
    thread->data = NULL;
}
