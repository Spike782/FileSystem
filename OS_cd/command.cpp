#include "command.h"
#include "fs.h"
#include <iostream>
#include <algorithm>
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
        cout << "未找到文件\n"; 
        return; 
    }
    if (i->second.isLocked == false) {
        i->second.isOpen = true;
        cout << "文件已打开: " << file << "\n";
    }
    else if (i->second.isLocked == true) {
        cout << "文件已锁定，无法打开: " << file << "\n";
        return;
    }
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
    if (i->second.isLocked == true) {
        cout << "文件已锁定，无法读取: " << file << "\n";
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
    File& f = i->second;
    if (!f.isOpen) { 
        cout << "文件未打开: " << file << "\n";
        return; 
    }
    if (f.isLocked == true) {
        cout << "文件已锁定，无法写入: " << file << "\n";
        return;
    }
    cout << "[多行输入] 输入写入内容，单独一行“.”结束。\n";
    string line, data;
    while (true) {
        // 读取一行；若遇到 EOF，则退出输入循环
        if (!getline(cin, line)) break; 
        // 如果用户输入"."，表示结束多行输入
        if (line == ".") break;  
        // 将该行加入 data，并在末尾添加换行符
        data += line;
        data += '\n'; 
    }
    // 从文件的当前读写指针位置开始写
    size_t pos = f.readPtr;
    if (pos > f.content.size()) {
        pos = f.content.size();
    }
    //插入新内容
    f.content.insert(f.content.begin() + pos, data.begin(), data.end());
    // 更新指针到写入末尾
    f.readPtr = pos + data.size();

    // 同步 size 和 modifiedTime
    f.size = static_cast<int>(f.content.size());
    f.modifiedTime = time(nullptr);

    cout << "写入完成: " << file << "\n";
}

void moveFile(Directory* dir, const string& src, const string& dest) {
    //在当前目录查找源文件
    auto i = dir->files.find(src);
    if (i == dir->files.end()) {
        cout << "找不到源文件：" << src << "\n";
        return;
    }
    if (i->second.isLocked == true) {
        cout << "文件已锁定，无法移动: " << src << "\n";
        return;
    }

    //拆分目标路径
    vector<string> parts;
    size_t start = 0;
    while (start <= dest.size()) {
        // 查找从 start 开始的下一个 '/'
        size_t pos = dest.find('/', start);
        if (pos == string::npos) {
            // 没有找到 '/'，把剩余部分作为最后一段
            parts.push_back(dest.substr(start));
            break;
        }
        // 截取 [start, pos) 之间的子串
        parts.push_back(dest.substr(start, pos - start));
        // 跳过分隔符，继续下一轮
        start = pos + 1;
        // 如果 '/' 是最后一个字符，就在 parts 加一个空串，表示“仅目录不重命名”
        if (start == dest.size()) {
            parts.push_back("");
            break;
        }
    }

    //从当前目录开始，按各级目录遍历或创建
    Directory* targetDir = dir;
    int levels = (int)parts.size();
    // 如果最后一项是空串，说明 dest 以 '/' 结尾，应保留原文件名
    bool toDirOnly = (!parts.empty() && parts.back().empty());
    // 实际要遍历的级数：若 toDirOnly，则最后一段不当目录处理
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

    //确定 newName
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

    //冲突检查
    if (targetDir->files.count(newName)) {
        cout << "目标文件已存在：" << newName << "\n";
        return;
    }

    //执行移动：复制文件对象，更新名称和修改时间
    File f = i->second;
    f.name = newName;
    f.modifiedTime = time(nullptr);

    //从原目录移除，插入目标目录
    dir->files.erase(i);
    targetDir->files[newName] = move(f);

    cout << "文件移动成功：" << src << " -> " << dest << "\n";
}

void copyFile(Directory* dir, const string& src, const string& dest) {
    //在当前目录查找源文件
    auto i = dir->files.find(src);
    if (i == dir->files.end()) {
        cout << "找不到源文件：" << src << "\n";
        return;
    }

    //拆分目标路径
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

    //从当前目录开始，按各级目录遍历或创建
    Directory* targetDir = dir;
    // 如果最后一项是空串，说明 dest 以 '/' 结尾，应保留原文件名
    bool toDirOnly = (!parts.empty() && parts.back().empty());
    int limit = (int)parts.size() - 1;
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


    //复制文件对象，更新名称和修改时间
    File f = i->second;
    f.modifiedTime = time(nullptr);

    //插入目标目录
    targetDir->files[src] = move(f);

    cout << "文件复制成功：" << src << " -> " << dest << "\n";
}

void flockFile(Directory* dir, const string& file, bool judge) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "文件不存在\n"; 
        return; 
    }
    i->second.isLocked = judge;
    cout << (judge ? "加锁: " : "解锁: ") << file << "\n";
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
    while (c < n && getline(ss, line)) { 
        cout << line << "\n"; ++c; }

}

void tailFile(Directory* dir, const string& file, int n) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "文件不存在\n"; 
        return; 
    }
    // 将文件内容（二进制 vector<char>）转换为 string
    string data(i->second.content.begin(), i->second.content.end());
    istringstream ss(data);
    vector<string> lines;
    string line;
    // 把每一行读入 lines 向量
    while (getline(ss, line)) {
        lines.push_back(line);
    }
    // 计算从哪一行开始输出：总行数减去 n，最小为 0
    int start = max(0, (int)lines.size() - n);
    cout << "[后" << n << "行]\n";
    for (int i = start; i < lines.size(); ++i) {
        cout << lines[i] << "\n";
    }
}

void lseekFile(Directory* dir, const string& file, int off) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "文件不存在\n"; 
        return; 
    }
    File& f = i->second;
    // 计算新指针位置 = 当前 readPtr + off
    int newPos = static_cast<int>(f.readPtr) + off;
    // 边界检查：不低于 0
    if (newPos < 0) newPos = 0;
    // 不超过文件大小
    if (newPos > static_cast<int>(f.content.size())) {
        newPos = static_cast<int>(f.content.size());
    }
    f.readPtr = static_cast<size_t>(newPos);
    cout << "新的指针位置：" << f.readPtr << "\n";
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
        // 将 time_t 转成本地时间结构 tm
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
    // 遍历每个用户的文件系统
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