
#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/Theme/Theme.hpp"
#include "../../Foundation/Animation/AnimationSpec.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include <memory>
#include <string>
#include <functional>

namespace MirUI {

class AnimationEditor {
public:

    AnimationEditor(UIDocument& doc)
        : m_doc(doc)
    {
        loadFromTheme();
    }

    [[nodiscard]] double defaultDuration()  const { return m_duration; }
    [[nodiscard]] AnimationCurve defaultCurve() const { return m_curve; }
    [[nodiscard]] double springDamping()    const { return m_springDamping; }
    [[nodiscard]] double springResponse()   const { return m_springResponse; }
    [[nodiscard]] bool   enableAnimations() const { return m_enabled; }

    void setDuration(double duration) {
        if (duration == m_duration) return;
        auto cmd = std::make_unique<AnimationEditCommand>(*this, m_duration, m_curve, m_springDamping,
                                                          m_springResponse, m_enabled,
                                                          duration, m_curve, m_springDamping,
                                                          m_springResponse, m_enabled,
                                                          "Изменить длительность анимации");
        m_doc.history().execute(std::move(cmd));
        m_duration = std::clamp(duration, 0.1, 5.0);
        applyToTheme();
    }

    void setCurve(AnimationCurve curve) {
        if (curve == m_curve) return;
        auto cmd = std::make_unique<AnimationEditCommand>(*this, m_duration, m_curve, m_springDamping,
                                                          m_springResponse, m_enabled,
                                                          m_duration, curve, m_springDamping,
                                                          m_springResponse, m_enabled,
                                                          "Изменить кривую анимации");
        m_doc.history().execute(std::move(cmd));
        m_curve = curve;
        applyToTheme();
    }

    void setSpringDamping(double damping) {
        if (damping == m_springDamping) return;
        auto cmd = std::make_unique<AnimationEditCommand>(*this, m_duration, m_curve, m_springDamping,
                                                          m_springResponse, m_enabled,
                                                          m_duration, m_curve, damping,
                                                          m_springResponse, m_enabled,
                                                          "Изменить затухание пружины");
        m_doc.history().execute(std::move(cmd));
        m_springDamping = std::clamp(damping, 0.0, 1.0);
        applyToTheme();
    }

    void setSpringResponse(double response) {
        if (response == m_springResponse) return;
        auto cmd = std::make_unique<AnimationEditCommand>(*this, m_duration, m_curve, m_springDamping,
                                                          m_springResponse, m_enabled,
                                                          m_duration, m_curve, m_springDamping,
                                                          response, m_enabled,
                                                          "Изменить реакцию пружины");
        m_doc.history().execute(std::move(cmd));
        m_springResponse = std::clamp(response, 0.1, 1.0);
        applyToTheme();
    }

    void setEnableAnimations(bool enabled) {
        if (enabled == m_enabled) return;
        auto cmd = std::make_unique<AnimationEditCommand>(*this, m_duration, m_curve, m_springDamping,
                                                          m_springResponse, m_enabled,
                                                          m_duration, m_curve, m_springDamping,
                                                          m_springResponse, enabled,
                                                          enabled ? "Включить анимации" : "Выключить анимации");
        m_doc.history().execute(std::move(cmd));
        m_enabled = enabled;
        applyToTheme();
    }

    void resetToDefault() {
        setDuration(0.25);
        setCurve(AnimationCurve::EaseInOut);
        setSpringDamping(0.7);
        setSpringResponse(0.5);
        setEnableAnimations(true);
    }

    [[nodiscard]] static std::vector<std::string> curveNames() {
        return {"Linear", "EaseIn", "EaseOut", "EaseInOut", "Spring"};
    }

    [[nodiscard]] static AnimationCurve curveFromName(const std::string& name) {
        if (name == "Linear")    return AnimationCurve::Linear;
        if (name == "EaseIn")    return AnimationCurve::EaseIn;
        if (name == "EaseOut")   return AnimationCurve::EaseOut;
        if (name == "EaseInOut") return AnimationCurve::EaseInOut;
        if (name == "Spring")    return AnimationCurve::Spring;
        return AnimationCurve::EaseInOut;
    }

    [[nodiscard]] static std::string curveToName(AnimationCurve curve) {
        switch (curve) {
            case AnimationCurve::Linear:    return "Linear";
            case AnimationCurve::EaseIn:    return "EaseIn";
            case AnimationCurve::EaseOut:   return "EaseOut";
            case AnimationCurve::EaseInOut: return "EaseInOut";
            case AnimationCurve::Spring:    return "Spring";
            default: return "EaseInOut";
        }
    }

private:
    UIDocument& m_doc;

    double m_duration        = 0.25;
    AnimationCurve m_curve   = AnimationCurve::EaseInOut;
    double m_springDamping   = 0.7;
    double m_springResponse  = 0.5;
    bool   m_enabled         = true;

    void loadFromTheme() {

        const Theme& theme = m_doc.themeManager().current();
        m_duration       = theme.animations.defaultDuration;
        m_enabled        = theme.animations.enableAnimations;

        m_curve          = AnimationCurve::EaseInOut;
        m_springDamping  = 0.7;
        m_springResponse = 0.5;
    }

    void applyToTheme() {

        Theme current = m_doc.themeManager().current();
        current.animations.defaultDuration  = m_duration;
        current.animations.enableAnimations = m_enabled;

        m_doc.themeManager().setTheme(current);
        m_doc.setModified(true);
    }

    class AnimationEditCommand : public ICommand {
    public:
        AnimationEditCommand(AnimationEditor& editor,
                             double oldDuration, AnimationCurve oldCurve,
                             double oldDamping, double oldResponse, bool oldEnabled,
                             double newDuration, AnimationCurve newCurve,
                             double newDamping, double newResponse, bool newEnabled,
                             std::string description)
            : m_editor(editor)
            , m_oldDuration(oldDuration), m_oldCurve(oldCurve)
            , m_oldDamping(oldDamping), m_oldResponse(oldResponse), m_oldEnabled(oldEnabled)
            , m_newDuration(newDuration), m_newCurve(newCurve)
            , m_newDamping(newDamping), m_newResponse(newResponse), m_newEnabled(newEnabled)
            , m_desc(std::move(description))
        {}

        bool execute() override {
            m_editor.m_duration        = m_newDuration;
            m_editor.m_curve           = m_newCurve;
            m_editor.m_springDamping   = m_newDamping;
            m_editor.m_springResponse  = m_newResponse;
            m_editor.m_enabled         = m_newEnabled;
            m_editor.applyToTheme();
            m_editor.m_doc.setModified(true);
            return true;
        }

        bool undo() override {
            m_editor.m_duration        = m_oldDuration;
            m_editor.m_curve           = m_oldCurve;
            m_editor.m_springDamping   = m_oldDamping;
            m_editor.m_springResponse  = m_oldResponse;
            m_editor.m_enabled         = m_oldEnabled;
            m_editor.applyToTheme();
            m_editor.m_doc.setModified(true);
            return true;
        }

        [[nodiscard]] std::string description() const override { return m_desc; }

    private:
        AnimationEditor& m_editor;
        double m_oldDuration, m_newDuration;
        AnimationCurve m_oldCurve, m_newCurve;
        double m_oldDamping, m_newDamping;
        double m_oldResponse, m_newResponse;
        bool   m_oldEnabled, m_newEnabled;
        std::string m_desc;
    };
};

}