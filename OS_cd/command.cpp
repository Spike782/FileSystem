// command.cpp
#include "command.h"
#include "fs.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <ctime>

using namespace std;

extern Directory* currentDir;

void mkdir(Directory* dir, const string& name) {
    if (dir->subDirs.count(name)) {
        cout << "目录已存在\n"; return;
    }
    Directory* nd = new Directory;
    nd->name = name; nd->parent = dir;
    dir->subDirs[name] = nd;
    cout << "目录创建: " << name << "\n";
}

void rmdir(Directory* dir, const string& name) {
    auto i = dir->subDirs.find(name);
    if (i== dir->subDirs.end()) {
        cout << "未找到目录\n"; return;
    }
    Directory* t = i->second;
    if (!t->files.empty() || !t->subDirs.empty()) {
        cout << "目录非空\n"; return;
    }
    delete t;
    dir->subDirs.erase(i);
    cout << "目录删除: " << name << "\n";
}

void create(Directory* dir, const string& filename) {
    if (dir->files.count(filename)) {
        cout << "文件已存在：" << filename << "\n";
        return;
    }
    File f;
    f.name = filename;
    f.modifiedTime = time(nullptr);   // 记录创建时间
    dir->files[filename] = f;
    cout << "文件创建成功：" << filename << "\n";
}

void deleteFile(Directory* dir, const string& file) {
    if (!dir->files.erase(file)) {
        cout << "未找到文件\n"; return;
    }
    cout << "文件删除: " << file << "\n";
}

void openFile(Directory* dir, const string& file) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "未找到文件\n"; return; 
    }
    i->second.isOpen = true;
    cout << "文件已打开: " << file << "\n";
}

void closeFile(Directory* dir, const string& file) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "未找到文件\n"; return; 
    }
    i->second.isOpen = false;
    cout << "文件已关闭: " << file << "\n";
}

void readFile(Directory* dir, const string& file) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "未找到文件\n"; 
        return; 
    }
    if (!i->second.isOpen) { 
        cout << "文件未打开\n"; 
        return; 
    }
    string data(i->second.content.begin(), i->second.content.end());
    cout << "[内容] " << file << ":\n" << data << "\n";
}

void writeFile(Directory* dir, const string& file, const string& d) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "未找到文件\n"; 
        return; 
    }
    if (!i->second.isOpen) { 
        cout << "文件未打开\n"; 
        return; 
    }
    i->second.content.insert(i->second.content.end(), d.begin(), d.end());
    i->second.size = static_cast<int>(i->second.content.size());
    i->second.modifiedTime = time(nullptr);
    cout << "写入完成: " << file << "\n";
}

void moveFile(Directory* dir, const string& src, const string& dest) {
    // 1) 在当前目录查找源文件
    auto it = dir->files.find(src);
    if (it == dir->files.end()) {
        cout << "找不到源文件：" << src << "\n";
        return;
    }

    // 2) 拆分目标路径
    vector<string> parts;
    size_t start = 0;
    while (start <= dest.size()) {
        size_t pos = dest.find('/', start);
        if (pos == string::npos) {
            parts.push_back(dest.substr(start));
            break;
        }
        parts.push_back(dest.substr(start, pos - start));
        start = pos + 1;
        // 如果 dest 以 '/' 结尾，这一段会 push_back("")
        if (start == dest.size()) {
            parts.push_back("");
            break;
        }
    }

    // 3) 从当前目录开始，按各级目录遍历或创建
    Directory* targetDir = dir;
    int levels = (int)parts.size();
    // 如果最后一项是空串，说明 dest 以 '/' 结尾，应保留原文件名
    bool toDirOnly = (!parts.empty() && parts.back().empty());
    int limit = toDirOnly ? levels - 1 : levels - 1;
    for (int i = 0; i < limit; ++i) {
        const string& part = parts[i];
        if (part.empty() || part == ".") {
            // 忽略 "./"
            continue;
        }
        if (part == "..") {
            // 返回父目录（若存在）
            if (targetDir->parent) {
                targetDir = targetDir->parent;
            }
            // 否则留在根目录
        }
        else {
            // 进入或创建子目录
            auto sit = targetDir->subDirs.find(part);
            if (sit == targetDir->subDirs.end()) {
                Directory* nd = new Directory;
                nd->name = part;
                nd->parent = targetDir;
                targetDir->subDirs[part] = nd;
                targetDir = nd;
            }
            else {
                targetDir = sit->second;
            }
        }
    }

    // 4) 确定 newName
    string newName;
    if (toDirOnly) {
        newName = src; // 保留原名
    }
    else {
        newName = parts.back();
        if (newName.empty()) {
            cout << "[错误] 目标文件名为空\n";
            return;
        }
    }

    // 5) 冲突检查
    if (targetDir->files.count(newName)) {
        std::cout << "目标文件已存在：" << newName << "\n";
        return;
    }

    // 6) 执行移动：复制文件对象，更新名称和修改时间
    File f = it->second;
    f.name = newName;
    f.modifiedTime = time(nullptr);

    // 7) 从原目录移除，插入目标目录
    dir->files.erase(it);
    targetDir->files[newName] = move(f);

    cout << "文件移动成功：" << src << " -> " << dest << "\n";
}

void copyFile(Directory* dir, const string& s, const string& t) {
    auto i = dir->files.find(s);
    if (i == dir->files.end()) { 
        cout << "源文件不存在\n"; 
        return; 
    }
    if (dir->files.count(t)) { 
        cout << "目标已存在\n"; 
        return; 
    }
    File f = i->second; 
    f.name = t;
    f.modifiedTime = time(nullptr);
    dir->files[t] = f;
    cout << "复制: " << s << " → " << t << "\n";
}

void flockFile(Directory* dir, const string& file, bool lk) {
    auto it = dir->files.find(file);
    if (it == dir->files.end()) { 
        cout << "文件不存在\n"; 
        return; 
    }
    it->second.isLocked = lk;
    cout << (lk ? "加锁: " : "解锁: ") << file << "\n";
}

void headFile(Directory* dir, const string& file, int n) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "文件不存在\n"; 
        return; 
    }
    string data(i->second.content.begin(), i->second.content.end());
    istringstream ss(data);
    string line; int c = 0;
    cout << "[前" << n << "行]\n";
    while (c < n && getline(ss, line)) { cout << line << "\n"; ++c; }
}

void tailFile(Directory* dir, const string& file, int n) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "文件不存在\n"; 
        return; 
    }
    string data(i->second.content.begin(), i->second.content.end());
    istringstream ss(data);
    vector<string> lines;
    string line;
    while (getline(ss, line)) lines.push_back(line);
    int start = max(0, (int)lines.size() - n);
    cout << "[后" << n << "行]\n";
    for (int i = start; i < lines.size(); ++i) cout << lines[i] << "\n";
}

void lseekFile(Directory* dir, const string& file, int off) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "文件不存在\n"; 
        return; 
    }
    int np = i->second.readPtr + off;
    if (np<0 || np>i->second.size) { cout << "越界\n"; return; }
    i->second.readPtr = np;
    cout << "指针: " << np << "\n";
}

void listDirectory(Directory* dir) {
    cout << "[目录] " << dir->name << "\n";
    if (dir->subDirs.empty() && dir->files.empty()) {
        cout << "<空目录>\n";
        return;
    }
    // 列出子目录
    for (auto& kv : dir->subDirs) {
        cout << "<DIR> " << kv.first << "\n";
    }
    // 列出文件及其大小和修改时间
    for (auto& kv : dir->files) {
        tm tmBuf;
        localtime_s(&tmBuf, &kv.second.modifiedTime);
        cout << "      " << kv.first
            << " (" << kv.second.size << "B)"
            << "  " << put_time(&tmBuf, "%F %T") << "\n";
    }
}

static void printDirectoryTree(Directory* d, const string& prefix) {
    // 先打印子目录
    for (auto it = d->subDirs.begin(); it != d->subDirs.end(); ++it) {
        bool lastDir = (next(it) == d->subDirs.end()) && d->files.empty();
        cout << prefix
            << (lastDir ? "└── " : "├── ")
            << it->first
            << "/\n";
        printDirectoryTree(
            it->second,
            prefix + (lastDir ? "    " : "│   ")
        );
    }
    // 再打印文件
    for (auto it = d->files.begin(); it != d->files.end(); ++it) {
        bool lastFile = next(it) == d->files.end();
        cout << prefix
            << (lastFile ? "└── " : "├── ")
            << it->first
            << " (" << it->second.size << "B)";

        // 用 localtime_s 安全地格式化修改时间
        tm tmBuf;
        localtime_s(&tmBuf, &it->second.modifiedTime);
        cout << "  " << put_time(&tmBuf, "%F %T") << "\n";
    }
}

// 顶层调用，打印整个虚拟磁盘的目录树
void showTree(const VirtualDisk& disk) {
    cout << "/ (虚拟根目录)\n";
    auto it = disk.userFileSystems.begin();
    while (it != disk.userFileSystems.end()) {
        bool lastUser = next(it) == disk.userFileSystems.end();
        cout << (lastUser ? "└── " : "├── ")
            << it->first
            << "/\n";
        printDirectoryTree(
            it->second.root,
            lastUser ? "    " : "│   "
        );
        ++it;
    }
}