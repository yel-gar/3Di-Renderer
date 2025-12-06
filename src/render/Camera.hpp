#pragma once
#include "math/Matrix4x4.hpp"
#include "math/Vector3.hpp"

using namespace di_renderer::math;

namespace di_renderer::render {
    class Camera {
      public:
        Camera(Vector3 position, Vector3 target, float fov, float aspectRatio, float nearPlane, float farPlane);
        void setPosition(Vector3 position);
        void setTarget(Vector3 target);
        void setAspectRatio(float aspectRatio);

        Vector3 getPosition();
        Vector3 getTarget();

        void movePosition(Vector3 position);
        void moveTarget(Vector3 target);

        Matrix4x4 getViewMatrix();
        Matrix4x4 getProjectionMatrix();

      private:
        Vector3 _position;
        Vector3 _target;
        float _fov;
        float _aspectRatio;
        float _nearPlane;
        float _farPlane;
    };

} // namespace di_renderer::render
