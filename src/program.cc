#include "program.hh"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <fstream>

#define TEST_OPENGL_ERROR()                                                             \
  do {									\
    GLenum err = glGetError();					                        \
    if (err != GL_NO_ERROR) std::cerr << "OpenGL ERROR!" << __LINE__ << "\n" << err << std::endl;      \
  } while(0)


program::program()
{
    _is_active = false;
    _is_ready = false;
}

program::~program()
{

}

void program::add_logs(char *logs)
{
    if (_logs.length() != 0)
        _logs += "\n";
    _logs += std::string(logs);
}

std::string load(const std::string &filename) {
  std::ifstream input_src_file(filename, std::ios::in);
  std::string ligne;
  std::string file_content="";
  if (input_src_file.fail()) {
    std::cerr << "FAILURE: can not load " << filename << "\n";
    return "";
  }
  while(getline(input_src_file, ligne)) {
    file_content = file_content + ligne + "\n";
  }
  file_content += '\0';
  input_src_file.close();
  return file_content;
}

void program::set_ready()
{
    _is_ready = true;
}

program *program::make_program(const std::string& vertex_shader_src, const std::string& fragment_shader_src)
{
    static program *prog = new program();

    // Compile VERTEX shader
    GLint vertex_compile_status = GL_TRUE;
    GLuint vertex_id = glCreateShader(GL_VERTEX_SHADER);TEST_OPENGL_ERROR();

    std::string vertex_src = load(vertex_shader_src);
    const GLchar *vertex_sources[1];
    vertex_sources[0] = vertex_src.c_str();

    glShaderSource(vertex_id, 1, vertex_sources, 0);TEST_OPENGL_ERROR();
    glCompileShader(vertex_id);TEST_OPENGL_ERROR();
    glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &vertex_compile_status);TEST_OPENGL_ERROR();
    if (vertex_compile_status != GL_TRUE)
    {
        std::cout << "no compilation" << std::endl;
        GLint vertex_log_size;
        glGetShaderiv(vertex_id, GL_INFO_LOG_LENGTH, &vertex_log_size);TEST_OPENGL_ERROR();
        char *logs = new char[vertex_log_size+1];
        if(logs != nullptr) {
            glGetShaderInfoLog(vertex_id, vertex_log_size, &vertex_log_size, logs);TEST_OPENGL_ERROR();
            std::cout << "got logs" << std::endl;
            prog->add_logs(logs);
            std::cout << "vertex: " << prog->get_log() << std::endl;
            std::cout << "added logs" << std::endl;
            std::free(logs);
        }

        return nullptr;
    }

    // Compile FRAGMENT shader
    GLint fragment_compile_status = GL_TRUE;
    GLuint fragment_id = glCreateShader(GL_FRAGMENT_SHADER);

    std::string fragment_src = load(fragment_shader_src);
    const GLchar *fragment_sources[1];
    fragment_sources[0] = fragment_src.c_str();

    glShaderSource(fragment_id, 1, fragment_sources, 0);TEST_OPENGL_ERROR();
    glCompileShader(fragment_id);TEST_OPENGL_ERROR();
    glGetShaderiv(fragment_id, GL_COMPILE_STATUS, &fragment_compile_status);TEST_OPENGL_ERROR();
    if (fragment_compile_status != GL_TRUE)
    {
        GLint fragment_log_size;
        glGetShaderiv(fragment_id, GL_INFO_LOG_LENGTH, &fragment_log_size);TEST_OPENGL_ERROR();
        char *logs = new char[fragment_log_size+1];
        if(logs != nullptr) {
            glGetShaderInfoLog(fragment_id, fragment_log_size, &fragment_log_size, logs);TEST_OPENGL_ERROR();
            prog->add_logs(logs);
            std::cout << "fragment: " << logs << std::endl;
            std::free(logs);
        }

        return nullptr;
    }

    // Editer de lien
    GLint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_id);TEST_OPENGL_ERROR();
    glAttachShader(program_id, fragment_id);TEST_OPENGL_ERROR();

    glLinkProgram(program_id);TEST_OPENGL_ERROR();

    // Compiler le program
    GLint program_compile_status = GL_TRUE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &program_compile_status);TEST_OPENGL_ERROR();
    if (program_compile_status != GL_TRUE)
    {
        GLint program_log_size;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &program_log_size);TEST_OPENGL_ERROR();
        char *logs = new char[program_log_size+1];
        if (logs != nullptr)
        {
            glGetProgramInfoLog(program_id, program_log_size, &program_log_size, logs);TEST_OPENGL_ERROR();
            prog->add_logs(logs);
            std::cout<< "program: "  << logs << std::endl;
            std::free(logs);
        }

        return nullptr;
    }

    prog->vertex_id = vertex_id;
    prog->fragment_id = fragment_id;
    prog->program_id = program_id;
    prog->set_ready();

    return prog;
}

char *program::get_log()
{
    return _logs.data();
}

bool program::is_ready()
{
    return _is_ready;
}

void program::use()
{
    _is_active = true; // idk man
}

