#include "object.hh"

#include <fstream>
#include <sstream>
#include <array>

#include "matrix.hh"

objectData from_obj(const std::string& path, const Point& offset,
                    const float& scale, const Point& rotation)
{
    const auto rad = M_PI / 180.0f;

    std::vector<std::array<float, 3>> raw_verts;
    std::vector<std::array<float, 3>> raw_normals;
    std::vector<GLfloat> out_positions;
    std::vector<GLfloat> out_normals;
    std::vector<GLfloat> uv_position;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return { out_positions, out_normals, uv_position };
    }

    int totvt = 0;
    int totv = 0;
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
            totv++;

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
        else if (token == "vt") {
            float x, y;
            ss >> x >> y;
            totvt++;

            uv_position.push_back(x);
            uv_position.push_back(y);
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

    std::cout << "nb v: " << totv << std::endl;
    std::cout << "nb vt: " << totvt << std::endl;

    return { out_positions, out_normals, uv_position };
}

objectData LoadOBJ(const std::string& path, const Point& offset,
                   const float& scale, const Point& rotation)
{
    
    const auto rad = M_PI / 180.0f;

    std::vector<std::array<float, 3>> raw_verts;
    std::vector<std::array<float, 3>> raw_normals;
    std::vector<std::array<float, 2>> raw_uvs;
    std::vector<GLfloat> out_positions;
    std::vector<GLfloat> out_normals;
    std::vector<GLfloat> out_uv;
    std::string lineStr;

    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return { out_positions, out_normals, out_uv };
    }

    while( std::getline(in, lineStr ) )
    {
        std::istringstream lineSS( lineStr );
        std::string lineType;
        lineSS >> lineType;

        // vertex
        if( lineType == "v" )
        {
            float x = 0, y = 0, z = 0;
            lineSS >> x >> y >> z;
            // raw_verts.push_back({x, y, z});

            // Scale then rotate
            auto rotated = Point(x * scale, y * scale, z * scale);
            if (std::abs(rotation.x) > 1e-3) rotated.rotateX(rotation.x * rad);
            if (std::abs(rotation.y) > 1e-3) rotated.rotateY(rotation.y * rad);
            if (std::abs(rotation.z) > 1e-3) rotated.rotateZ(rotation.z * rad);

            raw_verts.push_back({ rotated.x + offset.x,
                                   rotated.y + offset.y,
                                   rotated.z + offset.z });
        }

        // texture
        if( lineType == "vt" )
        {
            float u = 0, v = 0, w = 0;
            lineSS >> u >> v >> w;
            if (u > 1)
                std::cout << u << std::endl;
            if (v > 1)
                std::cout << v << std::endl;
            if (w > 1)
                std::cout << w << std::endl;
            raw_uvs.push_back({u, 1 - v});
        }

        // normal
        if( lineType == "vn" )
        {
            float i = 0, j = 0, k = 0;
            lineSS >> i >> j >> k;

            auto rotated = Point(i, j, k);
            if (std::abs(rotation.x) > 1e-3) rotated.rotateX(rotation.x * rad);
            if (std::abs(rotation.y) > 1e-3) rotated.rotateY(rotation.y * rad);
            if (std::abs(rotation.z) > 1e-3) rotated.rotateZ(rotation.z * rad);

            raw_normals.push_back({rotated.x, rotated.y, rotated.z});
            // raw_normals.push_back({i, j, k});
        }

        // polygon
        if( lineType == "f" )
        {
            std::vector<std::array<int, 3>> vertx;
            std::string refStr;
            while( lineSS >> refStr )
            {
                std::istringstream ref( refStr );
                std::string vStr, vtStr, vnStr;
                std::getline( ref, vStr, '/' );
                std::getline( ref, vtStr, '/' );
                std::getline( ref, vnStr, '/' );
                int v = atoi( vStr.c_str() );
                int vt = atoi( vtStr.c_str() );
                int vn = atoi( vnStr.c_str() );
                v  = (  v >= 0 ?  v - 1 : raw_verts.size() +  v );
                vt = ( vt >= 0 ? vt - 1 : raw_uvs.size() + vt );
                vn = ( vn >= 0 ? vn - 1 : raw_normals.size() + vn );
                vertx.push_back({v, vt, vn});
            }

            // triangulate, assuming n>3-gons are convex and coplanar
            for( size_t i = 1; i+1 < vertx.size(); ++i )
            {
                std::array<int, 3> p[3] = { vertx[0], vertx[i], vertx[i+1] };

                // http://www.opengl.org/wiki/Calculating_a_Surface_Normal
                int v10 = p[1][0];
                int v20 = p[2][0];
                int v00 = p[0][0];
                std::array<float, 3> val00 = raw_verts[v00];
                std::array<float, 3> val10 = raw_verts[v10];
                std::array<float, 3> val20 = raw_verts[v20];
                vec3 U(val10[0] - val00[0], val10[1] - val00[1], val10[2] - val00[2]);
                vec3 V(val20[0] - val00[0], val20[1] - val00[1], val20[2] - val00[2]);
                vec3 faceNormal = normalize(vec_prod(U, V));

                for( size_t j = 0; j < 3; ++j )
                {
                    out_positions.push_back(raw_verts[p[j][0]][0]);
                    out_positions.push_back(raw_verts[p[j][0]][1]);
                    out_positions.push_back(raw_verts[p[j][0]][2]);

                    out_normals.push_back(raw_normals[p[j][2]][0]);
                    out_normals.push_back(raw_normals[p[j][2]][1]);
                    out_normals.push_back(raw_normals[p[j][2]][2]);

                    out_uv.push_back(raw_uvs[p[j][1]][0]);
                    out_uv.push_back(raw_uvs[p[j][1]][1]);
                }
            }
        }
    }

    std::cout << "first uv: " << out_positions[1100] << ", " << out_positions[1101] << std::endl;
    return {out_positions, out_normals, out_uv};
}
