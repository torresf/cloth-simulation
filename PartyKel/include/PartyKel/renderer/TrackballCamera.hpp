#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace PartyKel {

class TrackballCamera {
public:
	TrackballCamera(): m_fDistance(5.f), m_fAngleX(0.f), m_fAngleY(0.f) {
	}

	void moveFront(float delta) {
		m_fDistance += delta;
		m_fDistance = glm::max(0.1f, m_fDistance);
	}

	void rotateLeft(float degrees) {
		m_fAngleY += degrees * .5f;
	}

	void rotateUp(float degrees) {
		m_fAngleX += degrees * .5f;
	}

	glm::mat4 getViewMatrix() const {
		glm::mat4 V = glm::lookAt(glm::vec3(0, 0, m_fDistance), glm::vec3(0.f), glm::vec3(0, 1, 0));
		V = glm::rotate(V, m_fAngleX, glm::vec3(1, 0, 0));
		V = glm::rotate(V, m_fAngleY, glm::vec3(0, 1, 0));
		return V;
	}

private:
	float m_fDistance;
	float m_fAngleX, m_fAngleY;
};

}