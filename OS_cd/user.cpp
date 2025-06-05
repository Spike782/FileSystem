#include "user.h"
#include <iostream>
using namespace std;

bool registeUser(VirtualDisk& disk, const string& username, const string& password) {
    if (disk.users.count(username)) {
        cout << "[错误] 用户名已存在！" << endl;
        return false;
    }

    disk.users[username] = { password, 0, false };

    FileSystem fs;
    fs.root = new Directory;
    fs.root->name = "/";
    fs.root->parent = nullptr;
    disk.userFileSystems[username] = fs;

    cout << "[成功] 用户注册成功！" << endl;
    return true;
}


bool loginUser(VirtualDisk& disk, const string& username, const string& password, string& currentUser) {
    auto it = disk.users.find(username);
    if (it == disk.users.end()) {
        cout << "[错误] 用户不存在！" << endl;
        return false;
    }

    User& user = it->second;
    if (user.isLocked) {
        cout << "[错误] 用户已被锁定！" <<endl;
        return false;
    }

    if (user.password == password) {
        user.loginAttempts = 0;
        currentUser = username;
        cout << "[成功] 登录成功，欢迎 " << username << "！" << endl;
        return true;
    }
    else {
        user.loginAttempts++;
        cout << "[错误] 密码错误！" << endl;
        if (user.loginAttempts >= 3) {
            user.isLocked = true;
            cout << "[警告] 密码错误三次，用户已被锁定！" << endl;
        }
        return false;
    }
}
