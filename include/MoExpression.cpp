#include "MoExpression.h"
#include <iostream>
#include <string>
#include <map>
#include <queue>
#include <stdexcept>
#include <cctype>
#include <algorithm>
#include <vector>
#include <stack>
#include <limits>
#include <memory>
#include <cstdint>
#include <cstring>

namespace {
std::int32_t signedValueFromBits(std::uint32_t bits) {
    static_assert(sizeof(std::int32_t) == sizeof(bits), "int32_t must be 32 bits");
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::int32_t parseInteger(const std::string& text) {
    const long long value = std::stoll(text);
    if (value < std::numeric_limits<std::int32_t>::min() ||
        value > std::numeric_limits<std::int32_t>::max())
        throw std::out_of_range("integer literal out of range");
    return static_cast<std::int32_t>(value);
}

std::int32_t wrapAdd(std::int32_t left, std::int32_t right) {
    const auto bits = static_cast<std::uint64_t>(static_cast<std::uint32_t>(left)) +
                      static_cast<std::uint32_t>(right);
    return signedValueFromBits(static_cast<std::uint32_t>(bits));
}

std::int32_t wrapSubtract(std::int32_t left, std::int32_t right) {
    const auto bits = static_cast<std::uint64_t>(static_cast<std::uint32_t>(left)) -
                      static_cast<std::uint32_t>(right);
    return signedValueFromBits(static_cast<std::uint32_t>(bits));
}

std::int32_t wrapMultiply(std::int32_t left, std::int32_t right) {
    const auto bits = static_cast<std::uint64_t>(static_cast<std::uint32_t>(left)) *
                      static_cast<std::uint32_t>(right);
    return signedValueFromBits(static_cast<std::uint32_t>(bits));
}
}

Bin::Bin(Exp* l, Exp* r, const std::string& o) : left(l), right(r), op(o) {}

Bin::~Bin() {
    delete left;
    delete right;
}

Exp* Bin::getLeft() const {
    return left;
}

Exp* Bin::getRight() const {
    return right;
}

const std::string& Bin::getOp() const {
    return op;
}

void Bin::print(std::ostream& out) {
    left->print(out);
    out << op;
    right->print(out);
}

std::int32_t Bin::eval(Env& env) {
    std::int32_t l = left->eval(env);
    std::int32_t r = right->eval(env);
    if (op == "+")
        return wrapAdd(l, r);
    if (op == "-")
        return wrapSubtract(l, r);
    if (op == "*")
        return wrapMultiply(l, r);
    if (op == "/") {
        if (r == 0) throw std::runtime_error("division by zero");
        if (l == std::numeric_limits<std::int32_t>::min() && r == -1)
            return std::numeric_limits<std::int32_t>::min();
        return l / r;
    }
    if (op == "%") {
        if (r == 0) throw std::runtime_error("modulo by zero");
        if (l == std::numeric_limits<std::int32_t>::min() && r == -1)
            return 0;
        return l % r;
    }
    if (op == "==")
        return l == r;
    if (op == "!=")
        return l != r;
    if (op == "<")
        return l < r;
    if (op == ">")
        return l > r;
    if (op == "<=")
        return l <= r;
    if (op == ">=")
        return l >= r;
    if (op == "&")
        return signedValueFromBits(static_cast<std::uint32_t>(l) &
                                   static_cast<std::uint32_t>(r));
    if (op == "|")
        return signedValueFromBits(static_cast<std::uint32_t>(l) |
                                   static_cast<std::uint32_t>(r));
    throw std::invalid_argument("unknown operator");
}

Var::Var(const std::string& n) : name(n) {}

std::string Var::getName() const {
    return name;
}

void Var::print(std::ostream& out) {
    out << '{' << name << '}';
}

std::int32_t Var::eval(Env& env) {
    return env[name];
}

Num::Num(std::int32_t v) : value(v) {}

std::int32_t Num::getValue() const {
    return value;
}

void Num::print(std::ostream& out) {
    out << value;
}

std::int32_t Num::eval([[maybe_unused]] Env& env) {
    return value;
}

AtExp::AtExp(Exp* e) : addrExp(e) {}

AtExp::~AtExp()  {
    delete addrExp;
}

Exp* AtExp::getAddrExp() const {
    return addrExp;
}

std::int32_t AtExp::eval(Env& env) {
    std::int32_t addr = addrExp->eval(env);
    std::string varName = "@" + std::to_string(addr);
    return env[varName];
}

std::string AtExp::getTargetName(Env& env) const {
    std::int32_t addr = addrExp->eval(env);
    return "@" + std::to_string(addr);
}

void AtExp::print(std::ostream& out) {
    out << "@";
    addrExp->print(out);
}

Token::Token(int t, const std::string& txt) : type(t), text(txt) {}

StreamTokenizer::StreamTokenizer(std::istream& is) : input(is), end_of_stream(false) {}

char StreamTokenizer::next_char() {
    char c;
    while (input.get(c)) {
        if (!std::isspace(c))
            return c;
    }
    return EOF;
}

void StreamTokenizer::fill_buffer() {
    if (end_of_stream)
        return;
    char c = next_char();
    if (c == EOF) {
        end_of_stream = true;
        return;
    }
    if (std::isdigit(c)) {
        std::string num(1, c);
        while (input.peek() != EOF && std::isdigit(input.peek()))
            num += input.get();
        buffer.push(num);
    }
    else if (std::isalpha(c)) {
        std::string ident(1, c);
        while (input.peek() != EOF && std::isalpha(input.peek()))
            ident += input.get();
        buffer.push(ident);
    }
    else if (c == '=') {
        if (input.peek() == '=') {
            input.get();
            buffer.push("==");
        }
        else
            buffer.push("=");
    }
    else if (c == '!') {
        if (input.peek() == '=') {
            input.get();
            buffer.push("!=");
        }
        else
            throw std::runtime_error("unexpected '!'");
    }
    else if (c == '<') {
        if (input.peek() == '=') {
            input.get();
            buffer.push("<=");
        }
        else
            buffer.push("<");
    }
    else if (c == '>') {
        if (input.peek() == '=') {
            input.get();
            buffer.push(">=");
        }
        else
            buffer.push(">");
    }
    else
        buffer.push(std::string(1, c));
}

bool StreamTokenizer::eof() {
    if (!buffer.empty())
        return false;
    if (end_of_stream)
        return true;
    fill_buffer();
    return buffer.empty();
}

std::string StreamTokenizer::peek() {
    if (buffer.empty())
        fill_buffer();
    if (buffer.empty())
        throw std::runtime_error("peek on empty token stream");
    return buffer.front();
}

std::string StreamTokenizer::get() {
    if (buffer.empty())
        fill_buffer();
    if (buffer.empty())
        throw std::runtime_error("get on empty token stream");
    std::string token = buffer.front();
    buffer.pop();
    return token;
}

Parser::Parser(TokenStream& ts) : tokens(ts) {}

Token Parser::lex(const std::string& token) const {

    if (!token.empty()) {
        size_t start = 0;
        if (token[0] == '+' || token[0] == '-') start = 1;
        if (start < token.size() && std::all_of(token.begin() + start, token.end(), ::isdigit))
            return Token(NUMBER, token);
    }
    if (!token.empty() && std::all_of(token.begin(), token.end(), ::isalpha))
        return Token(ID, token);
    if (token == "+")
        return Token(PLUS, token);
    if (token == "-")
        return Token(MINUS, token);
    if (token == "*")
        return Token(MUL, token);
    if (token == "/")
        return Token(DIV, token);
    if (token == "%")
        return Token(MOD, token);
    if (token == "==")
        return Token(EQ, token);
    if (token == "!=")
        return Token(NEQ, token);
    if (token == "<")
        return Token(LT, token);
    if (token == ">")
        return Token(GT, token);
    if (token == "<=")
        return Token(LE, token);
    if (token == ">=")
        return Token(GE, token);
    if (token == "&")
        return Token(AND, token);
    if (token == "|")
        return Token(OR, token);
    if (token == "@")
        return Token(AT, token);
    throw std::invalid_argument("unknown token: " + token);
}

int Parser::precedence(const std::string& op) const {
    if (op == "*" || op == "/" || op == "%")
        return 3;
    if (op == "+" || op == "-")
        return 2;
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
        return 1;
    if (op == "&")
        return 0;
    if (op == "|")
        return -1;
    throw std::invalid_argument("unknown operator in infix: " + op);
}

void Parser::calc(std::stack<Exp*>& nums, std::stack<std::string>& ops) {
    if (ops.empty() || nums.size() < 2)
        throw std::runtime_error("infix parse error: operator is missing an operand");
    std::string op = ops.top();
    ops.pop();
    Exp* right = nums.top();
    nums.pop();
    Exp* left = nums.top();
    nums.pop();
    nums.push(new Bin(left, right, op));
}


Exp* Parser::parse_infix() {
    std::stack<Exp*> nums;
    std::stack<std::string> ops;
    bool expectsOperand = true;
    bool closed = false;
    try {
        while (!tokens.eof()) {
            std::string tok = tokens.peek();
            if (tok == ")") {
                if (expectsOperand)
                    throw std::runtime_error("infix parse error: expected an operand before ')'");
                tokens.get();
                closed = true;
                break;
            }

            if (tok == "(") {
                if (!expectsOperand)
                    throw std::runtime_error("infix parse error: expected an operator before '('");
                tokens.get();
                nums.push(parse_infix());
                expectsOperand = false;
                continue;
            }

            Token t = lex(tok);
            if (t.type == NUMBER || t.type == ID) {
                if (!expectsOperand)
                    throw std::runtime_error("infix parse error: missing operator before '" + tok + "'");
                tokens.get();
                if (t.type == NUMBER)
                    nums.push(new Num(parseInteger(t.text)));
                else
                    nums.push(new Var(t.text));
                expectsOperand = false;
                continue;
            }

            if (expectsOperand)
                throw std::runtime_error("infix parse error: expected an operand before '" + tok + "'");

            tokens.get();
            int prec = precedence(tok);
            while (!ops.empty() && precedence(ops.top()) >= prec)
                calc(nums, ops);
            ops.push(tok);
            expectsOperand = true;
        }

        if (!closed)
            throw std::runtime_error("infix parse error: missing ')'");
        if (expectsOperand)
            throw std::runtime_error("infix parse error: expression ends with an operator");
        while (!ops.empty())
            calc(nums, ops);
        if (nums.size() != 1)
            throw std::runtime_error("infix parse error: expected one expression");

        Exp* result = nums.top();
        nums.pop();
        return result;
    }
    catch (...) {
        while (!nums.empty()) {
            delete nums.top();
            nums.pop();
        }
        throw;
    }
}


Exp* Parser::parse_exp() {
    if (!tokens.eof() && tokens.peek() == "(") {
        tokens.get();
        return parse_infix();
    }
    if (tokens.eof())
        throw std::runtime_error("unexpected end of token stream");
    std::string token = tokens.get();
    Token t = lex(token);
    switch (t.type) {
    case NUMBER: {
        std::int32_t val = parseInteger(t.text);
        return new Num(val);
    }
    case ID:
        return new Var(t.text);
    case PLUS: case MINUS: case MUL: case DIV: case MOD:
    case EQ: case NEQ: case LT: case GT: case LE: case GE:
    case AND: case OR: {
        std::unique_ptr<Exp> left(parse_exp());
        std::unique_ptr<Exp> right(parse_exp());
        return new Bin(left.release(), right.release(), t.text);
    }
    case AT: {
        std::unique_ptr<Exp> inner(parse_exp());
        return new AtExp(inner.release());
    }
    default:
        throw std::invalid_argument("unexpected token in prefix expression");
    }
}

Exp* Parser::parse() {
    return parse_exp();
}
