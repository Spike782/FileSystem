#include "fs.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

// ��һ���ַ���д�������������д���ȣ�4 �ֽڣ�����д����
static void writeString(ostream& out, const string& s) {
    uint32_t len = static_cast<uint32_t>(s.size());
    //д���ַ�������
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    //���ݳ���д���ַ���
    out.write(s.data(), len);
}

static string readString(istream& in) {
    uint32_t len;
    //��ȡ�ַ�������
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    // �����㹻�ռ䣬\0 ��ʼ��
    string s(len, '\0');
    //�����ַ������ȶ�ȡ�ַ���
    in.read(&s[0], len);
    return s;
}

// �ݹ�ؽ�һ��Ŀ¼���������ļ�/��Ŀ¼д���������
static void saveDirectory(ostream& out, Directory* dir) {
    // д�뵱ǰĿ¼��
    writeString(out, dir->name);
    
    // д���Ŀ¼���ļ�������4 �ֽڣ�
    uint32_t fc = static_cast<uint32_t>(dir->files.size());
    out.write(reinterpret_cast<const char*>(&fc), sizeof(fc));
    // ����д��ÿ���ļ���Ԫ���ݺ�����
    for (auto& kv : dir->files) {
        // �ļ���
        writeString(out, kv.first);
        File& f = kv.second;
        
        // �ļ����ݳ��� + ����
        uint32_t cl = static_cast<uint32_t>(f.content.size());
        out.write(reinterpret_cast<const char*>(&cl), sizeof(cl));
        out.write(reinterpret_cast<const char*>(f.content.data()), cl);
        // �Ƿ�򿪱�־��1 �ֽڣ�
        uint8_t o = f.isOpen ? 1 : 0;
        // �Ƿ�������־��1 �ֽڣ�
        uint8_t l = f.isLocked ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&o), sizeof(o));
        out.write(reinterpret_cast<const char*>(&l), sizeof(l));
        // �ļ���С��4 �ֽڣ��Ͷ�дָ��λ�ã�4 �ֽڣ�
        uint32_t sz = static_cast<uint32_t>(f.size);
        uint32_t rp = static_cast<uint32_t>(f.readPtr);
        out.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
        out.write(reinterpret_cast<const char*>(&rp), sizeof(rp));
        // ����޸�ʱ�䣨time_t��8 �ֽڣ�
        out.write(reinterpret_cast<const char*>(&f.modifiedTime),sizeof(f.modifiedTime));
    }
    uint32_t dc = static_cast<uint32_t>(dir->subDirs.size());
    out.write(reinterpret_cast<const char*>(&dc), sizeof(dc));
    //�ݹ鱣����Ŀ¼
    for (auto& kv : dir->subDirs) {
        saveDirectory(out, kv.second);
    }
}

// �Ӷ��������еݹ�ؼ���һ��Ŀ¼���������ļ�/��Ŀ¼
static Directory* loadDirectory(ifstream& in, Directory* parent) {
    Directory* dir = new Directory;
    dir->parent = parent;
    // ��ȡĿ¼��
    dir->name = readString(in);
    // ��ȡ�ļ�����
    uint32_t fc;
    in.read(reinterpret_cast<char*>(&fc), sizeof(fc));
    for (uint32_t i = 0; i < fc; ++i) {
        // ��ȡ�ļ���
        string fname = readString(in);
        File f;

        // ��ȡ�ļ����ݳ��Ȳ���� content
        uint32_t cl;
        in.read(reinterpret_cast<char*>(&cl), sizeof(cl));
        f.content.resize(cl);
        in.read(reinterpret_cast<char*>(f.content.data()), cl);
        // ��ȡ�Ƿ�򿪡��Ƿ�����
        uint8_t o, l;
        in.read(reinterpret_cast<char*>(&o), sizeof(o));
        in.read(reinterpret_cast<char*>(&l), sizeof(l));
        f.isOpen = o;
        f.isLocked = l;
        // ��ȡ size �� readPtr
        uint32_t sz, rp;
        in.read(reinterpret_cast<char*>(&sz), sizeof(sz));
        in.read(reinterpret_cast<char*>(&rp), sizeof(rp));
        f.size = sz;
        f.readPtr = rp;
        // ��ȡ�޸�ʱ��
        time_t mt;
        in.read(reinterpret_cast<char*>(&mt), sizeof(mt));
        f.modifiedTime = mt;
        // ���뵱ǰĿ¼�� files map
        dir->files[fname] = move(f);
    }
    // ��ȡ��Ŀ¼�������ݹ����ÿ����Ŀ¼
    uint32_t dc;
    in.read(reinterpret_cast<char*>(&dc), sizeof(dc));
    for (uint32_t i = 0; i < dc; ++i) {
        Directory* sub = loadDirectory(in, dir);
        dir->subDirs[sub->name] = sub;
    }
    return dir;
}

// ������ VirtualDisk ���л���д��ָ���ļ�
void savetoDisk(const VirtualDisk& disk, const string& filename) {
    // ���ļ�����վ�����
    ofstream out(filename, ios::binary | ios::trunc);
    if (!out) {
        cerr << "[ERROR] �޷�������д���ļ�: " << filename << "\n";
        return;
    }

    //д���û�����32 λ��
    uint32_t userCount = static_cast<uint32_t>(disk.users.size());
    out.write(reinterpret_cast<const char*>(&userCount), sizeof(userCount));

    // ��ÿ���û���д�û��������롢��¼״̬��Ȼ��д���Ŀ¼
    for (auto& kv : disk.users) {
        writeString(out, kv.first);// �û���
        writeString(out, kv.second.password);// ����

        uint32_t attempts = static_cast<uint32_t>(kv.second.loginAttempts);
        uint8_t  locked = kv.second.isLocked ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&attempts), sizeof(attempts));
        out.write(reinterpret_cast<const char*>(&locked), sizeof(locked));
        
        // ������û���Ŀ¼��
        saveDirectory(out, disk.userFileSystems.at(kv.first).root);
    }
    out.close();
}

// ���ļ����� VirtualDisk ���ڴ�
bool loadFromDisk(VirtualDisk& disk, const string& filename) {
    ifstream in(filename, ios::binary);
    if (!in) return false;

    //�����û�����32 λ��
    uint32_t userCount;
    in.read(reinterpret_cast<char*>(&userCount), sizeof(userCount));

    // ѭ��ÿ���û����ؽ��û���Ϣ��Ŀ¼��
    for (uint32_t i = 0; i < userCount; ++i) {
        string username = readString(in);
        string password = readString(in);

        uint32_t attempts32;// ��¼���Դ���
        in.read(reinterpret_cast<char*>(&attempts32), sizeof(attempts32));
        int      loginAttempts = static_cast<int>(attempts32);

        uint8_t lockedFlag; // ����״̬
        in.read(reinterpret_cast<char*>(&lockedFlag), sizeof(lockedFlag));
        bool    isLocked = lockedFlag != 0;

        disk.users[username] = { password, isLocked, loginAttempts };

        // �ݹ���ظ��û��ĸ�Ŀ¼
        FileSystem fs;
        fs.root = loadDirectory(in, nullptr);
        disk.userFileSystems[username] = fs;
    }

    return true;
}

// �������ļ�ϵͳ����һ����ʵ�ļ�����ǰĿ¼
bool importFile(Directory* dir, const string& src, const string& dest) {
    // ���㵼�����ļ������� dest Ϊ����ȡ src �� basename��
    string fn;
    if (dest.empty()) {
        // �ҵ����һ�� '/' �� '\\'
        auto pos = src.find_last_of("/\\");
        if (pos == string::npos) {
            fn = src;               // û��·���ָ��������� src �����ļ���
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

    // ��ȡ�����ļ����ݵ� f.content
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

// �������ļ�ϵͳ�е��ļ������������ļ�ϵͳ
bool exportFile(Directory* dir, const string& name, const string& dst) {
    auto i = dir->files.find(name);
    if (i == dir->files.end()) {
        return false;
    }
    // ��� dst ��б�ܽ�β������ԭ�ļ���
    string outPath = dst;
    if (!dst.empty() && (dst.back() == '/' || dst.back() == '\\'))
        outPath += name;
    ofstream out(outPath, ios::binary);
    if (!out) return false;
    out.write(i->second.content.data(), i->second.content.size());
    return true;
}
