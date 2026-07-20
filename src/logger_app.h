#ifndef LOGGER_APP_H
#define LOGGER_APP_H

#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include "logger.h"

namespace LoggerApp {

struct LogCommand {
    std::string message;
    Logger::LogLevel level;
    bool isShutdown = false;
    LogCommand() = default;
    LogCommand(const std::string& msg, Logger::LogLevel lvl, bool shutdown = false)
        : message(msg), level(lvl), isShutdown(shutdown) {}
};

class LoggerApp {
public:
    LoggerApp(const std::string& logFile, Logger::LogLevel defaultLevel);
    ~LoggerApp();
    int run();
    void stop();

private:
    void workerThread();
    bool processInput();
    bool parseCommand(const std::string& input, LogCommand& command);
    bool stringToLevel(const std::string& levelStr, Logger::LogLevel& level);
    void showHelp() const;
    void showStatus() const;

    std::string logFile_;
    Logger::LogLevel defaultLevel_;
    std::atomic<bool> running_;
    std::queue<LogCommand> commandQueue_;
    mutable std::mutex queueMutex_;
    std::condition_variable queueCond_;
    std::thread worker_;
};

} // namespace LoggerApp

#endif // LOGGER_APP_H