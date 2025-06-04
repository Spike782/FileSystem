#include"command.h"
#include<iostream>
using namespace std;

Directory* currentDir = nullptr;

void mkdir(Directory* dir, const string& dirname) {
	if (dir->subDirs.find(dirname) != dir->subDirs.end()) {
		cout << "目录已存在！" << endl;
		return;
	}
	Directory newDir;
	newDir.name = dirname;
	dir->subDirs[dirname] = newDir;
	cout << "目录创建成功：" << dirname << endl;
}

void rmdir(Directory* dir, const string& dirname) {
	auto i = dir->subDirs.find(dirname);
	if (i == dir->subDirs.end()) {
		cout << "找不到目录：" << dirname << endl;
		return;
	}
	if (!i->second.subDirs.empty() || !i->second.files.empty()) {
		cout << "目录不为空，不能删除！" << endl;
		return;
	}
	dir->subDirs.erase(i);
	cout << "目录已删除：" << dirname << endl;
}

void create(Directory* dir, const string& filename) {
	if (dir->files.find(filename) != dir->files.end()) {
		cout << "文件已存在：" << filename << endl;
		return;
	}
	File newfile;
	newfile.name = filename;
	dir->files[filename] = newfile;
	cout << "文件创建成功：" << filename << endl;
}

void deleteFile(Directory* dir, const string& filename) {
	auto i = dir->files.find(filename);
	if (i == dir->files.end()) {
		cout << "找不到文件：" << filename << endl;
		return;
	}
	dir->files.erase(i);
	cout << "文件已删除：" << filename << endl;
}

void openFile(Directory* dir, const string& filename) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "找不到文件：" << filename << endl;
        return;
    }
    if (i->second.isOpen) {
        cout << "文件已打开：" << filename <<endl;
    }
    else {
        i->second.isOpen = true;
        cout << "文件已打开：" << filename << endl;
    }
}

void closeFile(Directory* dir, const string& filename) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "找不到文件：" << filename << endl;
        return;
    }
    if (!i->second.isOpen) {
       cout << "[提示] 文件尚未打开：" << filename << endl;
    }
    else {
        i->second.isOpen = false;
        cout << "文件已关闭：" << filename << endl;
    }
}

void readFile(Directory* dir, const string& filename) {
    auto it = dir->files.find(filename);
    if (it == dir->files.end()) {
        cout << "找不到文件：" << filename << endl;
        return;
    }
    if (!it->second.isOpen) {
        cout << "文件未打开，无法读取。" << endl;
        return;
    }
    cout << "[内容] " << filename << ":\n" << it->second.content << endl;
}

void writeFile(Directory* dir, const string& filename, const string& data) {
    auto it = dir->files.find(filename);
    if (it == dir->files.end()) {
        cout << " 找不到文件：" << filename << endl;
        return;
    }
    if (!it->second.isOpen) {
        cout << " 文件未打开，无法写入。" << endl;
        return;
    }
    it->second.content += data;
    it->second.size = it->second.content.size();
    it->second.modifiedTime = time(nullptr);
    cout << "写入完成：" << filename << endl;
}