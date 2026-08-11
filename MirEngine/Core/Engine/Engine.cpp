// MirEngine/Core/Engine/Engine.cpp
// ⚙️ Реализация главного класса движка MirEngine.
//
// Engine — центральный объект, владеющий IDGenerator, Logger и (в будущем)
// остальными подсистемами. Создаётся один раз при старте и живёт до завершения.
//
// Через Engine все подсистемы получают доступ к общим ресурсам без глобалов
// и синглтонов. Это делает код тестируемым и предсказуемым.
//
// Изменения относительно исходной версии:
//   • Полная поддержка EngineConfig (logLevel, enableLogging, applicationName).
//   • Использование enum EngineState вместо простого bool.
//   • Корректный переход состояний Created → Initializing → Ready → ShuttingDown → Shutdown.
//   • Применение настроек логирования при инициализации.
//   • Более подробные сообщения и проверки.

#include "Engine.hpp"
#include "../Logging/Logger.hpp"
#include "../IDs/IDGenerator.hpp"

namespace mir {

// ── Конструктор ──────────────────────────────────────────────
// Сохраняет конфигурацию и создаёт подсистемы (ещё не инициализированные).
Engine::Engine(EngineConfig config)
    : m_config(std::move(config))
    , m_idGenerator(std::make_unique<IDGenerator>())
    , m_logger(std::make_unique<Logger>())
    , m_state(EngineState::Created)
{
    // Движок создан, но ресурсы ещё не выделены и конфигурация не применена.
    // Всё это происходит в initialize().
}

// ── Деструктор ───────────────────────────────────────────────
// Гарантирует корректное завершение, даже если пользователь забыл вызвать shutdown().
Engine::~Engine()
{
    if (m_state != EngineState::Shutdown && m_state != EngineState::Created) {
        shutdown();
    }
}

// ── Инициализация ────────────────────────────────────────────
// Применяет конфигурацию, настраивает логгер и переводит движок в Ready.
Result<void> Engine::initialize()
{
    // Повторная инициализация запрещена.
    if (m_state != EngineState::Created) {
        return ErrorCode::InvalidState;
    }

    m_state = EngineState::Initializing;

    // ── Применение конфигурации логирования ──────────────────
    if (m_config.enableLogging) {
        // Преобразуем int logLevel из конфига в enum LogLevel.
        // 0=Trace/Debug, 1=Debug, 2=Info, 3=Warning, 4=Error, 5=Critical
        LogLevel level = LogLevel::Info;
        switch (m_config.logLevel) {
            case 0: // Trace — в текущем Logger нет отдельного Trace, используем Debug
            case 1: level = LogLevel::Debug;    break;
            case 2: level = LogLevel::Info;     break;
            case 3: level = LogLevel::Warning;  break;
            case 4: level = LogLevel::Error;    break;
            case 5: level = LogLevel::Critical; break;
            default: level = LogLevel::Info;    break;
        }
        m_logger->setLevel(level);
    } else {
        // Полностью отключаем логирование, поднимая порог выше Critical.
        // (В будущем можно добавить отдельный флаг в Logger.)
        m_logger->setLevel(static_cast<LogLevel>(6));
    }

    // Здесь в будущем:
    //   - загрузка конфигурации из файла
    //   - инициализация графического бэкенда (Metal / DirectX / Vulkan / OpenGL)
    //   - подключение плагинов
    //   - выделение внутренних структур

    m_state = EngineState::Ready;

    // Логируем успешный старт (если логирование включено).
    if (m_config.enableLogging) {
        m_logger->info("Engine initialized. Application: " + m_config.applicationName);
    }

    return {}; // успех
}

// ── Завершение работы ───────────────────────────────────────
void Engine::shutdown()
{
    // Уже остановлен или ещё не создан — ничего не делаем.
    if (m_state == EngineState::Shutdown || m_state == EngineState::Created) {
        return;
    }

    m_state = EngineState::ShuttingDown;

    // Здесь в будущем:
    //   - сохранение состояния
    //   - выгрузка плагинов
    //   - освобождение графических ресурсов
    //   - публикация события EngineShutdown

    if (m_config.enableLogging) {
        m_logger->info("Engine shutting down. Application: " + m_config.applicationName);
    }

    m_state = EngineState::Shutdown;
}

// ── Проверка состояния ──────────────────────────────────────
bool Engine::isInitialized() const noexcept
{
    return m_state == EngineState::Ready || m_state == EngineState::Running;
}

EngineState Engine::state() const noexcept
{
    return m_state;
}

// ── Доступ к генератору идентификаторов ─────────────────────
IDGenerator& Engine::idGenerator() noexcept
{
    return *m_idGenerator;
}

const IDGenerator& Engine::idGenerator() const noexcept
{
    return *m_idGenerator;
}

// ── Доступ к логгеру ─────────────────────────────────────────
Logger& Engine::logger() noexcept
{
    return *m_logger;
}

const Logger& Engine::logger() const noexcept
{
    return *m_logger;
}

// ── Доступ к конфигурации ────────────────────────────────────
const EngineConfig& Engine::config() const noexcept
{
    return m_config;
}

} // namespace mir