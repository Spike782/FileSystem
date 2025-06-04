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
    while (true) {
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
                currentDir = &disk.userFileSystems[currentUser].root;
                break;
            }
        }
        else {
            cout << "�˳�����\n";
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
           cout << "���˳���¼��\n";
            break;
        }
        else {
            cout << "[��ʾ] ��֧�ֵ����\n";
        }
    }

    return 0;
}
