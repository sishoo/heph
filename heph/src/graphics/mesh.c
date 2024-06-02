#include "include/graphics/mesh.h"

typedef struct
{
        char *path;
        uint32_t nvertices, nindices, index_start, vertex_start;
} heph_mesh_t;


void heph_mesh_init(heph_mesh_t *const m);