#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

enum class token_type
{
    Identifier, Number, String,
    Let, Const, Class, Function,
    Int, Double, Bool, Return,
    Plus, Minus, Star, Slash,
    Equal, EqualEqual, PlusPlus,
    LParen, RParen, LBrace, RBrace,
    Semicolon, Colon, Comma, Print, MinusMinus,
    For, While, In, Less, Greater, BangEqual, LessEqual, GreaterEqual,
    LBracket, RBracket,
    End, Unexpected
};

inline const std::unordered_map<token_type, std::string> tokenToString = {
    {token_type::Identifier, "Identifier"},
    {token_type::Number, "Number"},
    {token_type::String, "String"},
    {token_type::Let, "Let"},
    {token_type::Const, "Const"},
    {token_type::Class, "Class"},
    {token_type::Function, "Function"},
    {token_type::Int, "Int"},
    {token_type::Print, "print"},
    {token_type::Double, "Double"},
    {token_type::Bool, "Bool"},
    {token_type::Return, "Return"},
    {token_type::Less, "Less"},
    {token_type::Greater, "Greater"},
    {token_type::GreaterEqual, "GreaterEqual"},
    {token_type::BangEqual, "BangEqual"},
    {token_type::LessEqual, "LessEqual"},
    {token_type::LBracket, "LBracket"},
    {token_type::RBracket, "RBracket"},
    {token_type::Plus, "Plus"},
    {token_type::Minus, "Minus"},
    {token_type::For, "For"},
    {token_type::In, "In"},
    {token_type::While, "While"},
    {token_type::Star, "Star"},
    {token_type::Slash, "Slash"},
    {token_type::Equal, "Equal"},
    {token_type::EqualEqual, "EqualEqual"},
    {token_type::PlusPlus, "PlusPlus"},
    {token_type::MinusMinus, "MinusMinus"},
    {token_type::LParen, "LParen"},
    {token_type::RParen, "RParen"},
    {token_type::LBrace, "LBrace"},
    {token_type::RBrace, "RBrace"},
    {token_type::Semicolon, "Semicolon"},
    {token_type::Colon, "Colon"},
    {token_type::Comma, "Comma"},
    {token_type::End, "End"},
    {token_type::Unexpected, "Unexpected"}
};

struct Token {
    token_type type;
    std::string lexeme;
    int line;
    int column;

    bool operator==(const Token& other) const {
        return type == other.type && lexeme == other.lexeme;
    }
};

class Lexer {
public:
    Lexer(const std::string& source)
        : source(source), current(0), line(1), column(1) {}

    Token nextToken() {
        skipWhitespace();
        start = current;
        line_start = line;
        column_start = column;

        if (isAtEnd()) return makeToken(token_type::End);

        char c = advance();

        if (isAlpha(c)) return identifier();
        if (isdigit(c)) return number();
        if (c == '"') return stringLiteral();

        switch (c) {
        case '(': return makeToken(token_type::LParen);
        case ')': return makeToken(token_type::RParen);
        case '{': return makeToken(token_type::LBrace);
        case '[': return makeToken(token_type::LBracket);
        case '<': return makeToken(token_type::Less);
        case '<=': return makeToken(token_type::LessEqual);
        case '>': return makeToken(token_type::Greater);
        case '>=': return makeToken(token_type::GreaterEqual);
        case ']': return makeToken(token_type::RBracket);
        case '}': return makeToken(token_type::RBrace);
        case ';': return handleSemicolon();
        case ':': return makeToken(token_type::Colon);
        case ',': return makeToken(token_type::Comma);
        case '=': return match('=') ? makeToken(token_type::EqualEqual) : makeToken(token_type::Equal);
        case '+': return match('+') ? makeToken(token_type::PlusPlus) : makeToken(token_type::Plus);
        case '-': return match('-') ? makeToken(token_type::MinusMinus) : makeToken(token_type::Minus);
        case '*': return makeToken(token_type::Star);
        case '/': return makeToken(token_type::Slash);
        }

        return errorToken("Unexpected character");
    }

private:
    const std::string source;
    size_t current;
    size_t start = 0;
    int line;
    int column;
    int line_start = 1;
    int column_start = 1;

    static const std::unordered_map<std::string, token_type> keywords;

    bool isAtEnd() const { return current >= source.size(); }

    char advance() {
        if (isAtEnd()) return '\0';
        char c = source[current++];
        if (c == '\n') {
            line++;
            column = 1;
        }
        else {
            column++;
        }
        return c;
    }

    char peek() const { return isAtEnd() ? '\0' : source[current]; }
    char peekNext() const { return (current + 1 >= source.size()) ? '\0' : source[current + 1]; }

    bool match(char expected) {
        if (isAtEnd()) return false;
        if (source[current] != expected) return false;
        advance();
        return true;
    }

    Token makeToken(token_type type) {
        return { type, source.substr(start, current - start), line_start, column_start };
    }

    Token errorToken(const std::string& message) {
        return { token_type::Unexpected, message, line_start, column_start };
    }

    void skipWhitespace() {
        while (true) {
            switch (peek()) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    while (peek() != '\n' && !isAtEnd()) advance();
                }
                else {
                    return;
                }
                break;
            default:
                return;
            }
        }
    }

    Token identifier() {
        while (isAlphaNumeric(peek())) advance();

        std::string text = source.substr(start, current - start);
        auto it = keywords.find(text);
        if (it != keywords.end()) {
            return { it->second, text, line_start, column_start };
        }
        return makeToken(token_type::Identifier);
    }

    Token number() {
        while (isdigit(peek())) advance();

        if (peek() == '.' && isdigit(peekNext())) {
            advance();
            while (isdigit(peek())) advance();
        }

        return makeToken(token_type::Number);
    }

    Token stringLiteral() {
        while (peek() != '"' && !isAtEnd()) {
            if (peek() == '\\') advance(); // handle escape sequences
            advance();
        }

        if (isAtEnd()) return errorToken("Unterminated string");
        advance(); // Closing "
        return makeToken(token_type::String);
    }

    Token handleSemicolon() {
        if (current > 1 && source[current - 2] == '\n') {
            current--;
            return nextToken();
        }
        return makeToken(token_type::Semicolon);
    }

    static bool isAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    static bool isAlphaNumeric(char c) {
        return isAlpha(c) || isdigit(c);
    }
};

inline const std::unordered_map<std::string, token_type> Lexer::keywords = {
    {"let", token_type::Let},
    {"const", token_type::Const},
    {"class", token_type::Class},
    {"function", token_type::Function},
    {"int", token_type::Int},
    {"double", token_type::Double},
    {"bool", token_type::Bool},
    {"for", token_type::For},
    {"while", token_type::While},
    {"in", token_type::In},
    {"print", token_type::Print},
    {"return", token_type::Return}
};

inline std::string tokenTypeToString(token_type type) {
    auto it = tokenToString.find(type);
    return it != tokenToString.end() ? it->second : "Unknown";
}