#ifndef __IBUS_LUNISPIM_ENGINE_H__
#define __IBUS_LUNISPIM_ENGINE_H__

#include <ibus.h>
#include <stdio.h>
#define IBUS_TYPE_UNISPIM_ENGINE \
        (ibus_unispim_engine_get_type())

GType ibus_unispim_engine_get_type();
#define DEBUG_ECHO(fmt, arg...) printf("=====[File: %s Func: %s Line: %d]=====\n", strrchr(__FILE__, '/'), __FUNCTION__, __LINE__);printf(fmt, ##arg);printf("\n\n");
#define TICK "✓"
#endif
