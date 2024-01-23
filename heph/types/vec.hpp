#pragma once

typedef struct Vec2 {
    float x, y;

    Vec2 operator+(Vec2 const& addend)
    {
        Vec2 sum;
        sum.x = x + addend.x;
        sum.y = y + addend.y;
        return sum;
    }

    void operator+=(Vec2 const& addend)
    {
        x += addend.x;
        y += addend.y;
    }

    Vec2 operator-(Vec2 const& subtrahend)
    {
        Vec2 difference;
        difference.x = x - subtrahend.x;
        difference.y = y - subtrahend.y;
        return difference;
    }

    void operator-=(Vec2 const& subtrahend)
    {
        x -= subtrahend.x;
        y -= subtrahend.y;
    }
        
} Vec2, Point2;

typedef struct Vec3 {
    float x, y, z;

    Vec3 operator+(Vec3 const& addend)
    {
        Vec3 sum;
        sum.x = x + addend.x;
        sum.y = y + addend.y;
        sum.z = z + addend.z;
        return sum;
    }

    void operator+=(Vec3 const& addend)
    {
        x += addend.x;
        y += addend.y;
        z += addend.z;
    }

    Vec3 operator-(Vec3 const& subtrahend)
    {
        Vec3 difference;
        difference.x = x - subtrahend.x;
        difference.y = y - subtrahend.y;
        difference.z = z - subtrahend.z;
        return difference;
    }

    void operator-=(Vec3 const& subtrahend)
    {
        x -= subtrahend.x;
        y -= subtrahend.y;
        z -= subtrahend.z;
    }
        
} Vec3, Point3;


