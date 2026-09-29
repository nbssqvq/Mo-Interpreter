#include <iostream>
#include <stdexcept>
#include <cctype>
#include <chrono>
#include <limits>
#include <memory>
#include <MoExpression.h>
#include "MoStatement.h"

namespace {
[[noreturn]] void throwInputFailure(const char* message) {
    if (!std::cin.eof()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    throw std::runtime_error(message);
}

std::unique_ptr<Block> asBlock(std::unique_ptr<Stmt> statement) {
    if (auto* block = dynamic_cast<Block*>(statement.get())) {
        statement.release();
        return std::unique_ptr<Block>(block);
    }
    return std::unique_ptr<Block>(new Block(statement.release(), new Empty()));
}
}

bool FormatUtils::is_decimal(int format) {
    return format == 0;
}

bool FormatUtils::is_asciil(int format) {
    return format == 1;
}

Assign::Assign(Exp* l, Exp* r) : lv(l), rv(r) {}

Assign::~Assign() {
    delete lv;
    delete rv;
}

void Assign::execute(Env& env) {
    std::string varName;
    if (auto* var = dynamic_cast<Var*>(lv)) {
        varName = var->getName();
    }
    else if (auto* at = dynamic_cast<AtExp*>(lv)) {
        std::int32_t addr = at->getAddrExp()->eval(env);
        varName = "@" + std::to_string(addr);
    }
    else
        throw std::runtime_error("invalid lvalue in assign");
    env[varName] = rv->eval(env);
}

Write::Write(Exp* v, int fmt) : value(v), format(fmt) {}

Write::~Write() {
    delete value;
}

void Write::execute(Env& env) {
    std::int32_t val = value->eval(env);
    if (FormatUtils::is_decimal(format))
        std::cout << val;
    else if (FormatUtils::is_asciil(format))
        std::cout << static_cast<char>(val);
    else
        throw std::invalid_argument("unknown write format");
    std::cout.flush();
}

Read::Read(Var* v, int fmt) : var(v), format(fmt) {}

Read::~Read() {
    delete var;
}

void Read::execute(Env& env) {
    std::int32_t val = 0;
    if (FormatUtils::is_decimal(format)) {
        if (!(std::cin >> val))
            throwInputFailure("failed to read integer input");
    }
    else if (FormatUtils::is_asciil(format)) {
        char c = '\0';
        if (!(std::cin >> c))
            throwInputFailure("failed to read character input");
        val = static_cast<int>(c);
    }
    else
        throw std::invalid_argument("unknown read format");
    env[var->getName()] = val;
}

Block::Block(Stmt* h, Stmt* r) : head(h), rest(r) {}

Stmt* Block::getHead() const {
    return head;
}

Stmt* Block::getRest() const {
    return rest;
}

Block::~Block() {
    delete head;
    delete rest;
}

void Block::execute(Env& env) {
    if (head) head->execute(env);
    if (rest) rest->execute(env);
}

Loop::Loop(Exp* t, Block* b) : test(t), body(b) {}

Exp* Loop::getTest() const {
	return test;
}

Block* Loop::getBody() const {
    return body;
}

Loop::~Loop() {
    delete test;
    delete body;
}

void Loop::execute(Env& env) {
    const unsigned long long iterLim = 100000000ULL;
    const std::chrono::seconds timeLim(10);
    unsigned long long iter = 0;
    auto start = std::chrono::steady_clock::now();
    while (test->eval(env) != 0) {
        body->execute(env);
        if (++iter > iterLim)
            throw std::runtime_error("Loop iteration limit exceeded");
        auto now = std::chrono::steady_clock::now();
        if (now - start > timeLim)
            throw std::runtime_error("Loop time limit exceeded");
    }
}



Cond::Cond(Exp* t, Block* thenB, Block* alterB): test(t), thenBlock(thenB), alterBlock(alterB) {}

Cond::~Cond() {
    delete test;
    delete thenBlock;
    delete alterBlock;
}

Exp* Cond::getTest() const {
	return test;
}

Block* Cond::getThenBlock() const {
	return thenBlock;
}

Block* Cond::getAlterBlock() const {
	return alterBlock;
}

void Cond::execute(Env& env) {
    if (test->eval(env) != 0) {
        if (thenBlock)
            thenBlock->execute(env);
    }
    else {
        if (alterBlock)
            alterBlock->execute(env);
    }
}

void Empty::execute([[maybe_unused]] Env& env) {}

StatementParser::StatementParser(TokenStream& ts) : tokens(ts) {}

void StatementParser::expect(const std::string& expected) {
    if (tokens.eof())
        throw std::runtime_error("expected '" + expected + "' but reached end of input");
    std::string token = tokens.get();
    if (token != expected)
        throw std::runtime_error("expected '" + expected + "' but got '" + token + "'");
}

Exp* StatementParser::parseExp() {
    Parser parser(tokens);
    return parser.parse();
}

Var* StatementParser::parseVar() {
    std::string name = tokens.get();
    if (name.empty() || !std::isalpha(name[0]))
        throw std::runtime_error("expected variable name");
    return new Var(name);
}

int StatementParser::parseFormat() {
    std::string token = tokens.get();
    if (token == "0")
        return 0;
    if (token == "1")
        return 1;
    throw std::runtime_error("expected format specifier (0 or 1)");
}

Stmt* StatementParser::parseAssign() {
    expect("let");
    std::unique_ptr<Exp> left(parseExp());
    if (!dynamic_cast<Var*>(left.get()) && !dynamic_cast<AtExp*>(left.get()))
        throw std::runtime_error("invalid lvalue in assignment");
    std::unique_ptr<Exp> right(parseExp());
    return new Assign(left.release(), right.release());
}

Stmt* StatementParser::parseLoop() {
    expect("while");
    std::unique_ptr<Exp> cond(parseExp());
    std::unique_ptr<Block> body = asBlock(std::unique_ptr<Stmt>(parseStatements()));
    expect("end");
    return new Loop(cond.release(), body.release());
}

Stmt* StatementParser::parseCond() {
    expect("if");
    std::unique_ptr<Exp> cond(parseExp());
    std::unique_ptr<Block> thenPart = asBlock(std::unique_ptr<Stmt>(parseStatements()));
    std::unique_ptr<Block> elsePart;
    if (!tokens.eof() && tokens.peek() == "else") {
        tokens.get();
        elsePart = asBlock(std::unique_ptr<Stmt>(parseStatements()));
    }
    expect("end");
    return new Cond(cond.release(), thenPart.release(), elsePart.release());
}

Stmt* StatementParser::parseWrite() {
    expect("write");
    Exp* exp = parseExp();
    return new Write(exp, 0);
}

Stmt* StatementParser::parsePut() {
    expect("put");
    Exp* exp = parseExp();
    return new Write(exp, 1);
}

Stmt* StatementParser::parseRead() {
    expect("read");
    Var* var = parseVar();
    return new Read(var, 0);
}

Stmt* StatementParser::parseGet() {
    expect("get");
    Var* var = parseVar();
    return new Read(var, 1);
}

Stmt* StatementParser::parseSingleStmt() {
    std::string token = tokens.peek();
    if (token == "let")
        return parseAssign();
    else if (token == "while")
        return parseLoop();
    else if (token == "if")
        return parseCond();
    else if (token == "write")
        return parseWrite();
    else if (token == "put")
        return parsePut();
    else if (token == "read")
        return parseRead();
    else if (token == "get")
        return parseGet();
    else throw std::runtime_error("unexpected statement header: " + token);
}

Stmt* StatementParser::parseStatements() {
    if (tokens.eof())
        return new Empty();
    std::string token = tokens.peek();
    if (token == "end" || token == "else")
        return new Empty();

    std::unique_ptr<Stmt> head(parseSingleStmt());
    std::unique_ptr<Stmt> rest(parseStatements());
    return new Block(head.release(), rest.release());
}

Stmt* StatementParser::parseBlock() {
    std::unique_ptr<Stmt> program(parseStatements());

    // Existing programs may use one final `end` as a top-level terminator.
    if (!tokens.eof() && tokens.peek() == "end")
        tokens.get();

    if (!tokens.eof()) {
        std::string token = tokens.get();
        throw std::runtime_error("unexpected '" + token + "' at top level");
    }
    return program.release();
}
