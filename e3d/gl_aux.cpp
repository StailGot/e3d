#include "gl_aux.hpp"

#include <boost/range/adaptor/transformed.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/adaptor/filtered.hpp>

#include <fstream>
#include <filesystem>

namespace gl
{

  namespace detail
  {
    template<typename Fn>
    auto get_iv(GLuint obj, GLenum pname, Fn fn)
    {
      GLint result{};
      fn(obj, pname, &result);
      return result;
    }
  }

  void check_for_errors(std::ostream * os)
  {
    std::string error_message;

    if (os)
      switch (::glGetError())
      {
      case GL_NO_ERROR: break;
      case GL_INVALID_ENUM: error_message = "GL_INVALID_ENUM"; break;
      case GL_INVALID_VALUE: error_message = "GL_INVALID_VALUE"; break;
      case GL_INVALID_OPERATION: error_message = "GL_INVALID_OPERATION"; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION: error_message = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
      case GL_OUT_OF_MEMORY: error_message = "GL_OUT_OF_MEMORY"; break;
      case GL_STACK_UNDERFLOW: error_message = "GL_STACK_UNDERFLOW"; break;
      case  GL_STACK_OVERFLOW: error_message = "GL_STACK_OVERFLOW"; break;
      }

    if (os && !error_message.empty())
      *os << error_message << "\n";
  }

  GLuint create_shader(const std::string & src, GLenum shader_type, std::ostream * os)
  {
    GLuint shader = ::glCreateShader(shader_type);
    auto ptr = src.c_str();
    ::glShaderSource(shader, 1, &ptr, nullptr);
    ::glCompileShader(shader);

    if (detail::get_iv(shader, GL_COMPILE_STATUS, ::glGetShaderiv) == GL_FALSE)
    {
      if (os)
        if (GLint length = detail::get_iv(shader, GL_INFO_LOG_LENGTH, ::glGetShaderiv))
        {
          std::vector<GLchar> info_log(length);
          ::glGetShaderInfoLog(shader, static_cast<GLsizei>(std::size(info_log)), &length, std::data(info_log));
          *os << std::data(info_log) << "\n";
        }
      ::glDeleteShader(shader);
      shader = {};
    }
    return shader;
  }

  GLuint create_program(const std::vector<GLenum>& shaders, std::ostream * os)
  {
    GLuint program = ::glCreateProgram();
    for (auto && shader : shaders)
      ::glAttachShader(program, shader)
      //, ::glDeleteProgram( program )
      ;

    ::glLinkProgram(program);

    if (detail::get_iv(program, GL_LINK_STATUS, ::glGetProgramiv) == GL_FALSE)
    {
      if (os)
        if (GLint length = detail::get_iv(program, GL_INFO_LOG_LENGTH, ::glGetProgramiv))
        {
          std::vector<GLchar> info_log(length);
          ::glGetProgramInfoLog(program, static_cast<GLsizei>(std::size(info_log)), &length, std::data(info_log));
          *os << std::data(info_log) << "\n";
        }
      ::glDeleteProgram(program);
      program = {};
    }
    return program;
  }


  namespace detail
  {
    boost::optional<GLenum> get_shader_type(const std::string & shader_name)
    {
      static std::pair<std::string, GLenum> types[] =
      {
        { "_vs", GL_VERTEX_SHADER }
        ,{ "_fs", GL_FRAGMENT_SHADER }
        ,{ "_cs", GL_COMPUTE_SHADER }
        ,{ "_gs", GL_GEOMETRY_SHADER }
        ,{ "_tc", GL_TESS_CONTROL_SHADER }
        ,{ "_te", GL_TESS_EVALUATION_SHADER }
      };

      auto type_it =
        boost::find_if(types, [&shader_name](auto && v)
      {
        return boost::algorithm::ends_with(shader_name, v.first);
      });

      return (std::end(types) != type_it) ? type_it->second : boost::optional<GLenum>{};
    }

    std::string read_file_to_string(const std::string & path)
    {
      std::fstream file{ path };
      return { std::istreambuf_iterator<char>(file),{} };
    }
  }

  std::vector<GLenum> gl::load_shaders(const std::string shaders_path)
  {
    auto files = boost::make_iterator_range(std::tr2::sys::directory_iterator{ shaders_path }, {});

    namespace sys = std::tr2::sys;
    namespace rn = boost::adaptors;

    auto shaders = files
      | rn::transformed([](auto && f) { return f.path();                                                                })
      | rn::filtered([](auto && f) { return f.extension() == ".glsl";                                                   })
      | rn::transformed([](auto && f) { return std::make_pair(f, detail::get_shader_type(f.stem().string()));           })
      | rn::filtered([](auto && f) { return !!f.second;                                                                 })
      | rn::transformed([](auto && f) { return std::make_pair(detail::read_file_to_string(f.first.string()), f.second); })
      | rn::transformed([](auto && f) { return create_shader(f.first, f.second.get(), &std::clog);                      })
      ;
    return { std::begin(shaders), std::end(shaders) };
  }
}
