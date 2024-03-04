#pragma once

static long HALLOC_COUNT = 0;

#if HEPH_VALIDATE
#define HCALLOC(nitems, size) ({ void *ptr; HALLOC_COUNT++; std::cout << "Hcalloc: " << (ptr = calloc(nitems, size)) << "\n"; ptr; });
#define HALLOC(size) ({ void *ptr; HALLOC_COUNT++; std::cout << "Halloc: " << (ptr = malloc(size)) << "\n"; ptr; });
#define HFREE(ptr)                                     \
        do                                             \
        {                                              \
                HALLOC_COUNT--;                        \
                std::cout << "Hfree: " << ptr << "\n"; \
                free(ptr);                             \
        } while (0);

#else
#define HCALLOC(nitems, size) calloc(nitems, size);
#define HALLOC(size) malloc(size);
#define HFREE(ptr) free(ptr);
#endif