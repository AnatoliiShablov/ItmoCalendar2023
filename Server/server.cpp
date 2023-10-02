#include <iostream>

#include "service/CalendarService.hpp"
#include "service/NotifierService.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#include "utils/ParseUtils.hpp"

int main(int argc, char** argv) {
    spdlog::register_logger(
        std::make_shared<spdlog::logger>("db",

                                         std::make_shared<spdlog::sinks::rotating_file_sink<std::mutex>>(

                                             "db_log.txt",

                                             1024 * 1024,

                                             5,

                                             false)));

    spdlog::register_logger(
        std::make_shared<spdlog::logger>("calendar",

                                         std::make_shared<spdlog::sinks::rotating_file_sink<std::mutex>>(

                                             "calendar_log.txt",

                                             1024 * 1024,

                                             5,

                                             false)));

    spdlog::register_logger(
        std::make_shared<spdlog::logger>("notifier",

                                         std::make_shared<spdlog::sinks::rotating_file_sink<std::mutex>>(

                                             "notifier_log.txt",

                                             1024 * 1024,

                                             5,

                                             false)));

    std::string_view dbPath = argc > 1 ? argv[1] : ":memory:";

    if (dbPath == ":memory:") {
        spdlog::info("Starting inmemory db");
    } else {
        spdlog::info("Starting db with path {}", dbPath);
    }

    shablov::DBService db{dbPath};
    shablov::CalendarService calendar{db, 50051};
    shablov::NotifierService notifier{db, 50052};

    return 0;
}
