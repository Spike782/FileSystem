#pragma once

#include "fs.h"
#include <string>
using namespace std;

extern Directory* currentDir;

void mkdir(Directory* dir, const string& dirname);
void rmdir(Directory* dir, const string& dirname);
void create(Directory* dir, const string& filename);
void deleteFile(Directory* dir, const string& filename);
void openFile(Directory* dir, const string& filename);
void closeFile(Directory* dir, const string& filename);
void readFile(Directory* dir, const string& filename);
void writeFile(Directory* dir, const string& filename, const string& data);
void moveFile(Directory* dir, const string& src, const string& dest);
void copyFile(Directory* dir, const string& src, const string& dest);
void flockFile(Directory* dir, const string& filename, bool lockFlag);
void headFile(Directory* dir, const string& filename, int num);
void tailFile(Directory* dir, const string& filename, int num);
void lseekFile(Directory* dir, const string& filename, int offset);
void listDirectory(Directory* dir);
void showTree(const VirtualDisk& disk);
