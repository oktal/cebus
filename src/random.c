#include "cebus/utils/random.h"

#include "cebus/platform.h"

#if defined(__linux__)
  #include <sys/random.h>
#else
# error "Missing random implementation"
#endif

#if defined(CEBUS_PLATFORM_LINUX)
    cebus_bool cb_get_random_bytes(uint8_t* dst, size_t len)
    {
        ssize_t res = getrandom(dst, len, GRND_NONBLOCK); 
        return cebus_bool_from_int(res > 0);
    }
#else
# error "Missing random implementation"
#endif
