#include "PartyKel/renderer/FlagRenderer3D.hpp"
#include "PartyKel/renderer/GLtools.hpp"
#include "PartyKel/glm.hpp"

#include <iostream>

namespace PartyKel {

const GLchar* FlagRenderer3D::VERTEX_SHADER =
"#version 330 core\n"
GL_STRINGIFY(
    layout(location = 0) in vec3 aVertexPosition;
    layout(location = 1) in vec3 aVertexNormal;

    uniform mat4 uMVPMatrix;
    uniform mat4 uMVMatrix;

    out vec3 vFragPosition;
    out vec3 vFragNormal;

    void main() {
        vFragPosition = vec3(uMVPMatrix * vec4(aVertexPosition, 1));
        vFragNormal = vec3(uMVMatrix * vec4(aVertexNormal, 0));
        gl_Position = uMVPMatrix * vec4(aVertexPosition, 1);
    }
);

const GLchar* FlagRenderer3D::FRAGMENT_SHADER =
"#version 330 core\n"
GL_STRINGIFY(
    in vec3 vFragPosition;
    in vec3 vFragNormal;

    out vec3 fFragColor;

    void main() {
        fFragColor = vec3(abs(dot(normalize(vFragPosition), normalize(vFragNormal))));
    }
);

FlagRenderer3D::FlagRenderer3D(int gridWidth, int gridHeight):
    m_ProgramID(buildProgram(VERTEX_SHADER, FRAGMENT_SHADER)),
    m_ProjMatrix(1.f), m_ViewMatrix(1.f),
    m_nGridWidth(gridWidth), m_nGridHeight(gridHeight), m_nIndexCount(0),
    m_VertexBuffer(gridWidth * gridHeight) {

    // Création du VBO
    glGenBuffers(1, &m_VBOID);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOID);

    glGenBuffers(1, &m_IBOID);

    std::vector<GLuint> indexBuffer;
    for (int j = 0; j < gridHeight - 1; ++j) {
        for (int i = 0; i < gridWidth - 1; ++i) {
            indexBuffer.push_back(i + j * gridWidth);
            indexBuffer.push_back((i + 1) + j * gridWidth);
            indexBuffer.push_back((i + 1) + (j + 1) * gridWidth);
            indexBuffer.push_back(i + j * gridWidth);
            indexBuffer.push_back((i + 1) + (j + 1) * gridWidth);
            indexBuffer.push_back(i + (j + 1) * gridWidth);
        }
    }
    m_nIndexCount = indexBuffer.size();

    // Création du VAO
    glGenVertexArrays(1, &m_VAOID);
    glBindVertexArray(m_VAOID);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBOID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(indexBuffer[0]), indexBuffer.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*) offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*) offsetof(Vertex, normal));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_uMVPMatrix = glGetUniformLocation(m_ProgramID, "uMVPMatrix");
    m_uMVMatrix = glGetUniformLocation(m_ProgramID, "uMVMatrix");
}

FlagRenderer3D::~FlagRenderer3D() {
    glDeleteBuffers(1, &m_VBOID);
    glDeleteBuffers(1, &m_IBOID);
    glDeleteVertexArrays(1, &m_VAOID);
    glDeleteProgram(m_ProgramID);
}

void FlagRenderer3D::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FlagRenderer3D::drawGrid(const glm::vec3* positionArray, bool wireframe) {
    glEnable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBOID);

    for (int j = 0; j < m_nGridHeight; ++j) {
        for (int i = 0; i < m_nGridWidth; ++i) {
            m_VertexBuffer[i + j * m_nGridWidth].position = positionArray[i + j * m_nGridWidth];
            
            glm::vec3 N(0.f);

            glm::vec3 A = positionArray[i + j * m_nGridWidth];

            if (i > 0 && j > 0) {
                glm::vec3 B = positionArray[(i - 1) + j * m_nGridWidth];
                glm::vec3 C = positionArray[(i - 1) + (j - 1) * m_nGridWidth];

                glm::vec3 BxC = glm::cross(B - A, C - A);
                float l = glm::length(BxC);

                if (l > 0.0001f) {
                    N += BxC / l;
                }
            }

            if (i > 0 && j > 0) {
                glm::vec3 B = positionArray[(i - 1) + (j - 1) * m_nGridWidth];
                glm::vec3 C = positionArray[i + (j - 1) * m_nGridWidth];

                glm::vec3 BxC = glm::cross(B - A, C - A);
                float l = glm::length(BxC);

                if (l > 0.0001f) {
                    N += BxC / l;
                }
            }

            if (i < m_nGridWidth - 1 && j > 0) {
                glm::vec3 B = positionArray[i + (j - 1) * m_nGridWidth];
                glm::vec3 C = positionArray[(i + 1) + (j - 1) * m_nGridWidth];

                glm::vec3 BxC = glm::cross(B - A, C - A);
                float l = glm::length(BxC);

                if (l > 0.0001f) {
                    N += BxC / l;
                }
            }

            if (i < m_nGridWidth - 1 && j > 0) {
                glm::vec3 B = positionArray[(i + 1) + (j - 1) * m_nGridWidth];
                glm::vec3 C = positionArray[(i + 1) + j * m_nGridWidth];

                glm::vec3 BxC = glm::cross(B - A, C - A);
                float l = glm::length(BxC);

                if (l > 0.0001f) {
                    N += BxC / l;
                }
            }

            if (i < m_nGridWidth - 1 && j < m_nGridHeight - 1) {
                glm::vec3 B = positionArray[(i + 1) + j * m_nGridWidth];
                glm::vec3 C = positionArray[(i + 1) + (j + 1) * m_nGridWidth];

                glm::vec3 BxC = glm::cross(B - A, C - A);
                float l = glm::length(BxC);

                if (l > 0.0001f) {
                    N += BxC / l;
                }
            }

            if (i < m_nGridWidth - 1 && j < m_nGridHeight - 1) {
                glm::vec3 B = positionArray[(i + 1) + (j + 1) * m_nGridWidth];
                glm::vec3 C = positionArray[i + (j + 1) * m_nGridWidth];

                glm::vec3 BxC = glm::cross(B - A, C - A);
                float l = glm::length(BxC);

                if (l > 0.0001f) {
                    N += BxC / l;
                }
            }

            if (i > 0 && j < m_nGridHeight - 1) {
                glm::vec3 B = positionArray[i + (j + 1) * m_nGridWidth];
                glm::vec3 C = positionArray[(i - 1) + (j + 1) * m_nGridWidth];

                glm::vec3 BxC = glm::cross(B - A, C - A);
                float l = glm::length(BxC);

                if (l > 0.0001f) {
                    N += BxC / l;
                }
            }

            if (i > 0 && j < m_nGridHeight - 1) {
                glm::vec3 B = positionArray[(i - 1) + (j + 1) * m_nGridWidth];
                glm::vec3 C = positionArray[(i - 1) + j * m_nGridWidth];

                glm::vec3 BxC = glm::cross(B - A, C - A);
                float l = glm::length(BxC);

                if (l > 0.0001f) {
                    N += BxC / l;
                }
            }

            m_VertexBuffer[i + j * m_nGridWidth].normal = N != glm::vec3(0.f) ? glm::normalize(N) : glm::vec3(0.f);
        }
    }

    glBufferData(GL_ARRAY_BUFFER, m_VertexBuffer.size() * sizeof(m_VertexBuffer[0]), m_VertexBuffer.data(), GL_DYNAMIC_DRAW);

    glUseProgram(m_ProgramID);

    glUniformMatrix4fv(m_uMVPMatrix, 1, GL_FALSE, glm::value_ptr(m_ProjMatrix * m_ViewMatrix));
    glUniformMatrix4fv(m_uMVMatrix, 1, GL_FALSE, glm::value_ptr(m_ViewMatrix));

    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glBindVertexArray(m_VAOID);
        glDrawElements(GL_TRIANGLES, m_nIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

}
