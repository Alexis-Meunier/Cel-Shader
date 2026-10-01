#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

class program
{
public:
    program();
    ~program();
    static program *make_program(const std::string& vertex_shader_src, const std::string& fragment_shader);
    char *get_log();
    bool is_ready();
    void use();
    void add_logs(char *logs);
    void set_ready();

    GLint vertex_id;
    GLint fragment_id;
    GLint program_id;

private:
    bool _is_ready;
    bool _is_active;
    std::string _logs;
};
