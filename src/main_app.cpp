#include "logger_app.h"
#include <iostream>
#include <getopt.h>
#include <algorithm>   // <-- добавлено для std::transform
#include <cctype>      // для ::toupper

void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " [OPTIONS]" << std::endl;
    std::cout << "  -f, --file FILE     Log file (default: app.log)" << std::endl;
    std::cout << "  -l, --level LEVEL   Default level (DEBUG, INFO, ERROR)" << std::endl;
    std::cout << "  -h, --help          Show this help" << std::endl;
}

bool parseLogLevel(const std::string& str, Logger::LogLevel& level) {
    std::string u = str;
    std::transform(u.begin(), u.end(), u.begin(), ::toupper);
    if (u == "DEBUG") { level = Logger::LogLevel::DEBUG; return true; }
    if (u == "INFO")  { level = Logger::LogLevel::INFO;  return true; }
    if (u == "ERROR") { level = Logger::LogLevel::ERR;   return true; }   // исправлено: ERR вместо ERROR
    return false;
}

int main(int argc, char* argv[]) {
    std::string logFile = "app.log";
    Logger::LogLevel defaultLevel = Logger::LogLevel::INFO;
    
    static struct option longOpts[] = {
        {"file",  required_argument, 0, 'f'},
        {"level", required_argument, 0, 'l'},
        {"help",  no_argument,       0, 'h'},
        {0,0,0,0}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, "f:l:h", longOpts, nullptr)) != -1) {
        switch (opt) {
            case 'f': logFile = optarg; break;
            case 'l':
                if (!parseLogLevel(optarg, defaultLevel)) {
                    std::cerr << "Invalid level: " << optarg << std::endl;
                    return 1;
                }
                break;
            case 'h': printUsage(argv[0]); return 0;
            default:  printUsage(argv[0]); return 1;
        }
    }
    try {
        LoggerApp::LoggerApp app(logFile, defaultLevel);
        return app.run();
    } catch (const std::exception& e) {
        std::cerr << "FATAL: " << e.what() << std::endl;
        return 1;
    }
}