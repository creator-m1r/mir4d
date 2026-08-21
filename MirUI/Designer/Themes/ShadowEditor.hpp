// MirUI/Designer/Themes/ShadowEditor.hpp
// 🌑 Редактор теней — позволяет изменять параметры тени для любого теневого токена.
//
// Каждая тень в теме MirUI описывается пятью параметрами:
//   • color      — цвет тени (обычно чёрный с разной прозрачностью).
//   • offsetX    — смещение по горизонтали (положительное = вправо).
//   • offsetY    — смещение по вертикали (положительное = вниз).
//   • blurRadius — радиус размытия (0 = чёткая тень).
//   • spread     — расширение/сжатие тени (положительное = больше, отрицательное = меньше).
//
// ShadowEditor позволяет редактировать эти параметры для любого токена
// (например, "shadow.panel", "shadow.floating", "shadow.modal").
// Все изменения автоматически применяются к теме и попадают в историю Undo/Redo.
//
// Редактор не содержит визуального интерфейса — только логику и данные.
// Отображение (ползунки, поля ввода, палитра цвета) делает платформенный UI.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/Theme/Theme.hpp"
#include "../../Core/Theme/WidgetStateStyle.hpp"  // ShadowData
#include "../../Foundation/Color/Color.hpp"
#include "../../Foundation/Shadow/ShadowToken.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace MirUI {

class ShadowEditor {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Принимает документ и имя теневого токена (например, "shadow.panel").
    ShadowEditor(UIDocument& doc, const std::string& tokenName)
        : m_doc(doc)
        , m_tokenName(tokenName)
    {
        // Загружаем текущее значение тени из темы.
        // Пока тени хранятся как часть WidgetStyle, а не как отдельная карта токенов.
        // Для простоты будем хранить настройки локально, а применять их к теме
        // через прямой доступ к WidgetStyle конкретного типа.
        // В будущем появится Theme::shadows — карта токенов.
        loadCurrentShadow();
    }

    // ── Текущие значения (для отображения в UI) ──────────────
    [[nodiscard]] Color  color()      const { return m_color; }
    [[nodiscard]] double offsetX()    const { return m_offsetX; }
    [[nodiscard]] double offsetY()    const { return m_offsetY; }
    [[nodiscard]] double blurRadius() const { return m_blurRadius; }
    [[nodiscard]] double spread()     const { return m_spread; }
    [[nodiscard]] const std::string& tokenName() const { return m_tokenName; }

    // ── Установка новых значений ────────────────────────────
    void setColor(const Color& color) {
        if (color == m_color) return;
        auto cmd = std::make_unique<ShadowEditCommand>(*this, m_color, m_offsetX, m_offsetY, m_blurRadius, m_spread,
                                                       color, m_offsetX, m_offsetY, m_blurRadius, m_spread,
                                                       "Изменить цвет тени «" + m_tokenName + "»");
        m_doc.history().execute(std::move(cmd));
        m_color = color;
        applyToTheme();
    }

    void setOffsetX(double value) {
        if (value == m_offsetX) return;
        auto cmd = std::make_unique<ShadowEditCommand>(*this, m_color, m_offsetX, m_offsetY, m_blurRadius, m_spread,
                                                       m_color, value, m_offsetY, m_blurRadius, m_spread,
                                                       "Изменить смещение X тени «" + m_tokenName + "»");
        m_doc.history().execute(std::move(cmd));
        m_offsetX = value;
        applyToTheme();
    }

    void setOffsetY(double value) {
        if (value == m_offsetY) return;
        auto cmd = std::make_unique<ShadowEditCommand>(*this, m_color, m_offsetX, m_offsetY, m_blurRadius, m_spread,
                                                       m_color, m_offsetX, value, m_blurRadius, m_spread,
                                                       "Изменить смещение Y тени «" + m_tokenName + "»");
        m_doc.history().execute(std::move(cmd));
        m_offsetY = value;
        applyToTheme();
    }

    void setBlurRadius(double value) {
        if (value == m_blurRadius) return;
        auto cmd = std::make_unique<ShadowEditCommand>(*this, m_color, m_offsetX, m_offsetY, m_blurRadius, m_spread,
                                                       m_color, m_offsetX, m_offsetY, value, m_spread,
                                                       "Изменить размытие тени «" + m_tokenName + "»");
        m_doc.history().execute(std::move(cmd));
        m_blurRadius = value;
        applyToTheme();
    }

    void setSpread(double value) {
        if (value == m_spread) return;
        auto cmd = std::make_unique<ShadowEditCommand>(*this, m_color, m_offsetX, m_offsetY, m_blurRadius, m_spread,
                                                       m_color, m_offsetX, m_offsetY, m_blurRadius, value,
                                                       "Изменить расширение тени «" + m_tokenName + "»");
        m_doc.history().execute(std::move(cmd));
        m_spread = value;
        applyToTheme();
    }

    // ── Сброс к значениям по умолчанию ───────────────────────
    void resetToDefault() {
        ShadowData defaultShadow;
        setColor(defaultShadow.color);
        setOffsetX(defaultShadow.offsetX);
        setOffsetY(defaultShadow.offsetY);
        setBlurRadius(defaultShadow.blurRadius);
        setSpread(0.0);
    }

private:
    UIDocument& m_doc;
    std::string m_tokenName;

    Color  m_color      = Color::rgba(0, 0, 0, 0.15f);
    double m_offsetX    = 0.0;
    double m_offsetY   = 2.0;
    double m_blurRadius = 4.0;
    double m_spread     = 0.0;

    // Загружает текущие значения тени из темы (заглушка).
    void loadCurrentShadow() {
        // В будущем здесь будет чтение из Theme::shadows[m_tokenName].
        // Пока используем значения по умолчанию.
    }

    // Применяет текущие настройки к теме.
    void applyToTheme() {
        // Пока нет централизованного хранилища теней в теме,
        // оставляем заглушку. В будущем будем обновлять Theme::shadows.
        m_doc.setModified(true);
    }

    // ── Внутренняя команда для Undo/Redo ─────────────────────
    class ShadowEditCommand : public ICommand {
    public:
        ShadowEditCommand(ShadowEditor& editor,
                          Color oldColor, double oldX, double oldY, double oldBlur, double oldSpread,
                          Color newColor, double newX, double newY, double newBlur, double newSpread,
                          std::string description)
            : m_editor(editor)
            , m_oldColor(oldColor), m_oldX(oldX), m_oldY(oldY), m_oldBlur(oldBlur), m_oldSpread(oldSpread)
            , m_newColor(newColor), m_newX(newX), m_newY(newY), m_newBlur(newBlur), m_newSpread(newSpread)
            , m_desc(std::move(description))
        {}

        bool execute() override {
            m_editor.m_color      = m_newColor;
            m_editor.m_offsetX    = m_newX;
            m_editor.m_offsetY    = m_newY;
            m_editor.m_blurRadius = m_newBlur;
            m_editor.m_spread     = m_newSpread;
            m_editor.applyToTheme();
            m_editor.m_doc.setModified(true);
            return true;
        }

        bool undo() override {
            m_editor.m_color      = m_oldColor;
            m_editor.m_offsetX    = m_oldX;
            m_editor.m_offsetY    = m_oldY;
            m_editor.m_blurRadius = m_oldBlur;
            m_editor.m_spread     = m_oldSpread;
            m_editor.applyToTheme();
            m_editor.m_doc.setModified(true);
            return true;
        }

        [[nodiscard]] std::string description() const override { return m_desc; }

    private:
        ShadowEditor& m_editor;
        Color  m_oldColor, m_newColor;
        double m_oldX, m_newX;
        double m_oldY, m_newY;
        double m_oldBlur, m_newBlur;
        double m_oldSpread, m_newSpread;
        std::string m_desc;
    };
};

} // namespace MirUI