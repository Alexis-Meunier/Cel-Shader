#include "utils/matrix.hh"

#include <cmath>

void matrix4::operator*=(const matrix4& rhs)
{
    std::vector<std::vector<GLfloat>> vals;
    vals.resize(4);
    vals[0].resize(4);
    vals[1].resize(4);
    vals[2].resize(4);
    vals[3].resize(4);

    for(int x = 0; x < 4; x++)
    {
        for(int y = 0; y < 4; y++)
        {
            GLfloat sum = 0.;

            for(int i = 0; i < 4; i++)
            {
                sum += this->values[y][i] * rhs.values[i][x];
            }

            vals[y][x] = sum;
        }
    }

    this->values = vals;
}

matrix4::matrix4(std::vector<std::vector<GLfloat>> values)
{
    this->values = values;
}

matrix4 matrix4::identity()
{
    static auto mat = matrix4(
        {
            {1., 0., 0., 0.},
            {0., 1., 0., 0.},
            {0., 0., 1., 0.},
            {0., 0., 0., 1.}
        }
    );

    return mat;
}

std::ostream& operator<<(std::ostream& out, const matrix4& m)
{
    for (auto& row : m.values)
    {
        out << "| ";
        for (auto& val : row)
        {
            out << val << " ";
        }
        out << "|\n";
    }
    out << std::endl;
    return out;
}

vec3::vec3(const GLfloat& x, const GLfloat& y, const GLfloat& z)
: x(x), y(y), z(z)
{}

vec3 vec_prod(vec3& lhs, const vec3& rhs)
{
    return vec3(
        {
            lhs.y * rhs.z - lhs.z * rhs.y,
            -(lhs.x * rhs.z - lhs.z * rhs.x),
            lhs.x * rhs.y - lhs.y * rhs.x
        }
    );
}

vec3 normalize(const vec3& vec)
{
    GLfloat tot = 0.0;
    tot += std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    return {vec.x / tot, vec.y / tot, vec.z / tot};
}

matrix4 look_at(const GLfloat& eyeX, const GLfloat &eyeY, const GLfloat& eyeZ,
             const GLfloat& centerX, const GLfloat& centerY, const GLfloat& centerZ,
             const GLfloat& upX, const GLfloat& upY, const GLfloat& upZ)
{
    vec3 F = normalize({ centerX - eyeX, centerY - eyeY, centerZ - eyeZ });
    vec3 s = normalize(vec_prod(F, { upX, upY, upZ }));
    vec3 u = vec_prod(s, F);

    matrix4 L(
        {
            {s.x, s.y, s.z, 0.},
            {u.x, u.y, u.z, 0.},
            {-F.x, -F.y, -F.z, 0},
            {0., 0., 0., 1.}
        }
    );

    matrix4 view(
        {
            {1., 0., 0., -eyeX},
            {0., 1., 0., -eyeY},
            {0., 0., 1., -eyeZ},
            {0., 0., 0., 1.}
        }
    );

    L *= view;
    return L;
}

matrix4 frustum(const GLfloat& left, const GLfloat &right, const GLfloat& bottom,
             const GLfloat& top, const GLfloat& z_near, const GLfloat& z_far)
{
    auto A = (right + left) / (right - left);
    auto B = (top + bottom) / (top - bottom);
    auto C = -(z_far + z_near) / (z_far - z_near);
    auto D = -2*(z_far * z_near) / (z_far - z_near);

    return matrix4(
        {
            {(2 * (z_near) / (right - left)), 0., A, 0.},
            {0., (2 * (z_near) / (top - bottom)), B, 0.},
            {0., 0., C, D},
            {0., 0., -1., 0.}
        }
    );
}

GLfloat *matrix4::get_values()
{
    int index = 0;
    GLfloat *vals = new GLfloat[16 * sizeof(GLfloat)];
    for(const auto& arr : values)
    {
        for (const auto val : arr)
        {
            vals[index++] = GLfloat(val);
        }
    }

    return vals;
}
