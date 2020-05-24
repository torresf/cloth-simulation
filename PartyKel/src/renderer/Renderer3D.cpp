#include "PartyKel/renderer/Renderer3D.hpp"
#include "PartyKel/renderer/GLtools.hpp"
#include "PartyKel/renderer/Sphere.hpp"

namespace PartyKel {

const GLchar* Renderer3D::SPHERE_VERTEX_SHADER =
"#version 330 core\n"
GL_STRINGIFY(
    layout(location = 0) in vec3 aVertexPosition;
    layout(location = 1) in vec3 aVertexNormal;

    uniform mat4 uMVPMatrix;
    uniform mat4 uMVMatrix;

    out vec3 vFragPositionViewSpace;
    out vec3 vFragNormalViewSpace;

    void main() {
        vFragPositionViewSpace = vec3(uMVMatrix * vec4(aVertexPosition, 1));
        vFragNormalViewSpace = vec3(uMVMatrix * vec4(aVertexNormal, 0));
        gl_Position = uMVPMatrix * vec4(aVertexPosition, 1);
    }
);

const GLchar* Renderer3D::SPHERE_FRAGMENT_SHADER =
"#version 330 core\n"
GL_STRINGIFY(
    in vec3 vFragPositionViewSpace;
    in vec3 vFragNormalViewSpace;

    uniform vec3 uParticleColor;

    out vec3 fFragColor;

    void main() {
        fFragColor = uParticleColor * vec3(abs(dot(normalize(vFragPositionViewSpace), normalize(vFragNormalViewSpace))));
    }
);

Renderer3D::Renderer3D():
    m_SphereProgramID(buildProgram(SPHERE_VERTEX_SHADER, SPHERE_FRAGMENT_SHADER)) {
    // Récuperation des uniforms
    m_uParticleColor = glGetUniformLocation(m_SphereProgramID, "uParticleColor");
    m_uMVPMatrix = glGetUniformLocation(m_SphereProgramID, "uMVPMatrix");
    m_uMVMatrix = glGetUniformLocation(m_SphereProgramID, "uMVMatrix");

    // Création du VBO
    glGenBuffers(1, &m_SphereVBOID);
    glBindBuffer(GL_ARRAY_BUFFER, m_SphereVBOID);

    Sphere sphere(1.f, 64, 32);
    m_nSphereVertexCount = sphere.getVertexCount();

    glBufferData(GL_ARRAY_BUFFER, m_nSphereVertexCount * sizeof(Sphere::Vertex), sphere.getDataPointer(), GL_STATIC_DRAW);

    // Création du VAO
    glGenVertexArrays(1, &m_SphereVAOID);
    glBindVertexArray(m_SphereVAOID);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Sphere::Vertex), (const GLvoid*) offsetof(Sphere::Vertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Sphere::Vertex), (const GLvoid*) offsetof(Sphere::Vertex, normal));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Renderer3D::~Renderer3D() {
    glDeleteProgram(m_SphereProgramID);

    glDeleteBuffers(1, &m_SphereVBOID);
    glDeleteVertexArrays(1, &m_SphereVAOID);
}

void Renderer3D::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer3D::drawParticles(uint32_t count,
                   const glm::vec3* positionArray,
                   const float* massArray,
                   const glm::vec3* colorArray,
                   float massScale) {
    glUseProgram(m_SphereProgramID);

    glBindVertexArray(m_SphereVAOID);

    glEnable(GL_DEPTH_TEST);

    // Dessine chacune des particules
    for(uint32_t i = 0; i < count; ++i) {
        auto modelMatrix = glm::scale(glm::translate(glm::mat4(1.f), positionArray[i]), glm::vec3(massScale * massArray[i]));
        glUniformMatrix4fv(m_uMVPMatrix, 1, GL_FALSE, glm::value_ptr(m_ProjMatrix * m_ViewMatrix * modelMatrix));
        glUniformMatrix4fv(m_uMVMatrix, 1, GL_FALSE, glm::value_ptr(m_ViewMatrix * modelMatrix));

        glUniform3fv(m_uParticleColor, 1, glm::value_ptr(colorArray[i]));
        glDrawArrays(GL_TRIANGLES, 0, m_nSphereVertexCount);
    }

    glBindVertexArray(0);
}

}
