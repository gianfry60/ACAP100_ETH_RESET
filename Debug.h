#ifndef Debug_h
#define Debug_h
#include "DefineDir.h"


#ifndef NDEBUG
#define DSTART(speed) Serial.begin(speed)
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#define DSWRITE(...) Serial.write(__VA_ARGS__)

#else
#define DSTART(...) \
    do              \
    {               \
    } while (0)
#define DPRINT(...) \
    do              \
    {               \
    } while (0)
#define DPRINTLN(...) \
    do                \
    {                 \
    } while (0)
#define DSWRITE(...) \
    do               \
    {                \
    } while (0)
#undef DSERIAL
#endif
#endif // Debug_h
