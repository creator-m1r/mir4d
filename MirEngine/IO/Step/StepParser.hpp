#pragma once

#include <cctype>
#include <cstdlib>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Header-only ISO 10303-21 (STEP) parser used by the native tessellated
// STEP codec. It intentionally covers the subset MIR 4D emits (faceted_brep)
// and the common AP242 tessellated representations, without linking
// OpenCASCADE. Exact B-Rep mapping is provided natively by BRepStepBridge.

namespace mir::io::step::parser
{

struct Param
{
    enum class Kind
    {
        Real,
        Integer,
        String,
        Enum,
        Reference,
        List
    };

    Kind kind{Kind::List};
    double realValue{0.0};
    long long intValue{0};
    std::string text; // String content, raw Enum (e.g. ".T.") or Reference digits
    std::vector<Param> items;

    [[nodiscard]] bool asReference(int& outId) const
    {
        if (kind != Kind::Reference)
            return false;
        try
        {
            outId = std::stoi(text);
        }
        catch (...)
        {
            return false;
        }
        return true;
    }

    [[nodiscard]] bool asReal(double& out) const
    {
        if (kind == Kind::Real)
        {
            out = realValue;
            return true;
        }
        if (kind == Kind::Integer)
        {
            out = static_cast<double>(intValue);
            return true;
        }
        return false;
    }
};

struct Entity
{
    int id{0};
    std::string type;
    std::vector<Param> params;
};

struct StepFile
{
    std::unordered_map<int, Entity> entities;

    [[nodiscard]] const Entity* find(int id) const
    {
        auto it = entities.find(id);
        return it == entities.end() ? nullptr : &it->second;
    }
};

class Scanner
{
public:
    explicit Scanner(const std::string& source) : s_(source) {}

    void skipWhitespaceAndComments()
    {
        while (pos_ < s_.size())
        {
            const char c = s_[pos_];
            if (std::isspace(static_cast<unsigned char>(c)) != 0)
            {
                ++pos_;
                continue;
            }
            if (c == '/' && pos_ + 1 < s_.size() && s_[pos_ + 1] == '*')
            {
                pos_ += 2;
                while (pos_ + 1 < s_.size() && !(s_[pos_] == '*' && s_[pos_ + 1] == '/'))
                    ++pos_;
                pos_ += 2;
                continue;
            }
            break;
        }
    }

    [[nodiscard]] bool atEnd() const { return pos_ >= s_.size(); }

    char peek() const { return pos_ < s_.size() ? s_[pos_] : '\0'; }

    bool startsWith(const char* token) const
    {
        const std::size_t len = std::char_traits<char>::length(token);
        if (pos_ + len > s_.size())
            return false;
        for (std::size_t i = 0; i < len; ++i)
            if (s_[pos_ + i] != token[i])
                return false;
        return true;
    }

    void advance(std::size_t n = 1) { pos_ += n; }

    char next() { return pos_ < s_.size() ? s_[pos_++] : '\0'; }

    // Reads one parameter value (recursively for nested lists).
    Param readParam()
    {
        skipWhitespaceAndComments();
        Param p;

        if (peek() == '(')
        {
            next(); // consume '('
            p.kind = Param::Kind::List;
            for (;;)
            {
                skipWhitespaceAndComments();
                if (peek() == ')')
                {
                    next();
                    break;
                }
                // A list element (combined EXPRESS instance members may be
                // whitespace-separated rather than comma-separated).
                p.items.push_back(readParam());
                skipWhitespaceAndComments();
                if (peek() == ',')
                {
                    next();
                    continue;
                }
                if (peek() == ')')
                {
                    next();
                    break;
                }
                // Whitespace-only separator: continue and let the next
                // iteration either read the next item or stop at ')'.
            }
            return p;
        }

        if (peek() == '#')
        {
            next();
            std::string digits;
            while (std::isdigit(static_cast<unsigned char>(peek())) != 0)
                digits.push_back(next());
            p.kind = Param::Kind::Reference;
            p.text = digits;
            return p;
        }

        if (peek() == '\'')
        {
            next(); // opening quote
            std::string str;
            while (!atEnd() && peek() != '\'')
                str.push_back(next());
            if (peek() == '\'')
                next();
            // STEP doubles an embedded quote as '' — collapse them.
            std::string collapsed;
            collapsed.reserve(str.size());
            for (std::size_t i = 0; i < str.size(); ++i)
            {
                if (str[i] == '\'' && i + 1 < str.size() && str[i + 1] == '\'')
                {
                    collapsed.push_back('\'');
                    ++i;
                }
                else
                {
                    collapsed.push_back(str[i]);
                }
            }
            p.kind = Param::Kind::String;
            p.text = collapsed;
            return p;
        }

        if (peek() == '$' || peek() == '*')
        {
            p.kind = Param::Kind::Enum;
            p.text.push_back(next());
            return p;
        }

        const char c = peek();
        bool isNumberStart = std::isdigit(static_cast<unsigned char>(c)) != 0 ||
                             c == '+' || c == '-';
        if (!isNumberStart && c == '.')
        {
            // '.' starts a number only when followed by a digit (e.g. ".5").
            // Enum tokens like .T. / .FALSE. start with '.' + letter.
            if (pos_ + 1 < s_.size() && std::isdigit(static_cast<unsigned char>(s_[pos_ + 1])) != 0)
                isNumberStart = true;
        }
        if (isNumberStart)
        {
            std::string token;
            token.push_back(next());
            while (!atEnd())
            {
                const char d = peek();
                if (std::isalnum(static_cast<unsigned char>(d)) != 0 ||
                    d == '.' || d == '+' || d == '-' || d == 'e' || d == 'E')
                    token.push_back(next());
                else
                    break;
            }
            bool isReal = false;
            for (char d : token)
                if (d == '.' || d == 'e' || d == 'E')
                    isReal = true;
            if (isReal)
            {
                p.kind = Param::Kind::Real;
                p.realValue = std::strtod(token.c_str(), nullptr);
            }
            else
            {
                p.kind = Param::Kind::Integer;
                p.intValue = std::strtoll(token.c_str(), nullptr, 10);
            }
            return p;
        }

        // Enum / keyword token, e.g. .T. .FALSE. .PLUS.FC.
        std::string token;
        while (!atEnd())
        {
            const char d = peek();
            if (std::isspace(static_cast<unsigned char>(d)) != 0 ||
                d == '(' || d == ')' || d == ',' || d == ';' || d == '\'')
                break;
            token.push_back(next());
        }
        p.kind = Param::Kind::Enum;
        p.text = token;
        return p;
    }

    // Reads "#id = TYPE( ... );"
    bool readEntity(Entity& out)
    {
        skipWhitespaceAndComments();
        if (peek() != '#')
            return false;
        next();
        std::string digits;
        while (std::isdigit(static_cast<unsigned char>(peek())) != 0)
            digits.push_back(next());
        if (digits.empty())
            return false;
        try
        {
            out.id = std::stoi(digits);
        }
        catch (...)
        {
            return false;
        }

        skipWhitespaceAndComments();
        if (peek() != '=')
            return false;
        next();

        skipWhitespaceAndComments();
        std::string type;
        while (!atEnd())
        {
            const char d = peek();
            if (std::isalnum(static_cast<unsigned char>(d)) != 0 || d == '_')
                type.push_back(next());
            else
                break;
        }
        out.type = type;

        skipWhitespaceAndComments();
        if (peek() != '(')
            return false;
        // The top-level parameter list is a list; reuse readParam.
        Param root = readParam();
        out.params = std::move(root.items);

        skipWhitespaceAndComments();
        if (peek() == ';')
            next();
        return true;
    }

private:
    const std::string& s_;
    std::size_t pos_{0};
};

inline std::unique_ptr<StepFile> parse(const std::string& text)
{
    auto file = std::make_unique<StepFile>();
    Scanner scanner(text);

    // Jump to the DATA section.
    std::size_t dataPos = text.find("DATA;");
    if (dataPos != std::string::npos)
        scanner.advance(dataPos + 5);

    for (;;)
    {
        scanner.skipWhitespaceAndComments();
        if (scanner.atEnd())
            break;
        if (scanner.startsWith("ENDSEC"))
            break;
        if (scanner.startsWith("END-ISO-10303-21"))
            break;

        Entity entity;
        if (!scanner.readEntity(entity))
            break;
        if (entity.id != 0)
            file->entities.emplace(entity.id, std::move(entity));
    }

    return file;
}

} // namespace mir::io::step::parser
