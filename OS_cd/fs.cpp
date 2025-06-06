#include"fs.h"
#include <windows.h>
#include<fstream>
#include <string>
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

HANDLE lockDiskFile(const std::string& filename) {
	// 1. 先用 CreateFile 以 GENERIC_READ|GENERIC_WRITE 方式打开文件，shareMode 设置为 0
	//    表示不允许其他进程以任何方式访问（完全独占）。
	HANDLE hFile = CreateFileA(
		filename.c_str(),            // 要打开的文件名
		GENERIC_READ | GENERIC_WRITE,// 需要读写权限
		0,                           // dwShareMode = 0，表示“完全独占”，不允许其他进程打开
		nullptr,                     // 默认安全属性
		OPEN_ALWAYS,                 // 文件不存在就创建，存在就打开
		FILE_ATTRIBUTE_NORMAL,       // 普通文件属性
		nullptr                      // 不用模板句柄
	);

	if (hFile == INVALID_HANDLE_VALUE) {
		// 打开（或创建）失败
		std::cerr << "[锁定失败] 无法打开或创建文件: " << filename
			<< "，错误码：" << GetLastError() << std::endl;
		return INVALID_HANDLE_VALUE;
	}

	// 2. 调用 LockFileEx 对整个文件做排他锁
	//    使用 OVERLAPPED 結構表示要锁定的文件区域
	OVERLAPPED ovl = { 0 };
	BOOL locked = LockFileEx(
		hFile,
		LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, // 排他锁 + 如果锁不到立即失败
		0,                   // 保留，一般写 0
		MAXDWORD,            // 要锁定的高 32 位 (代表文件大小覆盖到文件尾)
		MAXDWORD,            // 要锁定的低 32 位
		&ovl
	);

	if (!locked) {
		// 无法获取锁，立即失败
		std::cerr << "[锁定失败] 无法对文件加锁: " << filename
			<< "，错误码：" << GetLastError() << std::endl;
		CloseHandle(hFile);
		return INVALID_HANDLE_VALUE;
	}

	// 成功加锁，返回文件句柄
	return hFile;
}

void unlockDiskFile(HANDLE hFile) {
	if (hFile == INVALID_HANDLE_VALUE) {
		// 如果传入的是无效句柄，什么也不做
		return;
	}

	// 用同样的 OVERLAPPED 结构来解锁整个文件
	OVERLAPPED ovl = { 0 };
	BOOL unlocked = UnlockFileEx(
		hFile,
		0,          // 保留，一般写 0
		MAXDWORD,   // 要解锁的高 32 位
		MAXDWORD,   // 要解锁的低 32 位
		&ovl
	);

	if (!unlocked) {
		std::cerr << "[解锁失败] 无法对文件解锁，错误码："
			<< GetLastError() << std::endl;
		// 但不管成功与否，都要 CloseHandle
	}

	// 最后关闭句柄
	CloseHandle(hFile);
}
