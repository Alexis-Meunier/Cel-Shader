#include "object.hh"

#include <fstream>
#include <sstream>

#include "object.hh"
#include <fstream>
#include <sstream>
#include <array>

objectData from_obj(const std::string& path, const Point& offset,
                    const float& scale, const Point& rotation)
{
    const auto rad = M_PI / 180.0f;

    std::vector<std::array<float, 3>> raw_verts;
    std::vector<std::array<float, 3>> raw_normals;
    std::vector<GLfloat> out_positions;
    std::vector<GLfloat> out_normals;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return { out_positions, out_normals };
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v") {
            float x, y, z;
            ss >> x >> y >> z;

            // Scale then rotate
            auto rotated = Point(x * scale, y * scale, z * scale);
            if (std::abs(rotation.x) > 1e-3) rotated.rotateX(rotation.x * rad);
            if (std::abs(rotation.y) > 1e-3) rotated.rotateY(rotation.y * rad);
            if (std::abs(rotation.z) > 1e-3) rotated.rotateZ(rotation.z * rad);

            raw_verts.push_back({ rotated.x + offset.x,
                                   rotated.y + offset.y,
                                   rotated.z + offset.z });
        }
        else if (token == "vn") {
            float x, y, z;
            ss >> x >> y >> z;

            auto rotated = Point(x, y, z);
            if (std::abs(rotation.x) > 1e-3) rotated.rotateX(rotation.x * rad);
            if (std::abs(rotation.y) > 1e-3) rotated.rotateY(rotation.y * rad);
            if (std::abs(rotation.z) > 1e-3) rotated.rotateZ(rotation.z * rad);

            raw_normals.push_back({ rotated.x, rotated.y, rotated.z });
        }
        else if (token == "f") {
            std::string part;
            std::vector<int> vi, ni;

            while (ss >> part) {
                auto s1 = part.find('/');
                vi.push_back(std::stoi(part.substr(0, s1)) - 1);

                if (s1 != std::string::npos) {
                    auto s2 = part.find('/', s1 + 1);
                    if (s2 != std::string::npos && s2 > s1 + 1)
                        ni.push_back(std::stoi(part.substr(s2 + 1)) - 1);
                    else if (s2 != std::string::npos)
                        ni.push_back(std::stoi(part.substr(s2 + 1)) - 1);
                }
            }

            for (int i = 1; i + 1 < (int)vi.size(); i++) {
                for (int idx : {0, i, i + 1}) {
                    int v = vi[idx];
                    if (v < 0 || v >= (int)raw_verts.size()) continue;

                    auto& p = raw_verts[v];
                    out_positions.push_back(p[0]);
                    out_positions.push_back(p[1]);
                    out_positions.push_back(p[2]);

                    if (idx < (int)ni.size()) {
                        int n = ni[idx];
                        if (n >= 0 && n < (int)raw_normals.size()) {
                            auto& nm = raw_normals[n];
                            out_normals.push_back(nm[0]);
                            out_normals.push_back(nm[1]);
                            out_normals.push_back(nm[2]);
                        }
                    }
                }
            }
        }
    }

    return { out_positions, out_normals };
}