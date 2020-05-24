#pragma once

#include <vector>
#include <GL/glew.h>
#include "PartyKel/glm.hpp"

namespace PartyKel {

// Représente une sphère discrétisée centrée en (0, 0, 0) (dans son repère local)
// Son axe vertical est (0, 1, 0) et ses axes transversaux sont (1, 0, 0) et (0, 0, 1)
class Sphere {
    // Alloue et construit les données (implantation dans le .cpp)
    void build(GLfloat radius, GLsizei discLat, GLsizei discLong);

public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
    };

    std::vector<glm::vec3> _positions;

    // Constructeur: alloue le tableau de données et construit les attributs des vertex
    Sphere(GLfloat radius, GLsizei discLat, GLsizei discLong):
        m_nVertexCount(0) {
        build(radius, discLat, discLong); // Construction (voir le .cpp)
    }

    // Renvoit le pointeur vers les données
    const Vertex* getDataPointer() const {
        return &m_Vertices[0];
    }

    // Renvoit le nombre de vertex
    GLsizei getVertexCount() const {
        return m_nVertexCount;
    }

private:
    std::vector<Vertex> m_Vertices;
    GLsizei m_nVertexCount; // Nombre de sommets
};

struct SphereHandler{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> colors;
    std::vector<float> radius;
};
    
}
