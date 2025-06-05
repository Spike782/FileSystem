#include"fs.h"
#include<fstream>
#include<filesystem>
#include<iostream>
#include<iomanip>
#include<vector>
using namespace std;

// 辅助函数：写入字符串（带长度）
void writeString(ofstream& out, const string& str) {
	size_t len = str.size();//先拿到字符串长度
	out.write(reinterpret_cast<const char*>(&len), sizeof(len));//写入长度
	out.write(str.c_str(), len);//写入字符串内容
}

// 辅助函数：读取字符串
string readString(ifstream& in) {
	size_t len;
	in.read(reinterpret_cast<char*>(&len), sizeof(len));//先读出字符串长度
	string str(len, '\0');//初始化一个指定长度的字符串
	in.read(&str[0], len);//填充读取内容
	return str;
}

void saveDirectory(ofstream& out, Directory* dir) {
	writeString(out, dir->name);
	//文件数
	size_t filecount = dir->files.size();
	out.write(reinterpret_cast<const char*>(&filecount), sizeof(filecount));
	for (auto i = dir->files.begin(); i != dir->files.end(); ++i) {
		string name = i->first;
		File& file = i->second;
		writeString(out, name);
		// 写入二进制文件内容（长度 + 数据）
		size_t contentLen = file.content.size();
		out.write(reinterpret_cast<const char*>(&contentLen), sizeof(contentLen));
		out.write(file.content.data(), contentLen);

		out.write(reinterpret_cast<const char*>(&file.isOpen), sizeof(file.isOpen));
		out.write(reinterpret_cast<const char*>(&file.isLocked), sizeof(file.isLocked));
		out.write(reinterpret_cast<const char*>(&file.size), sizeof(file.size));
		out.write(reinterpret_cast<const char*>(&file.readPtr), sizeof(file.readPtr));
	}
	// 子目录数
	size_t dirCount = dir->subDirs.size();
	out.write(reinterpret_cast<const char*>(&dirCount), sizeof(dirCount));
	for (auto i = dir->subDirs.begin(); i != dir->subDirs.end(); ++i) {
		const string& subname = i->first;
		Directory* subptr = i->second;
		saveDirectory(out, subptr);
	}
}

Directory* loadDirectory(ifstream& in, Directory* parent) {
	Directory* dir = new Directory;
	dir->name = readString(in);
	dir->parent = parent;

	size_t fileCount;
	in.read(reinterpret_cast<char*>(&fileCount), sizeof(fileCount));
	for (size_t i = 0; i < fileCount; ++i) {
		string name = readString(in);
		File f;
		size_t contentLen;
		in.read(reinterpret_cast<char*>(&contentLen), sizeof(contentLen));
		f.content.resize(contentLen);
		in.read(&f.content[0], contentLen);
		in.read(reinterpret_cast<char*>(&f.isOpen), sizeof(f.isOpen));
		in.read(reinterpret_cast<char*>(&f.isLocked), sizeof(f.isLocked));
		in.read(reinterpret_cast<char*>(&f.size), sizeof(f.size));
		in.read(reinterpret_cast<char*>(&f.readPtr), sizeof(f.readPtr));
		dir->files[name] = f;
	}

	size_t dirCount;
	in.read(reinterpret_cast<char*>(&dirCount), sizeof(dirCount));
	for (size_t i = 0; i < dirCount; ++i) {
		Directory* sub = loadDirectory(in, dir);
		dir->subDirs[sub->name] = sub;
	}
	return dir;
}

void savetoDisk(const VirtualDisk& disk, const string& filename) {
	ofstream out(filename, ios::binary);
	size_t userCount = disk.users.size();
	out.write(reinterpret_cast<const char*>(&userCount), sizeof(userCount));
	for (auto i = disk.users.begin(); i !=disk.users.end();++i) {
		string username = i->first;
		User user = i->second;
		writeString(out, username);
		writeString(out, user.password);
		out.write(reinterpret_cast<const char*>(&user.loginAttempts), sizeof(user.loginAttempts));
		out.write(reinterpret_cast<const char*>(&user.isLocked), sizeof(user.isLocked));

		saveDirectory(out, disk.userFileSystems.at(username).root);
	}
	out.close();
}

bool loadFromDisk(VirtualDisk& disk, const string& filename) {
	ifstream in(filename, ios::binary);
	if (!in.is_open()) {
		return false;
	}

	size_t userCount;
	in.read(reinterpret_cast<char*>(&userCount), sizeof(userCount));
	for (size_t i = 0; i < userCount; ++i) {
		string username = readString(in);
		string password = readString(in);
		int loginAttempts;
		bool isLocked;
		in.read(reinterpret_cast<char*>(&loginAttempts), sizeof(loginAttempts));
		in.read(reinterpret_cast<char*>(&isLocked), sizeof(isLocked));
		disk.users[username] = { password,  isLocked,loginAttempts };

		FileSystem fs;
		fs.root = loadDirectory(in, nullptr);
		disk.userFileSystems[username] = fs;
	}
	in.close();
	return true;
}

bool importFile(Directory* currentDir, const string& srcPath, const string &destName) {
	string filename = destName;
	if (filename.empty()) {
		size_t pos = srcPath.find_last_of("/\\");
		filename = (pos == string::npos) ? srcPath : srcPath.substr(pos + 1);
	}
	ifstream in(srcPath, ios::binary);
	if (!in.is_open()) {
		return false;
	}

	File f;
	f.content.assign((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
	f.size = f.content.size();
	f.isLocked = false;
	f.isOpen = false;
	f.readPtr = 0;

	currentDir->files[filename] = f;
	return true;
}

bool exportFile(Directory* currentDir, const string& fileName, string destPath) {
	auto i = currentDir->files.find(fileName);
	if (i == currentDir->files.end()) {
		return false;
	}

	// 自动补全路径
	if (!destPath.empty() && (destPath.back() == '\\' || destPath.back() == '/')) {
		destPath += fileName;
	}

	ofstream out(destPath, ios::binary);
	if (!out.is_open()) {
		return false;
	}
	out.write(i->second.content.data(), i->second.content.size());
	out.close();
	return true;
}