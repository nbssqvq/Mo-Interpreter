#ifndef MORUN_H
#define MORUN_H

#include <iostream>
#include <string>
#include <map>
#include <unordered_set>
#include <stdexcept>
#include <MoExpression.h>
#include <MoStatement.h>
#include <vector>

class Machine {
private:
    static constexpr int MemorySize = 262144;  //最大容量（1MB）
    std::istream& in;
    std::ostream& out;
    std::vector<std::int32_t> memory;
public:
    Machine(std::istream& i, std::ostream& o);
	int memorySize() const;                    //获取最大容量
	std::istream& inStream();                  //获取输入流
	std::ostream& outStream();                 //获取输出流
    void store(int access, std::int32_t value); //存储值到内存
    std::int32_t load(int access);              //从内存读取值
    void output(std::int32_t value, int format);//输出值
    std::int32_t input(int format);             //输入值
};

class SymbolTable : public Env {
private:
    int offset;
    std::unordered_set<int> usedSlots;
public:
    SymbolTable();
    int resolve(const std::string& name);
};

class MoInterpreter {
private:
    Machine& machine;                            //虚拟机
    SymbolTable env;                             //符号表
	Stmt* program;                               //Stmt树（根节点）
public:
    MoInterpreter(Machine& m, Stmt* stmtTree);
    void run();
    ~MoInterpreter();
};

#endif
