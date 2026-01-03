#pragma once
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace di_renderer::ui {
    enum class TransformType : std::uint8_t { TRANSLATE, ROTATE, SCALE };

    enum class VectorComponent : std::uint8_t { X, Y, Z };

    struct TransformTypeHelper {
        TransformType type;
        VectorComponent component;
    };

    inline const std::unordered_map<std::string, TransformType> TRANSFORM_TYPE_MAP = {
        {"translation", TransformType::TRANSLATE},
        {"rotation", TransformType::ROTATE},
        {"scale", TransformType::SCALE}};
    inline const std::unordered_map<std::string, VectorComponent> VECTOR_COMPONENT_MAP = {
        {"x", VectorComponent::X}, {"y", VectorComponent::Y}, {"z", VectorComponent::Z}};

    inline TransformTypeHelper get_transform_type(const std::string& name) {
        const auto pos = name.find('_');
        if (pos == std::string::npos) {
            throw std::invalid_argument("Transform type not found");
        }

        const auto type = name.substr(0, pos);
        const auto component = name.substr(pos + 1);
        return {TRANSFORM_TYPE_MAP.at(type), VECTOR_COMPONENT_MAP.at(component)};
    }

    inline math::Vector3 get_new_vector(const math::Vector3& vec, const VectorComponent component, const float val) {
        math::Vector3 new_vec = vec;
        switch (component) {
        case VectorComponent::X:
            new_vec.x = val;
            break;
        case VectorComponent::Y:
            new_vec.y = val;
            break;
        case VectorComponent::Z:
            new_vec.z = val;
            break;
        }

        return new_vec;
    }

    inline float get_vector_component(const math::Vector3& vec, const VectorComponent component) {
        switch (component) {
        case VectorComponent::X:
            return vec.x;
        case VectorComponent::Y:
            return vec.y;
        case VectorComponent::Z:
            return vec.z;
        }

        throw std::invalid_argument("VectorComponent not found"); // shouldn't happen
    }
} // namespace di_renderer::ui
