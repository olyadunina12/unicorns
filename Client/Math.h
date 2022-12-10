#pragma once

template <typename T>
T lerp(T a, T b, float alpha)
{
    return a * (1.f - alpha) + b * alpha;
}

#define RADIANS(x) (x * (3.14f / 180.f))

template <typename T>
T clamp(T x, T min, T max)
{
    if (x < min)
        return min;
    else if(x > max)
        return max;
    return x;
}