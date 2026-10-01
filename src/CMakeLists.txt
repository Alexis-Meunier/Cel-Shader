add_executable(CelShading
    utils/matrix.cc program/program.cc utils/point.cc object/object.cc texture/texture.cc main.cpp
)

target_include_directories(CelShading PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(CelShading PRIVATE
    -Wall -Wextra -O3 -g -m64 -march=native -fopt-info-vec-optimized
)

target_link_libraries(CelShading PRIVATE GL GLEW glut pthread)