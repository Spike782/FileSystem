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
    loadFromDisk(disk, "vfs.dat");
    while (true) {
        currentUser = "";
        while (currentUser.empty()) {
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
                    currentDir = disk.userFileSystems[currentUser].root;
                    break;
                }
            }
            else {
                cout << "退出程序。\n";
                savetoDisk(disk, "vfs.dat");
                return 0;
            }
        }


        string line;
        while (true) {
            printPrompt();
            getline(cin, line);
            istringstream iss(line);
            string cmd;
            iss >> cmd;

            if (cmd == "exit") {
                cout << "已退出登录。\n";
                break;
            }

            string arg1, arg2;
            iss >> arg1;
            iss >> ws;
            getline(iss, arg2); // 支持写入带空格
            if (cmd == "mkdir") {
                mkdir(currentDir, arg1);
            }
            else if (cmd == "rmdir") {
                rmdir(currentDir, arg1);
            }
            else if (cmd == "cd") {
                changeDirectory(currentDir, arg1);
            }
            else if (cmd == "dir") {
                listDirectory(currentDir);
            }
            else if (cmd == "create") {
                create(currentDir, arg1);
            }
            else if (cmd == "delete") {
                deleteFile(currentDir, arg1);
            }
            else if (cmd == "open") {
                openFile(currentDir, arg1);
            }
            else if (cmd == "close") {
                closeFile(currentDir, arg1);
            }
            else if (cmd == "read") {
                readFile(currentDir, arg1);
            }
            else if (cmd == "write") {
                writeFile(currentDir, arg1, arg2);
            }
            else if (cmd == "move") {
                moveFile(currentDir, arg1, arg2);
            }
            else if (cmd == "copy") {
                copyFile(currentDir, arg1, arg2);
            }
            else if (cmd == "flock") {
                if (arg2 == "lock") flockFile(currentDir, arg1, true);
                else if (arg2 == "unlock") flockFile(currentDir, arg1, false);
                else std::cout << "[错误] 格式应为：flock <文件名> lock/unlock\n";
            }
            else if (cmd == "head") {
                if (arg1.substr(0, 1) == "-") {
                    int num = stoi(arg1.substr(1));
                    iss >> arg2;
                    headFile(currentDir, arg2, num);
                }
                else {
                    std::cout << "[错误] 格式应为：head -[行数] 文件名\n";
                }
            }
            else if (cmd == "tail") {
                if (arg1.substr(0, 1) == "-") {
                    int num = stoi(arg1.substr(1));
                    iss >> arg2;
                    tailFile(currentDir, arg2, num);
                }
                else {
                    std::cout << "[错误] 格式应为：tail -[行数] 文件名\n";
                }
            }
            else if (cmd == "lseek") {
                int offset = stoi(arg2);
                lseekFile(currentDir, arg1, offset);
            }
            else if (cmd == "tree") {
                showTree(disk);
            }
            else if (cmd == "import") {
                if (!importFile(currentDir, arg1, arg2)) std::cout << "[错误] 导入失败\n";
                else std::cout << "[成功] 文件已导入\n";
            }
            else if (cmd == "export") {
                if (!exportFile(currentDir, arg1, arg2)) std::cout << "[错误] 导出失败\n";
                else std::cout << "[成功] 文件已导出\n";
            }
            else {
                cout << "[提示] 不支持的命令。\n";
            }
        }

    }
    return 0;
}
