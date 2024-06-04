#pragma once

static long HALLOC_COUNT = 0;

#if HEPH_DEBUG
#define HCALLOC(nitems, size) ({ void *ptr; HALLOC_COUNT++; printf("Hcalloc: %p\n", (ptr = calloc(nitems, size))); HEPH_NASSERT(ptr, NULL); ptr; });
#define HALLOC(size) ({ void *ptr; HALLOC_COUNT++; printf("Halloc: %p\n", (ptr = malloc(size))); HEPH_NASSERT(ptr, NULL); ptr; });
#define HFREE(ptr)                                                                       \
        do                                                                               \
        {                                                                                \
                HALLOC_COUNT--;                                                          \
                printf("Hfree: %p\n", ptr); \
                free(ptr);                                                               \
        } while (0);
#else
#define HCALLOC(nitems, size) ({ void *ptr; ptr = calloc(nitems, size); HEPH_NASSERT(ptr, NULL); ptr; });
#define HALLOC(size) ({ void *ptr; ptr = malloc(size); HEPH_NASSERT(ptr, NULL); ptr; });
#define HFREE(ptr) free(ptr);
#endif