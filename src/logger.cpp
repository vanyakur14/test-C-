#include "logger.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cerrno>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>
#endif

namespace Logger {

std::unique_ptr<LoggerImpl> LoggerImpl::instance_ = nullptr;
std::once_flag LoggerImpl::initFlag_;

bool LoggerImpl::init(const std::string& filename, LogLevel defaultLevel) {
    bool result = false;
    std::call_once(initFlag_, [&]() {
        instance_.reset(new LoggerImpl());
        result = instance_->initialize(filename, defaultLevel);
    });
    return result;
}

bool LoggerImpl::initialize(const std::string& filename, LogLevel defaultLevel) {
    logFilename_ = filename;
    defaultLevel_ = defaultLevel;
    initialized_ = false;
    
#ifdef _WIN32
    HANDLE hFile = CreateFileA(
        filename.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "ERROR: Failed to open log file '" << filename 
                  << "' (error: " << GetLastError() << ")" << std::endl;
        return false;
    }
    SetFilePointer(hFile, 0, nullptr, FILE_END);
    logFileHandle_ = hFile;
#else
    logFileDescriptor_ = open(filename.c_str(),
        O_WRONLY | O_CREAT | O_APPEND,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (logFileDescriptor_ < 0) {
        std::cerr << "ERROR: Failed to open log file '" << filename 
                  << "' (error: " << std::strerror(errno) << ")" << std::endl;
        return false;
    }
#endif
    
    initialized_ = true;
    writeToFile("=== Logger initialized ===", LogLevel::INFO);
    writeToFile("Default log level: " + levelToString(defaultLevel), LogLevel::INFO);
    return true;
}

void LoggerImpl::log(const std::string& message, LogLevel level) {
    if (!instance_ || !instance_->initialized_) {
        std::cerr << "ERROR: Logger not initialized. Call Logger::init() first." 
                  << std::endl;
        return;
    }
    if (level < instance_->getDefaultLevel()) return;
    instance_->writeToFile(message, level);
}

void LoggerImpl::debug(const std::string& message) { log(message, LogLevel::DEBUG); }
void LoggerImpl::info(const std::string& message)  { log(message, LogLevel::INFO); }
void LoggerImpl::error(const std::string& message) { log(message, LogLevel::ERR); }

void LoggerImpl::setDefaultLevel(LogLevel level) {
    if (!instance_ || !instance_->initialized_) {
        std::cerr << "ERROR: Logger not initialized." << std::endl;
        return;
    }
    std::lock_guard<std::mutex> lock(instance_->levelMutex_);
    LogLevel old = instance_->defaultLevel_;
    instance_->defaultLevel_ = level;
    instance_->writeToFile(
        "Log level changed: " + levelToString(old) + " -> " + levelToString(level),
        LogLevel::INFO
    );
}

LogLevel LoggerImpl::getDefaultLevel() {
    if (!instance_ || !instance_->initialized_) return LogLevel::INFO;
    std::lock_guard<std::mutex> lock(instance_->levelMutex_);
    return instance_->defaultLevel_;
}

void LoggerImpl::shutdown() {
    if (instance_ && instance_->initialized_) {
        instance_->writeToFile("=== Logger shutdown ===", LogLevel::INFO);
#ifdef _WIN32
        if (instance_->logFileHandle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(instance_->logFileHandle_);
            instance_->logFileHandle_ = INVALID_HANDLE_VALUE;
        }
#else
        close(instance_->logFileDescriptor_);
        instance_->logFileDescriptor_ = -1;
#endif
        instance_->initialized_ = false;
        instance_.reset();
    }
}

bool LoggerImpl::isInitialized() {
    return instance_ && instance_->initialized_;
}

void LoggerImpl::writeToFile(const std::string& message, LogLevel level) {
    std::lock_guard<std::mutex> lock(fileMutex_);
    auto now = std::chrono::system_clock::now();
    std::string formatted = formatTimestamp(now) + 
                           " [" + levelToString(level) + "] " + 
                           message + "\n";
#ifdef _WIN32
    DWORD written;
    WriteFile(logFileHandle_, formatted.c_str(), 
              static_cast<DWORD>(formatted.length()), &written, nullptr);
#else
    write(logFileDescriptor_, formatted.c_str(), formatted.length());
#endif
}

std::string LoggerImpl::levelToString(LogLevel level) {
    switch(level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::ERR:   return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string LoggerImpl::formatTimestamp(
    const std::chrono::system_clock::time_point& tp) {
    auto time = std::chrono::system_clock::to_time_t(tp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()) % 1000;
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
       << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

} // namespace Logger