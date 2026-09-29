#ifndef MOEXPRESSION_H
#define MOEXPRESSION_H
#include <string>
#include <map>
#include <queue>
#include <iosfwd>
#include <vector>
#include <stack>
#include <cstdint>

using Env = std::map<std::string, std::int32_t>;
class Machine;

class Exp {
public:
    virtual void print(std::ostream& out) = 0;   //打印
    virtual std::int32_t eval(Env& env) = 0;      //求值
    virtual ~Exp() = default;
};

class Bin : public Exp {
private:
    Exp* left;
    Exp* right;
    std::string op;
public:
    Bin(Exp* l, Exp* r, const std::string& o);
    ~Bin()override;
    Exp* getLeft() const;
    Exp* getRight() const;
    const std::string& getOp() const;
    void print(std::ostream& out) override;
    std::int32_t eval(Env& env) override;
};

class Var : public Exp {
private:
    std::string name;
public:
    Var(const std::string& n);
    std::string getName() const;
    void print(std::ostream& out) override;
    std::int32_t eval(Env& env) override;
};

class Num : public Exp {
private:
    std::int32_t value;
public:
    Num(std::int32_t v);
	std::int32_t getValue() const;
    void print(std::ostream& out) override;
    std::int32_t eval(Env& env) override;
};

class AtExp : public Exp {
private:
    Exp* addrExp;
public:
    explicit AtExp(Exp* e);
    ~AtExp()override;
    Exp* getAddrExp() const;
    std::int32_t eval(Env& env) override;
    std::string getTargetName(Env& env) const;
    void print(std::ostream& out) override;
};

const int ID = 0b000001;
const int NUMBER = 0b000010;
const int OR = 0b010000;
const int AND = 0b010001;
const int EQ = 0b010010;
const int NEQ = 0b010011;
const int LT = 0b010100;
const int GT = 0b010110;
const int LE = 0b010101;
const int GE = 0b010111;
const int PLUS = 0b011000;
const int MINUS = 0b011001;
const int MUL = 0b011100;
const int DIV = 0b011101;
const int MOD = 0b011111;
const int AT = 0b100000;

class Token {
public:
    int type;
    std::string text;
    Token(int t, const std::string& txt);
};

class TokenStream {
public:
    virtual bool eof() = 0;
    virtual std::string peek() = 0;
    virtual std::string get() = 0;
    virtual ~TokenStream() = default;
};

class StreamTokenizer : public TokenStream {
private:
    std::istream& input;
    std::queue<std::string> buffer;
    bool end_of_stream;
    char next_char();
    void fill_buffer();
public:
    explicit StreamTokenizer(std::istream& is);
    bool eof() override;
    std::string peek() override;
    std::string get() override;
};

class Parser {
private:
    TokenStream& tokens;
	Token lex(const std::string& token) const;                         //将string类转换为Token类
    //以下三个函数为中缀表达式解析专用
    int precedence(const std::string& op) const;                       //运算符优先级
    void calc(std::stack<Exp*>& nums, std::stack<std::string>& ops);   //将中缀表达式转化为Bin类
    Exp* parse_infix();                                                //解析中缀表达式

	Exp* parse_exp();                                                  //解析前缀和中缀表达式
public:
    explicit Parser(TokenStream& ts);
    Exp* parse();
};

#endif
