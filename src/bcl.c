#include "bcl.h"

#include "cebus/alloc.h"
#include "cebus/utils/time_internal.h"

cb_date_time cb_date_time_from_proto(const Bcl__DateTime *proto)
{
    return cb_date_time_from_ticks(proto->value + CB_EPOCH_TICKS);
}

Bcl__DateTime* cb_bcl_date_time_proto_init(Bcl__DateTime* proto, cb_date_time date)
{
    const cb_date_time epoch = cb_date_time_since_epoch(date);
    bcl__date_time__init(proto);
    proto->has_scale = 1;
    proto->has_value = 1;

    proto->scale = BCL__DATE_TIME__TIME_SPAN_SCALE__TICKS;
    proto->value = cb_date_time_ticks(epoch);

    return proto;
}

Bcl__DateTime* cb_bcl_date_time_proto_new(cb_date_time date)
{
    return cb_bcl_date_time_proto_init(cb_new(Bcl__DateTime, 1), date);
}

void cb_bcl_date_time_proto_free(Bcl__DateTime* proto)
{
}

