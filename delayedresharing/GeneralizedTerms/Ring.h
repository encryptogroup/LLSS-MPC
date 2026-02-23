
#pragma once

template<typename T>
T RingAdd(T a, T b, T ringSize)
{
    return (a+b) % (ringSize);
}

template<typename T>
T RingSub(T a, T b, T ringSize)
{
    int result = (a >= b)? ((a - b) % ringSize) : ringSize + ((int)a - (int)b);

    return (T) result;
}

template<typename T>
T RingMul(T a, T b, T ringSize)
{
    uint intermediate = ((uint)a)*((uint)b);
    return (T) (intermediate % ringSize);
}