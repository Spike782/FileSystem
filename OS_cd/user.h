// user.h
#pragma once

#include "fs.h"
#include <string>
using namespace std;

bool registeUser(VirtualDisk& disk, const string& username, const string& password);
bool loginUser(VirtualDisk& disk, const string& username, const string& password, string& currentUser);
