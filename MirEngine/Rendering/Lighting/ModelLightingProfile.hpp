#pragma once

namespace mir4d::rendering {

enum class ModelLightingMode {
    engineeringStudio,
    engineeringBright,
    inspection,
    sculpt
};

struct ModelLightingProfile {
    float keyIntensity;
    float fillIntensity;
    float ambientIntensity;
    float rimIntensity;

    static constexpr ModelLightingProfile engineeringStudio() noexcept {
        return {1.10f, 0.45f, 0.60f, 0.35f};
    }

    static constexpr ModelLightingProfile engineeringBright() noexcept {
        return {1.35f, 0.70f, 0.82f, 0.35f};
    }

    static constexpr ModelLightingProfile inspection() noexcept {
        return {1.45f, 0.58f, 0.70f, 0.50f};
    }

    static constexpr ModelLightingProfile sculpt() noexcept {
        return {1.00f, 0.85f, 0.90f, 0.28f};
    }
};

inline constexpr ModelLightingProfile modelLightingProfile(ModelLightingMode mode) noexcept {
    switch (mode) {
        case ModelLightingMode::engineeringBright:
            return ModelLightingProfile::engineeringBright();
        case ModelLightingMode::inspection:
            return ModelLightingProfile::inspection();
        case ModelLightingMode::sculpt:
            return ModelLightingProfile::sculpt();
        case ModelLightingMode::engineeringStudio:
        default:
            return ModelLightingProfile::engineeringStudio();
    }
}

}
