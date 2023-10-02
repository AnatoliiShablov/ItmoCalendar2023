#include "service/CalendarService.hpp"

#include "service/details/CalendarServiceImpl.hpp"

namespace shablov {

CalendarService::CalendarService(DBService& dbservice, std::uint16_t port)
    : workingThread{[&dbservice, port]() {
        details::CalendarServiceImpl service{dbservice};
        auto server{[&service, port]() {
            grpc::ServerBuilder builder;
            builder.AddListeningPort(fmt::format("0.0.0.0:{}", port), grpc::InsecureServerCredentials());
            builder.RegisterService(&service);
            return builder.BuildAndStart();
        }()};
        fmt::print("{} listening on 0.0.0.0:{}\n", typeid(details::CalendarServiceImpl).name(), port);
        std::flush(std::cout);
        server->Wait();
    }} {}
}  // namespace shablov