
#pragma once

#include "RenderCommand.hpp"
#include <vector>
#include <stack>

namespace MirUI {

class RenderCommandBuffer {
public:

    void clear() {
        m_commands.clear();

        while (!m_transformStack.empty()) {
            m_transformStack.pop();
        }
    }

    void drawRect(const Rect& rect, const Color& fill, const Color& stroke = {}, double strokeWidth = 1.0) {
        RenderCommand cmd;
        cmd.type = RenderCommandType::DrawRect;
        cmd.data = rect;
        cmd.fillColor = fill;
        cmd.strokeColor = stroke;
        cmd.strokeWidth = strokeWidth;
        m_commands.push_back(cmd);
    }

    void drawText(const std::string& text, const Rect& frame, const Color& color = Color::black()) {
        RenderCommand cmd;
        cmd.type = RenderCommandType::DrawText;
        cmd.data = text;

        cmd.fillColor = color;
        m_commands.push_back(cmd);
    }

    void drawLine(const Point& start, const Point& end, const Color& color, double thickness = 1.0) {
        RenderCommand cmd;
        cmd.type = RenderCommandType::DrawLine;
        cmd.data = std::make_pair(start, end);
        cmd.strokeColor = color;
        cmd.strokeWidth = thickness;
        m_commands.push_back(cmd);
    }

    void drawImage(const std::string& imageName, const Rect& rect) {
        RenderCommand cmd;
        cmd.type = RenderCommandType::DrawImage;
        cmd.data = imageName;

        m_commands.push_back(cmd);
    }

    void pushClip(const Rect& clipRect) {
        RenderCommand cmd;
        cmd.type = RenderCommandType::Clip;
        cmd.data = clipRect;

        m_commands.push_back(cmd);
    }

    void popClip() {

        RenderCommand cmd;
        cmd.type = RenderCommandType::PopTransform;

        m_commands.push_back(cmd);
    }

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

    [[nodiscard]] const std::vector<RenderCommand>& commands() const {
        return m_commands;
    }

private:
    std::vector<RenderCommand> m_commands;

    std::stack<Point> m_transformStack;
};

}