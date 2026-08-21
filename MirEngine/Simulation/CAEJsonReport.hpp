#pragma once

#include <cmath>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

namespace mir
{

class JsonValue
{
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    JsonValue() noexcept : type_{Type::Null} {}
    JsonValue(bool value) noexcept : type_{Type::Bool}, bool_{value} {}
    JsonValue(double value) noexcept : type_{Type::Number}, number_{value} {}
    JsonValue(int value) noexcept : type_{Type::Number}, number_{static_cast<double>(value)} {}
    JsonValue(std::string_view value) noexcept : type_{Type::String}, string_{value} {}

    [[nodiscard]] static JsonValue array() noexcept { JsonValue v; v.type_ = Type::Array; return v; }
    [[nodiscard]] static JsonValue object() noexcept { JsonValue v; v.type_ = Type::Object; return v; }

    JsonValue& push(const JsonValue& value) noexcept
    {
        type_ = Type::Array;
        array_.push_back(value);
        return *this;
    }

    JsonValue& set(std::string_view key, const JsonValue& value) noexcept
    {
        type_ = Type::Object;
        object_.emplace_back(std::string(key), value);
        return *this;
    }

    JsonValue& set(std::string_view key, std::string_view value) noexcept
    {
        type_ = Type::Object;
        object_.emplace_back(std::string(key), JsonValue(value));
        return *this;
    }

    [[nodiscard]] std::string toString() const
    {
        std::string out;
        write(out, 0);
        out += '\n';
        return out;
    }

private:
    static std::string indent(int level)
    {
        return std::string(static_cast<std::size_t>(level) * 2, ' ');
    }

    static std::string formatNumber(double value) noexcept
    {
        if (!std::isfinite(value))
            return "0";
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%.12g", value);
        return buffer;
    }

    static std::string escape(std::string_view value)
    {
        std::string out;
        out.reserve(value.size() + 2);
        for (const char ch : value)
        {
            switch (ch)
            {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\t': out += "\\t"; break;
                case '\r': out += "\\r"; break;
                default: out += ch; break;
            }
        }
        return out;
    }

    void write(std::string& out, int level) const
    {
        switch (type_)
        {
            case Type::Null:
                out += "null";
                break;
            case Type::Bool:
                out += bool_ ? "true" : "false";
                break;
            case Type::Number:
                out += formatNumber(number_);
                break;
            case Type::String:
                out += '"';
                out += escape(string_);
                out += '"';
                break;
            case Type::Array:
            {
                out += "[\n";
                for (std::size_t i = 0; i < array_.size(); ++i)
                {
                    out += indent(level + 1);
                    array_[i].write(out, level + 1);
                    if (i + 1 < array_.size())
                        out += ',';
                    out += '\n';
                }
                out += indent(level);
                out += ']';
                break;
            }
            case Type::Object:
            {
                out += "{\n";
                for (std::size_t i = 0; i < object_.size(); ++i)
                {
                    out += indent(level + 1);
                    out += '"';
                    out += escape(object_[i].first);
                    out += "\": ";
                    object_[i].second.write(out, level + 1);
                    if (i + 1 < object_.size())
                        out += ',';
                    out += '\n';
                }
                out += indent(level);
                out += '}';
                break;
            }
        }
    }

    Type type_{Type::Null};
    bool bool_{false};
    double number_{0.0};
    std::string string_{};
    std::vector<JsonValue> array_{};
    std::vector<std::pair<std::string, JsonValue>> object_{};
};

}
