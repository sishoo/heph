#pragma once


typedef struct Vec2 {

} Vec2;

typedef struct Vec3 {
    float x, y, z;

    Vertex operator+(Vertex const& addend)
    {
        Vertex sum;
        sum.x = x + addend.x;
        sum.y = y + addend.y;
        sum.z = z + addend.z;
        return sum;
    }

    void operator+=(Vertex const& addend)
    {
        x += addend.x;
        y += addend.y;
        z += addend.z;
    }

    Vertex operator-(Vertex const& subtrahend)
    {
        Vertex difference;
        difference.x = x - subtrahend.x;
        difference.y = y - subtrahend.y;
        difference.z = z - subtrahend.z;
        return difference;
    }

    void operator-=(Vertex const& subtrahend)
    {
        x -= subtrahend.x;
        y -= subtrahend.y;
        z -= subtrahend.z;
    }
        
} Vec3;


