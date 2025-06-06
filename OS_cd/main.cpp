#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <windows.h>
#include <shlwapi.h>       // ���� PathRemoveFileSpecA
#pragma comment(lib, "Shlwapi.lib")  // ���� Shlwapi ��
#include <filesystem>      // C++17 ��ʹ�� std::filesystem
#include <functional>
#include "fs.h"
#include "user.h"
#include "command.h"

using namespace std::filesystem;
using namespace std;

// ������̣����н��̲���ǰ��Ҫ load/save
VirtualDisk disk;

// ��ǰ��¼�û���
string currentUser;

// ��¼��ǰ�û������·�������û���Ŀ¼��ʼ��
static vector<string> cwdPath;

// ��ǰ�û�ʵ�ʵ� Directory* ָ�루resolveCwd ���غ󻺴棬������ʱʹ�ã�
static Directory* currentDir = nullptr;

//�̶�������������ļ������� <exe����Ŀ¼> / data / vfs.dat
static string g_vfsPath;

// Windows �»�ȡ��ǰ��ִ���ļ�����·����Ȼ��ȥ���ļ���ֻ����Ŀ¼
static string getExeFolder() {
    char buffer[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        // ��ȡʧ�ܣ����˵���ǰ����Ŀ¼
        return filesystem::current_path().string();
    }
    // PathRemoveFileSpecA ���� Windows ���Ƴ����� "\<exe_name>"
    PathRemoveFileSpecA(buffer);
    return string(buffer);
}

// ��ʼ�� g_vfsPath��<exeFolder>/data/vfs.dat����ȷ�� data Ŀ¼����
static void initVfsPath() {


    // 1. ��ȡ exe ����Ŀ¼
    path exeFolder = getExeFolder();

    // 2. ƴ�ӳ���<exeFolder>/data��
    path dataFolder = exeFolder / "data";

    // 3. ��� data Ŀ¼�����ڣ��ʹ�����
    if (!exists(dataFolder)) {
        try {
            create_directory(dataFolder);
        }
        catch (const filesystem_error& e) {
            cout << "[����] �޷�����Ŀ¼��" << dataFolder << "���쳣��" << e.what() << "\n";
        }
    }

    // 4. ����ƴ�ɡ�<exeFolder>/data/vfs.dat��
    path vfsFile = dataFolder / "vfs.dat";
    g_vfsPath = vfsFile.string();

    cout << "[Info] ��������ļ�·���ѹ̶�Ϊ��" << g_vfsPath << "\n";
    // ��� vfs.dat �����ڣ��ʹ���һ�����ļ�
    if (!exists(vfsFile)) {
        // �� ofstream �Զ����Ʒ�ʽ����һ�����ֽ��ļ�
        fstream ofs(g_vfsPath, ios::binary);
        if (!ofs) {
            cout << "[����] �޷�������ʼ�հ��ļ���" << g_vfsPath << "\n";
        }
        else {
            // �����رգ���������һ����СΪ 0 ���ļ�
            ofs.close();
            cout << "[Info] �Ѵ����հ��������ļ���" << g_vfsPath << "\n";
        }
    }
}

// ��������: ���� cwdPath ��λ��ʵ�ʵ� Directory*
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
//�ļ��� + load/save ��װ
// op ��һ�����������յ�ǰĿ¼ Directory*��������״̬�¶�����в���
// ���� �� load �� ���� �� save �� ����
void executeCommandSerialized(function<void(Directory*)> op) {
    const string& diskFile = g_vfsPath;

    // 1. ����
    HANDLE hLock = lockDiskFile(diskFile);
    if (hLock == INVALID_HANDLE_VALUE) {
        cout << "[����] �޷����������ļ�: " << diskFile
            << "��������=" << GetLastError() << "\n";
        return;
    }

    // 2. ��� vfs.dat ��С > 0���� load
    try {
        if (filesystem::exists(diskFile) && filesystem::file_size(diskFile) > 0) {
            bool ok = loadFromDisk(disk, diskFile);
            if (!ok) {
                cout << "[����] loadFromDisk ʧ�ܣ���ʹ�ÿ��̡�\n";
            }
        }
    }
    catch (...) {}

    // 3. ��λ����ǰ�û�Ŀ¼
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

    // 4. ִ�в���
    op(cwd);

    // 5. ����ش���
    savetoDisk(disk, diskFile);

    // 6. ����
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
    cout << "��ӭʹ�ü����ļ�ϵͳ��\n";

    // �ȹ̶� vfs.dat ·������ȷ�� data ��Ŀ¼����
    initVfsPath();

    // ����ʱ��� vfs.dat �����Ҵ�С > 0��������һ��
    try {
        if (filesystem::exists(g_vfsPath) && filesystem::file_size(g_vfsPath) > 0) {
            bool ok = loadFromDisk(disk, g_vfsPath);
            if (!ok) {
                cout << "[����] �޷��������������̣���ʹ�ÿ������̡�\n";
            }
        }
    }
    catch (...) {
        // �����쳣
    }

    while (true) {
        // �ǳ�����������ʱ currentUser Ϊ�գ�ǿ������¼/ע��/�˳�
        currentUser.clear();
        cwdPath.clear();
        currentDir = nullptr;

        while (currentUser.empty()) {
            cout << "1. ע��\n2. ��¼\n3. �˳�\n��ѡ�����: ";
            int choice;
            cin >> choice;
            cin.ignore(); // ���Իس�

            if (choice == 1) {
                // ע�᣺ֱ���޸��ڴ沢�־û�һ��
                string username, password;
                cout << "�������û���: "; getline(cin, username);
                cout << "����������: "; getline(cin, password);
                registeUser(disk, username, password);

                // ע��������־û�������
                executeCommandSerialized([](Directory*) {});
            }
            else if (choice == 2) {
                // ��¼���ȴӴ��̼��أ���֤��ȡ��������״̬
                executeCommandSerialized([](Directory*) {});

                string username, password;
                cout << "�������û���: "; getline(cin, username);
                cout << "����������: "; getline(cin, password);

                string loggedIn;
                if (loginUser(disk, username, password, loggedIn)) {
                    currentUser = loggedIn;
                    // ��¼�ɹ�����ǰĿ¼ָ����û���Ŀ¼
                    currentDir = disk.userFileSystems[currentUser].root;
                    cwdPath.clear();
                    break;
                }
            }
            else {
                // �˳�����
                cout << "�˳�����\n";
                // �˳�ǰ�ٱ���һ��
                executeCommandSerialized([](Directory*) {});
                return 0;
            }
        }

        //�ѵ�¼�û�����ѭ�� 
        string line;
        while (true) {
            printPrompt();
            getline(cin, line);
            istringstream iss(line);
            string cmd;
            iss >> cmd;

            if (cmd == "exit") {
                // �˳���ǰ�û��Ự
                cout << "���˳���¼��\n";
                break;
            }

            string arg1, arg2;
            iss >> arg1;
            iss >> ws;
            getline(iss, arg2);


            // �л�Ŀ¼ (cd) 
            if (cmd == "cd") {
                if (arg1 == "..") {
                    if (cwdPath.empty()) {
                        cout << "[��ʾ] ��ǰ���ڸ�Ŀ¼���޷����ء�\n";
                    }
                    else {
                        cwdPath.pop_back();
                        cout << "������һ��Ŀ¼��\n";
                    }
                }
                else {
                    // cd <Ŀ¼��>���ȼ������´���״̬���ټ����Ŀ¼
                    executeCommandSerialized([&](Directory* cwd) {
                        if (!cwd) return;
                        if (cwd->subDirs.find(arg1) == cwd->subDirs.end()) {
                            cout << "[����] ��Ŀ¼�����ڣ�" << arg1 << "\n";
                        }
                        else {
                            cwdPath.push_back(arg1);
                            cout << "[�ɹ�] ��ǰĿ¼�л�����" << arg1 << "\n";
                        }
                        });
                }
                continue;
            }
            // �г�Ŀ¼ (dir)
            else if (cmd == "dir") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (cwd) listDirectory(cwd);
                    });
                continue;
            }
            // ������ʾ (tree)
            else if (cmd == "tree") {
                executeCommandSerialized([&](Directory*) {
                    showTree(disk);
                    });
                continue;
            }

            // ��Ҫ����+load+ִ��+save ������

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
                    else cout << "[����] ��ʽӦΪ��flock <�ļ���> lock/unlock\n";
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
                        cout << "[����] ��ʽӦΪ��head -[����] �ļ���\n";
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
                        cout << "[����] ��ʽӦΪ��tail -[����] �ļ���\n";
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
                    if (!importFile(cwd, arg1, arg2)) cout << "[����] ����ʧ��\n";
                    else cout << "[�ɹ�] �ļ��ѵ���\n";
                    });
            }
            else if (cmd == "export") {
                executeCommandSerialized([&](Directory* cwd) {
                    if (!cwd) return;
                    if (!exportFile(cwd, arg1, arg2)) cout << "[����] ����ʧ��\n";
                    else cout << "[�ɹ�] �ļ��ѵ���\n";
                    });
            }
            else {
                cout << "[��ʾ] ��֧�ֵ����\n";
            }
        }
    }

    return 0;
}
