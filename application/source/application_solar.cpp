#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"
#include "structs.hpp"


#include <glbinding/gl/gl.h>
// use gl definitions from glbinding
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <iostream>


model planet_model{};
model star_model{};

struct quadOb {
    GLuint vertex_AO = 0;
    GLuint vertex_BO = 0;
    GLuint element_BO = 0;
};

struct fb_texOb {
    GLenum context = GL_TEXTURE0;
    GLenum target = GL_TEXTURE_2D;
    GLuint obj_ptr = 0;
};


fb_texOb screen_QuadTexture;
quadOb screen_QuadObj;
GLuint rb_handle;
GLuint fbo_handle;
int width_ = 640;
int height_ = 480;

bool luminance = false;
bool horizontalmirror = false;
bool verticalmirror = false;
bool gaussian = false;

int stars_number= 4000;
std::string shader = "planet";


//forward initialization

void upload_planet_transforms(planet &p,  texture_object const& tex_obj);
void render();
void updateView();
void updateProjection();
void uploadUniforms();
void keyCallback(int key, int scancode, int action, int mods);
void mouseCallback(double pos_x, double pos_y);
void initializeShaderPrograms();
void initializeGeometry(model& mdl, model_object& object);


// initializing of planets
 //                                 name{n}, size{s},   color,               rot_speed{r}, dist2origin{d},moon{m} {};*/
planet sun       {"sun"    ,   0.7f,  {1.0f, 0.8f , 0.0f}, 0.0f,         0.0f,       };
planet mercury   {"mercury",   0.05f, {0.5f, 0.5f , 0.5f}, 365/88.0f,    15.0f,      };
planet venus     {"venus"  ,   0.2f,  {0.5f, 0.5f , 0.5f}, 365/225.0f,   18.0f, true };
planet earth     {"earth"  ,   0.15f, {0.1f, 0.4f , 0.7f}, 1.0f,         21.0f, true };
planet mars      {"mars"   ,   0.1f,  {0.8f, 0.6f , 0.3f}, 365/687.0f,   26.0f, true };
planet jupiter   {"jupiter",   0.35f, {0.8f, 0.8f , 0.5f}, 365/4329.f,   31.0f,      };
planet saturn    {"saturn" ,   0.2f,  {0.9f, 0.7f , 0.5f}, 365/1751.0f,  36.0f, true };
planet uranus    {"uranus" ,   0.2f,  {0.5f, 1.0f , 1.0f}, 365/30664.0f, 40.0f,      };
//ApplicationSolar::planet skydome   {"skydome",   50.0f, {0.0f, 0.0f, 0.0f},  0.0f,           0.0f,       };


//container for planets
std::vector<planet> planetVector = {sun,mercury,venus,earth, mars, jupiter, saturn, uranus};
std::vector<texture_object> textureVector;

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}, star_object{}
{

  std::string textureDir = resource_path + "textures/";
  // Load texture maps
  //auto skyTexture   = utils::create_texture_object(texture_loader::file(textureDir + "galaxymap.png"));
  auto sunTexture      = utils::create_texture_object(texture_loader::file(textureDir + "sunmap.png"));
  auto mercuryTexture  = utils::create_texture_object(texture_loader::file(textureDir + "mercurymap.png"));
  auto venusTexture    = utils::create_texture_object(texture_loader::file(textureDir + "venusmap.png"));
  auto earthTexture    = utils::create_texture_object(texture_loader::file(textureDir + "earthmap.png"));
  auto marsTexture     = utils::create_texture_object(texture_loader::file(textureDir + "marsmap.png"));
  auto jupiterTexture  = utils::create_texture_object(texture_loader::file(textureDir + "jupitermap.png"));
  auto saturnTexture   = utils::create_texture_object(texture_loader::file(textureDir + "saturnmap.png"));
  auto uranusTexture   = utils::create_texture_object(texture_loader::file(textureDir + "uranusmap.png"));
  auto moonTexture     = utils::create_texture_object(texture_loader::file(textureDir + "moonmap.png"));


  //textureVector.push_back(skyTexture);
  textureVector.push_back(sunTexture);
  textureVector.push_back(mercuryTexture);
  textureVector.push_back(venusTexture);
  textureVector.push_back(earthTexture);
  textureVector.push_back(marsTexture);
  textureVector.push_back(jupiterTexture);
  textureVector.push_back(saturnTexture);
  textureVector.push_back(uranusTexture);
  textureVector.push_back(moonTexture);



  //container for all star coordinates
  std::vector<float> starVector;
  // random functions for random star positions
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis1(-30, 30);
  std::uniform_real_distribution<float> dis2(0.5, 1.0);

  for(int j = 0; j< stars_number; j++){

        starVector.push_back( dis1(gen) );
        starVector.push_back( dis1(gen) );
        starVector.push_back( dis1(gen) );
        starVector.push_back( 1.0f );
        starVector.push_back( 1.0f );
        starVector.push_back( dis2(gen) );
  }

  star_model  = {starVector, model::POSITION | model::NORMAL};
  planet_model = model_loader::obj(resource_path + "models/sphere.obj", model::NORMAL | model::TEXCOORD);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  initializeGeometry(star_model, star_object);
  initializeGeometry(planet_model, planet_object);

  initializeShaderPrograms();
  initialize_screenquad();

}

void ApplicationSolar::initialize_screenquad() {

  std::vector<float> vertices {
          -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // v1
           1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // v2
          -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // v4
           1.0f,  1.0f, 0.0f, 1.0f, 1.0f  // v3
  };

  std::vector<GLuint> indices {
        0, 1, 2,
        0, 2, 3
    };

    auto num_bytes = 5 * sizeof(GLfloat);
    glGenVertexArrays(1, &screen_QuadObj.vertex_AO);
    glBindVertexArray(screen_QuadObj.vertex_AO);
    glGenBuffers(1, &screen_QuadObj.vertex_BO);
    glBindBuffer(GL_ARRAY_BUFFER, screen_QuadObj.vertex_BO);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(GLsizei(sizeof(float) * vertices.size())), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    uintptr_t offset0 = 0 * sizeof(GLfloat);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GLsizei(num_bytes), (const GLvoid*) offset0);
    glEnableVertexAttribArray(1);
    uintptr_t offset1 = 3 * sizeof(GLfloat);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, GLsizei(num_bytes), (const GLvoid*) offset1);
}




void ApplicationSolar::upload_planet_transforms(planet &p,  texture_object const& tex_obj) const{


  if(shader == "planet"){

    glUseProgram(m_shaders.at(shader).handle);

    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_obj.handle);
    glUniform1i(m_shaders.at(shader).u_locs.at("ColorTex"), tex_obj.handle);
  }


  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime() * p.rot_speed), glm::fvec3{0.0f, 1.0f, 0.0f});

  // set the size for the planets
  model_matrix = glm::scale(model_matrix, glm::vec3(p.size));

  //determine the distance of the planets to the origin
  model_matrix = glm::translate(model_matrix, glm::fvec3{p.dist2origin,0.0f, -1.0f}); // here change for distance and size



  glUniformMatrix4fv(m_shaders.at(shader).u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  glUniformMatrix4fv(m_shaders.at(shader).u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));

  // Setting the colors for the planets
  glUniform3f(m_shaders.at(shader).u_locs.at("ColorVector"), p.color.x, p.color.y, p.color.z);


  // bind the VAO to draw
  glBindVertexArray(planet_object.vertex_AO);

  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);

  if(p.moon)
  {

    if(shader == "planet"){

      // bind shader to upload uniforms
      glUseProgram(m_shaders.at(shader).handle);

      glActiveTexture(GL_TEXTURE9);
      glBindTexture(GL_TEXTURE_2D, textureVector[8].handle);
      glUniform1i(m_shaders.at(shader).u_locs.at("ColorTex"), textureVector[8].handle);
    }



    // Using the already used model_matrix from above instantly
    model_matrix = glm::rotate(model_matrix,  float(glfwGetTime() * 5.0f), glm::fvec3{0.0f, 1.0f, 0.0f});
    model_matrix = glm::translate(model_matrix,  glm::fvec3{1.5f,0.0f, -1.0f});
    model_matrix = glm::scale(model_matrix, glm::vec3(0.35f));

    glUniformMatrix4fv(m_shaders.at(shader).u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));

    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
    glUniformMatrix4fv(m_shaders.at(shader).u_locs.at("NormalMatrix"),
                       1, GL_FALSE, glm::value_ptr(normal_matrix));

    glUniform3f(m_shaders.at(shader).u_locs.at("ColorVector"), 1.0f, 1.0f, 0.0f);

    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }

}




void ApplicationSolar::render() const {

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);

  glClearColor(0.0, 0.0, 0.0,0.0);
  glClearDepth(1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


   // draw stars
  glUseProgram(m_shaders.at("star").handle);
  glBindVertexArray(star_object.vertex_AO);
  glDrawArrays(GL_POINTS, 0, stars_number);

  // bind the VAO to draw
  glUseProgram(m_shaders.at(shader).handle);
  glBindVertexArray(planet_object.vertex_AO);

  int index_= 1;

    //glDisable(GL_DEPTH_TEST);
    //glEnable(GL_DEPTH_TEST);

    std::vector<texture_object>::iterator j = textureVector.begin();
    for (std::vector<planet>::iterator i = planetVector.begin(); i != planetVector.end(); ++i){

      glActiveTexture(GL_TEXTURE0+index_);
      upload_planet_transforms(*i, *j);
      ++j;
      ++index_;
   }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClearDepth(1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // draw screen squad
  glUseProgram(m_shaders.at("screenquad").handle);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, screen_QuadTexture.obj_ptr);
  glUniform1i(m_shaders.at("screenquad").u_locs.at("ColorTex"), 0);
  glBindVertexArray(screen_QuadObj.vertex_AO);
  utils::validate_program(m_shaders.at("screenquad").handle);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}



void ApplicationSolar::updateView() {
    glUseProgram(m_shaders.at(shader).handle);
    initialize_rendBuffer();
    initialize_frameBuffer();
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);


  // add light vector to sun
  glm::vec4 sun = {0.0f, 0.0f, 0.0f, 1.0f};
  sun = view_matrix * sun;
  glUniform3f(m_shaders.at(shader).u_locs.at("LightSource"), sun.x, sun.y, sun.z);

  glUseProgram(m_shaders.at(shader).handle);

  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at(shader).u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("star").handle);

  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at(shader).handle);
  glUseProgram(m_shaders.at("screenquad").handle);
  glUniformMatrix4fv(m_shaders.at("screenquad").u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
  glUniform2f(m_shaders.at("screenquad").u_locs.at("resoulotion"), GLfloat(width_), GLfloat(height_));

}

void ApplicationSolar::updateProjection() {


  // upload matrix to gpu
  glUseProgram(m_shaders.at(shader).handle);

  glUniformMatrix4fv(m_shaders.at(shader).u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("star").handle);

  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
  glUseProgram(m_shaders.at(shader).handle);

}


// update uniform locations
void ApplicationSolar::uploadUniforms() {

  updateUniformLocations();
  // bind new shader
  glUseProgram(m_shaders.at(shader).handle);
  updateView();
  updateProjection();
}




// handle key input
void ApplicationSolar::keyCallback(int key, int scancode, int action, int mods) {

  if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.1f});
    updateView();
  }
  else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.1f});
    updateView();
  }
  else if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS|| action == GLFW_REPEAT)){
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{1.0f, 0.0f, 0.0f});
    updateView();
  }
  else if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{-1.0f, 0.0f, 0.0f});
    updateView();
  }
  else if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)){
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, -1.0f, 0.0f});
    updateView();
  }
  else if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)){
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 1.0f, 0.0f});
    updateView();
  }
  else if(key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT)){
    m_view_transform = glm::rotate(m_view_transform, 0.05f, glm::fvec3{0.0f, -1.0f, 0.0f});
    updateView();
  }
  else if(key == GLFW_KEY_V && (action == GLFW_PRESS || action == GLFW_REPEAT)){
    m_view_transform = glm::rotate(m_view_transform, 0.05f, glm::fvec3{0.0f, 1.0f, 0.0f});
    updateView();
  }

  // 1 = enable Blinn-Phong
  else if(key == GLFW_KEY_1 && action == GLFW_PRESS) {
    shader = "planet";
    initializeShaderPrograms();
    uploadUniforms();
  }
  // 2 = enable cel-Shading
  else if(key == GLFW_KEY_2 && action == GLFW_PRESS) {
    shader = "cel";
    initializeShaderPrograms();
    uploadUniforms();
  }
  // luminance preserving greyscale image
  else if(key == GLFW_KEY_7 && action == GLFW_PRESS) {
    luminance = !luminance;
    glUseProgram(m_shaders.at("screenquad").handle);
    glUniform1i(m_shaders.at("screenquad").u_locs.at("Luminance"), luminance);
  }
  // horizontally mirrored image
  else if(key == GLFW_KEY_8 && action == GLFW_PRESS) {
    horizontalmirror = !horizontalmirror;
    glUseProgram(m_shaders.at("screenquad").handle);
    glUniform1i(m_shaders.at("screenquad").u_locs.at("HorizontalMirror"), horizontalmirror);
  }
  // vertically mirrored image
  else if(key == GLFW_KEY_9 && action == GLFW_PRESS) {
    verticalmirror = !verticalmirror;
    glUseProgram(m_shaders.at("screenquad").handle);
    glUniform1i(m_shaders.at("screenquad").u_locs.at("VerticalMirror"), verticalmirror);
  }
  //blurred image with 3x3 gaussian kernel
  else if(key == GLFW_KEY_0 && action == GLFW_PRESS) {
    gaussian = !gaussian;
    glUseProgram(m_shaders.at("screenquad").handle);
    glUniform1i(m_shaders.at("screenquad").u_locs.at("Gaussian"), gaussian);
  }
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  // mouse handling

  //zooming in by moving the mouse up
  if ( pos_x>0 && pos_y <0 ) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.1f});
  }
  // zooming out by moving the mouse down
  else if ( pos_x<0 && pos_y >0){

    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.1f});
  }
  updateView();
}

// load shader programs
void ApplicationSolar::initializeShaderPrograms() {



  // store shader program objects in container
  m_shaders.emplace("star", shader_program{m_resource_path + "shaders/stars.vert",
                                           m_resource_path + "shaders/stars.frag"});

  // request uniform locations for shader program
  m_shaders.at("star").u_locs["ViewMatrix"] = -1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;


  m_shaders.emplace("planet", shader_program{m_resource_path + "shaders/simple.vert",
                                           m_resource_path + "shaders/simple.frag"});

  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet").u_locs["ColorVector"] = -1;
  m_shaders.at("planet").u_locs["LightSource"] = -1;
  m_shaders.at("planet").u_locs["ColorTex"] = -1;

  m_shaders.emplace("cel", shader_program{m_resource_path + "shaders/cel.vert",
                                           m_resource_path + "shaders/cel.frag"});

  // request uniform locations for shader program
  m_shaders.at("cel").u_locs["NormalMatrix"] = -1;
  m_shaders.at("cel").u_locs["ModelMatrix"] = -1;
  m_shaders.at("cel").u_locs["ViewMatrix"] = -1;
  m_shaders.at("cel").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("cel").u_locs["ColorVector"] = -1;
  m_shaders.at("cel").u_locs["LightSource"] = -1;
  m_shaders.emplace("screenquad", shader_program{m_resource_path + "shaders/screenquad.vert",
                                           m_resource_path + "shaders/screenquad.frag"});
  //initialize screen squad shader
  m_shaders.at("screenquad").u_locs["ColorTex"] = -1;
  m_shaders.at("screenquad").u_locs["ModelMatrix"] = -1;
  m_shaders.at("screenquad").u_locs["ViewMatrix"] = -1;
  m_shaders.at("screenquad").u_locs["Luminance"] = -1;
  m_shaders.at("screenquad").u_locs["resoulotion"] = -1;
  m_shaders.at("screenquad").u_locs["HorizontalMirror"] = -1;
  m_shaders.at("screenquad").u_locs["VerticalMirror"] = -1;
  m_shaders.at("screenquad").u_locs["Gaussian"] = -1;

}
void ApplicationSolar::initialize_rendBuffer(){
    glGenRenderbuffers(1, &rb_handle);
    glBindRenderbuffer(GL_RENDERBUFFER, rb_handle);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24, width_,height_);
}

void ApplicationSolar::initialize_frameBuffer(){
    glGenTextures(1, &screen_QuadTexture.obj_ptr);
    glBindTexture(GL_TEXTURE_2D, screen_QuadTexture.obj_ptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    glTexImage2D(GL_TEXTURE_2D, 0, GLint(GL_RGBA8), width_, height_, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    glGenFramebuffers(1, &fbo_handle);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle);

    glFramebufferTexture(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,screen_QuadTexture.obj_ptr,0);

    glFramebufferRenderbuffer(
                              GL_FRAMEBUFFER,
                              GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER_EXT,
                              rb_handle
                              );

    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers);

}

// load models
void ApplicationSolar::initializeGeometry(model& mdl, model_object& object) {

    // generate vertex array object
    glGenVertexArrays(1, &object.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(object.vertex_AO);

    // generate generic buffer
    glGenBuffers(1, &object.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, object.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mdl.data.size(), mdl.data.data(), GL_STATIC_DRAW);

    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, mdl.vertex_bytes, mdl.offsets[model::POSITION]);
    // activate second attribute on gpu
    glEnableVertexAttribArray(1);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, mdl.vertex_bytes, mdl.offsets[model::NORMAL]);
    // activate third attribute on gpu
    glEnableVertexAttribArray(2);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(2, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, mdl.vertex_bytes, mdl.offsets[model::TEXCOORD]);

     // generate generic buffer
    glGenBuffers(1, &object.element_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.element_BO);
    // configure currently bound array buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * mdl.indices.size(), mdl.indices.data(), GL_STATIC_DRAW);

    // store type of primitive to draw
    object.draw_mode = GL_TRIANGLES;
    // transfer number of indices to model object
    object.num_elements = GLsizei(mdl.indices.size());
}



ApplicationSolar::~ApplicationSolar() {

  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);

  glDeleteBuffers(1, &star_object.vertex_BO);
  glDeleteBuffers(1, &star_object.element_BO);
  glDeleteVertexArrays(1, &star_object.vertex_AO);

  glDeleteBuffers(1, &screen_quad_object.vertex_BO);
  glDeleteBuffers(1, &screen_quad_object.element_BO);
  glDeleteVertexArrays(1, &screen_quad_object.vertex_AO);

}



// exe entry point
int main(int argc, char* argv[]) {

  Launcher::run<ApplicationSolar>(argc, argv);
}
