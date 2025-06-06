#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <windows.h>
#include <shlwapi.h>       // 声明 PathRemoveFileSpecA
#pragma comment(lib, "Shlwapi.lib")  // 链接 Shlwapi 库
#include <filesystem>      // C++17 下使用 std::filesystem
#include <functional>
#include "fs.h"
#include "user.h"
#include "command.h"

using namespace std::filesystem;
using namespace std;

// 虚拟磁盘，所有进程操作前后都要 load/save
VirtualDisk disk;

// 当前登录用户名
string currentUser;

// 记录当前用户的相对路径（从用户根目录开始）
static vector<string> cwdPath;

// 当前用户实际的 Directory* 指针（resolveCwd 返回后缓存，供操作时使用）
static Directory* currentDir = nullptr;

//固定所有虚拟磁盘文件都放在 <exe所在目录> / data / vfs.dat
static string g_vfsPath;

// Windows 下获取当前可执行文件完整路径，然后去掉文件名只保留目录
static string getExeFolder() {
    char buffer[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        // 获取失败，回退到当前工作目录
        return filesystem::current_path().string();
    }
    // PathRemoveFileSpecA 会在 Windows 下移除最后的 "\<exe_name>"
    PathRemoveFileSpecA(buffer);
    return string(buffer);
}

// 初始化 g_vfsPath：<exeFolder>/data/vfs.dat，并确保 data 目录存在
static void initVfsPath() {


    // 1. 获取 exe 所在目录
    path exeFolder = getExeFolder();

    // 2. 拼接出“<exeFolder>/data”
    path dataFolder = exeFolder / "data";

    // 3. 如果 data 目录不存在，就创建它
    if (!exists(dataFolder)) {
        try {
            create_directory(dataFolder);
        }
        catch (const filesystem_error& e) {
            cout << "[错误] 无法创建目录：" << dataFolder << "，异常：" << e.what() << "\n";
        }
    }

    // 4. 最终拼成“<exeFolder>/data/vfs.dat”
    path vfsFile = dataFolder / "vfs.dat";
    g_vfsPath = vfsFile.string();

    cout << "[Info] 虚拟磁盘文件路径已固定为：" << g_vfsPath << "\n";
    // 如果 vfs.dat 不存在，就创建一个空文件
    if (!exists(vfsFile)) {
        // 用 ofstream 以二进制方式创建一个零字节文件
        fstream ofs(g_vfsPath, ios::binary);
        if (!ofs) {
            cout << "[警告] 无法创建初始空白文件：" << g_vfsPath << "\n";
        }
        else {
            // 立即关闭，保持它是一个大小为 0 的文件
            ofs.close();
            cout << "[Info] 已创建空白虚拟盘文件：" << g_vfsPath << "\n";
        }
    }
}

// 辅助函数: 根据 cwdPath 定位到实际的 Directory*
Directory* resolveCwd(Directory* userRoot) {
    Directory* dir = userRoot;
    for (const auto& sub : cwdPath) {
        auto it = dir->subDirs.find(sub);
        if (it == dir->subDirs.end()) {
            return nullptr;
        }
        dir = it->second;
    }
    return dir;
}
//文件锁 + load/save 封装
// op 是一个函数，接收当前目录 Directory*，在锁定状态下对其进行操作
// 加锁 → load → 操作 → save → 解锁
void executeCommandSerialized(function<void(Directory*)> op) {
    const string& diskFile = g_vfsPath;

    // 1. 锁定
    HANDLE hLock = lockDiskFile(diskFile);
    if (hLock == INVALID_HANDLE_VALUE) {
        cout << "[错误] 无法锁定磁盘文件: " << diskFile
            << "，错误码=" << GetLastError() << "\n";
        return;
    }

    // 2. 如果 vfs.dat 大小 > 0，就 load
    try {
        if (filesystem::exists(diskFile) && filesystem::file_size(diskFile) > 0) {
            bool ok = loadFromDisk(disk, diskFile);
            if (!ok) {
                cout << "[警告] loadFromDisk 失败，将使用空盘。\n";
            }
        }
    }
    catch (...) {}

    // 3. 定位到当前用户目录
    Directory* userRoot = nullptr;
    if (!currentUser.empty()) {
        userRoot = disk.userFileSystems[currentUser].root;
    }
    Directory* cwd = nullptr;
    if (userRoot) {
        cwd = resolveCwd(userRoot);
    }
    if (!cwd && userRoot) {
        cwd = userRoot;
        cwdPath.clear();
    }
    currentDir = cwd;

    // 4. 执行操作
    op(cwd);

    // 5. 保存回磁盘
    savetoDisk(disk, diskFile);

    // 6. 解锁
    unlockDiskFile(hLock);
}

void printPrompt() {
    cout << currentUser << ":~";
    for (const auto& p : cwdPath) {
        cout << "/" << p;
    }
    cout << "$ ";
}

int main() {
    cout << "欢迎使用简易文件系统！\n";

    // 先固定 vfs.dat 路径，并确保 data 子目录存在
    initVfsPath();

    // 启动时如果 vfs.dat 存在且大小 > 0，就载入一次
    try {
        if (filesystem::exists(g_vfsPath) && filesystem::file_size(g_vfsPath) > 0) {
            bool ok = loadFromDisk(disk, g_vfsPath);
            if (!ok) {
                cout << "[警告] 无法载入现有虚拟盘，将使用空虚拟盘。\n";
            }
        }
    }
    catch (...) {
        // 忽略异常
    }

    while (true) {
        // 登出后或初次启动时 currentUser 为空，强制做登录/注册/退出
        currentUser.clear();
        cwdPath.clear();
        currentDir = nullptr;

        while (currentUser.empty()) {
            cout << "1. 注册\n2. 登录\n3. 退出\n请选择操作: ";
            int choice;
            cin >> choice;
            cin.ignore(); // 忽略回车

            if (choice == 1) {
                // 注册：直接修改内存并持久化一次
                string username, password;
                cout << "请输入用户名: "; getline(cin, username);
                cout << "请输入密码: "; getline(cin, password);
                registeUser(disk, username, password);

                // 注册后立即持久化到磁盘
                executeCommandSerialized([](Directory*) {});
            }
            else if (choice == 2) {
                // 登录：先从磁盘加载，保证获取最新密码状态
                executeCommandSerialized([](Directory*) {});

                string username, password;
                cout << "请输入用户名: "; getline(cin, username);
                cout << "请输入密码: "; getline(cin, password);

                string loggedIn;
                if (loginUser(disk, username, password, loggedIn)) {
                    currentUser = loggedIn;
                    // 登录成功，当前目录指向该用户根目录
                    currentDir = disk.userFileSystems[currentUser].root;
                    cwdPath.clear();
                    break;
                }
            }
            else {
                // 退出程序
                cout << "退出程序。\n";
                // 退出前再保存一次
                executeCommandSerialized([](Directory*) {});
                return 0;
            }
        }

        //已登录用户命令循环 
        string line;
        while (true) {
            printPrompt();
            getline(cin, line);
            istringstream iss(line);
            string cmd;
            iss >> cmd;

            if (cmd == "exit") {
                // 退出当前用户会话
                cout << "已退出登录。\n";
                break;
            }

            string arg1, arg2;
            iss >> arg1;
            iss >> ws;
            getline(iss, arg2);


            // 切换目录 (cd) 
            if (cmd == "cd") {
                if (arg1 == "..") {
                    if (cwdPath.empty()) {
                        cout << "[提示] 当前已在根目录，无法返回。\n";
                    }
                    else {
                        cwdPath.pop_back();
                        cout << "返回上一级目录。\n";
                    }
                }
                else {
                    // cd <目录名>：先加载最新磁盘状态，再检查子目录
                    executeCommandSerialized([&](Directory* cwd) {
                        if (!cwd) return;
                        if (cwd->subDirs.find(arg1) == cwd->subDirs.end()) {
                            cout << "[错误] 子目录不存在：" << arg1 << "\n";
                        }
                        else {
                            cwdPath.push_back(arg1);
                            cout << "[成功] 当前目录切换到：" << arg1 << "\n";
                        }
                        });
                }
                continue;
            }
            // 列出目录 (dir)
            else if (cmd == "dir") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) listDirectory(cwd);
                    });
                continue;
            }
            // 树形显示 (tree)
            else if (cmd == "tree") {
                executeCommandSerialized([&](Directory*) {
                    showTree(disk);
                    });
                continue;
            }

            // 需要加锁+load+执行+save 的命令

            if (cmd == "mkdir") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) mkdir(cwd, arg1);
                    });
            }
            else if (cmd == "rmdir") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) rmdir(cwd, arg1);
                    });
            }
            else if (cmd == "create") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) create(cwd, arg1);
                    });
            }
            else if (cmd == "delete") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) deleteFile(cwd, arg1);
                    });
            }
            else if (cmd == "open") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) openFile(cwd, arg1);
                    });
            }
            else if (cmd == "close") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) closeFile(cwd, arg1);
                    });
            }
            else if (cmd == "read") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) readFile(cwd, arg1);
                    });
            }
            else if (cmd == "write") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) writeFile(cwd, arg1, arg2);
                    });
            }
            else if (cmd == "move") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) moveFile(cwd, arg1, arg2);
                    });
            }
            else if (cmd == "copy") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) copyFile(cwd, arg1, arg2);
                    });
            }
            else if (cmd == "flock") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (!cwd) return;
                    if (arg2 == "lock") flockFile(cwd, arg1, true);
                    else if (arg2 == "unlock") flockFile(cwd, arg1, false);
                    else cout << "[错误] 格式应为：flock <文件名> lock/unlock\n";
                    });
            }
            else if (cmd == "head") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (!cwd) return;
                    if (!arg1.empty() && arg1[0] == '-') {
                        int num = stoi(arg1.substr(1));
                        headFile(cwd, arg2, num);
                    }
                    else {
                        cout << "[错误] 格式应为：head -[行数] 文件名\n";
                    }
                    });
            }
            else if (cmd == "tail") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (!cwd) return;
                    if (!arg1.empty() && arg1[0] == '-') {
                        int num = stoi(arg1.substr(1));
                        tailFile(cwd, arg2, num);
                    }
                    else {
                        cout << "[错误] 格式应为：tail -[行数] 文件名\n";
                    }
                    });
            }
            else if (cmd == "lseek") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (!cwd) return;
                    int offset = stoi(arg2);
                    lseekFile(cwd, arg1, offset);
                    });
            }
            else if (cmd == "import") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (!cwd) return;
                    if (!importFile(cwd, arg1, arg2)) cout << "[错误] 导入失败\n";
                    else cout << "[成功] 文件已导入\n";
                    });
            }
            else if (cmd == "export") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (!cwd) return;
                    if (!exportFile(cwd, arg1, arg2)) cout << "[错误] 导出失败\n";
                    else cout << "[成功] 文件已导出\n";
                    });
            }
            else {
                cout << "[提示] 不支持的命令。\n";
            }
        }
    }

    return 0;
}
