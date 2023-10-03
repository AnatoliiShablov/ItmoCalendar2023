#include "service/CalendarService.hpp"

#include "service/details/CalendarServiceImpl.hpp"
#include "spdlog/spdlog.h"

namespace shablov {

CalendarService::CalendarService(PrometheusService& prometheusservice,DBService& dbservice, std::uint16_t port)
    : workingThread{[&prometheusservice,&dbservice, port]() {
        details::CalendarServiceImpl service{prometheusservice, dbservice};
        auto server{[&service, port]() {
            grpc::ServerBuilder builder;
            builder.AddListeningPort(fmt::format("0.0.0.0:{}", port), grpc::InsecureServerCredentials());
            builder.RegisterService(&service);
            return builder.BuildAndStart();
        }()};
        spdlog::info("{} listening on 0.0.0.0:{}\n", typeid(details::CalendarServiceImpl).name(), port);
        server->Wait();
    }} {}
}  // namespace shablov