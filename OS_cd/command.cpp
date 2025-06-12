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
        cout << "Ŀ¼�Ѵ���\n"; return;
    }
    Directory* nd = new Directory;
    nd->name = name; nd->parent = dir;
    dir->subDirs[name] = nd;
    cout << "Ŀ¼����: " << name << "\n";
}

void rmdir(Directory* dir, const string& name) {
    auto i = dir->subDirs.find(name);
    if (i== dir->subDirs.end()) {
        cout << "δ�ҵ�Ŀ¼\n"; return;
    }
    Directory* t = i->second;
    if (!t->files.empty() || !t->subDirs.empty()) {
        cout << "Ŀ¼�ǿ�\n"; return;
    }
    delete t;
    dir->subDirs.erase(i);
    cout << "Ŀ¼ɾ��: " << name << "\n";
}

void create(Directory* dir, const string& filename) {
    if (dir->files.count(filename)) {
        cout << "�ļ��Ѵ��ڣ�" << filename << "\n";
        return;
    }
    File f;
    f.name = filename;
    f.modifiedTime = time(nullptr);   // ��¼����ʱ��
    dir->files[filename] = f;
    cout << "�ļ������ɹ���" << filename << "\n";
}

void deleteFile(Directory* dir, const string& file) {
    if (!dir->files.erase(file)) {
        cout << "δ�ҵ��ļ�\n"; return;
    }
    cout << "�ļ�ɾ��: " << file << "\n";
}

void openFile(Directory* dir, const string& file) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "δ�ҵ��ļ�\n"; 
        return; 
    }
    if (i->second.isLocked == false) {
        i->second.isOpen = true;
        cout << "�ļ��Ѵ�: " << file << "\n";
    }
    else if (i->second.isLocked == true) {
        cout << "�ļ����������޷���: " << file << "\n";
        return;
    }
}

void closeFile(Directory* dir, const string& file) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "δ�ҵ��ļ�\n"; return; 
    }
    i->second.isOpen = false;
    cout << "�ļ��ѹر�: " << file << "\n";
}

void readFile(Directory* dir, const string& file) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "δ�ҵ��ļ�\n"; 
        return; 
    }
    if (!i->second.isOpen) { 
        cout << "�ļ�δ��\n"; 
        return; 
    }
    if (i->second.isLocked == true) {
        cout << "�ļ����������޷���ȡ: " << file << "\n";
        return;
    }
    string data(i->second.content.begin(), i->second.content.end());
    cout << "[����] " << file << ":\n" << data << "\n";
}

void writeFile(Directory* dir, const string& file, const string& d) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "δ�ҵ��ļ�\n"; 
        return; 
    }
    File& f = i->second;
    if (!f.isOpen) { 
        cout << "�ļ�δ��: " << file << "\n";
        return; 
    }
    if (f.isLocked == true) {
        cout << "�ļ����������޷�д��: " << file << "\n";
        return;
    }
    cout << "[��������] ����д�����ݣ�����һ�С�.��������\n";
    string line, data;
    while (true) {
        // ��ȡһ�У������� EOF�����˳�����ѭ��
        if (!getline(cin, line)) break; 
        // ����û�����"."����ʾ������������
        if (line == ".") break;  
        // �����м��� data������ĩβ��ӻ��з�
        data += line;
        data += '\n'; 
    }
    // ���ļ��ĵ�ǰ��дָ��λ�ÿ�ʼд
    size_t pos = f.readPtr;
    if (pos > f.content.size()) {
        pos = f.content.size();
    }
    //����������
    f.content.insert(f.content.begin() + pos, data.begin(), data.end());
    // ����ָ�뵽д��ĩβ
    f.readPtr = pos + data.size();

    // ͬ�� size �� modifiedTime
    f.size = static_cast<int>(f.content.size());
    f.modifiedTime = time(nullptr);

    cout << "д�����: " << file << "\n";
}

void moveFile(Directory* dir, const string& src, const string& dest) {
    //�ڵ�ǰĿ¼����Դ�ļ�
    auto i = dir->files.find(src);
    if (i == dir->files.end()) {
        cout << "�Ҳ���Դ�ļ���" << src << "\n";
        return;
    }
    if (i->second.isLocked == true) {
        cout << "�ļ����������޷��ƶ�: " << src << "\n";
        return;
    }

    //���Ŀ��·��
    vector<string> parts;
    size_t start = 0;
    while (start <= dest.size()) {
        // ���Ҵ� start ��ʼ����һ�� '/'
        size_t pos = dest.find('/', start);
        if (pos == string::npos) {
            // û���ҵ� '/'����ʣ�ಿ����Ϊ���һ��
            parts.push_back(dest.substr(start));
            break;
        }
        // ��ȡ [start, pos) ֮����Ӵ�
        parts.push_back(dest.substr(start, pos - start));
        // �����ָ�����������һ��
        start = pos + 1;
        // ��� '/' �����һ���ַ������� parts ��һ���մ�����ʾ����Ŀ¼����������
        if (start == dest.size()) {
            parts.push_back("");
            break;
        }
    }

    //�ӵ�ǰĿ¼��ʼ��������Ŀ¼�����򴴽�
    Directory* targetDir = dir;
    int levels = (int)parts.size();
    // ������һ���ǿմ���˵�� dest �� '/' ��β��Ӧ����ԭ�ļ���
    bool toDirOnly = (!parts.empty() && parts.back().empty());
    // ʵ��Ҫ�����ļ������� toDirOnly�������һ�β���Ŀ¼����
    int limit = toDirOnly ? levels - 1 : levels - 1;
    for (int i = 0; i < limit; ++i) {
        const string& part = parts[i];
        if (part.empty() || part == ".") {
            // ���� "./"
            continue;
        }
        if (part == "..") {
            // ���ظ�Ŀ¼�������ڣ�
            if (targetDir->parent) {
                targetDir = targetDir->parent;
            }
            // �������ڸ�Ŀ¼
        }
        else {
            // ����򴴽���Ŀ¼
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

    //ȷ�� newName
    string newName;
    if (toDirOnly) {
        newName = src; // ����ԭ��
    }
    else {
        newName = parts.back();
        if (newName.empty()) {
            cout << "[����] Ŀ���ļ���Ϊ��\n";
            return;
        }
    }

    //��ͻ���
    if (targetDir->files.count(newName)) {
        cout << "Ŀ���ļ��Ѵ��ڣ�" << newName << "\n";
        return;
    }

    //ִ���ƶ��������ļ����󣬸������ƺ��޸�ʱ��
    File f = i->second;
    f.name = newName;
    f.modifiedTime = time(nullptr);

    //��ԭĿ¼�Ƴ�������Ŀ��Ŀ¼
    dir->files.erase(i);
    targetDir->files[newName] = move(f);

    cout << "�ļ��ƶ��ɹ���" << src << " -> " << dest << "\n";
}

void copyFile(Directory* dir, const string& src, const string& dest) {
    //�ڵ�ǰĿ¼����Դ�ļ�
    auto i = dir->files.find(src);
    if (i == dir->files.end()) {
        cout << "�Ҳ���Դ�ļ���" << src << "\n";
        return;
    }

    //���Ŀ��·��
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
        // ��� dest �� '/' ��β����һ�λ� push_back("")
        if (start == dest.size()) {
            parts.push_back("");
            break;
        }
    }

    //�ӵ�ǰĿ¼��ʼ��������Ŀ¼�����򴴽�
    Directory* targetDir = dir;
    // ������һ���ǿմ���˵�� dest �� '/' ��β��Ӧ����ԭ�ļ���
    bool toDirOnly = (!parts.empty() && parts.back().empty());
    int limit = (int)parts.size() - 1;
    for (int i = 0; i < limit; ++i) {
        const string& part = parts[i];
        if (part.empty() || part == ".") {
            // ���� "./"
            continue;
        }
        if (part == "..") {
            // ���ظ�Ŀ¼�������ڣ�
            if (targetDir->parent) {
                targetDir = targetDir->parent;
            }
            // �������ڸ�Ŀ¼
        }
        else {
            // ����򴴽���Ŀ¼
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


    //�����ļ����󣬸������ƺ��޸�ʱ��
    File f = i->second;
    f.modifiedTime = time(nullptr);

    //����Ŀ��Ŀ¼
    targetDir->files[src] = move(f);

    cout << "�ļ����Ƴɹ���" << src << " -> " << dest << "\n";
}

void flockFile(Directory* dir, const string& file, bool judge) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "�ļ�������\n"; 
        return; 
    }
    i->second.isLocked = judge;
    cout << (judge ? "����: " : "����: ") << file << "\n";
}

void headFile(Directory* dir, const string& file, int n) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "�ļ�������\n"; 
        return; 
    }
    string data(i->second.content.begin(), i->second.content.end());
    istringstream ss(data);
    string line; int c = 0;
    cout << "[ǰ" << n << "��]\n";
    while (c < n && getline(ss, line)) { 
        cout << line << "\n"; ++c; }

}

void tailFile(Directory* dir, const string& file, int n) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "�ļ�������\n"; 
        return; 
    }
    // ���ļ����ݣ������� vector<char>��ת��Ϊ string
    string data(i->second.content.begin(), i->second.content.end());
    istringstream ss(data);
    vector<string> lines;
    string line;
    // ��ÿһ�ж��� lines ����
    while (getline(ss, line)) {
        lines.push_back(line);
    }
    // �������һ�п�ʼ�������������ȥ n����СΪ 0
    int start = max(0, (int)lines.size() - n);
    cout << "[��" << n << "��]\n";
    for (int i = start; i < lines.size(); ++i) {
        cout << lines[i] << "\n";
    }
}

void lseekFile(Directory* dir, const string& file, int off) {
    auto i = dir->files.find(file);
    if (i == dir->files.end()) { 
        cout << "�ļ�������\n"; 
        return; 
    }
    File& f = i->second;
    // ������ָ��λ�� = ��ǰ readPtr + off
    int newPos = static_cast<int>(f.readPtr) + off;
    // �߽��飺������ 0
    if (newPos < 0) newPos = 0;
    // �������ļ���С
    if (newPos > static_cast<int>(f.content.size())) {
        newPos = static_cast<int>(f.content.size());
    }
    f.readPtr = static_cast<size_t>(newPos);
    cout << "�µ�ָ��λ�ã�" << f.readPtr << "\n";
}

void listDirectory(Directory* dir) {
    cout << "[Ŀ¼] " << dir->name << "\n";
    if (dir->subDirs.empty() && dir->files.empty()) {
        cout << "<��Ŀ¼>\n";
        return;
    }
    // �г���Ŀ¼
    for (auto& kv : dir->subDirs) {
        cout << "<DIR> " << kv.first << "\n";
    }
    // �г��ļ������С���޸�ʱ��
    for (auto& kv : dir->files) {
        tm tmBuf;
        // �� time_t ת�ɱ���ʱ��ṹ tm
        localtime_s(&tmBuf, &kv.second.modifiedTime);
        cout << "      " << kv.first
            << " (" << kv.second.size << "B)"
            << "  " << put_time(&tmBuf, "%F %T") << "\n";
    }
}

static void printDirectoryTree(Directory* d, const string& prefix) {
    // �ȴ�ӡ��Ŀ¼
    for (auto it = d->subDirs.begin(); it != d->subDirs.end(); ++it) {
        bool lastDir = (next(it) == d->subDirs.end()) && d->files.empty();
        cout << prefix
            << (lastDir ? "������ " : "������ ")
            << it->first
            << "/\n";
        printDirectoryTree(
            it->second,
            prefix + (lastDir ? "    " : "��   ")
        );
    }
    // �ٴ�ӡ�ļ�
    for (auto it = d->files.begin(); it != d->files.end(); ++it) {
        bool lastFile = next(it) == d->files.end();
        cout << prefix
            << (lastFile ? "������ " : "������ ")
            << it->first
            << " (" << it->second.size << "B)";

        // �� localtime_s ��ȫ�ظ�ʽ���޸�ʱ��
        tm tmBuf;
        localtime_s(&tmBuf, &it->second.modifiedTime);
        cout << "  " << put_time(&tmBuf, "%F %T") << "\n";
    }
}

// ������ã���ӡ����������̵�Ŀ¼��
void showTree(const VirtualDisk& disk) {
    cout << "/ (�����Ŀ¼)\n";
    auto it = disk.userFileSystems.begin();
    // ����ÿ���û����ļ�ϵͳ
    while (it != disk.userFileSystems.end()) {
        bool lastUser = next(it) == disk.userFileSystems.end();
        cout << (lastUser ? "������ " : "������ ")
            << it->first
            << "/\n";
        printDirectoryTree(
            it->second.root,
            lastUser ? "    " : "��   "
        );
        ++it;
    }
}