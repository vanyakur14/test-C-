#include "logger.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <chrono>

using namespace Logger;

void testInit() {
    std::cout << "Testing init..." << std::endl;
    assert(Logger::init("test.log", LogLevel::INFO) == true);
    assert(Logger::isInitialized() == true);
    assert(Logger::getDefaultLevel() == LogLevel::INFO);
    Logger::shutdown();
    std::cout << "  OK" << std::endl;
}

void testLevels() {
    std::cout << "Testing levels..." << std::endl;
    Logger::init("test.log", LogLevel::INFO);
    Logger::debug("debug");
    Logger::info("info");
    Logger::error("error");
    Logger::setDefaultLevel(LogLevel::DEBUG);
    assert(Logger::getDefaultLevel() == LogLevel::DEBUG);
    Logger::debug("debug after change");
    Logger::shutdown();
    std::cout << "  OK" << std::endl;
}

void testMultithread() {
    std::cout << "Testing multithread..." << std::endl;
    Logger::init("test.log", LogLevel::DEBUG);
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i]() {
            for (int j = 0; j < 20; ++j)
                Logger::info("Thread " + std::to_string(i) + " msg " + std::to_string(j));
        });
    }
    for (auto& t : threads) t.join();
    Logger::shutdown();
    std::cout << "  OK" << std::endl;
}

int main() {
    std::cout << "=== Running tests ===" << std::endl;
    testInit();
    testLevels();
    testMultithread();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
