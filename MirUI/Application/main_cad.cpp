// MirUI/Application/main_cad.cpp
// Пример запуска CAD-приложения из C++.
// На macOS этот код вызывается из тонкого Swift/ObjC++ хоста.

#include "CADApplication.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int runCADApplication() {
    MirUI::CADApplication app;

    if (!app.initialize()) {
        std::cerr << "Failed to initialize CAD Application\n";
        return 1;
    }

    // Простой цикл (в реальности управляется платформой + DisplayLink)
    using clock = std::chrono::high_resolution_clock;
    auto last = clock::now();

    bool running = true;
    while (running) {
        auto now = clock::now();
        double dt = std::chrono::duration<double>(now - last).count();
        last = now;

        app.update(dt);
        app.render();

        // В реальном приложении здесь будет ожидание событий / vsync
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    app.shutdown();
    return 0;
}