#include <string>
#include "fs.h"

// ע���û�
bool registeUser(VirtualDisk& disk, const string& username, const string& password);

// ��¼�û�
bool loginUser(VirtualDisk& disk, const string& username, const string& password, string& currentUser);


