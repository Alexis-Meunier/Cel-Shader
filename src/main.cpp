#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>

#include "matrix.hh"
#include "object.hh"
#include "program.hh"
#include "object_data.hh"
#include "texture.hh"
#include "tiny_obj_loader.hh"

#define WINDOW_HORIZ_MID 512
#define WINDOW_VERT_MID 512

#define TEST_OPENGL_ERROR()                                                             \
  do {									\
    GLenum err = glGetError();					                        \
    if (err != GL_NO_ERROR) std::cerr << "OpenGL ERROR!" << __LINE__ << "\n" << err << std::endl;      \
  } while(0)

program *prog_edge = nullptr;
program* prog_cel_shading = nullptr;
GLsizei skull_vertex_count = 0;
GLuint object_id = 0;

bool is_smoothed = GL_FALSE;
bool is_toonShaded = GL_FALSE;
float scale = 1.0f;

std::string obj_dir;
std::string obj_file;

/*
"../objects/Other/source/Meshy_AI_Elven_Warrior_in_Gree_0512141219_texture_obj/"
../objects/Batman/2567_open3dmodel/Batman/
../objects/Woman/source/
../objects/Woman2/source/
../objects/Woman3/source/
*/

bool locked = true;

float camX = 0.0f, camY = 0.0f, camZ = -50.0f;
float lightX = 0.0f, lightY = 50.0f, lightZ = -20.0f;
float horizAngl = 90.0f;
float vertAngl = 0.0f;
GLfloat lineThickness = 0.03f;
GLfloat lineStep = 0.02f;
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
    GLint smooth_location;

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
        case 'u': lightX += fx * CAM_SPEED; lightY += fy * CAM_SPEED; lightZ += fz * CAM_SPEED; break;
        case 'j': lightX -= fx * CAM_SPEED; lightY -= fy * CAM_SPEED; lightZ -= fz * CAM_SPEED; break;
        case 'k': lightX -= rx * CAM_SPEED; lightY -= ry * CAM_SPEED; lightZ -= rz * CAM_SPEED; break;
        case 'h': lightX += rx * CAM_SPEED; lightY += ry * CAM_SPEED; lightZ += rz * CAM_SPEED; break;
        case 'y': lightY += CAM_SPEED; break;
        case 'i': lightY -= CAM_SPEED; break;
      // lock cursor when `space`
        case ' ':
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
        case '\t':
          is_smoothed = !is_smoothed;
          smooth_location = glGetUniformLocation(prog_cel_shading->program_id, "smoothed");TEST_OPENGL_ERROR();
          glUniform1i(smooth_location, is_smoothed);TEST_OPENGL_ERROR();
          break;
        case 't':
          is_toonShaded = !is_toonShaded;
          smooth_location = glGetUniformLocation(prog_cel_shading->program_id, "toonShaded");TEST_OPENGL_ERROR();
          glUniform1i(smooth_location, is_toonShaded);TEST_OPENGL_ERROR();
          break;
        case '+':
            lineThickness += lineStep;
            glUseProgram(prog_edge->program_id);
            smooth_location = glGetUniformLocation(prog_edge->program_id, "LineThickness");
            if (smooth_location == -1)
                std::cout << "LineThickness error" << std::endl;
            else
                glUniform1f(smooth_location, lineThickness);
            break;
        case '-':
            lineThickness -= lineStep;
            lineThickness = std::max(lineThickness, 0.0f);
            glUseProgram(prog_edge->program_id);
            smooth_location = glGetUniformLocation(prog_edge->program_id, "LineThickness");
            if (smooth_location == -1)
                std::cout << "LineThickness error" << std::endl;
            else
                glUniform1f(smooth_location, lineThickness);
            break;
      // if `escape` then close window
        case 27: exit(0);
    }

    glutPostRedisplay();
}

void update_camera() {
    float fx, fy, fz, rx, ry, rz;
    get_camera_dirs(fx, fy, fz, rx, ry, rz);

    auto camera_mat = look_at(
        camX, camY, camZ, // eye
        camX + fx, camY + fy, camZ + fz, // eye + forward
        0.0f, 1.0f, 0.0f // world up
    );

    // Update Cel Shading Program
    glUseProgram(prog_cel_shading->program_id);TEST_OPENGL_ERROR();
    GLint camera_location = glGetUniformLocation(prog_cel_shading->program_id, "camera");
    glUniformMatrix4fv(camera_location, 1, GL_TRUE, camera_mat.get_values());
    
    // Update Edge Program
    glUseProgram(prog_edge->program_id);TEST_OPENGL_ERROR();
    camera_location = glGetUniformLocation(prog_edge->program_id, "camera");
    glUniformMatrix4fv(camera_location, 1, GL_TRUE, camera_mat.get_values());
}

void update_light() {
    // Update Cel Shading Program
    glUseProgram(prog_cel_shading->program_id);TEST_OPENGL_ERROR();
    GLint light_pos_location = glGetUniformLocation(prog_cel_shading->program_id, "light_pos");
    glUniform3f(light_pos_location, lightX, lightY, lightZ);
    
    // Update Edge Program
    glUseProgram(prog_edge->program_id);TEST_OPENGL_ERROR();
    light_pos_location = glGetUniformLocation(prog_edge->program_id, "light_pos");
    glUniform3f(light_pos_location, lightX, lightY, lightZ);
}

void display() {
  // Initialization for both program 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);TEST_OPENGL_ERROR();
  update_camera();
  update_light();
  glBindVertexArray(object_id);TEST_OPENGL_ERROR();
  glEnable(GL_CULL_FACE);TEST_OPENGL_ERROR();
  
  // First draw to show black edges of the object
  glUseProgram(prog_edge->program_id);TEST_OPENGL_ERROR();
  glCullFace(GL_FRONT);TEST_OPENGL_ERROR();
  glDrawArrays(GL_TRIANGLES, 0, skull_vertex_count);TEST_OPENGL_ERROR();

  // Second draw to do cel shading on the object
  glUseProgram(prog_cel_shading->program_id);TEST_OPENGL_ERROR();
  glCullFace(GL_BACK);TEST_OPENGL_ERROR();
  glDrawArrays(GL_TRIANGLES, 0, skull_vertex_count);TEST_OPENGL_ERROR();

  // Swap for double buffering
  glBindVertexArray(0);TEST_OPENGL_ERROR();
  glutSwapBuffers();TEST_OPENGL_ERROR();
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

void init_textures(program *prog, const std::vector<tinyobj::material_t>& materials)
{
  for (size_t i = 0; i < materials.size(); i++) {
    if (materials[i].diffuse_texname.empty()) continue;
    std::string tex_path = obj_dir + materials[i].diffuse_texname;

    ImageInfo img = load_image(tex_path.c_str());

    GLuint texture_id;
    glGenTextures(1, &texture_id);TEST_OPENGL_ERROR();

    std::cout << "size: " << img.width << "x" << img.height << std::endl;

    std::string name = "textures[" + std::to_string(i) + "]";

    GLint sampler_id = glGetUniformLocation(prog->program_id, name.c_str());TEST_OPENGL_ERROR();
    if (sampler_id == -1)
      std::cout << "Sampler error" << std::endl;
    glUniform1i(sampler_id, i);TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0 + i);TEST_OPENGL_ERROR();


    glBindTexture(GL_TEXTURE_2D, texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_RGB,  GL_UNSIGNED_BYTE, img.pixels.data());TEST_OPENGL_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);TEST_OPENGL_ERROR();
  }
}

void init_GL() {
  glEnable(GL_DEPTH_TEST);TEST_OPENGL_ERROR();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);TEST_OPENGL_ERROR();
  glEnable(GL_CULL_FACE);TEST_OPENGL_ERROR();
  glClearColor(1.0, 0.0, 1.0, 1.0);TEST_OPENGL_ERROR();
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glPixelStorei(GL_PACK_ALIGNMENT,1);

  std::cout << "Finished init_GL" << std::endl;
}

bool init_shaders(program* prog) {
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

bool init_object(std::vector<tinyobj::material_t>& materials) {
  GLuint vbo_ids[1];

  // Cleaner way of doing it but for some reason it fails for some variables ???
  // GLint vertex_location = glGetAttribLocation(prog->program_id,"position");TEST_OPENGL_ERROR();

  GLint vertex_location = 0;
  GLint color_location = 1;
  GLint normal_flat_location = 2;
  GLint uv_location = 3;
  GLint material_ids_location = 4;

  // if (vertex_location == -1)
  //   std::cout << "Vertex location is -1 :(" << std::endl;
  // if (color_location == -1)
  //   std::cout << "Color location is -1 :(" << std::endl;
  // if (normal_flat_location == -1)
  //   std::cout << "Normal flat location is -1 :(" << std::endl;
  // if (uv_location == -1)
  //   std::cout << "uv location is -1 :(" << std::endl;
  tinyobj::attrib_t attributes;
  std::vector<tinyobj::shape_t> shapes;
  std::string warnings;
  std::string errors;
  tinyobj::LoadObj(&attributes, &shapes, &materials, &warnings, &errors, (obj_dir + obj_file).c_str(), obj_dir.c_str());

  std::vector<GLfloat> positions;
  std::vector<GLfloat> normals;
  std::vector<GLfloat> uv_positions;
  std::vector<GLuint> material_ids;


  for (int i = 0; i < shapes.size(); i ++) {
    tinyobj::shape_t &shape = shapes[i];
    tinyobj::mesh_t &mesh = shape.mesh;

    for (int j = 0; j < mesh.material_ids.size(); j++) {
      material_ids.push_back(mesh.material_ids[j]);
      material_ids.push_back(mesh.material_ids[j]);
      material_ids.push_back(mesh.material_ids[j]);
    }

    for (int j = 0; j < mesh.indices.size(); j++) {
        tinyobj::index_t i = mesh.indices[j];

        auto x = attributes.vertices[i.vertex_index * 3];
        auto y = attributes.vertices[i.vertex_index * 3 + 1];
        auto z = attributes.vertices[i.vertex_index * 3 + 2];

        auto x_normal = attributes.normals[i.normal_index * 3];
        auto y_normal = attributes.normals[i.normal_index * 3 + 1];
        auto z_normal = attributes.normals[i.normal_index * 3 + 2];

        auto rad = M_PI / 180.0;
        auto rotation = Point{0,180,0};

        auto rotated = Point(x * scale, y * scale, z * scale);
        if (std::abs(rotation.x) > 1e-3) rotated.rotateX(rotation.x * rad);
        if (std::abs(rotation.y) > 1e-3) rotated.rotateY(rotation.y * rad);
        if (std::abs(rotation.z) > 1e-3) rotated.rotateZ(rotation.z * rad);

        auto rotated_normal = Point(x_normal, y_normal, z_normal);
        if (std::abs(rotation.x) > 1e-3) rotated_normal.rotateX(rotation.x * rad);
        if (std::abs(rotation.y) > 1e-3) rotated_normal.rotateY(rotation.y * rad);
        if (std::abs(rotation.z) > 1e-3) rotated_normal.rotateZ(rotation.z * rad);
        
        positions.push_back(rotated.x);
        positions.push_back(rotated.y);
        positions.push_back(rotated.z);

        normals.push_back(rotated_normal.x);
        normals.push_back(rotated_normal.y);
        normals.push_back(rotated_normal.z);

        uv_positions.push_back(attributes.texcoords[i.texcoord_index * 2]);
        uv_positions.push_back(1 - attributes.texcoords[i.texcoord_index * 2 + 1]);
    }
  }
  
  //objectData skull = LoadOBJ("../objects/Other/source/Meshy_AI_Elven_Warrior_in_Gree_0512141219_texture_obj/Meshy_AI_Elven_Warrior_in_Gree_0512141219_texture.obj", {0, 0, 0}, 5.0, {0, 180, 0});
  skull_vertex_count = positions.size() / 3;
  std::cout << "count: " << skull_vertex_count << std::endl;

  std::vector<GLfloat> vertex_buffer_data = positions;
  std::vector<GLfloat> normal_flat_buffer_data = normals;
  std::vector<GLfloat> uv_buffer_data = uv_positions;

  auto vertex_size = vertex_buffer_data.size() * sizeof(GLfloat);
  auto color_size = color_buffer_data.size() * sizeof(GLfloat);
  auto normal_flat_size = normal_flat_buffer_data.size() * sizeof(GLfloat);
  auto uv_size = uv_buffer_data.size() * sizeof(GLfloat);
  auto material_ids_size = material_ids.size() * sizeof(GLuint);

  glGenBuffers(1, vbo_ids);TEST_OPENGL_ERROR();
  glGenVertexArrays(1, &object_id);TEST_OPENGL_ERROR();
  glBindVertexArray(object_id);TEST_OPENGL_ERROR();

  auto size = vertex_size + color_size + normal_flat_size + uv_size + material_ids_size;
  char *vbo = new char[size];
  std::memcpy(vbo, vertex_buffer_data.data(), vertex_size);
  std::memcpy(vbo + vertex_size, color_buffer_data.data(), color_size);
  std::memcpy(vbo + vertex_size + color_size, normal_flat_buffer_data.data(), normal_flat_size);
  std::memcpy(vbo + vertex_size + color_size + normal_flat_size, uv_positions.data(), uv_size);
  std::memcpy(vbo + vertex_size + color_size + normal_flat_size + uv_size, material_ids.data(), material_ids_size);


  glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[0]);TEST_OPENGL_ERROR();
  glBufferData(GL_ARRAY_BUFFER, size, vbo, GL_STATIC_DRAW);TEST_OPENGL_ERROR();
  
  glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, 0);TEST_OPENGL_ERROR();
  glVertexAttribPointer(color_location, 3, GL_FLOAT, GL_FALSE, 0, (void *)(vertex_size));TEST_OPENGL_ERROR();
  glVertexAttribPointer(normal_flat_location, 3, GL_FLOAT, GL_FALSE, 0, (void *)(vertex_size + color_size));TEST_OPENGL_ERROR();
  glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, 0, (void *)(vertex_size + color_size + normal_flat_size));TEST_OPENGL_ERROR();
  glVertexAttribIPointer(material_ids_location, 1, GL_INT, GL_FALSE, (void*)(vertex_size + color_size + normal_flat_size + uv_size));TEST_OPENGL_ERROR();

  glEnableVertexAttribArray(vertex_location);TEST_OPENGL_ERROR();
  glEnableVertexAttribArray(color_location);TEST_OPENGL_ERROR();
  glEnableVertexAttribArray(normal_flat_location);TEST_OPENGL_ERROR();
  glEnableVertexAttribArray(uv_location);TEST_OPENGL_ERROR();
  glEnableVertexAttribArray(material_ids_location);TEST_OPENGL_ERROR();

  std::cout << "Finished init_object" << std::endl;
  return true;
}

void init_materials(program* prog, const std::vector<tinyobj::material_t>& materials) {
    glUseProgram(prog->program_id);

    for (size_t i = 0; i < materials.size(); i++) {
        // Upload Kd
        std::string kd_name = "materials[" + std::to_string(i) + "].Kd";
        GLint kd_loc = glGetUniformLocation(prog->program_id, kd_name.c_str());
        if (kd_loc != -1)
            glUniform3f(kd_loc, materials[i].diffuse[0],
                                materials[i].diffuse[1],
                                materials[i].diffuse[2]);

        // Upload has_texture
        std::string ht_name = "materials[" + std::to_string(i) + "].has_texture";
        GLint ht_loc = glGetUniformLocation(prog->program_id, ht_name.c_str());
        bool has_tex = !materials[i].diffuse_texname.empty();
        if (ht_loc != -1)
            glUniform1i(ht_loc, has_tex ? 1 : 0);
    }
}

bool init_POV(program* prog) {

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
  glUniform3f(light_color_location, 1, 1, 1);TEST_OPENGL_ERROR();

  std::cout << "camera_location: " << camera_location << std::endl;
  std::cout << "proj_location: "   << proj_location   << std::endl;
  std::cout << "Finished init_POV" << std::endl;
  return true;
}

// void init_uniforms()
// {
//   GLint light_pos_location = glGetUniformLocation(prog->program_id, "light_pos");TEST_OPENGL_ERROR();
//   GLint light_color_location = glGetUniformLocation(prog->program_id, "light_color");TEST_OPENGL_ERROR();
//   // GLint nbImageColors_location = glGetUniformLocation(prog->program_id, "nbImageColors");TEST_OPENGL_ERROR();
//   // GLint nbLightColors_location = glGetUniformLocation(prog->program_id, "nbLightColors");TEST_OPENGL_ERROR();

//   glUniform3f(light_pos_location, -50.0, 50.0, 100.0);TEST_OPENGL_ERROR(); // The position of the light
//   glUniform3f(light_color_location, 1, 1, 1);TEST_OPENGL_ERROR(); // The color of the light
//   // glUniform1f(nbImageColors_location, 6.0); TEST_OPENGL_ERROR(); // The number of colors for the image of our cel-shading
//   // glUniform1f(nbLightColors_location, 8.0); TEST_OPENGL_ERROR(); // The number of colors for the light of our cel-shading

//   std::cout << "Finished init_uniforms" << std::endl;
// }

int main(int argc, char *argv[]) {
  if (argc != 5) {
    std::cerr << "CelShading [WITH_TEXTURE] [DIRECTORY] [FILE] [SCALE]" << std::endl;
    return 1;
  }

  std::string with_texture = argv[1];
  obj_dir = argv[2];
  obj_dir += "/";
  obj_file = argv[3];
  scale = std::stof(argv[4]);

  init_glut(argc, argv);
  if (!init_glew())
    std::exit(-1);
  init_GL();

  std::cout << "making programs" << std::endl;

  // Create first program with the edge shaders
  prog_edge = program::make_program("shaders/vertex_edge.shd", "shaders/fragment_edge.shd");
  if (prog_edge == nullptr || !prog_edge->is_ready())
    return 1;

  // Create second program with the cel shading shaders
  if (with_texture == "0") {
    prog_cel_shading = program::make_program("shaders/vertex_cel_shading_without_texture.shd", "shaders/fragment_cel_shading_without_texture.shd");
  } else {
    prog_cel_shading = program::make_program("shaders/vertex_cel_shading.shd", "shaders/fragment_cel_shading.shd");
  }
  
  if (prog_cel_shading == nullptr || !prog_cel_shading->is_ready())
    return 1;

  std::cout << "make programs" << std::endl;
  
  // Initialization for the edge program
  init_shaders(prog_edge);
  init_POV(prog_edge);

  // Initialization for both program (exact same data)
  std::vector<tinyobj::material_t> materials;
  init_object(materials);
  
  // Initialization for the cel shading program
  init_shaders(prog_cel_shading);
  init_POV(prog_cel_shading);
  init_materials(prog_cel_shading, materials);  // always
  // init_textures(prog_cel_shading, materials);
  if (with_texture != "0") {
    init_textures(prog_cel_shading, materials);
  }

  glutMainLoop();
}
