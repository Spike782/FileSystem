#include "user.h"
#include <iostream>
using namespace std;

bool registeUser(VirtualDisk& disk, const string& username, const string& password) {
    if (disk.users.find(username) != disk.users.end()) {
        cout << "[错误] 用户名已存在！" << endl;
        return false;
    }
    User newUser{ username, password };
    disk.users[username] = newUser;

    // 初始化该用户的文件系统
    FileSystem fs;
    fs.owner = username;
    fs.root.name = "/";
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
        cout << "[错误] 用户已被锁定！" << endl;
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
