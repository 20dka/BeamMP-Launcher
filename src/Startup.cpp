// Copyright (c) 2019-present Anonymous275.
// BeamMP Launcher code is not in the public domain and is not free software.
// One must be granted explicit permission by the copyright holder in order to modify or distribute any part of the source or binaries.
// Anything else is prohibited. Modified works may not be published and have be upstreamed to the official repository.
///
/// Created by Anonymous275 on 7/16/2020
///
//#include "Discord/discord_info.h"
#include "Network/network.h"
#include "Security/Init.h"
#include "Curl/http.h"
#include <curl/curl.h>
#include <filesystem>
#include "Startup.h"
#include "Logger.h"
#include <thread>

std::string UserFolderOverride;
bool Dev = false;
namespace fs = std::filesystem;
std::string GetEN(){
    return "BeamMP-Launcher.exe";
}
std::string GetVer(){
    return "1.80";
}
std::string GetPatch(){
    return ".94";
}
void ReLaunch(int argc,char*args[]){
    std::string Arg;
    for(int c = 2; c <= argc; c++){
        Arg += " ";
        Arg += args[c-1];
    }
    system("cls");
    ShellExecute(nullptr,"runas",GetEN().c_str(),Arg.c_str(),nullptr,SW_SHOWNORMAL);
    ShowWindow(GetConsoleWindow(),0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    exit(1);
}
void URelaunch(int argc,char* args[]){
    std::string Arg;
    for(int c = 2; c <= argc; c++){
        Arg += " ";
        Arg += args[c-1];
    }
    ShellExecute(nullptr,"open",GetEN().c_str(),Arg.c_str(),nullptr,SW_SHOWNORMAL);
    ShowWindow(GetConsoleWindow(),0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    exit(1);
}
void CheckName(int argc,char* args[]){
    struct stat info{};
    std::string DN = GetEN(),CDir = args[0],FN = CDir.substr(CDir.find_last_of('\\')+1);
    if(FN != DN){
        if(stat(DN.c_str(),&info)==0)remove(DN.c_str());
        if(stat(DN.c_str(),&info)==0)ReLaunch(argc,args);
        std::rename(FN.c_str(), DN.c_str());
        URelaunch(argc,args);
    }
}

std::string findArg(int argc, char* argv[], const std::string& argName){
	for(int i = 1;i<argc;i++){
		if ("-"+argName == argv[i] && argc > i+1){
			return argv[i+1];
		}
	}
	return "";
}

void HandleArgs(int argc, char* argv[]){
	//warn("argc: " + std::to_string(argc));
    if(argc > 1){
        std::string Port = findArg(argc, argv,"port");
        if(Port != "" && Port.find_first_not_of("0123456789") == std::string::npos){
            if(std::stoi(Port) > 1000){
                DEFAULT_PORT = std::stoi(Port);
                warn("Running on custom port: " + std::to_string(DEFAULT_PORT));
            }
        }

        std::string usrfldr = findArg(argc, argv,"userpath");
		if (usrfldr != ""){
			UserFolderOverride = usrfldr;
			warn("Using custom userfolder path: " + UserFolderOverride); 
		}

		if (findArg(argc, argv,"devmode") == "true"){
			Dev = true;
			warn("Developer mode enabled"); 
		}
	}
}
void InitLauncher(int argc, char* argv[]) {
    system("cls");
    curl_global_init(CURL_GLOBAL_DEFAULT);
    SetConsoleTitleA(("BeamMP Launcher v" + std::string(GetVer()) + GetPatch() + " Beep").c_str());
    InitLog();
    CheckName(argc, argv);
    CheckLocalKey(); //will replace RequestRole

    HandleArgs(argc, argv);
}
size_t DirCount(const std::filesystem::path& path){
    return (size_t)std::distance(std::filesystem::directory_iterator{path}, std::filesystem::directory_iterator{});
}
void CheckMP(const std::string& Path) {
    if (!fs::exists(Path))return;
    size_t c = DirCount(fs::path(Path));
    if (c > 3) {
        warn(std::to_string(c - 1) + " multiplayer mods will be wiped from mods/multiplayer! Close this if you don't want that!");
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    try {
        for (auto& p : fs::directory_iterator(Path)){
            if(p.exists() && !p.is_directory()){
                std::string Name = p.path().filename().u8string();
                for(char&Ch : Name)Ch = char(tolower(Ch));
                if(Name != "beammp.zip")fs::remove(p.path());
            }
        }
    } catch (...) {
        fatal("Please close the game, and try again!");
    }

}
void PreGame(const std::string& GamePath){
    const std::string CurrVer("0.21.2.0");
    std::string GameVer = CheckVer(GamePath);
    info("Game Version : " + GameVer);
    if(GameVer < CurrVer){
        fatal("Game version is old! Please update.");
    }else if(GameVer > CurrVer){
        //warn("Game is newer than recommended, multiplayer may not work as intended!");
    }
    CheckMP(GetGamePath() + "mods/multiplayer");

    if(!Dev) {
        info("Downloading mod...");
        try {
            if (!fs::exists(GetGamePath() + "mods/multiplayer")) {
                fs::create_directories(GetGamePath() + "mods/multiplayer");
            }
        }catch(std::exception&e){
            fatal(e.what());
        }
        Download("https://beammp.com/builds/client", GetGamePath() + R"(mods\multiplayer\BeamMP.zip)", true);
        info("Download Complete!");
    }

   /*debug("Name : " + GetDName());
    debug("Discriminator : " + GetDTag());
    debug("Unique ID : " + GetDID());*/
}