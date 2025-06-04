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

