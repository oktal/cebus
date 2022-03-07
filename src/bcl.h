#pragma once

#include "cebus/utils/time.h"
#include "bcl.pb-c.h"

cb_date_time cb_date_time_from_proto(const Bcl__DateTime* proto);

Bcl__DateTime* cb_bcl_date_time_proto_new(cb_date_time date);
void cb_bcl_date_time_proto_free(Bcl__DateTime* proto);

