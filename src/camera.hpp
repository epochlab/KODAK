#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera {
public:
    // filmbackMm: sensor width (36mm = full-frame). focalLengthMm: e.g. 50mm.
    Camera(glm::vec3 position, float aspect,
           float filmbackMm    = 35.0f,
           float focalLengthMm = 50.0f,
           float near          = 0.1f,
           float far           = 100.0f);

    glm::mat4 viewMatrix()       const;
    glm::mat4 projectionMatrix() const;

    void setAspect(float aspect);
    void setFocalLength(float mm);
    void setPosition(glm::vec3 p)      { m_pos = p; }
    void setYaw(float deg)             { m_yaw = deg; }
    void setPitch(float deg);
    void setNear(float n)              { m_near = n; }
    void setFar(float f)               { m_far  = f; }
    void setFilmback(float mm)         { m_filmbackMm = mm; }
    void processInput(GLFWwindow* window, float dt);
    void processMouseMove(double xpos, double ypos);
    void resetMouse() { m_firstMouse = true; }

    // Orbit mode: camera tumbles around a fixed world-space pivot.
    void      setOrbitPivot(glm::vec3 pivot);
    void      clearOrbit();
    bool      isOrbiting()  const { return m_orbiting; }
    glm::vec3 orbitPivot()  const { return m_pivot; }

    glm::vec3 position()      const { return m_pos; }
    glm::vec3 front()         const;
    float     yaw()           const { return m_yaw; }
    float     pitch()         const { return m_pitch; }
    float     nearPlane()     const { return m_near; }
    float     farPlane()      const { return m_far; }
    float     filmback()      const { return m_filmbackMm; }
    float     focalLength()   const { return m_focalLengthMm; }

    float moveSpeed        = 5.0f;
    float mouseSensitivity = 0.05f;

private:
    glm::vec3 m_pos;
    float m_yaw;
    float m_pitch;
    float m_aspect;
    float m_filmbackMm;
    float m_focalLengthMm;
    float m_near, m_far;
    bool   m_firstMouse = true;
    double m_lastX = 0.0, m_lastY = 0.0;

    glm::vec3 m_pivot    {0.f, 0.f, 0.f};
    float     m_radius   = 5.f;
    bool      m_orbiting = false;
};
