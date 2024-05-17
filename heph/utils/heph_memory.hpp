#pragma once

static long HALLOC_COUNT = 0;

#if HEPH_VALIDATE
#define HCALLOC(nitems, size) ({ void *ptr; HALLOC_COUNT++; std::cout << "Hcalloc: " << (ptr = calloc(nitems, size)) << " @ " << FILE_LINE_INFO_STRING << "\n"; if (ptr == NULL) HEPH_ABORT("Hcalloc was null."); ptr; });
#define HALLOC(size) ({ void *ptr; HALLOC_COUNT++; std::cout << "Halloc: " << (ptr = malloc(size)) << " @ " << FILE_LINE_INFO_STRING << "\n"; if (ptr == NULL) HEPH_ABORT("Halloc was null."); ptr; });
#define HFREE(ptr)                                                                       \
        do                                                                               \
        {                                                                                \
                HALLOC_COUNT--;                                                          \
                std::cout << "Hfree: " << ptr << " @ " << FILE_LINE_INFO_STRING << "\n"; \
                free(ptr);                                                               \
        } while (0);
#else
#define HCALLOC(nitems, size) ({ void *ptr; if ((ptr = calloc(nitems, size)) == NULL) HEPH_ABORT("Hcalloc was null."); ptr; });
#define HALLOC(size) ({ void *ptr; if ((ptr = malloc(size) == NULL)) HEPH_ABORT("Halloc was null."); ptr; });
#define HFREE(ptr) free(ptr);
#endif