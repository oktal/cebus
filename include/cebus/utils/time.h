#pragma once

#include "cebus/utils/timespan.h"

typedef struct time_instant
{
} time_instant;

time_instant time_instant_now();
timespan time_instant_elapsed(const time_instant* instant);
