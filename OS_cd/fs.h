#pragma once
#include <string>
#include<map>
#include<ctime>
using namespace std;


//用户结构体
struct User {
	string username;
	string password;
	bool isLocked = false;
	int loginAttempts = 0;
};

// 文件结构体
struct File {
    string name;
    string content;
    int size = 0;
    time_t modifiedTime = time(nullptr);
    bool isOpen = false;
    bool isLocked = false;
    int readPtr = 0;
};

// 目录结构体
struct Directory {
    string name;
    map<string, File> files;
    map<string, Directory> subDirs;
};

// 每个用户的文件系统
struct FileSystem {
    string owner;
    Directory root;
};

// 虚拟磁盘结构
struct VirtualDisk {
    map<string, FileSystem> userFileSystems;
    map<string, User> users;
};
