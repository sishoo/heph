#include "quadtree.hpp"


void Node::subdivide()
{
    Point3 *point = (Point3 *) element;
    Node *child_memory = (Node *) malloc(sizeof(Node) * 4);
    uint32_t stride = sizeof(Node);
    *child_memory = Node();  
    *(child_memory + stride) = Node();
    *(child_memory + stride * 2) = Node();
    *(child_memory + stride * 3) = Node();
}

void QuadTree::insert(Point3 point)
{
    Node *current = root;
    uint32_t depth = 1; // starts at one because we start at root
    while (true)
    {   
        if (!current->count)
        {
            current->subdivide();
        }

        for (uint32_t i = 0; i < current->count; i++)
        {
            
        }
            
    }
}

QuadTree::QuadTree(Node * const root, float width, float height) : root(root), width(width), height(height) {
        Node *child_memory = (Node *) malloc(sizeof(Node) * 4);
        uint32_t stride = sizeof(Node);
        *child_memory = Node();  
        *(child_memory + stride) = Node();
        *(child_memory + stride * 2) = Node();
        *(child_memory + stride * 3) = Node();
        root->element = child_memory;
        root->count = 4;
    }
