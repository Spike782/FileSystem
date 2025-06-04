#pragma once
#include <string>
#include "fs.h"

// ��ǰĿ¼ָ�룬ȫ�ֱ���
extern Directory* currentDir;

// ��������
void mkdir(Directory* dir, const std::string& dirname);
void rmdir(Directory* dir, const std::string& dirname);
void create(Directory* dir, const std::string& filename);
void deleteFile(Directory* dir, const std::string& filename);
void openFile(Directory* dir, const string& filename);
void closeFile(Directory* dir, const string& filename);
void readFile(Directory* dir, const string& filename);
void writeFile(Directory* dir, const string& filename, const string& data);

