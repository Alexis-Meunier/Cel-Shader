#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#include "matrix.hh"
#include "object.hh"
#include "program.hh"
#include "object_data.hh"
#include "texture.hh"

#define WINDOW_HORIZ_MID 512
#define WINDOW_VERT_MID 512

#define TEST_OPENGL_ERROR()                                                             \
  do {									\
    GLenum err = glGetError();					                        \
    if (err != GL_NO_ERROR) std::cerr << "OpenGL ERROR!" << __LINE__ << "\n" << err << std::endl;      \
  } while(0)

program *prog = nullptr;
GLsizei skull_vertex_count = 0;
GLuint object_id = 0;

bool locked = true;

float camX = 0.0f, camY = 0.0f, camZ = -50.0f;
float horizAngl = 90.0f;
float vertAngl = 0.0f;
const float CAM_SPEED = 2.0f;
const float MOUSE_SENS = 0.2f;

bool mouseWarped = false;

int lastMouseX = WINDOW_HORIZ_MID;
int lastMouseY = WINDOW_VERT_MID;

void handleMouseLook(int x, int y) {
    const int CX = WINDOW_HORIZ_MID;
    const int CY = WINDOW_VERT_MID;

    if (mouseWarped) {
        mouseWarped = false;
        return;
    }

    float dx = (x - CX) * MOUSE_SENS;
    // We flip on y because the coordinates are inversed ?
    float dy = (CY - y) * MOUSE_SENS;

    horizAngl += dx;
    vertAngl += dy;
    // Clamp so we can't loop
    if (vertAngl >  89.0f) vertAngl =  89.0f;
    if (vertAngl < -89.0f) vertAngl = -89.0f;

    mouseWarped = true;
    glutWarpPointer(CX, CY);

    glutPostRedisplay();
}

void get_camera_dirs(float& fx, float& fy, float& fz,
                     float& rx, float& ry, float& rz)
{
    // get Angle as radians
    float radhorizAngl = horizAngl * M_PI / 180.0f;
    float radvertAngl = vertAngl * M_PI / 180.0f;

    // Forward vector
    fx = cos(radvertAngl) * cos(radhorizAngl);
    fy = sin(radvertAngl);
    fz = cos(radvertAngl) * sin(radhorizAngl);

    float len = sqrt(fz*fz + fx*fx);
    rx =  fz / len;
    ry =  0.0f;
    rz = -fx / len;
}

void handleKeyboard(unsigned char key, int x, int y) {
    float fx, fy, fz, rx, ry, rz;
    get_camera_dirs(fx, fy, fz, rx, ry, rz);

    switch (key) {
      // Z-Axis movement
        case 'w': camX += fx * CAM_SPEED; camY += fy * CAM_SPEED; camZ += fz * CAM_SPEED; break;
        case 's': camX -= fx * CAM_SPEED; camY -= fy * CAM_SPEED; camZ -= fz * CAM_SPEED; break;
      // X-Axis movement
        case 'd': camX -= rx * CAM_SPEED; camY -= ry * CAM_SPEED; camZ -= rz * CAM_SPEED; break;
        case 'a': camX += rx * CAM_SPEED; camY += ry * CAM_SPEED; camZ += rz * CAM_SPEED; break;
      // Y-Axis movement
        case 'q': camY += CAM_SPEED; break;
        case 'e': camY -= CAM_SPEED; break;
      // lock cursor when `space`
        case 32:
          locked = !locked;
          if (locked) {
              glutSetCursor(GLUT_CURSOR_NONE);
              glutPassiveMotionFunc(handleMouseLook);
              glutWarpPointer(512, 512);
          } else {
              glutSetCursor(GLUT_CURSOR_INHERIT);
              glutPassiveMotionFunc(nullptr);
          }
          break;
        case 13:
          camX = 0.0f, camY = 0.0f, camZ = -50.0f;
          horizAngl = 90.0f;
          vertAngl = 0.0f;
          break;
      // if `escape` then close window
        case 27: exit(0);
    }

    glutPostRedisplay();
}

void update_camera() {
    float fx, fy, fz, rx, ry, rz;
    get_camera_dirs(fx, fy, fz, rx, ry, rz);

    GLint camera_location = glGetUniformLocation(prog->program_id, "camera");
    auto camera_mat = look_at(
        camX, camY, camZ, // eye
        camX + fx, camY + fy, camZ + fz, // eye + forward
        0.0f, 1.0f, 0.0f // world up
    );

    glUniformMatrix4fv(camera_location, 1, GL_TRUE, camera_mat.get_values());
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);TEST_OPENGL_ERROR();
  update_camera();
  glBindVertexArray(object_id);TEST_OPENGL_ERROR();
  glDrawArrays(GL_TRIANGLES, 0, skull_vertex_count);TEST_OPENGL_ERROR();
  glBindVertexArray(0);TEST_OPENGL_ERROR();
  glutSwapBuffers();TEST_OPENGL_ERROR();

  // std::cout << "Finished display" << std::endl;
}

bool init_glut(int& argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitContextVersion(4,5);
    glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition ( 100, 100 );
    glutCreateWindow("Shader Programming");

    // Glut Callbacks
    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeyboard);
    glutPassiveMotionFunc(handleMouseLook);

    std::cout << "Finished init_glut" << std::endl;
    return true;
}

bool init_glew()
{
    glewExperimental=GL_TRUE;
    if (glewInit()) {
      std::cerr << " Error while initializing glew";
      return false;
    }
    glGetError();
    return true;
}

void init_textures()
{
  ImageInfo img = load_image("image.jpg");

  // Texture
  GLuint texture_id;
  glGenTextures(1, &texture_id);TEST_OPENGL_ERROR();

  // GLint sampler_id = 12;
  GLint sampler_id = glGetUniformLocation(prog->program_id, "kirby_sampler");TEST_OPENGL_ERROR();
  if (sampler_id == -1)
    std::cout << "Sampler error" << std::endl;
  glUniform1i(sampler_id, 0);TEST_OPENGL_ERROR();
  glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();

  std::cout << "size: " << img.width << "x" << img.height << std::endl;

  glBindTexture(GL_TEXTURE_2D, texture_id);TEST_OPENGL_ERROR();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_RGB,  GL_UNSIGNED_BYTE, img.pixels.data());TEST_OPENGL_ERROR();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);TEST_OPENGL_ERROR();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);TEST_OPENGL_ERROR();

  std::cout << "pixels: " << img.pixels.size() << std::endl;
}

void init_GL() {
  glEnable(GL_DEPTH_TEST);TEST_OPENGL_ERROR();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);TEST_OPENGL_ERROR();
  // glEnable(GL_CULL_FACE);TEST_OPENGL_ERROR();
  glClearColor(1.0, 0.0, 1.0, 1.0);TEST_OPENGL_ERROR();
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glPixelStorei(GL_PACK_ALIGNMENT,1);

  std::cout << "Finished init_GL" << std::endl;
}

bool init_shaders() {
  auto program_id = prog->program_id;
  auto vertex_id = prog->vertex_id;
  auto fragment_id = prog->fragment_id;

  glDetachShader(program_id, vertex_id);TEST_OPENGL_ERROR();
  glDetachShader(program_id, fragment_id);TEST_OPENGL_ERROR();

  glDeleteShader(vertex_id);TEST_OPENGL_ERROR();
  glDeleteShader(fragment_id);TEST_OPENGL_ERROR();

  glUseProgram(program_id);TEST_OPENGL_ERROR();
  std::cout << "Finished init_shaders" << std::endl;
  return true;
}

bool init_object() {
  if (prog == nullptr || !prog->is_ready())
    return false;

  GLuint vbo_ids[1];
  // GLint vertex_location = glGetAttribLocation(prog->program_id,"position");TEST_OPENGL_ERROR();
  // GLint color_location = glGetAttribLocation(prog->program_id,"color");TEST_OPENGL_ERROR();
  // GLint normal_flat_location = 10;
  // glBindAttribLocation(prog->program_id, normal_flat_location, "normalFlat");TEST_OPENGL_ERROR();
  // GLint normal_smooth_location = 11;
  // glBindAttribLocation(prog->program_id, normal_smooth_location, "normalSmooth");TEST_OPENGL_ERROR();
  // GLint uv_location = 12;
  // glBindAttribLocation(prog->program_id, uv_location, "uv");TEST_OPENGL_ERROR();
  // GLint normal_flat_location = glGetAttribLocation(prog->program_id,"normalFlat");TEST_OPENGL_ERROR();
  // GLint normal_smooth_location = glGetAttribLocation(prog->program_id,"normalSmooth");TEST_OPENGL_ERROR();
  // GLint uv_location = glGetAttribLocation(prog->program_id,"uv");TEST_OPENGL_ERROR();
  GLint vertex_location = 0;
  GLint color_location = 1;
  GLint normal_flat_location = 2;
  GLint normal_smooth_location = 3;
  GLint uv_location = 4;

  if (vertex_location == -1)
    std::cout << "Vertex location is -1 :(" << std::endl;
  if (color_location == -1)
    std::cout << "Color location is -1 :(" << std::endl;
  if (normal_flat_location == -1)
    std::cout << "Normal flat location is -1 :(" << std::endl;
  if (normal_smooth_location == -1)
    std::cout << "Normal Smooth location is -1 :(" << std::endl;
  if (uv_location == -1)
    std::cout << "uv location is -1 :(" << std::endl;
  
  objectData skull = LoadOBJ("../objects/test.obj", {0, 0, 100}, 1.0, {90, 0, 180});
  skull_vertex_count = skull.position.size() / 3;
  std::cout << "Loaded skull: " << skull.position.size() << " vertices" << std::endl;
  std::cout << "uv: " << skull.uv_position.size() << std::endl;

  // auto vertex_size = vertex_buffer_data.size() * sizeof(GLfloat);
  // auto normal_flat_size = normal_flat_buffer_data.size() * sizeof(GLfloat);
  // auto uv_size = uv_buffer_data.size() * sizeof(GLfloat);
  
  auto vertex_size = skull.position.size() * sizeof(GLfloat);
  auto normal_flat_size = skull.normals.size() * sizeof(GLfloat);
  auto color_size = color_buffer_data.size() * sizeof(GLfloat);
  auto normal_smooth_size = normal_smooth_buffer_data.size() * sizeof(GLfloat);
  auto uv_size = skull.uv_position.size() * sizeof(GLfloat);

  glGenBuffers(1, vbo_ids);TEST_OPENGL_ERROR();
  glGenVertexArrays(1, &object_id);TEST_OPENGL_ERROR();
  glBindVertexArray(object_id);TEST_OPENGL_ERROR();

  auto size = vertex_size + color_size + uv_size + normal_smooth_size + normal_flat_size;
  char *vbo = new char[size];
  std::memcpy(vbo, skull.position.data(), vertex_size);
  // std::memcpy(vbo, vertex_buffer_data.data(), vertex_size);
  std::memcpy(vbo + vertex_size, color_buffer_data.data(), color_size);
  // std::memcpy(vbo + vertex_size + color_size, normal_flat_buffer_data.data(), normal_flat_size);
  std::memcpy(vbo + vertex_size + color_size, skull.normals.data(), normal_flat_size);
  std::memcpy(vbo + vertex_size + color_size + normal_flat_size, normal_smooth_buffer_data.data(), normal_smooth_size);
  std::memcpy(vbo + vertex_size + color_size + normal_flat_size + normal_smooth_size, skull.uv_position.data(), uv_size);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[0]);TEST_OPENGL_ERROR();
  glBufferData(GL_ARRAY_BUFFER, size, vbo, GL_STATIC_DRAW);TEST_OPENGL_ERROR();
  
  glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, 0);TEST_OPENGL_ERROR();
  glVertexAttribPointer(color_location, 3, GL_FLOAT, GL_FALSE, 0, (void *)(vertex_size));TEST_OPENGL_ERROR();
  glVertexAttribPointer(normal_flat_location, 3, GL_FLOAT, GL_FALSE, 0, (void *)(vertex_size + color_size));TEST_OPENGL_ERROR();
  glVertexAttribPointer(normal_smooth_location, 3, GL_FLOAT, GL_FALSE, 0, (void *)(vertex_size + color_size + normal_flat_size));TEST_OPENGL_ERROR();
  glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, 0, (void *)(vertex_size + color_size + normal_flat_size + normal_smooth_size));TEST_OPENGL_ERROR();

  glEnableVertexAttribArray(vertex_location);TEST_OPENGL_ERROR();
  glEnableVertexAttribArray(color_location);TEST_OPENGL_ERROR();
  glEnableVertexAttribArray(normal_flat_location);TEST_OPENGL_ERROR();
  glEnableVertexAttribArray(normal_smooth_location);TEST_OPENGL_ERROR();
  glEnableVertexAttribArray(uv_location);TEST_OPENGL_ERROR();

  std::cout << "Finished init_object" << std::endl;
  return true;
}

bool init_POV() {

  GLint camera_location = glGetUniformLocation(prog->program_id, "camera");TEST_OPENGL_ERROR();
  GLint proj_location = glGetUniformLocation(prog->program_id, "projection");TEST_OPENGL_ERROR();
  GLint light_pos_location = glGetUniformLocation(prog->program_id, "light_pos");TEST_OPENGL_ERROR();
  GLint light_color_location = glGetUniformLocation(prog->program_id, "light_color");TEST_OPENGL_ERROR();

  // auto camera_mat = look_at(20, 20, 20,
  	      //  0, 0, 0,
  	      //  0, 1, 0
  	      //  );
  // auto proj_mat = frustum(-1, 1, -1, 1,
	    //   //  5, 50000
	    //  );
  // auto camera_mat = look_at(0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 3.0, 0.0);
  // auto proj_mat = frustum(-1.0, 1.0, -1.0, 1.0, 3.0, 30000.0);
  auto camera_mat = look_at(0.0, 0.0, -50.0,   // eye
                           0.0, 0.0,  100.0,  // look at skull
                           0.0, 1.0,  0.0);   // up
  auto proj_mat = frustum(-1.0, 1.0, -1.0, 1.0, 1.0, 500.0);
  glUniformMatrix4fv(camera_location, 1, GL_TRUE, camera_mat.get_values());TEST_OPENGL_ERROR();
  glUniformMatrix4fv(proj_location, 1, GL_TRUE, proj_mat.get_values());TEST_OPENGL_ERROR();
  glUniform3f(light_pos_location, 0.0, 50.0, -20.0);TEST_OPENGL_ERROR();
  glUniform3f(light_color_location, 0.34, 0.15, 1.0);TEST_OPENGL_ERROR();

  std::cout << "camera_location: " << camera_location << std::endl;
  std::cout << "proj_location: "   << proj_location   << std::endl;
  std::cout << "Finished init_POV" << std::endl;
  return true;
}

int main(int argc, char *argv[]) {
  //  tmp();
  init_glut(argc, argv);
  if (!init_glew())
    std::exit(-1);
  init_GL();

  std::cout << "making programs" << std::endl;
  prog = program::make_program("shaders/vertex.shd", "shaders/fragment.shd");
  if (prog == nullptr || !prog->is_ready())
    return 1;
  std::cout << "make programs" << std::endl;

  init_shaders();
  init_object();
  init_textures();
  init_POV();
  glutMainLoop();
}
