#include <iostream>
#include <sstream>
#include "fs.h"
#include "user.h"
#include "command.h"

VirtualDisk disk;  // �������
string currentUser;  // ��ǰ��¼�û�

void printPrompt() {
    cout << currentUser << ":~$ ";
}

int main() {
    cout << "��ӭʹ�ü����ļ�ϵͳ��\n";
    loadFromDisk(disk, "vfs.dat");
    while (true) {
        currentUser = "";
        while (currentUser.empty()) {
            cout << "1. ע��\n2. ��¼\n3. �˳�\n��ѡ�����: ";
            int choice;
            cin >> choice;
            cin.ignore(); // ���Իس���

            string username, password;

            if (choice == 1) {
                cout << "�������û���: "; getline(cin, username);
                cout << "����������: "; getline(cin, password);
                registeUser(disk, username, password);
            }
            else if (choice == 2) {
                cout << "�������û���: "; getline(cin, username);
                cout << "����������: "; getline(cin, password);
                if (loginUser(disk, username, password, currentUser)) {
                    currentDir = disk.userFileSystems[currentUser].root;
                    break;
                }
            }
            else {
                cout << "�˳�����\n";
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
                cout << "���˳���¼��\n";
                break;
            }

            string arg1, arg2;
            iss >> arg1;
            iss >> ws;
            getline(iss, arg2); // ֧��д����ո�
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
                else std::cout << "[����] ��ʽӦΪ��flock <�ļ���> lock/unlock\n";
            }
            else if (cmd == "head") {
                if (arg1.substr(0, 1) == "-") {
                    int num = stoi(arg1.substr(1));
                    iss >> arg2;
                    headFile(currentDir, arg2, num);
                }
                else {
                    std::cout << "[����] ��ʽӦΪ��head -[����] �ļ���\n";
                }
            }
            else if (cmd == "tail") {
                if (arg1.substr(0, 1) == "-") {
                    int num = stoi(arg1.substr(1));
                    iss >> arg2;
                    tailFile(currentDir, arg2, num);
                }
                else {
                    std::cout << "[����] ��ʽӦΪ��tail -[����] �ļ���\n";
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
                if (!importFile(currentDir, arg1, arg2)) std::cout << "[����] ����ʧ��\n";
                else std::cout << "[�ɹ�] �ļ��ѵ���\n";
            }
            else if (cmd == "export") {
                if (!exportFile(currentDir, arg1, arg2)) std::cout << "[����] ����ʧ��\n";
                else std::cout << "[�ɹ�] �ļ��ѵ���\n";
            }
            else {
                cout << "[��ʾ] ��֧�ֵ����\n";
            }
        }

    }
    return 0;
}
