// MirEngine/Core/Engine/Engine.hpp
// ⚙️ Главный класс движка MirEngine — владеет всеми ключевыми подсистемами.
//
// Engine — центральная точка входа. Создаётся один раз при старте приложения
// и предоставляет доступ к:
//   • IDGenerator    — генератору уникальных идентификаторов
//   • Logger         — системе логирования
//   • (в будущем)    — менеджерам сцен, документов, геометрии, рендеринга
//
// Engine управляет жизненным циклом: initialize() → Ready/Running → shutdown().
// Никаких глобальных переменных — всё хранится внутри Engine и передаётся
// по ссылке. Поддерживает EngineConfig и EngineState.
//
// Использование:
//   EngineConfig config;
//   config.applicationName = "MIR4D";
//   config.logLevel = 1; // Debug
//   Engine engine(config);
//   if (auto res = engine.initialize(); !res) {
//       // обработка ошибки
//   }
//   auto& gen = engine.idGenerator();
//   EntityID id = gen.createEntity();
//   engine.logger().info("Движок запущен");
//   engine.shutdown();
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Result/Result.hpp"         // Result<void>, ErrorCode
#include "../IDs/IDGenerator.hpp"       // IDGenerator
#include "../Logging/Logger.hpp"        // Logger, LogLevel
#include "EngineConfig.hpp"             // EngineConfig
#include "EngineState.hpp"              // EngineState
#include <memory>                       // std::unique_ptr
#include <string>                       // std::string (для applicationName)

namespace mir {

class Engine {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Создаёт движок в состоянии Created.
    // Принимает конфигурацию (по умолчанию — разумные значения из EngineConfig).
    explicit Engine(EngineConfig config = EngineConfig{});

    // ── Деструктор ───────────────────────────────────────────
    // Автоматически вызывает shutdown(), если движок ещё не остановлен.
    ~Engine();

    // ── Запрет копирования и перемещения ─────────────────────
    // Engine — единственный владелец ресурсов. Копирование/перемещение
    // нарушило бы инварианты владения.
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    // ── Инициализация ────────────────────────────────────────
    // Выделяет ресурсы, применяет конфигурацию, переводит движок в Ready.
    // Можно вызывать только из состояния Created.
    // Возвращает Result<void>: успех или ErrorCode (InvalidState и т.д.).
    [[nodiscard]] Result<void> initialize();

    // ── Завершение работы ───────────────────────────────────
    // Корректно освобождает ресурсы и переводит в Shutdown.
    // Безопасно вызывать несколько раз.
    void shutdown();

    // ── Проверка состояния ──────────────────────────────────
    // Возвращает true, если движок успешно инициализирован
    // и находится в состоянии Ready или Running.
    [[nodiscard]] bool isInitialized() const noexcept;

    // Текущее состояние жизненного цикла (Created, Ready, Running и т.д.).
    [[nodiscard]] EngineState state() const noexcept;

    // ── Генератор идентификаторов ───────────────────────────
    // Создаёт EntityID, ObjectID, ComponentID, FeatureID, DocumentID, ProjectID.
    [[nodiscard]] IDGenerator& idGenerator() noexcept;
    [[nodiscard]] const IDGenerator& idGenerator() const noexcept;

    // ── Логгер ──────────────────────────────────────────────
    // Центральная система логирования. Уровень и включение берутся из EngineConfig.
    [[nodiscard]] Logger& logger() noexcept;
    [[nodiscard]] const Logger& logger() const noexcept;

    // ── Конфигурация (только для чтения после создания) ─────
    [[nodiscard]] const EngineConfig& config() const noexcept;

private:
    EngineConfig                 m_config;          // копия конфигурации
    std::unique_ptr<IDGenerator> m_idGenerator;     // владеет генератором ID
    std::unique_ptr<Logger>      m_logger;          // владеет логгером
    EngineState                  m_state = EngineState::Created; // текущее состояние
};

} // namespace mir