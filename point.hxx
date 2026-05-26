#pragma once

#include "point.hh"

const inline Point Point::operator+(const Point& p) const
{
    return Point(x + p.x, y + p.y, z + p.z);
}

const inline Point Point::operator-(const Point& p) const
{
    return Point(x - p.x, y - p.y, z - p.z);
}

const inline Point Point::operator*(const double& scalar) const
{
    return Point(x * scalar, y * scalar, z * scalar);
}

const inline float Point::operator*(const Point& p) const
{
    return x * p.x + y * p.y + z * p.z;
}

inline Point& Point::operator=(const Point& v) {
    x = v.x; y = v.y; z = v.z;
    return *this;
}

inline std::ostream& Point::operator<<(std::ostream& os)
{
    os << "(" << x << ", " << y << ", " << z << ")" << std::endl;
    return os;
}

inline float& Point::operator[](size_t idx)
{
    switch (idx)
    {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    default:
        throw std::out_of_range("Point indexing gone wrong");
    }
}

inline float Point::operator[](size_t idx) const
{
    switch (idx)
    {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    default:
        throw std::out_of_range("Point indexing gone wrong");
    }
}
