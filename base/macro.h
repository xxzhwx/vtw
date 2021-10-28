#pragma once

// ASSERT
#ifdef _DEBUG
#include <assert.h>
#define ASSERT assert
#else
#define ASSERT
#endif

// MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
// MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// SAFE_FREE
#define SAFE_FREE(ptr)      \
    if ((ptr) != nullptr) { \
        free(ptr);          \
        (ptr) = nullptr;    \
    }

#define SAFE_DELETE(ptr)    \
    if ((ptr) != nullptr) { \
        delete (ptr);       \
        (ptr) = nullptr;    \
    }

#define SAFE_DELETE_ARRAY(ptr) \
    if ((ptr) != nullptr) {    \
        delete[] (ptr);        \
        (ptr) = nullptr;       \
    }

// MAKEINT LOWINT HIGHINT
#define MAKEINT16(x, y) ((uint16_t)((((uint16_t)(x) & 0xff) << 8) | ((uint16_t)(y) & 0xff)))
#define LOWINT8(x) ((uint8_t)((uint16_t)(x) & 0xff))
#define HIGHINT8(x) ((uint8_t)((uint16_t)(x) >> 8))

#define MAKEINT32(x, y) ((uint32_t)((((uint32_t)(x) & 0xffff) << 16) | ((uint32_t)(y) & 0xffff)))
#define LOWINT16(x) ((uint16_t)((uint32_t)(x) & 0xffff))
#define HIGHINT16(x) ((uint16_t)((uint32_t)(x) >> 16))

#define MAKEINT64(x, y) ((uint64_t)((((uint64_t)(x) & 0xffffffff) << 32) | ((uint64_t)(y) & 0xffffffff)))
#define LOWINT32(x) ((uint32_t)((uint64_t)(x) & 0xffffffff))
#define HIGHINT32(x) ((uint32_t)((uint64_t)(x) >> 32))

