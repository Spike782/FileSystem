// fs.cpp
#include "fs.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

static void writeString(ostream& out, const string& s) {
    uint32_t len = static_cast<uint32_t>(s.size());
    //先写入存的字符串长度
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    //根据长度写入字符串
    out.write(s.data(), len);
}

static string readString(istream& in) {
    uint32_t len;
    //读取字符串长度
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    string s(len, '\0');
    //根据字符串长度读取字符串
    in.read(&s[0], len);
    return s;
}

static void saveDirectory(ostream& out, Directory* dir) {
    writeString(out, dir->name);
    //先写入文件夹包含的文件的大小
    uint32_t fc = static_cast<uint32_t>(dir->files.size());
    out.write(reinterpret_cast<const char*>(&fc), sizeof(fc));
    //写入文件内容
    for (auto& kv : dir->files) {
        writeString(out, kv.first);
        auto& f = kv.second;
        uint32_t cl = static_cast<uint32_t>(f.content.size());
        out.write(reinterpret_cast<const char*>(&cl), sizeof(cl));
        out.write(reinterpret_cast<const char*>(f.content.data()), cl);
        uint8_t o = f.isOpen ? 1 : 0;
        uint8_t l = f.isLocked ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&o), sizeof(o));
        out.write(reinterpret_cast<const char*>(&l), sizeof(l));
        uint32_t sz = static_cast<uint32_t>(f.size);
        uint32_t rp = static_cast<uint32_t>(f.readPtr);
        out.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
        out.write(reinterpret_cast<const char*>(&rp), sizeof(rp));
        out.write(reinterpret_cast<const char*>(&f.modifiedTime),sizeof(f.modifiedTime));
    }
    uint32_t dc = static_cast<uint32_t>(dir->subDirs.size());
    out.write(reinterpret_cast<const char*>(&dc), sizeof(dc));
    //递归保存子目录
    for (auto& kv : dir->subDirs) {
        saveDirectory(out, kv.second);
    }
}

static Directory* loadDirectory(ifstream& in, Directory* parent) {
    Directory* dir = new Directory;
    dir->parent = parent;
    dir->name = readString(in);
    uint32_t fc;
    in.read(reinterpret_cast<char*>(&fc), sizeof(fc));
    for (uint32_t i = 0; i < fc; ++i) {
        string fname = readString(in);
        File f;
        uint32_t cl;
        in.read(reinterpret_cast<char*>(&cl), sizeof(cl));
        f.content.resize(cl);
        in.read(reinterpret_cast<char*>(f.content.data()), cl);
        uint8_t o, l;
        in.read(reinterpret_cast<char*>(&o), sizeof(o));
        in.read(reinterpret_cast<char*>(&l), sizeof(l));
        f.isOpen = o;
        f.isLocked = l;
        uint32_t sz, rp;
        in.read(reinterpret_cast<char*>(&sz), sizeof(sz));
        in.read(reinterpret_cast<char*>(&rp), sizeof(rp));
        f.size = sz;
        f.readPtr = rp;
        time_t mt;
        in.read(reinterpret_cast<char*>(&mt), sizeof(mt));
        f.modifiedTime = mt;
        dir->files[fname] = std::move(f);
    }
    uint32_t dc;
    in.read(reinterpret_cast<char*>(&dc), sizeof(dc));
    for (uint32_t i = 0; i < dc; ++i) {
        Directory* sub = loadDirectory(in, dir);
        dir->subDirs[sub->name] = sub;
    }
    return dir;
}

void savetoDisk(const VirtualDisk& disk, const string& filename) {
    ofstream out(filename, ios::binary | ios::trunc);
    if (!out) {
        cerr << "[ERROR] 无法创建或写入文件: " << filename << "\n";
        return;
    }

    //写入用户数（32 位）
    uint32_t userCount = static_cast<uint32_t>(disk.users.size());
    out.write(reinterpret_cast<const char*>(&userCount), sizeof(userCount));

    //写入每个用户
    for (auto& kv : disk.users) {
        writeString(out, kv.first);
        writeString(out, kv.second.password);

        uint32_t attempts = static_cast<uint32_t>(kv.second.loginAttempts);
        uint8_t  locked = kv.second.isLocked ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&attempts), sizeof(attempts));
        out.write(reinterpret_cast<const char*>(&locked), sizeof(locked));

        saveDirectory(out, disk.userFileSystems.at(kv.first).root);
    }
    out.close();
}

bool loadFromDisk(VirtualDisk& disk, const string& filename) {
    ifstream in(filename, ios::binary);
    if (!in) return false;

    //读入用户数（32 位）
    uint32_t userCount;
    in.read(reinterpret_cast<char*>(&userCount), sizeof(userCount));

    //读每个用户
    for (uint32_t i = 0; i < userCount; ++i) {
        string username = readString(in);
        string password = readString(in);

        uint32_t attempts32;
        in.read(reinterpret_cast<char*>(&attempts32), sizeof(attempts32));
        int      loginAttempts = static_cast<int>(attempts32);

        uint8_t lockedFlag;
        in.read(reinterpret_cast<char*>(&lockedFlag), sizeof(lockedFlag));
        bool    isLocked = lockedFlag != 0;

        disk.users[username] = { password, isLocked, loginAttempts };

        FileSystem fs;
        fs.root = loadDirectory(in, nullptr);
        disk.userFileSystems[username] = fs;
    }

    return true;
}

bool importFile(Directory* dir, const string& src, const string& dest) {
    string fn = dest.empty()
        ? src.substr(src.find_last_of("/\\") + 1)
        : dest;
    ifstream in(src, ios::binary);
    if (!in) return false;
    File f;
    f.content.assign(istreambuf_iterator<char>(in), {});
    f.size = static_cast<int>(f.content.size());
    f.isOpen = false;
    f.isLocked = false;
    f.readPtr = 0;
    dir->files[fn] = move(f);
    return true;
}

bool exportFile(Directory* dir, const string& name, const string& dst) {
    auto i = dir->files.find(name);
    if (i == dir->files.end()) {
        return false;
    }
    string outPath = dst;
    if (!dst.empty() && (dst.back() == '/' || dst.back() == '\\'))
        outPath += name;
    ofstream out(outPath, ios::binary);
    if (!out) return false;
    out.write(i->second.content.data(), i->second.content.size());
    return true;
}
