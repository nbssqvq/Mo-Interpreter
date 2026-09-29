#include "MoRun.h"

Machine::Machine(std::istream& i, std::ostream& o) : in(i), out(o), memory(Machine::MemorySize, 0) {}

int Machine::memorySize() const {
    return MemorySize;
}

void Machine::store(int access, std::int32_t value) {
    if (access < 0 || access >= Machine::MemorySize)
        throw std::out_of_range("Machine::store: access out of bounds");
    memory[access] = value;
}

std::int32_t Machine::load(int access) {
    if (access < 0 || access >= Machine::MemorySize)
        throw std::out_of_range("Machine::load: access out of bounds");
    return memory[access];
}

void Machine::output(std::int32_t value, int format) {
    if (FormatUtils::is_decimal(format))
        out << value;
    else if (FormatUtils::is_asciil(format))
        out << static_cast<char>(value);
    else
        throw std::invalid_argument("Machine::output: unknown format");
}

std::int32_t Machine::input(int format) {
    if (FormatUtils::is_decimal(format)) {
        std::int32_t buf = 0;
        if (!(in >> buf))
            throw std::runtime_error("Machine::input: failed to read integer input");
        return buf;
    }
    else if (FormatUtils::is_asciil(format)) {
        char buf = '\0';
        if (!(in >> buf))
            throw std::runtime_error("Machine::input: failed to read character input");
        return static_cast<int>(buf);
    }
    else
        throw std::invalid_argument("Machine::input: unknown format");
}

std::istream& Machine::inStream() {
    return in;
}

std::ostream& Machine::outStream() {
    return out;
}

SymbolTable::SymbolTable() : offset(0) {}

int SymbolTable::resolve(const std::string& name) {
    auto it = find(name);
    if (it != end())
        return it->second;
	//处理代表内存地址的变量名（@+地址）
    if (!name.empty() && name[0] == '@') {
        try {
            long n = std::stol(name.substr(1));
            if (n <= 0) throw std::out_of_range("address out of range");
            int slot = static_cast<int>(n - 1);
            (*this)[name] = slot;
			usedSlots.insert(slot);                                        //将该内存地址标记为已使用
            return slot;
        } catch (...) {}
    }
    //确保分配的内存地址未被使用
    while (usedSlots.count(offset))
        offset++;
    int addr = offset++;
    (*this)[name] = addr;
    usedSlots.insert(addr);
    return addr;
}

MoInterpreter::MoInterpreter(Machine& m, Stmt* stmtTree): machine(m), program(stmtTree) {}

void MoInterpreter::run() {
    if (!program) {
        return;
    }
    Env localEnv;                                                              // 创建局部环境用于存储变量
    for (const auto& kv : env) {
        localEnv[kv.first] = machine.load(kv.second);
    }
    std::streambuf* cin_buf = std::cin.rdbuf();                                // 重定向标准I/O流至虚拟机的I/O流
    std::streambuf* cout_buf = std::cout.rdbuf();
    std::cin.rdbuf(machine.inStream().rdbuf());
    std::cout.rdbuf(machine.outStream().rdbuf());
    try {
        program->execute(localEnv);                                            //执行Stmt树
    }
    //异常处理
    catch (...) {
		std::cin.rdbuf(cin_buf);                                               //恢复标准I/O流
        std::cout.rdbuf(cout_buf);
		for (const auto& kv : localEnv) {                                      //将局部环境中的变量存储回虚拟机内存
            int slot = env.resolve(kv.first);
            try {
                machine.store(slot, kv.second);
            } catch (...) {}
        }
        throw;
    }
    //程序执行完毕，与异常处理一致
    std::cin.rdbuf(cin_buf);
    std::cout.rdbuf(cout_buf);

    for (const auto& kv : localEnv) {
        const std::string& name = kv.first;
        //处理代表内存地址的变量名（@+地址）
        if (!name.empty() && name[0] == '@') {
            try {
                long n = std::stol(name.substr(1));
                if (n <= 0 || n > machine.memorySize())
                    throw std::out_of_range("address out of range");
                int idx = static_cast<int>(n - 1);
                machine.store(idx, kv.second);
                continue;
            } catch (...) {}
        }
        int slot = env.resolve(name);
        machine.store(slot, kv.second);
    }
}

MoInterpreter::~MoInterpreter() {}
