#include "logger.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <string>

using namespace Logger;

void workerThread(int id, int iterations) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sleepDis(1, 50);
    std::uniform_int_distribution<> levelDis(0, 2);
    for (int i = 0; i < iterations; ++i) {
        LogLevel level = static_cast<LogLevel>(levelDis(gen));
        std::string message = "[Thread " + std::to_string(id) + 
                             "] Iteration " + std::to_string(i);
        switch (level) {
            case LogLevel::DEBUG: Logger::debug(message); break;
            case LogLevel::INFO:  Logger::info(message);  break;
            case LogLevel::ERR:   Logger::error(message); break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDis(gen)));
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Logger Library Demonstration" << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (!Logger::init("app.log", LogLevel::INFO)) {
        std::cerr << "ERROR: Failed to initialize logger!" << std::endl;
        return 1;
    }
    std::cout << "Logger initialized. Default level: INFO" << std::endl;
    
    Logger::debug("Debug message - should NOT appear (level < INFO)");
    Logger::info("Info message - should appear");
    Logger::error("Error message - should appear");
    
    Logger::setDefaultLevel(LogLevel::DEBUG);
    std::cout << "Changed default level to DEBUG" << std::endl;
    Logger::debug("Debug message - should appear now");
    
    const int NUM_THREADS = 5;
    const int ITERATIONS = 20;
    std::vector<std::thread> threads;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < NUM_THREADS; ++i)
        threads.emplace_back(workerThread, i, ITERATIONS);
    for (auto& t : threads) t.join();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    std::cout << "Logged " << (NUM_THREADS * ITERATIONS) 
              << " messages in " << duration.count() << " ms" << std::endl;
    
    const int BATCH = 5000;
    start = std::chrono::steady_clock::now();
    for (int i = 0; i < BATCH; ++i)
        Logger::info("Performance test message #" + std::to_string(i));
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    std::cout << "Logged " << BATCH << " messages in " 
              << duration.count() << " ms" << std::endl;
    std::cout << "Average: " << (duration.count() * 1000.0 / BATCH) 
              << " μs per message" << std::endl;
    
    Logger::shutdown();
    std::cout << "Demo finished. Check app.log" << std::endl;
    return 0;
}
