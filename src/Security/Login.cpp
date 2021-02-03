// Copyright (c) 2019-present Anonymous275.
// BeamMP Launcher code is not in the public domain and is not free software.
// One must be granted explicit permission by the copyright holder in order to modify or distribute any part of the source or binaries.
// Anything else is prohibited. Modified works may not be published and have be upstreamed to the official repository.
///
/// Created by Anonymous275 on 11/26/2020
///

#include "Curl/http.h"
#include <filesystem>
#include "Logger.h"
#include <fstream>
#include "Json.h"

using namespace std::filesystem;
std::string PublicKey;
extern bool LoginAuth;

void UpdateKey(const char* newKey){
    if(newKey){
        std::ofstream Key("key");
        if(Key.is_open()){
            Key << newKey;
            Key.close();
        }else fatal("Cannot write to disk!");
    }else if(exists("key")){
        remove("key");
    }
}

/// "username":"value","password":"value"
/// "Guest":"Name"
/// "pk":"private_key"

std::string GetFail(const std::string& R){
    std::string DRet = R"({"success":false,"message":)";
    DRet += "\""+R+"\"}";
    error(R);
    return DRet;
}

std::string Login(const std::string& fields){
    if(fields == "LO"){
        LoginAuth = false;
        UpdateKey(nullptr);
        return "";
    }
    info("Attempting to authenticate...");
    std::string Buffer = PostHTTP("https://auth.beammp.com/userlogin", fields);
	debug("login auth reply: " + Buffer);
    json::Document d;
    d.Parse(Buffer.c_str());
    if(Buffer == "-1"){
        return GetFail("Failed to communicate with the auth system!");
    }

    if (Buffer.find('{') == -1 || d.HasParseError()) {
        return GetFail("Invalid answer from authentication servers, please try again later!");
    }
    if(!d["success"].IsNull() && d["success"].GetBool()){
        LoginAuth = true;
        if(!d["private_key"].IsNull()){
            UpdateKey(d["private_key"].GetString());
			//debug("private key: " + d["private_key"].GetString());
        }
        if(!d["public_key"].IsNull()){
            PublicKey = d["public_key"].GetString();
			debug("public key: " + PublicKey);
        }
        info("Authentication successful!");
    }else info("Authentication failed!");
    if(!d["message"].IsNull()){
        d.RemoveMember("private_key");
        d.RemoveMember("public_key");
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);
        return buffer.GetString();
    }
    return GetFail("Invalid message parsing!");
}

int tryCount = 0;

void CheckLocalKey(){
	info("a");
	tryCount++;
    if(exists("key") && file_size("key") < 100){
		info("aa");
        std::ifstream Key("key");
        if(Key.is_open()) {
			info("aaa");
            auto Size = file_size("key");
            std::string Buffer(Size, 0);
            Key.read(&Buffer[0], Size);
            Key.close();
			info("aaaa");

            Buffer = PostHTTP("https://auth.beammp.com/userlogin", R"({"pk":")"+Buffer+"\"}");
			info("login auth reply (from local key): " + Buffer);
            json::Document d;
            d.Parse(Buffer.c_str());
            if (Buffer == "-1" || Buffer.find('{') == -1 || d.HasParseError()) {
				if (tryCount < 20) {
					error("Invalid answer from authentication servers, please try again later!");
					CheckLocalKey();
				} else fatal("Invalid answer from authentication servers, please try again later!");
            }
            if(d["success"].GetBool()){
                LoginAuth = true;
                UpdateKey(d["private_key"].GetString());
                PublicKey = d["public_key"].GetString();
				debug("public key (from local key): " + PublicKey);
            }else{
                error("Auto-Authentication unsuccessful please re-login!");
                UpdateKey(nullptr);
            }
        }else{
            warn("Could not open saved key!");
            UpdateKey(nullptr);
        }
    }else UpdateKey(nullptr);
}
