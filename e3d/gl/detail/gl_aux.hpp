#pragma once

#include <GL/glew.h>

#include <iostream>
#include <string>
#include <vector>

namespace gl
{
  void check_for_errors(std::ostream * os);
  GLuint create_shader(const std::string & src, GLenum shader_type, std::ostream * os);
  GLuint create_program(const std::vector<GLenum> & shaders, std::ostream * os);
  std::vector< GLenum > load_shaders(const std::string shaders_path);
}