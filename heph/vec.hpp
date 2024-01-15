#pragma once


typedef struct Vec3 {
    float x, y, z;

    Vec3() : x(0.0), y(0.0), z(0.0) {}

    Vec3(float a, float b, float c) : x(x), y(y), z(z) {}

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



typedef struct Vec2 {
    float x, y, z;

    Vec2() : x(0.0), y(0.0) {}

    Vec2(float x, float y) : x(x), y(y) {}

    void add(float a, float b, float c)
    {
        x += a;
        y += b;
        z += c;
    }

    static Vec2 add(Vec2 a, Vec2 b)
    {
        float x = a.x + b.x;
        float y = a.y + b.y;
        return Vec2(x, y);
    }
} Vec2, Point2;