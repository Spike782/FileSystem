#include "user.h"
#include <iostream>
using namespace std;

//�û�ע��
bool registeUser(VirtualDisk& disk, const string& username, const string& password) {
    if (disk.users.count(username)) {
        cout << "[����] �û��Ѵ���\n";
        return false;
    }
    disk.users[username] = { password, false, 0 };
    FileSystem fs;
    fs.root = new Directory;
    fs.root->name = "/";
    fs.root->parent = nullptr;
    disk.userFileSystems[username] = fs;
    cout << "[�ɹ�] ע��ɹ�\n";
    return true;
}

//�û���¼
bool loginUser(VirtualDisk& disk, const string& username, const string& password, string& currentUser) {
    auto i = disk.users.find(username);
    if (i == disk.users.end()) {
        cout << "[����] �û�������\n"; 
        return false;
    }
    User& u = i->second;
    if (u.isLocked) {
        cout << "[����] �û�������\n";
        return false;
    }
    if (u.password == password) {
        u.loginAttempts = 0;
        currentUser = username;
        cout << "[�ɹ�] ��¼�ɹ�\n";
        return true;
    }
    else {
        ++u.loginAttempts;
        if (u.loginAttempts >= 3) {
            u.isLocked = true;
            cout << "[����] ���δ��������û�\n";
        }
        else {
            cout << "[����] �������\n";
        }
        return false;
    }
}
