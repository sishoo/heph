#pragma once

#include <stdint.h>

typedef struct NoInit64
{
    uint64_t filler;
    NoInit64 () {}
} NoInit64;

typedef struct NoInit8
{
    char filler;
    NoInit8 () {}
} NoInit8;

typedef struct NoInit32
{
    uint32_t filler;
    NoInit32 () {}
} NoInit32;

typedef struct NoInit96
{
    uint32_t filler;
    uint32_t filler1;
    uint32_t filler2;
    NoInit96 () {}
} NoInit96;

typedef struct NoInitSize
{
    size_t filler;
    NoInitSize() {}
} NoInitSize;

