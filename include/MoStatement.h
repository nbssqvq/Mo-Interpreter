#ifndef MOSTATEMENT_H
#define MOSTATEMENT_H

#include <MoExpression.h>
#include <string>

class FormatUtils {
public:
    static bool is_decimal(int format);
    static bool is_asciil(int format);
};

class Stmt {
public:
    virtual void execute(Env& env) = 0;   //执行语句
    virtual ~Stmt() = default;
};

class Assign : public Stmt {
private:
    Exp* lv;
    Exp* rv;
public:
    Assign(Exp* l, Exp* r);
    ~Assign()override;
    void execute(Env& env) override;
};

class Write : public Stmt {
private:
    Exp* value;
    int format;
public:
    Write(Exp* v, int fmt);
    ~Write()override;
    void execute(Env& env) override;
};

class Read : public Stmt {
private:
    Var* var;
    int format;
public:
    Read(Var* v, int fmt);
    ~Read()override;
    void execute(Env& env) override;
};

class Block : public Stmt {
private:
    Stmt* head;
    Stmt* rest;
public:
    Stmt* getHead() const;
    Stmt* getRest() const;
    Block(Stmt* h, Stmt* r);
    ~Block()override;
    void execute(Env& env) override;
};

class Loop : public Stmt {
private:
    Exp* test;
    Block* body;
public:
	Exp* getTest() const;
    Block* getBody() const;
    Loop(Exp* t, Block* b);
    ~Loop()override;
    void execute(Env& env) override;
};

class Cond : public Stmt {
private:
    Exp* test;
    Block* thenBlock;
    Block* alterBlock;
public:
    Cond(Exp* t, Block* then_b, Block* alter_b = nullptr);
    ~Cond()override;
    Exp* getTest() const;
    Block* getThenBlock() const;
    Block* getAlterBlock() const;
    void execute(Env& env) override;
};

class Empty : public Stmt {
public:
    Empty() = default;
    void execute(Env& env) override;
};

class StatementParser {
private:
    TokenStream& tokens;
	void expect(const std::string& expected);   //检查下一个token是否为预期的token
    Stmt* parseStatements();
    Exp* parseExp();
    Var* parseVar();
    int parseFormat();
public:
    explicit StatementParser(TokenStream& ts);
    Stmt* parseAssign();
    Stmt* parseLoop();
    Stmt* parseCond();
    Stmt* parseWrite();
    Stmt* parsePut();
    Stmt* parseRead();
    Stmt* parseSingleStmt();
    Stmt* parseBlock();
    Stmt* parseGet();
};

#endif
