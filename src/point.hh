#pragma once

#include <iostream>

class Point
{
public:
    Point();
    Point(const float& x_, const float& y_, const float& z_);
    Point(const Point& v);

    void rotateX(const float& angle);
    void rotateY(const float& angle);
    void rotateZ(const float& angle);
    float distance(const Point& other) const;

    // Arithmetic
    const Point operator+(const Point& p) const;
    const Point operator-(const Point& p) const;
    const Point operator*(const double& scalar) const; // Translation
    const float operator*(const Point& p) const; // Dot product
    Point& operator=(const Point& v);

    // Debugging
    std::ostream& operator<<(std::ostream& os);

    // Indexing
    float& operator[](size_t idx);
    float operator[](size_t idx) const;

    float x;
    float y;
    float z;
};

std::ostream& operator<<(std::ostream& out, const Point& vect);
/**
 * Returns the euclidian norm/Distance between two points
 *
 * Reminder:
 *      In R^3:
 *          v1 = (x1, y1, z1)
 *          v2 = (x2, y2, z2)
 *
 * ||v1 - v2|| = sqrt(
 *      (x1 - x2)^2
 *    + (y1 - y2)^2
 *    + (z1 - z2)^2
 * )
 */
float distance(const Point& p1, const Point& p2);

#include "point.hxx"
