#pragma once

#include "PartyKel/glm.hpp"
#include <GL/glew.h>
#include <vector>

namespace PartyKel {

class Renderer3D {
public:
    Renderer3D();

    ~Renderer3D();

    Renderer3D(const Renderer3D&) = delete;

    Renderer3D& operator =(const Renderer3D&) = delete;

    void clear();

    void drawParticles(uint32_t count,
                       const glm::vec3* positionArray,
                       const float* massArray,
                       const glm::vec3* colorArray,
                       float massScale = 0.05);

    void setProjMatrix(const glm::mat4& P) {
        m_ProjMatrix = P;
    }

    void setViewMatrix(const glm::mat4& V) {
        m_ViewMatrix = V;
    }

private:
    static const GLchar *SPHERE_VERTEX_SHADER, *SPHERE_FRAGMENT_SHADER;

    // Ressources OpenGL
    GLuint m_SphereProgramID;
    GLuint m_SphereVBOID, m_SphereVAOID;

    uint32_t m_nSphereVertexCount;

    glm::mat4 m_ProjMatrix;
    glm::mat4 m_ViewMatrix;

    GLint m_uMVPMatrix, m_uMVMatrix;
    GLint m_uParticleColor;
};

}
