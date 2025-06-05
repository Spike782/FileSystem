#pragma once
#include <string>
#include "fs.h"

// 当前目录指针，全局变量
extern Directory* currentDir;

// 命令声明
void mkdir(Directory* dir, const std::string& dirname);
void rmdir(Directory* dir, const std::string& dirname);
void create(Directory* dir, const std::string& filename);
void deleteFile(Directory* dir, const std::string& filename);
void openFile(Directory* dir, const string& filename);
void closeFile(Directory* dir, const string& filename);
void readFile(Directory* dir, const string& filename);
void writeFile(Directory* dir, const string& filename, const string& data);
void moveFile(Directory* dir, const string& src, const string& dest);
void copyFile(Directory* dir, const string& src, const string& dest);
void flockFile(Directory* dir, const string& filename, bool lock);
void headFile(Directory* dir, const string& filename, int num);
void tailFile(Directory* dir, const string& filename, int num);
void lseekFile(Directory* dir, const string& filename, int offset);
void changeDirectory(Directory*& dir, const string& dirname);
void listDirectory(Directory* dir);
void showTree(const VirtualDisk& disk);

