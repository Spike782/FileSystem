#include <string>
#include "fs.h"

// 注册用户
bool registeUser(VirtualDisk& disk, const string& username, const string& password);

// 登录用户
bool loginUser(VirtualDisk& disk, const string& username, const string& password, string& currentUser);


