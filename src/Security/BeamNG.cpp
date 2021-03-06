// Copyright (c) 2019-present Anonymous275.
// BeamMP Launcher code is not in the public domain and is not free software.
// One must be granted explicit permission by the copyright holder in order to modify or distribute any part of the source or binaries.
// Anything else is prohibited. Modified works may not be published and have be upstreamed to the official repository.
///
/// Created by Anonymous275 on 7/18/2020
///

#include <filesystem>
#include <Windows.h>
#include "Startup.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <ShlDisp.h>
#include <iterator>
#include <algorithm>
#include "Curl/http.h"


#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

std::string GameDir;

void lowExit(int code){
	std::string msg =
   "Failed to find the game please launch it. Report this if the issue persists code ";
	error(msg+std::to_string(code));
	std::this_thread::sleep_for(std::chrono::seconds(10));
	exit(2);
}
std::string GetGameDir(){
	return GameDir.substr(0,GameDir.find_last_of('\\'));
}
LONG OpenKey(HKEY root,const char* path,PHKEY hKey){
	return RegOpenKeyEx(root, reinterpret_cast<LPCSTR>(path), 0, KEY_READ, hKey);
}
std::string QueryKey(HKEY hKey,int ID){
	TCHAR	achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD	cbName;				   // size of name string
	TCHAR	achClass[MAX_PATH] = TEXT("");  // buffer for class name
	DWORD	cchClassName = MAX_PATH;  // size of class string
	DWORD	cSubKeys=0;			   // number of subkeys
	DWORD	cbMaxSubKey;			  // longest subkey size
	DWORD	cchMaxClass;			  // longest class string
	DWORD	cValues;			  // number of values for key
	DWORD	cchMaxValue;		  // longest value name
	DWORD	cbMaxValueData;	   // longest value data
	DWORD	cbSecurityDescriptor; // size of security descriptor
	FILETIME ftLastWriteTime;	  // last write time

	DWORD i, retCode;

	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	retCode = RegQueryInfoKey(
			hKey,					// key handle
			achClass,				// buffer for class name
			&cchClassName,		   // size of class string
			nullptr,					// reserved
			&cSubKeys,			   // number of subkeys
			&cbMaxSubKey,			// longest subkey size
			&cchMaxClass,			// longest class string
			&cValues,				// number of values for this key
			&cchMaxValue,			// longest value name
			&cbMaxValueData,		 // longest value data
			&cbSecurityDescriptor,   // security descriptor
			&ftLastWriteTime);	   // last write time

	BYTE* buffer = new BYTE[cbMaxValueData];
	ZeroMemory(buffer, cbMaxValueData);
	if (cSubKeys){
		for (i=0; i<cSubKeys; i++){
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, i,achKey,&cbName,nullptr,nullptr,nullptr,&ftLastWriteTime);
			if (retCode == ERROR_SUCCESS){
				if(strcmp(achKey,"Steam App 284160") == 0){
					return achKey;
				}
			}
		}
	}
	if (cValues){
		for (i=0, retCode = ERROR_SUCCESS; i<cValues; i++){
			cchValue = MAX_VALUE_NAME;
			achValue[0] = '\0';
			retCode = RegEnumValue(hKey, i,achValue,&cchValue,nullptr,nullptr,nullptr,nullptr);
			if (retCode == ERROR_SUCCESS ){
				DWORD lpData = cbMaxValueData;
				buffer[0] = '\0';
				LONG dwRes = RegQueryValueEx(hKey, achValue, nullptr, nullptr, buffer, &lpData);
				std::string data = (char *)(buffer);
				std::string key = achValue;

				switch (ID){
					case 1: if(key == "SteamExe"){
								auto p = data.find_last_of("/\\");
								if(p != std::string::npos){
									return data.substr(0,p);
								}
							}
							break;
					case 2: if(key == "Name" && data == "BeamNG.drive")return data;break;
					case 3: if(key == "rootpath")return data;break;
					case 4: if(key == "userpath_override")return data;
					case 5: if(key == "Personal")return data;
					default: break;
				}
			}
		}
	}
	delete [] buffer;
	return "";
}
namespace fs = std::filesystem;

std::vector<std::string> GetID(const std::string& log){
	std::string vec,t,r;
	std::vector<std::string> Ret;
	std::ifstream f(log.c_str(), std::ios::binary);
	f.seekg(0, std::ios_base::end);
	std::streampos fileSize = f.tellg();
	vec.resize(size_t(fileSize) + 1);
	f.seekg(0, std::ios_base::beg);
	f.read(&vec[0], fileSize);
	f.close();
	std::stringstream ss(vec);
	bool S = false;
	while (std::getline(ss, t, '{')) {
		if(!S)S = true;
		else{
			for(char& c : t){
				if(isdigit(c))r += c;
			}
			break;
		}
	}
	Ret.emplace_back(r);
	r.clear();
	S = false;
	bool L = true;
	while (std::getline(ss, t, '}')) {
		if(L){
			L = false;
			continue;
		}
		for(char& c : t){
			if(c == '"'){
				if(!S)S = true;
				else{
					if(r.length() > 10) {
						Ret.emplace_back(r);
					}
					r.clear();
					S = false;
					continue;
				}
			}
			if(isdigit(c))r += c;
		}
	}
	vec.clear();
	return Ret;
}

void RegistryChecks(){

	if (!GameFolderOverride.empty()){
		GameDir = GameFolderOverride;
		return;
	}

	std::string Result,T;
	std::string K1 = R"(Software\Valve\Steam)";
	std::string K2 = R"(Software\Valve\Steam\Apps\284160)";
	std::string K3 = R"(Software\BeamNG\BeamNG.drive)";
	HKEY hKey;

	LONG dwRegOPenKey = OpenKey(HKEY_CURRENT_USER, K1.c_str(), &hKey);

	K1.clear();
	RegCloseKey(hKey);
	dwRegOPenKey = OpenKey(HKEY_CURRENT_USER, K2.c_str(), &hKey);
	if(dwRegOPenKey == ERROR_SUCCESS) {
		Result = QueryKey(hKey, 2);
		if(Result.empty())lowExit(1);

	}else lowExit(2);
	K2.clear();
	RegCloseKey(hKey);

		dwRegOPenKey = OpenKey(HKEY_CURRENT_USER, K3.c_str(), &hKey);
		if(dwRegOPenKey == ERROR_SUCCESS) {
			Result = QueryKey(hKey, 3);
			if(Result.empty())lowExit(3);
			//if(IDCheck(Result,T))lowExit(5);
			GameDir = Result;
		}else lowExit(4);
		K3.clear();
		Result.clear();

	RegCloseKey(hKey);
}
std::string CheckVer(const std::string &dir){ //returns basegame version
	std::string temp,Path = dir + "\\integrity.json";
	std::ifstream f(Path.c_str(), std::ios::binary);
	int Size = int(std::filesystem::file_size(Path));
	std::string vec(Size,0);
	f.read(&vec[0], Size);
	f.close();

	vec = vec.substr(vec.find_last_of("version"),vec.find_last_of('"'));
	for(const char &a : vec){
		if(isdigit(a) || a == '.')temp+=a;
	}
	return temp;
}

bool compareFiles(const std::string& p1, const std::string& p2) {
	std::ifstream f1(p1, std::ifstream::binary|std::ifstream::ate);
	std::ifstream f2(p2, std::ifstream::binary|std::ifstream::ate);
	
	if (f1.fail() || f2.fail()) { warn("failed opening one of the files"); return false; } //file problem
	
	//if (f1.tellg() != f2.tellg()) { warn("file sizes dont match"); return false; } //size mismatch
	
	//seek back to beginning and use std::equal to compare contents
	f1.seekg(0, std::ifstream::beg);
	f2.seekg(0, std::ifstream::beg);
	return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
						std::istreambuf_iterator<char>(),
						std::istreambuf_iterator<char>(f2.rdbuf()));
}

bool doMainLuasMatch(const std::string& p1, const std::string& p2) {
	std::string localLuaFile = GetGameDir() + "/lua/ge/main.lua";
	std::string remoteLuaFile = p1 + "temp.lua";
	remove(remoteLuaFile.c_str());

	int i = Download(p2, remoteLuaFile, true);
	if(i != -1){
		warn("oops couldnt download");
		return false;
	}

	info("comparing " + localLuaFile + " with " + remoteLuaFile);

	return compareFiles(localLuaFile, remoteLuaFile);
}
