#include"command.h"
#include<iostream>
using namespace std;

Directory* currentDir = nullptr;

void mkdir(Directory* dir, const string& dirname) {
	if (dir->subDirs.find(dirname) != dir->subDirs.end()) {
		cout << "Ŀ¼�Ѵ��ڣ�" << endl;
		return;
	}
	Directory newDir;
	newDir.name = dirname;
	dir->subDirs[dirname] = newDir;
	cout << "Ŀ¼�����ɹ���" << dirname << endl;
}

void rmdir(Directory* dir, const string& dirname) {
	auto i = dir->subDirs.find(dirname);
	if (i == dir->subDirs.end()) {
		cout << "�Ҳ���Ŀ¼��" << dirname << endl;
		return;
	}
	if (!i->second.subDirs.empty() || !i->second.files.empty()) {
		cout << "Ŀ¼��Ϊ�գ�����ɾ����" << endl;
		return;
	}
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