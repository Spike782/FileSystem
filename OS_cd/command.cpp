#include"command.h"
#include<iostream>
#include<sstream>
#include <vector>
using namespace std;

Directory* currentDir = nullptr;
Directory* parentDir = nullptr;  // 用于跟踪上一级目录

void mkdir(Directory* dir, const string& dirname) {
	if (dir->subDirs.find(dirname) != dir->subDirs.end()) {
		cout << "目录已存在！" << endl;
		return;
	}
	Directory* newDir=new Directory;
	newDir->name = dirname;
    newDir->parent = dir;
	dir->subDirs[dirname] = newDir;
	cout << "目录创建成功：" << dirname << endl;
}

void rmdir(Directory* dir, const string& dirname) {
	auto i = dir->subDirs.find(dirname);
	if (i == dir->subDirs.end()) {
		cout << "找不到目录：" << dirname << endl;
		return;
	}
    Directory* target = i->second;
	if (!target->subDirs.empty() || !target->files.empty()) {
		cout << "目录不为空，不能删除！" << endl;
		return;
	}
    delete target;
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

void moveFile(Directory* dir, const string& src, const string& dest) {
    auto i = dir->files.find(src);
    if (i == dir->files.end()) {
        cout << "找不到源文件：" << src << endl;
        return;
    }
    if (dir->files.find(dest) != dir->files.end()) {
        cout << "目标文件已存在：" << dest << endl;
        return;
    }
    File movefile = i->second;
    movefile.name = dest;
    dir->files.erase(i);
    dir->files[dest] = movefile;
    cout << "文件移动成功：" << src << "->" << dest << endl;
}

void copyFile(Directory* dir, const string& src, const string& dest) {
    auto i = dir->files.find(src);
    if (i== dir->files.end()) {
        cout << "找不到源文件：" << src << endl;
        return;
    }
    if (dir->files.find(dest) != dir->files.end()) {
        cout << "目标文件已存在：" << dest << endl;
        return;
    }
    File copiedFile = i->second;
    copiedFile.name = dest;
    dir->files[dest] = copiedFile;
    cout << "文件复制成功：" << src << " -> " << dest << endl;
}

void flockFile(Directory* dir, const string& filename, bool lock) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "找不到源文件：" << filename << endl;
        return;
    }
    i->second.isLocked = lock;
    cout << (lock ? " 文件已加锁：" : "文件已解锁：") << filename << endl;
}

void headFile(Directory* dir, const string& filename, int num) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "找不到源文件：" << filename << endl;
        return;
    }
    istringstream ss(i->second.content);
    string line;
    int count = 0;
    cout << "[前" << num << "行]" << endl;
    while (count<num) {
        getline(ss, line);
        cout << line << endl;
        count++;
    }
}

void tailFile(Directory* dir, const string& filename, int num) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "找不到源文件：" << filename << endl;
        return;
    }
    istringstream ss(i->second.content);
    vector<string>lines;
    string line;
    while (getline(ss, line)) {
        lines.push_back(line);
    }
    int start = max(0, static_cast<int>(lines.size()) - num);
    cout << "[后" << num << "行]" << endl;
    for (int i = start; i < lines.size(); ++i) {
        cout << lines[i] << endl;
    }
}

void lseekFile(Directory* dir, const string& filename, int offset) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "找不到源文件：" << filename << endl;
        return;
    }
    int newPos = i->second.readPtr + offset;
    if (newPos<0 || newPos>static_cast<int>(i->second.content.size())) {
        cout << "指针移动越界" << endl;
        return;
    }
    i->second.readPtr = newPos;
    cout << "新的指针位置：" << i->second.readPtr << endl;
}

void changeDirectory(Directory*& dir, const string& dirname) {
    if (dirname == "..") {
        if (dir->parent != nullptr) {
            dir = dir->parent;
            cout << "返回上一级目录。" << endl;
        }
        else {
            cout << "[提示] 当前已在根目录，无法返回。" << endl;
        }
        return;
    }
    auto i = dir->subDirs.find(dirname);
    if (i == dir->subDirs.end()) {
        cout << "[错误] 子目录不存在：" << dirname << endl;
        return;
    }
    dir = i->second;
    cout << "[成功] 当前目录切换到：" << dirname << endl;
}

void listDirectory(Directory* dir) {
    cout << "[目录] " << dir->name << endl;
    if (!dir->subDirs.empty()) {
        cout << "子目录：\n";
        map<string, Directory*>::iterator i;
        for (i = dir->subDirs.begin(); i != dir->subDirs.end(); ++i) {
            cout << " <DIR> " << i->first << endl;
        }
    }
    if (!dir->files.empty()) {
        cout << "文件：\n";
        map<string, File>::iterator i;
        for (i = dir->files.begin(); i != dir->files.end(); ++i) {
            cout << "       " << i->first << " (" << i->second.size << "B)" << endl;
        }
    }
    if (dir->subDirs.empty() && dir->files.empty()) {
        cout << "[空目录]" << endl;
    }
}

void printDirectoryTree(Directory* dir, const string& prefix) {
    auto it = dir->subDirs.begin();
    while (it != dir->subDirs.end()) {
        bool isLast = next(it) == dir->subDirs.end() && dir->files.empty();
        cout << prefix << (isLast ? "└── " : "├── ") << it->first << "/" << endl;
        printDirectoryTree(it->second, prefix + (isLast ? "    " : "│   "));
        ++it;
    }

    auto fit = dir->files.begin();
    while (fit != dir->files.end()) {
        bool isLast = next(fit) == dir->files.end();
        cout << prefix << (isLast ? "└── " : "├── ");
        cout << fit->first << " (" << fit->second.size << "B)" << endl;
        ++fit;
    }
}


void showTree(const VirtualDisk& disk) {
    cout << "/ (虚拟根目录)" << endl;
    auto it = disk.userFileSystems.begin();
    while (it != disk.userFileSystems.end()) {
        bool isLast = next(it) == disk.userFileSystems.end();
        cout << (isLast ? "└── " : "├── ") << it->first << "/" << endl;
        printDirectoryTree(it->second.root, isLast ? "    " : "│   ");
        ++it;
    }
}





