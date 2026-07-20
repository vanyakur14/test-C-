#include "logger_app.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>

namespace LoggerApp {

LoggerApp::LoggerApp(const std::string& logFile, Logger::LogLevel defaultLevel)
    : logFile_(logFile), defaultLevel_(defaultLevel), running_(false) {}

LoggerApp::~LoggerApp() { stop(); }

int LoggerApp::run() {
    if (!Logger::init(logFile_, defaultLevel_)) {
        std::cerr << "ERROR: Failed to initialize logger with file: " 
                  << logFile_ << std::endl;
        return 1;
    }
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Interactive Logger Application" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Log file: " << logFile_ << std::endl;
    std::cout << "Default level: " << Logger::levelToString(defaultLevel_) << std::endl;
    std::cout << "========================================" << std::endl;

    running_ = true;
    worker_ = std::thread(&LoggerApp::workerThread, this);

    bool continueRunning = true;
    while (continueRunning && running_) {
        std::cout << "\n> ";
        std::cout.flush();
        continueRunning = processInput();
    }
    stop();
    Logger::shutdown();
    std::cout << "\nApplication terminated." << std::endl;
    return 0;
}

void LoggerApp::stop() {
    if (running_.exchange(false)) {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            commandQueue_.push(LogCommand("", Logger::LogLevel::INFO, true));
            queueCond_.notify_one();
        }
        if (worker_.joinable()) worker_.join();
    }
}

void LoggerApp::workerThread() {
    Logger::info("Worker thread started");
    while (running_) {
        LogCommand command;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCond_.wait(lock, [this]() {
                return !commandQueue_.empty() || !running_;
            });
            if (!running_ && commandQueue_.empty()) break;
            if (commandQueue_.empty()) continue;
            command = commandQueue_.front();
            commandQueue_.pop();
        }
        if (command.isShutdown) {
            Logger::info("Worker thread received shutdown signal");
            break;
        }
        try {
            Logger::log(command.message, command.level);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::cout << "[✓] Logged: " << command.message << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[✗] Failed to log message: " << e.what() << std::endl;
        }
    }
    Logger::info("Worker thread stopped");
}

bool LoggerApp::processInput() {
    std::string input;
    std::getline(std::cin, input);

    if (!input.empty() && input.back() == '\r') input.pop_back();
    input.erase(0, input.find_first_not_of(" \t\n\r"));
    input.erase(input.find_last_not_of(" \t\n\r") + 1);
    if (input.empty()) return true;

    if (input == "exit" || input == "quit" || input == "q") {
        std::cout << "Exiting..." << std::endl;
        return false;
    }
    if (input == "help" || input == "h" || input == "?") {
        showHelp();
        return true;
    }
    if (input == "status" || input == "s") {
        showStatus();
        return true;
    }
    if (input == "clear") {
        std::system("clear");
        return true;
    }

    LogCommand command;
    if (!parseCommand(input, command)) {
        std::cerr << "[✗] Invalid command format. Type 'help' for usage." << std::endl;
        return true;
    }
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        commandQueue_.push(command);
        queueCond_.notify_one();
    }
    return true;
}

bool LoggerApp::parseCommand(const std::string& input, LogCommand& command) {
    size_t colonPos = input.find(':');
    size_t spacePos = input.find(' ');
    size_t sepPos = std::string::npos;
    if (colonPos != std::string::npos) {
        sepPos = colonPos;
    } else if (spacePos != std::string::npos) {
        std::string first = input.substr(0, spacePos);
        Logger::LogLevel test;
        if (stringToLevel(first, test)) sepPos = spacePos;
    }
    if (sepPos != std::string::npos) {
        std::string levelStr = input.substr(0, sepPos);
        if (!stringToLevel(levelStr, command.level)) return false;
        size_t start = sepPos + 1;
        while (start < input.length() && input[start] == ' ') ++start;
        if (start >= input.length()) return false;
        command.message = input.substr(start);
    } else {
        command.message = input;
        command.level = defaultLevel_;
    }
    return !command.message.empty();
}

bool LoggerApp::stringToLevel(const std::string& levelStr, Logger::LogLevel& level) {
    std::string upper = levelStr;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    if (!upper.empty() && upper.back() == '\r') upper.pop_back();
    if (upper == "DEBUG" || upper == "D") { level = Logger::LogLevel::DEBUG; return true; }
    if (upper == "INFO" || upper == "I")  { level = Logger::LogLevel::INFO;  return true; }
    if (upper == "ERROR" || upper == "E") { level = Logger::LogLevel::ERR;   return true; }
    return false;
}

void LoggerApp::showHelp() const {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  COMMAND HELP" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\nFORMATS:" << std::endl;
    std::cout << "  message              - Log with default level" << std::endl;
    std::cout << "  level:message        - Log with specified level (colon)" << std::endl;
    std::cout << "  level message        - Log with specified level (space)" << std::endl;
    std::cout << "\nLEVELS:" << std::endl;
    std::cout << "  DEBUG (D)  - Debug messages" << std::endl;
    std::cout << "  INFO  (I)  - Informational messages" << std::endl;
    std::cout << "  ERROR (E)  - Error messages" << std::endl;
    std::cout << "\nSPECIAL COMMANDS:" << std::endl;
    std::cout << "  help, h, ? - Show this help" << std::endl;
    std::cout << "  status, s  - Show current status" << std::endl;
    std::cout << "  clear      - Clear screen" << std::endl;
    std::cout << "  exit, q    - Exit application" << std::endl;
    std::cout << "\nEXAMPLES:" << std::endl;
    std::cout << "  Application started" << std::endl;
    std::cout << "  INFO:User logged in" << std::endl;
    std::cout << "  ERROR Database connection failed" << std::endl;
    std::cout << "  DEBUG:Processing request #123" << std::endl;
    std::cout << "========================================" << std::endl;
}

void LoggerApp::showStatus() const {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  APPLICATION STATUS" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Log file:     " << logFile_ << std::endl;
    std::cout << "Default level: " << Logger::levelToString(defaultLevel_) << std::endl;
    std::cout << "Running:      " << (running_ ? "Yes" : "No") << std::endl;
    std::cout << "Queue size:   " << commandQueue_.size() << std::endl;
    std::cout << "Logger initialized: " << (Logger::isInitialized() ? "Yes" : "No") << std::endl;
    std::cout << "========================================" << std::endl;
}

} // namespace LoggerApp
