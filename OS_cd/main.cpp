// main.cpp
#include <iostream>
#include <sstream>
#include <vector>
#include "fs.h"
#include "user.h"
#include "command.h"

using namespace std;

VirtualDisk disk;
string       currentUser;
vector<string> cwdPath;
Directory* currentDir = nullptr;
const string VFS_FILE_PATH = "D:\\C++project\\OS_cd\\x64\\Debug\\vfs.dat";

Directory* resolveCwd(Directory* root) {
    Directory* dir = root;
    for (auto& sub : cwdPath) {
        auto it = dir->subDirs.find(sub);
        if (it == dir->subDirs.end()) return nullptr;
        dir = it->second;
    }
    return dir;
}


void printPrompt() {
    cout << currentUser << ":~";
    for (auto& p : cwdPath) cout << "/" << p;
    cout << "$ ";
}


int main() {
    cout << "欢迎使用简易文件系统！\n";
    while (true) {
        // 登出 / 重置状态
        currentUser.clear();
        cwdPath.clear();
        currentDir = nullptr;

        // 注册 / 登录 / 退出 循环  
        while (currentUser.empty()) {
            cout << "1. 注册  2. 登录  3. 退出\n请选择: ";
            int c;
            cin >> c;
            cin.ignore();

            if (c == 1) {
                // 注册：load → registeUser → save
                if (!loadFromDisk(disk, VFS_FILE_PATH)) {
                    cout << "[提示] 初次注册，磁盘文件不存在，使用空盘\n";
                }
                string u, p;
                cout << "用户名: "; getline(cin, u);
                cout << "密码: "; getline(cin, p);

                registeUser(disk, u, p);
                savetoDisk(disk, VFS_FILE_PATH);
            }
            else if (c == 2) {
                // 登录：load → loginUser → (save 登录尝试计数变更)
                if (!loadFromDisk(disk, VFS_FILE_PATH)) {
                    cout << "[提示] 磁盘文件不存在，使用空盘\n";
                }
                string u, p, li;
                cout << "用户名: "; getline(cin, u);
                cout << "密码: "; getline(cin, p);
                if (loginUser(disk, u, p, li)) {
                    currentUser = li;
                    currentDir = disk.userFileSystems[currentUser].root;
                    cwdPath.clear();
                }
                // 无论登录成功与否，都保存一次（以更新 loginAttempts/isLocked）
                savetoDisk(disk, VFS_FILE_PATH);
            }
            else if (c == 3) {
                // 退出程序前保存一遍
                savetoDisk(disk, VFS_FILE_PATH);
                cout << "退出程序。\n";
                return 0;
            }
            else {
                cout << "[提示] 无效选项\n";
            }
        }

        // —— 已登录用户命令循环 —— 
        string line;
        while (true) {
            printPrompt();
            if (!getline(cin, line)) {
                // 输入流结束
                savetoDisk(disk, VFS_FILE_PATH);
                return 0;
            }
            if (line.empty()) continue;

            // 每次执行前，都先 load 一遍
            if (!loadFromDisk(disk, VFS_FILE_PATH)) {
                cout << "[提示] 磁盘加载失败，使用当前内存状态\n";
            }
            // 确保 currentDir 跟随 cwdPath
            currentDir = resolveCwd(disk.userFileSystems[currentUser].root);
            if (!currentDir) {
                currentDir = disk.userFileSystems[currentUser].root;
                cwdPath.clear();
            }

            istringstream iss(line);
            string cmd, arg1, arg2;
            iss >> cmd >> arg1;
            iss >> ws;
            getline(iss, arg2);

            if (cmd == "exit") {
                cout << "已登出。\n";
                break;
            }

            bool mutated = false;

            // —— 只读命令 —— 
            if (cmd == "cd") {
                Directory* root = disk.userFileSystems[currentUser].root;
                if (arg1 == "..") {
                    if (!cwdPath.empty()) cwdPath.pop_back();
                    else cout << "[提示] 已在根目录\n";
                }
                else {
                    Directory* cwd = resolveCwd(root);
                    if (cwd && cwd->subDirs.count(arg1)) {
                        cwdPath.push_back(arg1);
                    }
                    else {
                        cout << "[错误] 子目录不存在：" << arg1 << "\n";
                    }
                }
                currentDir = resolveCwd(disk.userFileSystems[currentUser].root);
            }
            else if (cmd == "dir") {
                listDirectory(currentDir);
            }
            else if (cmd == "tree") {
                showTree(disk);
            }
            else if (cmd == "read") {
                readFile(currentDir, arg1);
            }
            else if (cmd == "head") {
                int n = stoi(arg1.substr(1));
                headFile(currentDir, arg2, n);
            }
            else if (cmd == "tail") {
                int n = stoi(arg1.substr(1));
                tailFile(currentDir, arg2, n);
            }
            // —— 写命令 —— 
            else if (cmd == "mkdir") {
                mkdir(currentDir, arg1);
                mutated = true;
            }
            else if (cmd == "rmdir") {
                rmdir(currentDir, arg1);
                mutated = true;
            }
            else if (cmd == "create") {
                create(currentDir, arg1);
                mutated = true;
            }
            else if (cmd == "delete") {
                deleteFile(currentDir, arg1);
                mutated = true;
            }
            else if (cmd == "open") {
                openFile(currentDir, arg1);
                mutated = true;
            }
            else if (cmd == "close") {
                closeFile(currentDir, arg1);
                mutated = true;
            }
            else if (cmd == "write") {
                writeFile(currentDir, arg1, arg2);
                mutated = true;
            }
            else if (cmd == "move") {
                moveFile(currentDir, arg1, arg2);
                mutated = true;
            }
            else if (cmd == "copy") {
                copyFile(currentDir, arg1, arg2);
                mutated = true;
            }
            else if (cmd == "flock") {
                flockFile(currentDir, arg1, arg2 == "lock");
                mutated = true;
            }
            else if (cmd == "import") {
                importFile(currentDir, arg1, arg2);
                mutated = true;
            }
            else if (cmd == "export") {
                exportFile(currentDir, arg1, arg2);
                // exportFile 只是写宿主 FS，不影响 vfs.dat
            }
            else {
                cout << "[提示] 不支持的命令：" << cmd << "\n";
            }

            // 写了才保存
            if (mutated) {
                savetoDisk(disk, VFS_FILE_PATH);
            }
        }
    }
    return 0;
}
