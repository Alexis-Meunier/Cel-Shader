#pragma once

#include <GL/glew.h>
#include <vector>
#include <cmath>

#include "point.hh"

using std::vector;

struct objectData
{
    vector<GLfloat> position;
    vector<GLfloat> normals;
    vector<GLfloat> uv_position;
    // vector<GLfloat> colors;
};

objectData from_obj(const std::string& path, const Point& offset = Point{0, 0, 0},
                         const float& scale = 1.0f, const Point& rotation = Point{0,0,0});

objectData LoadOBJ(const std::string& path, const Point& offset,
                   const float& scale, const Point& rotation);
