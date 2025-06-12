#include <iostream>
#include <sstream>
#include <vector>
#include "fs.h"
#include "user.h"
#include "command.h"

using namespace std;

VirtualDisk disk;
string       currentUser;
Directory* currentDir = nullptr;
const string VFS_FILE_PATH = "D:\\Cprojects\\OS_cd\\x64\\Debug\\vfs.dat";

void printPrompt() {
    // �ռ��ӵ�ǰĿ¼����������
    vector<string> names;
    for (Directory* p = currentDir; p && p->parent; p = p->parent) {
        names.push_back(p->name);
    }
    reverse(names.begin(), names.end());

    // ��ӡ user:~[/a/b/c]$
    cout << currentUser << ":~";
    for (auto& n : names) {
        cout << "/" << n;
    }
    cout << "$ ";
}


int main() {
    cout << "��ӭʹ�ü����ļ�ϵͳ��\n";
    while (true) {
        // �ǳ� / ����״̬
        currentUser.clear();
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
                return 0;
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
                if (arg1 == "..") {
                    if (currentDir->parent) {
                        currentDir = currentDir->parent;
                    }
                    else {
                        cout << "[��ʾ] ���ڸ�Ŀ¼���޷����ء�\n";
                    }
                }
                else {
                    auto it = currentDir->subDirs.find(arg1);
                    if (it == currentDir->subDirs.end()) {
                        cout << "[����] ��Ŀ¼�����ڣ�" << arg1 << "\n";
                    }
                    else {
                        currentDir = it->second;
                    }
                }
                continue;
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
                //head -[����] �ļ���
                int n = stoi(arg1.substr(1));
                headFile(currentDir, arg2, n);
            }
            else if (cmd == "tail") {
                //tail -[����] �ļ���
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
                    writeFile(currentDir, arg1,arg2);
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
            else if (cmd == "lseek") {
                // arg1 �Ѿ����ļ�����arg2 ��ƫ�������ַ���
                if (arg2.empty()) {
                    cout << "[����] ��ʽ: lseek <�ļ���> <ƫ����>\n";
                }
                else {
                    try {
                        int offset = stoi(arg2);
                        lseekFile(currentDir, arg1, offset);
                        mutated = true;   // �������дģʽ��Ҫ����
                    }
                    catch (const exception&) {
                        cout << "[����] ��Ч��ƫ����: " << arg2 << "\n";
                    }
                }
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
