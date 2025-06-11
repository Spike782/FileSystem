#include "user.h"
#include <iostream>
using namespace std;

//用户注册
bool registeUser(VirtualDisk& disk, const string& username, const string& password) {
    if (disk.users.count(username)) {
        cout << "[错误] 用户已存在\n";
        return false;
    }
    disk.users[username] = { password, false, 0 };
    FileSystem fs;
    fs.root = new Directory;
    fs.root->name = "/";
    fs.root->parent = nullptr;
    disk.userFileSystems[username] = fs;
    cout << "[成功] 注册成功\n";
    return true;
}

//用户登录
bool loginUser(VirtualDisk& disk, const string& username, const string& password, string& currentUser) {
    auto i = disk.users.find(username);
    if (i == disk.users.end()) {
        cout << "[错误] 用户不存在\n"; 
        return false;
    }
    User& u = i->second;
    if (u.isLocked) {
        cout << "[错误] 用户被锁定\n";
        return false;
    }
    if (u.password == password) {
        u.loginAttempts = 0;
        currentUser = username;
        cout << "[成功] 登录成功\n";
        return true;
    }
    else {
        ++u.loginAttempts;
        if (u.loginAttempts >= 3) {
            u.isLocked = true;
            cout << "[警告] 三次错误，锁定用户\n";
        }
        else {
            cout << "[错误] 密码错误\n";
        }
        return false;
    }
}
