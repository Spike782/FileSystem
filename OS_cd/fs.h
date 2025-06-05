#pragma once
#include <string>
#include<map>
#include<ctime>
using namespace std;


//�û��ṹ��
struct User {
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
    map<string, Directory*> subDirs;
    Directory* parent = nullptr;//ָ����һ��Ŀ¼
};

// ÿ���û����ļ�ϵͳ
struct FileSystem {
    string owner;
    Directory* root;
};

// ������̽ṹ
struct VirtualDisk {
    map<string, User> users;
    map<string, FileSystem> userFileSystems;
};

void savetoDisk(const VirtualDisk& disk, const string& filename);
bool loadFromDisk(VirtualDisk& disk, const string& filename);
bool importFile(Directory* currentDir, const string& srcPath, const string& destName = "");
bool exportFile(Directory* currentDir, const string& fileName, string destPath);

