#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"

// gpu representation of model
class ApplicationSolar : public Application {
 public:
  // defining struct with properties for the planets
 
  // allocate and initialize objects
  ApplicationSolar(std::string const& resource_path);
  // free allocated objects
  ~ApplicationSolar();

  // update uniform locations and values
  void uploadUniforms();
  // update projection matrix
  void updateProjection();
  // react to key input
  void keyCallback(int key, int scancode, int action, int mods);
  //handle delta mouse movement input
  void mouseCallback(double pos_x, double pos_y);
  //void glfwGetCursorPos( double xpos, double ypos);
  // draw all objects
  void render() const;
  void upload_planet_transforms(planet  &p,  texture_object const& tex_obj) const;
  void initializeShaderPrograms();
  void initialize_screenquad();
  void initialize_framebuffer();

 protected:
  
  void initializeGeometry(model& mdl, model_object& object);
  void updateView();
  // cpu representation of model
  model_object planet_object;
  model_object star_object;
  model_object screen_quad_object;
  texture_object squad{};

};

#endif