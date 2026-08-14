// MirEngine/Core/Engine/Engine.cpp
// ⚙️ Реализация главного класса движка MirEngine.

#include "Engine.hpp"
#include "../Logging/Logger.hpp"
#include "../IDs/IDGenerator.hpp"

namespace mir {

Engine::Engine(EngineConfig config)
    : m_config(std::move(config))
    , m_idGenerator(std::make_unique<IDGenerator>())
    , m_logger(std::make_unique<Logger>())
    , m_state(EngineState::Created)
{
}

Engine::~Engine()
{
    if (m_state != EngineState::Shutdown && m_state != EngineState::Created) {
        shutdown();
    }
}

Result<void> Engine::initialize()
{
    if (m_state != EngineState::Created) {
        return ErrorCode::InvalidState;
    }

    m_state = EngineState::Initializing;

    if (m_config.enableLogging) {
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
        // Используем явный LogLevel::Off вместо магического числа
        m_logger->setLevel(LogLevel::Off);
    }

    m_state = EngineState::Ready;

    if (m_config.enableLogging) {
        m_logger->info("Engine initialized. Application: " + m_config.applicationName);
    }

    return {};
}

void Engine::shutdown()
{
    if (m_state == EngineState::Shutdown || m_state == EngineState::Created) {
        return;
    }

    m_state = EngineState::ShuttingDown;

    if (m_config.enableLogging) {
        m_logger->info("Engine shutting down. Application: " + m_config.applicationName);
    }

    m_state = EngineState::Shutdown;
}

bool Engine::isInitialized() const noexcept
{
    return m_state == EngineState::Ready || m_state == EngineState::Running;
}

EngineState Engine::state() const noexcept
{
    return m_state;
}

IDGenerator& Engine::idGenerator() noexcept
{
    return *m_idGenerator;
}

const IDGenerator& Engine::idGenerator() const noexcept
{
    return *m_idGenerator;
}

Logger& Engine::logger() noexcept
{
    return *m_logger;
}

const Logger& Engine::logger() const noexcept
{
    return *m_logger;
}

const EngineConfig& Engine::config() const noexcept
{
    return m_config;
}

} // namespace mir
