#pragma once

#include <GL/glew.h>

#define GL_STRINGIFY(s) #s

namespace PartyKel {

GLuint buildProgram(const GLchar* vertexShaderSrc, const GLchar* fragmentShaderSrc);

}
