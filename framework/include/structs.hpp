#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include <map>
#include <glbinding/gl/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
// use gl definitions from glbinding 
using namespace gl;

// gpu representation of model
struct model_object {
  // vertex array object
  GLuint vertex_AO = 0;
  // vertex buffer object
  GLuint vertex_BO = 0;
  // index buffer object
  GLuint element_BO = 0;
  // primitive type to draw
  GLenum draw_mode = GL_NONE;
  // indices number, if EBO exists
  GLsizei num_elements = 0;
};

struct planet{

  std::string name;
  float size;
  glm::vec3 color;
  float rot_speed;
  float dist2origin;
  bool moon;

  planet(std::string n, float s, float r, float d, bool m = false):
        name{n},
        size{s},
        color{1.0f, 1.0f, 1.0f},
        rot_speed{r},
        dist2origin{d},
        moon{m} {};

  planet(std::string n, float s,  glm::vec3 c, float r, float d, bool m = false):
        name{n},
        size{s},
        color{c.x, c.y, c.z},
        rot_speed{r},
        dist2origin{d},
        moon{m} {};
};



// gpu representation of texture
struct texture_object {
  // handle of texture object
  GLuint handle = 0;
  // binding point
  GLenum target = GL_NONE;
};

// shader handle and uniform storage
struct shader_program {
  shader_program(std::string const& vertex, std::string const& fragment)
   :vertex_path{vertex}
   ,fragment_path{fragment}
   ,handle{0}
   {}

  // path to shader source
  std::string vertex_path; 
  std::string fragment_path; 
  // object handle
  GLuint handle;
  // uniform locations mapped to name
  std::map<std::string, GLint> u_locs{};
};
#endif