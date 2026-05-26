#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <iostream>

class matrix4
{
public:
    matrix4() = default;
    matrix4(std::vector<std::vector<GLfloat>> values);
    void operator*=(const matrix4& rhx);
    static matrix4 identity();
    GLfloat *get_values();

    std::vector<std::vector<GLfloat>> values;
};

std::ostream& operator<<(std::ostream& out, const matrix4& m);

class vec3
{
public:
    vec3() = default;
    vec3(const GLfloat& x, const GLfloat& y, const GLfloat& z);
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

matrix4 look_at(const GLfloat& eyeX, const GLfloat &eyeY, const GLfloat& eyeZ,
             const GLfloat& centerX, const GLfloat& centerY, const GLfloat& centerZ,
             const GLfloat& upX, const GLfloat& upY, const GLfloat& upZ);

matrix4 frustum(const GLfloat& left, const GLfloat &right, const GLfloat& bottom,
             const GLfloat& top, const GLfloat& z_near, const GLfloat& z_far);
