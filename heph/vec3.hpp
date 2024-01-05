#pragma once


typedef struct Vec3 {
    float x, y, z;

    Vec3() 
    {
        x = y = z = 0.0;
    }
    Vec3(float a, float b, float c) 
    {
        x = a;
        y = b;
        z = c;
    }

    void add(float a, float b, float c)
    {
        x += a;
        y += b;
        z += c;
    }

    static Vec3 add(Vec3 a, Vec3 b)
    {
        float x = a.x + b.x;
        float y = a.y + b.y;
        float z = a.z + b.z;
        return Vec3(x, y, z);
    }
} Vec3, Point3;