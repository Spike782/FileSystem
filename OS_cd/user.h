#include <string>
#include "fs.h"

// ע���û�
bool registeUser(VirtualDisk& disk, const std::string& username, const std::string& password);

// ��¼�û�
bool loginUser(VirtualDisk& disk, const std::string& username, const std::string& password, std::string& currentUser);


