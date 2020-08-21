///
/// Created by Anonymous275 on 7/17/2020
///
#include "Security/Enc.h"
#include "Startup.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <chrono>
std::string getDate() {
    typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<24>>::type> days;
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::system_clock::duration tp = now.time_since_epoch();
    days d = std::chrono::duration_cast<days>(tp);tp -= d;
    auto h = std::chrono::duration_cast<std::chrono::hours>(tp);tp -= h;
    auto m = std::chrono::duration_cast<std::chrono::minutes>(tp);tp -= m;
    auto s = std::chrono::duration_cast<std::chrono::seconds>(tp);tp -= s;
    time_t tt = std::chrono::system_clock::to_time_t(now);
    tm local_tm{};
    localtime_s(&local_tm,&tt);
    std::stringstream date;
    int S = local_tm.tm_sec;
    int M = local_tm.tm_min;
    int H = local_tm.tm_hour;
    std::string Secs = (S > 9 ? std::to_string(S) : "0" + std::to_string(S));
    std::string Min = (M > 9 ? std::to_string(M) : "0" + std::to_string(M));
    std::string Hour = (H > 9 ? std::to_string(H) : "0" + std::to_string(H));
    date
            << "["
            << local_tm.tm_mday << "/"
            << local_tm.tm_mon + 1 << "/"
            << local_tm.tm_year + 1900 << " "
            << Hour << ":"
            << Min << ":"
            << Secs
            << "] ";
    return date.str();
}
void InitLog(){
    std::ofstream LFS;
    LFS.open (Sec("Launcher.log"));
    if(!LFS.is_open()){
        error(Sec("logger file init failed!"));
    }else LFS.close();
}
void addToLog(const std::string& Line){
    std::ofstream LFS;
    LFS.open (Sec("Launcher.log"), std::ios_base::app);
    LFS << Line.c_str();
    LFS.close();
}
void info(const std::string& toPrint) {
    std::string Print = getDate() + Sec("[INFO] ") + toPrint + "\n";
    std::cout << Print;
    addToLog(Print);
}
void debug(const std::string& toPrint) {
    if(!Dev)return;
    std::string Print = getDate() + Sec("[DEBUG] ") + toPrint + "\n";
    std::cout << Print;
    addToLog(Print);
}
void warn(const std::string& toPrint){
    std::string Print = getDate() + Sec("[WARN] ") + toPrint + "\n";
    std::cout << Print;
    addToLog(Print);
}
void error(const std::string& toPrint) {
    static int ECounter = 0;
    std::string Print = getDate() + Sec("[ERROR] ") + toPrint + "\n";
    std::cout << Print;
    addToLog(Print);
    if(ECounter > 10)exit(7);
    ECounter++;
}
void except(const std::string& toPrint) {
    std::string Print = getDate() + Sec("[EXCEP] ") + toPrint + "\n";
    std::cout << Print;
    addToLog(Print);
}
