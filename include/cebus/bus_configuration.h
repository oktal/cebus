#pragma once

#include <stddef.h>

#include "cebus/cebus_bool.h"
#include "cebus/utils/timespan.h"

/// Configuration options for a bus
typedef struct cb_bus_configuration
{
    /// A NULL-terminated list of directories that can be used by the bus to register
    char** directoryEndpoints;

    /// The time to wait for when registering to a particular directory. Upon timeout, the next
    /// directory of the list will be used
    timespan registration_timeout;

    /// The time to wait for when trying to replay messages from the persistence on startup
    timespan start_replay_timeout;

    /// Whether the peer is persistent or not.
    cebus_bool is_persistent;
} cb_bus_configuration;
