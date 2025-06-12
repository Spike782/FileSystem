#include "fs.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

// 把一条字符串写入二进制流：先写长度（4 字节），再写内容
static void writeString(ostream& out, const string& s) {
    uint32_t len = static_cast<uint32_t>(s.size());
    //写入字符串长度
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    //根据长度写入字符串
    out.write(s.data(), len);
}

static string readString(istream& in) {
    uint32_t len;
    //读取字符串长度
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    // 分配足够空间，\0 初始化
    string s(len, '\0');
    //根据字符串长度读取字符串
    in.read(&s[0], len);
    return s;
}

// 递归地将一个目录及其所有文件/子目录写入二进制流
static void saveDirectory(ostream& out, Directory* dir) {
    // 写入当前目录名
    writeString(out, dir->name);
    
    // 写入该目录下文件数量（4 字节）
    uint32_t fc = static_cast<uint32_t>(dir->files.size());
    out.write(reinterpret_cast<const char*>(&fc), sizeof(fc));
    // 遍历写入每个文件的元数据和内容
    for (auto& kv : dir->files) {
        // 文件名
        writeString(out, kv.first);
        File& f = kv.second;
        
        // 文件内容长度 + 内容
        uint32_t cl = static_cast<uint32_t>(f.content.size());
        out.write(reinterpret_cast<const char*>(&cl), sizeof(cl));
        out.write(reinterpret_cast<const char*>(f.content.data()), cl);
        // 是否打开标志（1 字节）
        uint8_t o = f.isOpen ? 1 : 0;
        // 是否锁定标志（1 字节）
        uint8_t l = f.isLocked ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&o), sizeof(o));
        out.write(reinterpret_cast<const char*>(&l), sizeof(l));
        // 文件大小（4 字节）和读写指针位置（4 字节）
        uint32_t sz = static_cast<uint32_t>(f.size);
        uint32_t rp = static_cast<uint32_t>(f.readPtr);
        out.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
        out.write(reinterpret_cast<const char*>(&rp), sizeof(rp));
        // 最后修改时间（time_t，8 字节）
        out.write(reinterpret_cast<const char*>(&f.modifiedTime),sizeof(f.modifiedTime));
    }
    uint32_t dc = static_cast<uint32_t>(dir->subDirs.size());
    out.write(reinterpret_cast<const char*>(&dc), sizeof(dc));
    //递归保存子目录
    for (auto& kv : dir->subDirs) {
        saveDirectory(out, kv.second);
    }
}

// 从二进制流中递归地加载一个目录及其所有文件/子目录
static Directory* loadDirectory(ifstream& in, Directory* parent) {
    Directory* dir = new Directory;
    dir->parent = parent;
    // 读取目录名
    dir->name = readString(in);
    // 读取文件数量
    uint32_t fc;
    in.read(reinterpret_cast<char*>(&fc), sizeof(fc));
    for (uint32_t i = 0; i < fc; ++i) {
        // 读取文件名
        string fname = readString(in);
        File f;

        // 读取文件内容长度并填充 content
        uint32_t cl;
        in.read(reinterpret_cast<char*>(&cl), sizeof(cl));
        f.content.resize(cl);
        in.read(reinterpret_cast<char*>(f.content.data()), cl);
        // 读取是否打开、是否锁定
        uint8_t o, l;
        in.read(reinterpret_cast<char*>(&o), sizeof(o));
        in.read(reinterpret_cast<char*>(&l), sizeof(l));
        f.isOpen = o;
        f.isLocked = l;
        // 读取 size 和 readPtr
        uint32_t sz, rp;
        in.read(reinterpret_cast<char*>(&sz), sizeof(sz));
        in.read(reinterpret_cast<char*>(&rp), sizeof(rp));
        f.size = sz;
        f.readPtr = rp;
        // 读取修改时间
        time_t mt;
        in.read(reinterpret_cast<char*>(&mt), sizeof(mt));
        f.modifiedTime = mt;
        // 存入当前目录的 files map
        dir->files[fname] = move(f);
    }
    // 读取子目录数量并递归加载每个子目录
    uint32_t dc;
    in.read(reinterpret_cast<char*>(&dc), sizeof(dc));
    for (uint32_t i = 0; i < dc; ++i) {
        Directory* sub = loadDirectory(in, dir);
        dir->subDirs[sub->name] = sub;
    }
    return dir;
}

// 将整个 VirtualDisk 序列化并写入指定文件
void savetoDisk(const VirtualDisk& disk, const string& filename) {
    // 打开文件，清空旧内容
    ofstream out(filename, ios::binary | ios::trunc);
    if (!out) {
        cerr << "[ERROR] 无法创建或写入文件: " << filename << "\n";
        return;
    }

    //写入用户数（32 位）
    uint32_t userCount = static_cast<uint32_t>(disk.users.size());
    out.write(reinterpret_cast<const char*>(&userCount), sizeof(userCount));

    // 对每个用户：写用户名、密码、登录状态，然后写其根目录
    for (auto& kv : disk.users) {
        writeString(out, kv.first);// 用户名
        writeString(out, kv.second.password);// 密码

        uint32_t attempts = static_cast<uint32_t>(kv.second.loginAttempts);
        uint8_t  locked = kv.second.isLocked ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&attempts), sizeof(attempts));
        out.write(reinterpret_cast<const char*>(&locked), sizeof(locked));
        
        // 保存该用户的目录树
        saveDirectory(out, disk.userFileSystems.at(kv.first).root);
    }
    out.close();
}

// 从文件加载 VirtualDisk 到内存
bool loadFromDisk(VirtualDisk& disk, const string& filename) {
    ifstream in(filename, ios::binary);
    if (!in) return false;

    //读入用户数（32 位）
    uint32_t userCount;
    in.read(reinterpret_cast<char*>(&userCount), sizeof(userCount));

    // 循环每个用户，重建用户信息和目录树
    for (uint32_t i = 0; i < userCount; ++i) {
        string username = readString(in);
        string password = readString(in);

        uint32_t attempts32;// 登录尝试次数
        in.read(reinterpret_cast<char*>(&attempts32), sizeof(attempts32));
        int      loginAttempts = static_cast<int>(attempts32);

        uint8_t lockedFlag; // 锁定状态
        in.read(reinterpret_cast<char*>(&lockedFlag), sizeof(lockedFlag));
        bool    isLocked = lockedFlag != 0;

        disk.users[username] = { password, isLocked, loginAttempts };

        // 递归加载该用户的根目录
        FileSystem fs;
        fs.root = loadDirectory(in, nullptr);
        disk.userFileSystems[username] = fs;
    }

    return true;
}

// 从宿主文件系统导入一个真实文件到当前目录
bool importFile(Directory* dir, const string& src, const string& dest) {
    // 计算导入后的文件名（若 dest 为空则取 src 的 basename）
    string fn;
    if (dest.empty()) {
        // 找到最后一个 '/' 或 '\\'
        auto pos = src.find_last_of("/\\");
        if (pos == string::npos) {
            fn = src;               // 没有路径分隔符，整个 src 就是文件名
        }
        else {
            fn = src.substr(pos + 1);
        }
    }
    else {
        fn = dest;
    }
    ifstream in(src, ios::binary);
    if (!in) return false;

    // 读取整个文件内容到 f.content
    File f;
    f.content.assign(istreambuf_iterator<char>(in), {});
    f.size = static_cast<int>(f.content.size());
    f.isOpen = false;
    f.modifiedTime = time(nullptr);
    f.isLocked = false;
    f.readPtr = 0;
    
    dir->files[fn] = move(f);
    return true;
}

// 将虚拟文件系统中的文件导出到宿主文件系统
bool exportFile(Directory* dir, const string& name, const string& dst) {
    auto i = dir->files.find(name);
    if (i == dir->files.end()) {
        return false;
    }
    // 如果 dst 以斜杠结尾，补上原文件名
    string outPath = dst;
    if (!dst.empty() && (dst.back() == '/' || dst.back() == '\\'))
        outPath += name;
    ofstream out(outPath, ios::binary);
    if (!out) return false;
    out.write(i->second.content.data(), i->second.content.size());
    return true;
}
