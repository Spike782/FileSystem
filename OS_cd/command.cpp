#include"command.h"
#include<iostream>
#include<sstream>
#include <vector>
using namespace std;

Directory* currentDir = nullptr;
Directory* parentDir = nullptr;  // ���ڸ�����һ��Ŀ¼

void mkdir(Directory* dir, const string& dirname) {
	if (dir->subDirs.find(dirname) != dir->subDirs.end()) {
		cout << "Ŀ¼�Ѵ��ڣ�" << endl;
		return;
	}
	Directory* newDir=new Directory;
	newDir->name = dirname;
    newDir->parent = dir;
	dir->subDirs[dirname] = newDir;
	cout << "Ŀ¼�����ɹ���" << dirname << endl;
}

void rmdir(Directory* dir, const string& dirname) {
	auto i = dir->subDirs.find(dirname);
	if (i == dir->subDirs.end()) {
		cout << "�Ҳ���Ŀ¼��" << dirname << endl;
		return;
	}
    Directory* target = i->second;
	if (!target->subDirs.empty() || !target->files.empty()) {
		cout << "Ŀ¼��Ϊ�գ�����ɾ����" << endl;
		return;
	}
    delete target;
	dir->subDirs.erase(i);
	cout << "Ŀ¼��ɾ����" << dirname << endl;
}

void create(Directory* dir, const string& filename) {
	if (dir->files.find(filename) != dir->files.end()) {
		cout << "�ļ��Ѵ��ڣ�" << filename << endl;
		return;
	}
	File newfile;
	newfile.name = filename;
	dir->files[filename] = newfile;
	cout << "�ļ������ɹ���" << filename << endl;
}

void deleteFile(Directory* dir, const string& filename) {
	auto i = dir->files.find(filename);
	if (i == dir->files.end()) {
		cout << "�Ҳ����ļ���" << filename << endl;
		return;
	}
	dir->files.erase(i);
	cout << "�ļ���ɾ����" << filename << endl;
}

void openFile(Directory* dir, const string& filename) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "�Ҳ����ļ���" << filename << endl;
        return;
    }
    if (i->second.isOpen) {
        cout << "�ļ��Ѵ򿪣�" << filename <<endl;
    }
    else {
        i->second.isOpen = true;
        cout << "�ļ��Ѵ򿪣�" << filename << endl;
    }
}

void closeFile(Directory* dir, const string& filename) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "�Ҳ����ļ���" << filename << endl;
        return;
    }
    if (!i->second.isOpen) {
       cout << "[��ʾ] �ļ���δ�򿪣�" << filename << endl;
    }
    else {
        i->second.isOpen = false;
        cout << "�ļ��ѹرգ�" << filename << endl;
    }
}

void readFile(Directory* dir, const string& filename) {
    auto it = dir->files.find(filename);
    if (it == dir->files.end()) {
        cout << "�Ҳ����ļ���" << filename << endl;
        return;
    }
    if (!it->second.isOpen) {
        cout << "�ļ�δ�򿪣��޷���ȡ��" << endl;
        return;
    }
    cout << "[����] " << filename << ":\n" << it->second.content << endl;
}

void writeFile(Directory* dir, const string& filename, const string& data) {
    auto it = dir->files.find(filename);
    if (it == dir->files.end()) {
        cout << " �Ҳ����ļ���" << filename << endl;
        return;
    }
    if (!it->second.isOpen) {
        cout << " �ļ�δ�򿪣��޷�д�롣" << endl;
        return;
    }
    it->second.content += data;
    it->second.size = it->second.content.size();
    it->second.modifiedTime = time(nullptr);
    cout << "д����ɣ�" << filename << endl;
}

void moveFile(Directory* dir, const string& src, const string& dest) {
    auto i = dir->files.find(src);
    if (i == dir->files.end()) {
        cout << "�Ҳ���Դ�ļ���" << src << endl;
        return;
    }
    if (dir->files.find(dest) != dir->files.end()) {
        cout << "Ŀ���ļ��Ѵ��ڣ�" << dest << endl;
        return;
    }
    File movefile = i->second;
    movefile.name = dest;
    dir->files.erase(i);
    dir->files[dest] = movefile;
    cout << "�ļ��ƶ��ɹ���" << src << "->" << dest << endl;
}

void copyFile(Directory* dir, const string& src, const string& dest) {
    auto i = dir->files.find(src);
    if (i== dir->files.end()) {
        cout << "�Ҳ���Դ�ļ���" << src << endl;
        return;
    }
    if (dir->files.find(dest) != dir->files.end()) {
        cout << "Ŀ���ļ��Ѵ��ڣ�" << dest << endl;
        return;
    }
    File copiedFile = i->second;
    copiedFile.name = dest;
    dir->files[dest] = copiedFile;
    cout << "�ļ����Ƴɹ���" << src << " -> " << dest << endl;
}

void flockFile(Directory* dir, const string& filename, bool lock) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "�Ҳ���Դ�ļ���" << filename << endl;
        return;
    }
    i->second.isLocked = lock;
    cout << (lock ? " �ļ��Ѽ�����" : "�ļ��ѽ�����") << filename << endl;
}

void headFile(Directory* dir, const string& filename, int num) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "�Ҳ���Դ�ļ���" << filename << endl;
        return;
    }
    istringstream ss(i->second.content);
    string line;
    int count = 0;
    cout << "[ǰ" << num << "��]" << endl;
    while (count<num) {
        getline(ss, line);
        cout << line << endl;
        count++;
    }
}

void tailFile(Directory* dir, const string& filename, int num) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "�Ҳ���Դ�ļ���" << filename << endl;
        return;
    }
    istringstream ss(i->second.content);
    vector<string>lines;
    string line;
    while (getline(ss, line)) {
        lines.push_back(line);
    }
    int start = max(0, static_cast<int>(lines.size()) - num);
    cout << "[��" << num << "��]" << endl;
    for (int i = start; i < lines.size(); ++i) {
        cout << lines[i] << endl;
    }
}

void lseekFile(Directory* dir, const string& filename, int offset) {
    auto i = dir->files.find(filename);
    if (i == dir->files.end()) {
        cout << "�Ҳ���Դ�ļ���" << filename << endl;
        return;
    }
    int newPos = i->second.readPtr + offset;
    if (newPos<0 || newPos>static_cast<int>(i->second.content.size())) {
        cout << "ָ���ƶ�Խ��" << endl;
        return;
    }
    i->second.readPtr = newPos;
    cout << "�µ�ָ��λ�ã�" << i->second.readPtr << endl;
}

void changeDirectory(Directory*& dir, const string& dirname) {
    if (dirname == "..") {
        if (dir->parent != nullptr) {
            dir = dir->parent;
            cout << "������һ��Ŀ¼��" << endl;
        }
        else {
            cout << "[��ʾ] ��ǰ���ڸ�Ŀ¼���޷����ء�" << endl;
        }
        return;
    }
    auto i = dir->subDirs.find(dirname);
    if (i == dir->subDirs.end()) {
        cout << "[����] ��Ŀ¼�����ڣ�" << dirname << endl;
        return;
    }
    dir = i->second;
    cout << "[�ɹ�] ��ǰĿ¼�л�����" << dirname << endl;
}

void listDirectory(Directory* dir) {
    cout << "[Ŀ¼] " << dir->name << endl;
    if (!dir->subDirs.empty()) {
        cout << "��Ŀ¼��\n";
        map<string, Directory*>::iterator i;
        for (i = dir->subDirs.begin(); i != dir->subDirs.end(); ++i) {
            cout << " <DIR> " << i->first << endl;
        }
    }
    if (!dir->files.empty()) {
        cout << "�ļ���\n";
        map<string, File>::iterator i;
        for (i = dir->files.begin(); i != dir->files.end(); ++i) {
            cout << "       " << i->first << " (" << i->second.size << "B)" << endl;
        }
    }
    if (dir->subDirs.empty() && dir->files.empty()) {
        cout << "[��Ŀ¼]" << endl;
    }
}

void printDirectoryTree(Directory* dir, const string& prefix) {
    auto it = dir->subDirs.begin();
    while (it != dir->subDirs.end()) {
        bool isLast = next(it) == dir->subDirs.end() && dir->files.empty();
        cout << prefix << (isLast ? "������ " : "������ ") << it->first << "/" << endl;
        printDirectoryTree(it->second, prefix + (isLast ? "    " : "��   "));
        ++it;
    }

    auto fit = dir->files.begin();
    while (fit != dir->files.end()) {
        bool isLast = next(fit) == dir->files.end();
        cout << prefix << (isLast ? "������ " : "������ ");
        cout << fit->first << " (" << fit->second.size << "B)" << endl;
        ++fit;
    }
}


void showTree(const VirtualDisk& disk) {
    cout << "/ (�����Ŀ¼)" << endl;
    auto it = disk.userFileSystems.begin();
    while (it != disk.userFileSystems.end()) {
        bool isLast = next(it) == disk.userFileSystems.end();
        cout << (isLast ? "������ " : "������ ") << it->first << "/" << endl;
        printDirectoryTree(it->second.root, isLast ? "    " : "��   ");
        ++it;
    }
}





