#pragma once
#include "math/Matrix4x4.hpp"
#include "math/Vector3.hpp"

using namespace di_renderer::math;

namespace di_renderer::render {
    class Camera {
      public:
        Camera(Vector3 position, Vector3 target, float fov, float aspectRatio, float nearPlane, float farPlane);
        void set_position(Vector3 position);
        void set_target(Vector3 target);
        void set_aspect_ratio(float aspectRatio);
        void set_up_vector(Vector3 up);
        void set_fov(float fov);
        void set_planes(float nearPlane, float farPlane);

        Vector3 get_position() const;
        Vector3 get_target() const;

        void move_position(Vector3 position);
        void move_target(Vector3 target);
        void move(Vector3 direction);

        Matrix4x4 get_view_matrix() const;
        Matrix4x4 get_projection_matrix() const;

      private:
        Vector3 m_position;
        Vector3 m_target;
        Vector3 m_up;

        float m_fov;
        float m_aspect_ratio;
        float m_near_plane;
        float m_far_plane;

        mutable Matrix4x4 m_view_matrix;
        mutable bool m_view_changed;

        mutable Matrix4x4 m_projection_matrix;
        mutable bool m_projection_changed;
    };

} // namespace di_renderer::render
