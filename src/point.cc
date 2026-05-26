#include "point.hh"

#include <cmath>

Point::Point()
    : x(0)
    , y(0)
    , z(0)
{}


Point::Point(const float& x_, const float& y_, const float& z_)
{
    this->x = x_;
    this->y = y_;
    this->z = z_;
}

Point::Point(const Point& v)
{
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
}

float Point::distance(const Point& other) const
{
    float difx = other.x - x;
    float dify = other.y - y;
    float difz = other.z - z;
    return std::sqrt(difx * difx + dify * dify + difz * difz);
}

void Point::rotateX(const float& angle)
{
    float oldY = this->y, oldZ = this->z;
    auto cos = std::cos(angle);
    auto sin = std::sin(angle);

    this->y = cos * oldY - sin * oldZ;
    this->z = sin * oldY + cos * oldZ;
}

void Point::rotateY(const float& angle)
{
    float oldX = this->x, oldZ = this->z;
    auto cos = std::cos(angle);
    auto sin = std::sin(angle);

    this->x = cos * oldX - sin * oldZ;
    this->z = sin * oldX + cos * oldZ;
}

void Point::rotateZ(const float& angle)
{
    float oldX = this->x, oldY = this->y;
    auto cos = std::cos(angle);
    auto sin = std::sin(angle);

    this->x = cos * oldX - sin * oldY;
    this->y = sin * oldX + cos * oldY;
}

float distance(const Point& p1, const Point& p2)
{
    // Norm computation
    return std::sqrt(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2)
                     + std::pow(p1.z - p2.z, 2));
}

std::ostream& operator<<(std::ostream& out, const Point& vect)
{
    return out << "(" << (vect.x) << ", " << (vect.y) << ", " << (vect.z)
               << ")\n";
}
