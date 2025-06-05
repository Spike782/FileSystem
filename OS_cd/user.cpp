#include "user.h"
#include <iostream>
using namespace std;

bool registeUser(VirtualDisk& disk, const string& username, const string& password) {
    if (disk.users.count(username)) {
        cout << "[����] �û����Ѵ��ڣ�" << endl;
        return false;
    }

    disk.users[username] = { password, 0, false };

    FileSystem fs;
    fs.root = new Directory;
    fs.root->name = "/";
    fs.root->parent = nullptr;
    disk.userFileSystems[username] = fs;

    cout << "[�ɹ�] �û�ע��ɹ���" << endl;
    return true;
}


bool loginUser(VirtualDisk& disk, const string& username, const string& password, string& currentUser) {
    auto it = disk.users.find(username);
    if (it == disk.users.end()) {
        cout << "[����] �û������ڣ�" << endl;
        return false;
    }

    User& user = it->second;
    if (user.isLocked) {
        cout << "[����] �û��ѱ�������" <<endl;
        return false;
    }

    if (user.password == password) {
        user.loginAttempts = 0;
        currentUser = username;
        cout << "[�ɹ�] ��¼�ɹ�����ӭ " << username << "��" << endl;
        return true;
    }
    else {
        user.loginAttempts++;
        cout << "[����] �������" << endl;
        if (user.loginAttempts >= 3) {
            user.isLocked = true;
            cout << "[����] ����������Σ��û��ѱ�������" << endl;
        }
        return false;
    }
}
