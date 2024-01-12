#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <vector>

#include "vec.hpp"


#define CONTAINS(x, range_start, range_len) (range_start <= x && x < (range_start + range_len))

typedef struct Node {
    void *element;
    char count : 1;

    Node() : element(nullptr), count(0) {}
    Node(void *element, char count) : element(element), count(count) {} 

    void subdivide();
} Node;

class QuadTree {
public:
    Node * const root;
    const float width, height;

    QuadTree() = delete;
    QuadTree(Node * const root, float width, float height);

    void insert(Point3 point);  
    void query(Point3 point);
    void remove(Point3 point);
};
