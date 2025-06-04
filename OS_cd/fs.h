#pragma once
#include <string>
#include<map>
#include<ctime>
using namespace std;


//�û��ṹ��
struct User {
	string username;
	string password;
	bool isLocked = false;
	int loginAttempts = 0;
};

// �ļ��ṹ��
struct File {
    string name;
    string content;
    int size = 0;
    time_t modifiedTime = time(nullptr);
    bool isOpen = false;
    bool isLocked = false;
    int readPtr = 0;
};

// Ŀ¼�ṹ��
struct Directory {
    string name;
    map<string, File> files;
    map<string, Directory> subDirs;
};

// ÿ���û����ļ�ϵͳ
struct FileSystem {
    string owner;
    Directory root;
};

// ������̽ṹ
struct VirtualDisk {
    map<string, FileSystem> userFileSystems;
    map<string, User> users;
};
