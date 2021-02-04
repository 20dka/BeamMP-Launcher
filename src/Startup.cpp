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

std::string ClientBuild = ""; //type of zip to download
std::string UserFolderOverride;  //path to userfolder (documents/beam)
std::string GameFolderOverride;  //path to beamng exe
bool Dev = false;
bool dontLaunchGame = false;

namespace fs = std::filesystem;
std::string GetExeName() { return "BeamMP-Launcher.exe"; } //get executable name
std::string GetVer() { return "1.80"; }
std::string GetPatch() { return ".94"; }


void ReLaunch(int argc,char*args[]){
	std::string Arg;
	for(int c = 2; c <= argc; c++){
		Arg += " ";
		Arg += args[c-1];
	}
	system("cls");
	ShellExecute(nullptr,"runas",GetExeName().c_str(),Arg.c_str(),nullptr,SW_SHOWNORMAL);
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
	ShellExecute(nullptr,"open",GetExeName().c_str(),Arg.c_str(),nullptr,SW_SHOWNORMAL);
	ShowWindow(GetConsoleWindow(),0);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	exit(1);
}

void CheckName(int argc,char* args[]){
	struct stat info{};
	std::string DN = GetExeName(),CDir = args[0],FN = CDir.substr(CDir.find_last_of('\\')+1);
	if(FN != DN){
		if(stat(DN.c_str(),&info)==0)remove(DN.c_str());
		if(stat(DN.c_str(),&info)==0)ReLaunch(argc,args);
		std::rename(FN.c_str(), DN.c_str());
		URelaunch(argc,args);
	}
}

void UpdateLauncher(const std::string& exePath){
	std::string launcherURL = "https://github.com/20dka/BeamMP-Launcher/releases/latest/download/BeamMP-Launcher.exe";
	info("Going to download to: " + exePath);
	struct stat buffer{};

	info("Updating...");

	std::string Back = exePath;
	Back.erase(Back.end()-3,Back.end());
	Back += "back";

	if(stat(Back.c_str(), &buffer) == 0) {
		info("found old backup, yeeting");
		remove(Back.c_str());
	}

	if(std::rename(exePath.c_str(), Back.c_str()))error("failed creating a backup!");

	int i = Download(launcherURL, exePath, true);
	if(i != -1){
		error("Launcher Update failed! trying again... code : " + std::to_string(i));
		std::this_thread::sleep_for(std::chrono::seconds(2));
		int i2 = Download(launcherURL, exePath, true);
		if(i2 != -1){
			error("Launcher Update failed! code : " + std::to_string(i2));
			std::this_thread::sleep_for(std::chrono::seconds(10));
			exit(1);
			//ReLaunch(argc,args);
		}
	}
	info("Download successful");
	exit(1);
	//URelaunch(argc,args);
}

void PrintHelp() {
	info(R"(BeamMP Launcher modded by deer boi. Options are:
	-port : port number for game communication
	-updateLauncher true : pulls the latest build from my github
	-userFolder : path for the game userfolder, must end with \
	-gameFolder : path for the game install folder
	-devMode true : enables debug prints
	-launchGame false : disables automatic game launch
	-skipMod true : skips the lua mod's download)");
}

std::string findArg(int argc, char* argv[], const std::string& argName){
	for(int i = 1;i<argc;i++){
		if ("-"+argName == argv[i]){
			if (argc == i+1 || std::string(argv[i+1]).at(0) == '-') return "noVal"; //no parameter
			else return argv[i+1];
		}
	}
	return ""; //not found
}

void HandleArgs(int argc, char* argv[]){
	if(argc == 1) return;

	if (findArg(argc, argv,"h") == "noVal" || findArg(argc, argv,"-help") == "noVal") { PrintHelp(); exit(1); } //-h or --help

	if (findArg(argc, argv,"updateLauncher") != ""){
		UpdateLauncher(std::string(argv[0]));
	}

	std::string Port = findArg(argc, argv,"port");
	if(Port != "" && Port.find_first_not_of("0123456789") == std::string::npos){
		if(std::stoi(Port) > 1000){
			GamePort = std::stoi(Port);
			warn("Running on custom port: " + std::to_string(GamePort));
		}
	}

	std::string buildName = findArg(argc, argv,"useBuild");
	if (buildName == "deer"){
		ClientBuild = "deer";
		warn("Using deer boi's build"); 
	} else if (buildName == "public"){
		ClientBuild = "public";
		warn("Using public build");
	} else if (buildName == "none" || findArg(argc, argv,"skipMod") == "true"){
		ClientBuild = "none";
		warn("Mod won't be downloaded");
	} else if (buildName != "" && buildName != "noVal") {
		ClientBuild = buildName;
		warn("Using custom build: " + buildName); 
	}

	std::string usrfldr = findArg(argc, argv,"userFolder");
	if (usrfldr != ""){
		UserFolderOverride = usrfldr;
		warn("Using custom userfolder path: " + UserFolderOverride); 
	}
	std::string gmfldr = findArg(argc, argv,"gameFolder");
	if (gmfldr != ""){
		GameFolderOverride = gmfldr;
		warn("Using custom gamefolder path: " + GameFolderOverride); 
	}

	if (findArg(argc, argv,"devMode") == "true"){
		Dev = true;
		warn("Developer mode enabled");
	}
	if (findArg(argc, argv,"launchGame") == "false"){
		dontLaunchGame = true;
		warn("Game won't be launched");
	}
}

void InitLauncher(int argc, char* argv[]) {
	system("cls");
	curl_global_init(CURL_GLOBAL_DEFAULT);
	SetConsoleTitleA(("DeerMP Launcher v" + std::string(GetVer()) + GetPatch() + " Deer").c_str());
	InitLog();
	//CheckName(argc, argv); //rename if launcher name is incorrect or something idk noboody needs this

	HandleArgs(argc, argv); //cmd args

	bool hasEA = CheckLocalKey(); //auth on startup

	if (ClientBuild == ""){
		if (hasEA) {
			info("yay you get EA");
			ClientBuild = "deer";
		} else {
			info("public build");
			ClientBuild = "public";
		}
	}
}

size_t DirCount(const std::filesystem::path& path){
	return (size_t)std::distance(std::filesystem::directory_iterator{path}, std::filesystem::directory_iterator{});
}
void CheckMP(const std::string& Path) {
	if (!fs::exists(Path))return;
	size_t c = DirCount(fs::path(Path));
	if (c > 3) {
		warn(std::to_string(c - 1) + " multiplayer mods will be wiped from mods/multiplayer! Close this if you don't want that!");
		std::this_thread::sleep_for(std::chrono::seconds(10));
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
void PreGame(const std::string& GamePath, const std::string& LauncherPath){
	CheckMP(GetUserFolder() + "mods/multiplayer");  //deletes existing mods from the mp folder

	info(ClientBuild);

	if (ClientBuild == "none") return;

	info("Downloading mod...");
	try {
		if (!fs::exists(GetUserFolder() + "mods/multiplayer")) {
			fs::create_directories(GetUserFolder() + "mods/multiplayer");
		}
	}catch(std::exception&e){
		fatal(e.what());
	}


	if (ClientBuild == "public")
		Download("https://beammp.com/builds/client", GetUserFolder() + R"(mods\multiplayer\BeamMP.zip)", true);
	else if (ClientBuild == "deer")
		Download("https://github.com/20dka/files/blob/master/BeamMP.zip?raw=true", GetUserFolder() + R"(mods\multiplayer\BeamMP.zip)", true);
	else Download(ClientBuild, GetUserFolder() + R"(mods\multiplayer\BeamMP.zip)", true);

	std::size_t found = LauncherPath.find_last_of("/\\");

	if (doMainLuasMatch(LauncherPath.substr(0,found+1), "https://github.com/20dka/files/blob/master/BeamMP/main.lua?raw=true")) warn("files match");
	else warn("files dont match :(");

	info("Download Complete!");
}
