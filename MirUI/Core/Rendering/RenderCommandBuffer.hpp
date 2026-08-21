// MirUI/Core/Rendering/RenderCommandBuffer.hpp
// Буфер команд отрисовки — как холст, на котором мы записываем,
// ЧТО и В КАКОМ ПОРЯДКЕ нужно нарисовать.
// Потом этот буфер целиком отправляется рендереру (SwiftUI, WinUI и т.д.),
// и тот уже превращает команды в настоящие пиксели на экране.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "RenderCommand.hpp"
#include <vector>
#include <stack>

namespace MirUI {

class RenderCommandBuffer {
public:
    // ── Очистка ──────────────────────────────────────────────
    // Полностью стирает все накопленные команды.
    // Обычно вызывается в начале каждого кадра.
    void clear() {
        m_commands.clear();
        // Также очищаем стек трансформаций, если остались незакрытые Push.
        while (!m_transformStack.empty()) {
            m_transformStack.pop();
        }
    }

    // ── Простые фигуры ──────────────────────────────────────

    // Нарисовать прямоугольник.
    void drawRect(const Rect& rect, const Color& fill, const Color& stroke = {}, double strokeWidth = 1.0) {
        RenderCommand cmd;
        cmd.type = RenderCommandType::DrawRect;
        cmd.data = rect;
        cmd.fillColor = fill;
        cmd.strokeColor = stroke;
        cmd.strokeWidth = strokeWidth;
        m_commands.push_back(cmd);
    }

    // Нарисовать текст.
    void drawText(const std::string& text, const Rect& frame, const Color& color = Color::black()) {
        RenderCommand cmd;
        cmd.type = RenderCommandType::DrawText;
        cmd.data = text; // сам текст
        // Пока не используем frame, но в будущем добавим позиционирование.
        cmd.fillColor = color;
        m_commands.push_back(cmd);
    }

    // Нарисовать линию от точки start до точки end.
    void drawLine(const Point& start, const Point& end, const Color& color, double thickness = 1.0) {
        RenderCommand cmd;
        cmd.type = RenderCommandType::DrawLine;
        cmd.data = std::make_pair(start, end);
        cmd.strokeColor = color;
        cmd.strokeWidth = thickness;
        m_commands.push_back(cmd);
    }

    // Нарисовать изображение (пока просто запоминаем имя файла).
    void drawImage(const std::string& imageName, const Rect& rect) {
        RenderCommand cmd;
        cmd.type = RenderCommandType::DrawImage;
        cmd.data = imageName; // идентификатор изображения
        // В будущем можно добавить rect как дополнительный параметр.
        m_commands.push_back(cmd);
    }

    // ── Отсечение (clipping) ────────────────────────────────
    // Всё, что нарисовано после pushClip, будет обрезано по границам указанного прямоугольника.
    // popClip отменяет последнее отсечение.

    void pushClip(const Rect& clipRect) {
        RenderCommand cmd;
        cmd.type = RenderCommandType::Clip;
        cmd.data = clipRect;
        // Помечаем, что это начало отсечения (можно добавить флаг)
        m_commands.push_back(cmd);
    }

    void popClip() {
        // Для простоты мы просто записываем специальную команду,
        // которая в рендерере будет означать "восстановить предыдущий клип".
        RenderCommand cmd;
        cmd.type = RenderCommandType::PopTransform; // пока используем PopTransform как маркер
        // На самом деле нужен отдельный тип, но для MVP сойдёт.
        m_commands.push_back(cmd);
    }

    // ── Трансформации (смещение, поворот, масштаб) ──────────
    // Позволяют временно сдвинуть систему координат.
    // Например, чтобы нарисовать кнопку в нужном месте,
    // можно сначала сделать PushTransform с координатами кнопки,
    // а потом все координаты внутри отсчитывать от левого верхнего угла кнопки.

    void pushTransform(double translateX, double translateY) {
        RenderCommand cmd;
        cmd.type = RenderCommandType::PushTransform;
        cmd.data = Point{translateX, translateY};
        m_commands.push_back(cmd);
    }

    void popTransform() {
        RenderCommand cmd;
        cmd.type = RenderCommandType::PopTransform;
        m_commands.push_back(cmd);
    }

    // ── Доступ к накопленным командам ──────────────────────
    // Рендерер будет перебирать все команды и выполнять их.

    [[nodiscard]] const std::vector<RenderCommand>& commands() const {
        return m_commands;
    }

private:
    std::vector<RenderCommand> m_commands;

    // Стек для отслеживания пар Push/Pop (на будущее).
    std::stack<Point> m_transformStack;
};

} // namespace MirUI