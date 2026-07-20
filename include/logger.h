#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <memory>
#include <chrono>
#include <mutex>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

namespace Logger {

enum class LogLevel : int {
    DEBUG = 0,
    INFO = 1,
    ERR  = 2
};

class LoggerImpl {
public:
    static bool init(const std::string& filename, 
                     LogLevel defaultLevel = LogLevel::INFO);
    static void log(const std::string& message, 
                    LogLevel level = LogLevel::INFO);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void error(const std::string& message);
    static void setDefaultLevel(LogLevel level);
    static LogLevel getDefaultLevel();
    static void shutdown();
    static bool isInitialized();
    
    static std::string levelToString(LogLevel level);
    
    ~LoggerImpl() = default;

private:
    LoggerImpl() = default;
    LoggerImpl(const LoggerImpl&) = delete;
    LoggerImpl& operator=(const LoggerImpl&) = delete;
    
    bool initialize(const std::string& filename, LogLevel defaultLevel);
    void writeToFile(const std::string& message, LogLevel level);
    static std::string formatTimestamp(
        const std::chrono::system_clock::time_point& tp);
    
    static std::unique_ptr<LoggerImpl> instance_;
    static std::once_flag initFlag_;
    
    std::string logFilename_;
    LogLevel defaultLevel_ = LogLevel::INFO;
    
#ifdef _WIN32
    HANDLE logFileHandle_ = INVALID_HANDLE_VALUE;
#else
    int logFileDescriptor_ = -1;
#endif
    
    bool initialized_ = false;
    mutable std::mutex fileMutex_;
    mutable std::mutex levelMutex_;
};

inline bool init(const std::string& filename, LogLevel defaultLevel = LogLevel::INFO) {
    return LoggerImpl::init(filename, defaultLevel);
}
inline void log(const std::string& message, LogLevel level = LogLevel::INFO) {
    LoggerImpl::log(message, level);
}
inline void debug(const std::string& message) { LoggerImpl::debug(message); }
inline void info(const std::string& message)  { LoggerImpl::info(message); }
inline void error(const std::string& message) { LoggerImpl::error(message); }
inline void setDefaultLevel(LogLevel level)   { LoggerImpl::setDefaultLevel(level); }
inline LogLevel getDefaultLevel()             { return LoggerImpl::getDefaultLevel(); }
inline void shutdown()                        { LoggerImpl::shutdown(); }
inline bool isInitialized()                   { return LoggerImpl::isInitialized(); }
inline std::string levelToString(LogLevel level) { return LoggerImpl::levelToString(level); }

} // namespace Logger

#endif // LOGGER_H
