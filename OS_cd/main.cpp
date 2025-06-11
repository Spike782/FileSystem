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
    cout << "��ӭʹ�ü����ļ�ϵͳ��\n";
    while (true) {
        // �ǳ� / ����״̬
        currentUser.clear();
        cwdPath.clear();
        currentDir = nullptr;

        // ע�� / ��¼ / �˳� ѭ��  
        while (currentUser.empty()) {
            cout << "1. ע��  2. ��¼  3. �˳�\n��ѡ��: ";
            int c;
            cin >> c;
            cin.ignore();

            if (c == 1) {
                // ע�᣺load �� registeUser �� save
                if (!loadFromDisk(disk, VFS_FILE_PATH)) {
                    cout << "[��ʾ] ����ע�ᣬ�����ļ������ڣ�ʹ�ÿ���\n";
                }
                string u, p;
                cout << "�û���: "; getline(cin, u);
                cout << "����: "; getline(cin, p);

                registeUser(disk, u, p);
                savetoDisk(disk, VFS_FILE_PATH);
            }
            else if (c == 2) {
                // ��¼��load �� loginUser �� (save ��¼���Լ������)
                if (!loadFromDisk(disk, VFS_FILE_PATH)) {
                    cout << "[��ʾ] �����ļ������ڣ�ʹ�ÿ���\n";
                }
                string u, p, li;
                cout << "�û���: "; getline(cin, u);
                cout << "����: "; getline(cin, p);
                if (loginUser(disk, u, p, li)) {
                    currentUser = li;
                    currentDir = disk.userFileSystems[currentUser].root;
                    cwdPath.clear();
                }
                // ���۵�¼�ɹ���񣬶�����һ�Σ��Ը��� loginAttempts/isLocked��
                savetoDisk(disk, VFS_FILE_PATH);
            }
            else if (c == 3) {
                // �˳�����ǰ����һ��
                savetoDisk(disk, VFS_FILE_PATH);
                cout << "�˳�����\n";
                return 0;
            }
            else {
                cout << "[��ʾ] ��Чѡ��\n";
            }
        }

        // ���� �ѵ�¼�û�����ѭ�� ���� 
        string line;
        while (true) {
            printPrompt();
            if (!getline(cin, line)) {
                // ����������
                savetoDisk(disk, VFS_FILE_PATH);
                return 0;
            }
            if (line.empty()) continue;

            // ÿ��ִ��ǰ������ load һ��
            if (!loadFromDisk(disk, VFS_FILE_PATH)) {
                cout << "[��ʾ] ���̼���ʧ�ܣ�ʹ�õ�ǰ�ڴ�״̬\n";
            }
            // ȷ�� currentDir ���� cwdPath
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
                cout << "�ѵǳ���\n";
                break;
            }

            bool mutated = false;

            // ���� ֻ������ ���� 
            if (cmd == "cd") {
                Directory* root = disk.userFileSystems[currentUser].root;
                if (arg1 == "..") {
                    if (!cwdPath.empty()) cwdPath.pop_back();
                    else cout << "[��ʾ] ���ڸ�Ŀ¼\n";
                }
                else {
                    Directory* cwd = resolveCwd(root);
                    if (cwd && cwd->subDirs.count(arg1)) {
                        cwdPath.push_back(arg1);
                    }
                    else {
                        cout << "[����] ��Ŀ¼�����ڣ�" << arg1 << "\n";
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
            // ���� д���� ���� 
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
                // exportFile ֻ��д���� FS����Ӱ�� vfs.dat
            }
            else {
                cout << "[��ʾ] ��֧�ֵ����" << cmd << "\n";
            }

            // д�˲ű���
            if (mutated) {
                savetoDisk(disk, VFS_FILE_PATH);
            }
        }
    }
    return 0;
}
