// fs.h
#pragma once

#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <ctime>
#include <Windows.h>

using namespace std;

struct User {
    string password;
    bool   isLocked = false;
    int    loginAttempts = 0;
};

struct File {
    string        name;
    vector<char>  content;
    int           size = 0;
    time_t        modifiedTime = 0;
    bool          isOpen = false;
    bool          isLocked = false;
    int           readPtr = 0;
};

struct Directory {
    string                         name;
    map<string, File>              files;
    map<string, Directory*>        subDirs;
    Directory* parent = nullptr;
};

struct FileSystem {
    Directory* root = nullptr;
};

struct VirtualDisk {
    map<string, User>       users;
    map<string, FileSystem> userFileSystems;
};

void savetoDisk(const VirtualDisk& disk, const string& filename);
bool loadFromDisk(VirtualDisk& disk, const string& filename);
bool importFile(Directory* dir, const string& src, const string& dest);
bool exportFile(Directory* dir, const string& name, const string& dst);