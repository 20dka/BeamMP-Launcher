// Copyright (c) 2019-present Anonymous275.
// BeamMP Launcher code is not in the public domain and is not free software.
// One must be granted explicit permission by the copyright holder in order to modify or distribute any part of the source or binaries.
// Anything else is prohibited. Modified works may not be published and have be upstreamed to the official repository.
///
/// Created by Anonymous275 on 7/18/2020
///
#pragma once
#include <string>
void InitLauncher(int argc, char* argv[]);
void CheckDir(int argc,char* args[]);
std::string GetUserFolder();
std::string GetVer();
std::string GetExeName();
extern bool Dev;

extern int LaunchGame;
extern std::string GameFolderOverride;
extern std::string UserFolderOverride;
extern std::string ClientBuild;
