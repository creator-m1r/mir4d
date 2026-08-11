// MirEngine/Core/Result/Result.hpp
// 🏆 Безопасный результат операции — значение или ошибка.
//
// В классическом C++ об ошибках часто сообщают через:
//   bool success = doSomething();
//   if (!success) { ... }
// Но такой подход не передаёт детали ошибки и неудобен,
// когда операция должна вернуть значение (например, созданный объект).
//
// Result<T> решает эту проблему элегантно: он хранит либо
// успешное значение типа T, либо код ошибки (ErrorCode) с описанием.
// Это как коробка, в которой лежит либо подарок (результат),
// либо записка с объяснением, почему подарка нет.
//
// Использование:
//   Result<Vector3> result = calculatePosition();
//   if (result) {
//       Vector3 pos = result.value();   // используем значение
//   } else {
//       ErrorCode code = result.error(); // обрабатываем ошибку
//       Logger::error(result.message());
//   }
//
// Чистый C++23, без внешних зависимостей.


#pragma once

#include <string>
#include <variant>
#include <stdexcept>
#include <optional>               // добавлен для std::optional

namespace mir {

enum class ErrorCode {
    None = 0,
    InvalidArgument,
    InvalidState,
    NotFound,
    AlreadyExists,
    GeometryError,
    OperationFailed,
    FileError,
    SerializationError,
    Unknown
};

inline const char* errorMessage(ErrorCode code) {
    switch (code) {
        case ErrorCode::None:              return "Нет ошибки";
        case ErrorCode::InvalidArgument:   return "Неверный аргумент";
        case ErrorCode::InvalidState:      return "Неверное состояние";
        case ErrorCode::NotFound:          return "Не найдено";
        case ErrorCode::AlreadyExists:     return "Уже существует";
        case ErrorCode::GeometryError:     return "Геометрическая ошибка";
        case ErrorCode::OperationFailed:   return "Операция не удалась";
        case ErrorCode::FileError:         return "Ошибка файла";
        case ErrorCode::SerializationError:return "Ошибка сериализации";
        default:                           return "Неизвестная ошибка";
    }
}

struct Error {
    ErrorCode code = ErrorCode::None;
    std::string message;

    explicit Error(ErrorCode c)
        : code(c), message(errorMessage(c))
    {}

    Error(ErrorCode c, std::string msg)
        : code(c), message(std::move(msg))
    {}
};

template<typename T>
class Result {
public:
    Result(const T& value) : m_data(value) {}
    Result(T&& value) noexcept : m_data(std::move(value)) {}
    Result(const Error& error) : m_data(error) {}
    Result(Error&& error) noexcept : m_data(std::move(error)) {}
    Result(ErrorCode code) : m_data(Error(code)) {}

    [[nodiscard]] bool ok() const noexcept { return std::holds_alternative<T>(m_data); }
    [[nodiscard]] explicit operator bool() const noexcept { return ok(); }
    [[nodiscard]] bool isError() const noexcept { return std::holds_alternative<Error>(m_data); }

    [[nodiscard]] T& value() {
        if (!ok()) throw std::runtime_error("Result::value() вызван на ошибочном результате");
        return std::get<T>(m_data);
    }
    [[nodiscard]] const T& value() const {
        if (!ok()) throw std::runtime_error("Result::value() вызван на ошибочном результате");
        return std::get<T>(m_data);
    }
    [[nodiscard]] T valueOr(T defaultValue) const noexcept {
        return ok() ? std::get<T>(m_data) : defaultValue;
    }

    [[nodiscard]] const Error& error() const {
        if (!isError()) throw std::runtime_error("Result::error() вызван на успешном результате");
        return std::get<Error>(m_data);
    }
    [[nodiscard]] ErrorCode errorCode() const {
        return isError() ? std::get<Error>(m_data).code : ErrorCode::None;
    }
    [[nodiscard]] std::string errorMessage() const {
        return isError() ? std::get<Error>(m_data).message : std::string();
    }

private:
    std::variant<T, Error> m_data;
};

// Специализация для void
template<>
class Result<void> {
public:
    Result() noexcept = default;
    Result(const Error& error) : m_error(error) {}
    Result(Error&& error) noexcept : m_error(std::move(error)) {}
    Result(ErrorCode code) : m_error(Error(code)) {}

    [[nodiscard]] bool ok() const noexcept { return !m_error.has_value(); }
    [[nodiscard]] explicit operator bool() const noexcept { return ok(); }
    [[nodiscard]] bool isError() const noexcept { return m_error.has_value(); }

    [[nodiscard]] ErrorCode errorCode() const {
        return m_error ? m_error->code : ErrorCode::None;
    }
    [[nodiscard]] std::string errorMessage() const {
        return m_error ? m_error->message : std::string();
    }

private:
    std::optional<Error> m_error;
};

} // namespace mir