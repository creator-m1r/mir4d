// MirUI/Foundation/Metrics/Metrics.hpp
// 📏 Семантические метрики темы — размеры, отступы, радиусы, высоты.
// Теперь с явными операторами сравнения, чтобы избежать ограничений
// старых анализаторов кода.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

namespace MirUI {

struct Metrics {
    double spacingXS   = 4.0;
    double spacingS    = 8.0;
    double spacingM    = 12.0;
    double spacingL    = 16.0;
    double spacingXL   = 24.0;

    double radiusS     = 4.0;
    double radiusM     = 8.0;
    double radiusL     = 12.0;

    double borderWidth = 1.0;

    double controlHeight = 28.0;
    double toolbarHeight = 44.0;
    double panelWidth    = 260.0;

    // ── Операторы сравнения (явная реализация) ──────────────
    bool operator==(const Metrics& other) const {
        return spacingXS == other.spacingXS &&
               spacingS == other.spacingS &&
               spacingM == other.spacingM &&
               spacingL == other.spacingL &&
               spacingXL == other.spacingXL &&
               radiusS == other.radiusS &&
               radiusM == other.radiusM &&
               radiusL == other.radiusL &&
               borderWidth == other.borderWidth &&
               controlHeight == other.controlHeight &&
               toolbarHeight == other.toolbarHeight &&
               panelWidth == other.panelWidth;
    }
    bool operator!=(const Metrics& other) const {
        return !(*this == other);
    }

    // Стандартные метрики
    static constexpr Metrics standard() {
        return {};
    }
};

} // namespace MirUI