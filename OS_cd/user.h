#include <string>
#include "fs.h"

// 注册用户
bool registeUser(VirtualDisk& disk, const std::string& username, const std::string& password);

// 登录用户
bool loginUser(VirtualDisk& disk, const std::string& username, const std::string& password, std::string& currentUser);


