#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <MoExpression.h>
#include <MoStatement.h>
#include <MoRun.h>
#include <memory>

//尝试多态
#if defined(_WIN32)
#include <windows.h>
#include <conio.h>

std::string getExeDir() {                          //获取当前可执行文件所在目录
    char buf[MAX_PATH];
    GetModuleFileNameA(NULL, buf, MAX_PATH);
    std::string s(buf);
    return s.substr(0, s.find_last_of("\\/"));
}

void clearScreen() { system("cls"); }              //清屏

void waitForKey() {                                //按任意键继续
    std::cout << "Press any key to continue...";
    std::cout.flush();
    _getch();
    std::cout << std::endl;
}
#else
#include <unistd.h>
#include <limits.h>

std::string getExeDir() {                          //获取当前可执行文件所在目录
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        std::string s(buf);
        return s.substr(0, s.find_last_of('/'));
    }
    return ".";
}

void clearScreen() { system("clear"); }           //清屏

void waitForKey() {                               //按任意键继续
    std::cout << "Press Enter to continue...";
    std::cout.flush();
    std::cin.ignore(1024, '\n');
    std::cin.get();
}
#endif

std::string pathJoin(const std::string& a, const std::string& b) {
    namespace fs = std::filesystem;
    return (fs::path(a) / fs::path(b)).string();
}

std::vector<std::string> scanMoFiles(const std::string& dir) {
    namespace fs = std::filesystem;
    std::vector<std::string> files;
    if (!fs::exists(dir) || !fs::is_directory(dir))
        return files;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() == ".mo")
            files.push_back(entry.path().filename().string());
    }
    std::sort(files.begin(), files.end());
    return files;
}

int runMoFile(const std::string& path) {
	std::ifstream file(path);                                    //打开.mo文件
    if (!file) {
        std::cerr << "Cannot open " << path << std::endl;
        return 1;
    }

    clearScreen();
    std::cout << "Execution started..." << std::endl;

	StreamTokenizer tokenizer(file);                             //创建词法分析器
	StatementParser parser(tokenizer);                           //创建语法解析器
	std::unique_ptr<Stmt> program;                               //用智能指针管理Stmt树
    try {
		program.reset(parser.parseBlock());                      //解析程序，生成Stmt树
    }
    catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return 1;
    }
	Machine machine(std::cin, std::cout);                        //创建虚拟机
	MoInterpreter interpreter(machine, program.get());           //创建Mo解释器
    try {
		interpreter.run();                                       //运行程序
    }
    catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

int main() {
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::ios::sync_with_stdio(false);
    std::string rootDir = getExeDir();
    std::string moDir = pathJoin(rootDir, "MoProgram");

    while (true) {
        clearScreen();
        std::cout << "Mo Interpreter v 1.0.3" << std::endl;
        std::cout << std::endl;
        std::cout << "Available programs:" << std::endl;

        std::vector<std::string> files = scanMoFiles(moDir);
        if (files.empty())
            std::cout << "  (no .mo files found in MoProgram folder)" << std::endl;
        else {
            for (size_t i = 0; i < files.size(); i++) {
                std::cout << std::setw(4)<< (i + 1);
                std::cout << ". " << files[i] << std::endl;
            }
        }

        std::cout << "   0. Enter file name manually" << std::endl;
        std::cout << "  -1. Exit" << std::endl;
        std::cout << std::endl;

        std::cout << "Enter your choice: " << std::flush;
        int choice;
        std::cin >> choice;
        if (std::cin.fail()) {
            if (std::cin.eof())
                break;
            std::cin.clear();
            std::cin.ignore(1024, '\n');
            std::cout << "Invalid input, please try again." << std::endl;
            waitForKey();
            continue;
        }
        if (choice == -1)
            break;
        else if (choice == 0) {
            std::cout << "Enter program file name (e.g., test or test.mo): " << std::flush;
            std::string fname;
            std::cin >> fname;
            std::string fullPath = pathJoin(moDir, fname);
            if (fname.find(".mo") == std::string::npos)
                fullPath += ".mo";
            runMoFile(fullPath);
        }
        else if (choice >= 1 && choice <= static_cast<int>(files.size())) {
            std::string fullPath = pathJoin(moDir, files[choice - 1]);
            runMoFile(fullPath);
        }
        else
            std::cout << "Invalid choice, please try again." << std::endl;
        std::cout << std::endl;
        waitForKey();
    }
    return 0;
}
