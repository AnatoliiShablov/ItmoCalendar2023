#include <iostream>

#include "service/CalendarService.hpp"
#include "service/NotifierService.hpp"
#include "service/PrometheusService.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#include "utils/ParseUtils.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        fmt::print("server (db.sqlite) (email password)\n");
        return -1;
    }

    spdlog::register_logger(
        std::make_shared<spdlog::logger>("prometheus",

                                         std::make_shared<spdlog::sinks::rotating_file_sink<std::mutex>>(

                                             "prometheus_log.txt",

                                             1024 * 1024,

                                             5,

                                             false)));

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

    std::string_view dbPath    = argv[1];
    std::string_view gmailPass = argv[2];

    if (dbPath == ":memory:") {
        spdlog::info("Starting inmemory db");
    } else {
        spdlog::info("Starting db with path {}", dbPath);
    }

    shablov::PrometheusService prometheus{gmailPass, 50050};

    shablov::DBService db{dbPath};
    shablov::CalendarService calendar{prometheus, db, 50051};
    shablov::NotifierService notifier{prometheus, db, 50052};

    return 0;
}
