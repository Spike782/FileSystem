#include <iostream>
#include <sstream>
#include "fs.h"
#include "user.h"
#include "command.h"

VirtualDisk disk;  // 虚拟磁盘
string currentUser;  // 当前登录用户

void printPrompt() {
    cout << currentUser << ":~$ ";
}

int main() {
    cout << "欢迎使用简易文件系统！\n";
    while (true) {
        cout << "1. 注册\n2. 登录\n3. 退出\n请选择操作: ";
        int choice;
        cin >> choice;
        cin.ignore(); // 忽略回车符

        string username, password;

        if (choice == 1) {
            cout << "请输入用户名: "; getline(cin, username);
            cout << "请输入密码: "; getline(cin, password);
            registeUser(disk, username, password);
        }
        else if (choice == 2) {
            cout << "请输入用户名: "; getline(cin, username);
            cout << "请输入密码: "; getline(cin, password);
            if (loginUser(disk, username, password, currentUser)) {
                currentDir = &disk.userFileSystems[currentUser].root;
                break;
            }
        }
        else {
            cout << "退出程序。\n";
            return 0;
        }
    }

    string line;
    while (true) {
        printPrompt();
        getline(cin, line);
        istringstream iss(line);
        string cmd, arg;
        iss >> cmd >> arg;

        if (cmd == "mkdir") {
            mkdir(currentDir, arg);
        }
        else if (cmd == "rmdir") {
            rmdir(currentDir, arg);
        }
        else if (cmd == "create") {
            create(currentDir, arg);
        }
        else if (cmd == "delete") {
            deleteFile(currentDir, arg);
        }
        else if (cmd == "exit") {
           cout << "已退出登录。\n";
            break;
        }
        else {
            cout << "[提示] 不支持的命令。\n";
        }
    }

    return 0;
}
