#include "service/NotifierService.hpp"

#include "service/details/NotifierServiceImpl.hpp"

namespace shablov {

NotifierService::NotifierService(DBService& dbservice, std::uint16_t port)
    : workingThread{[&dbservice, port]() {
        details::NotifierServiceImpl service{dbservice};
        auto server{[&service, port]() {
            grpc::ServerBuilder builder;
            builder.AddListeningPort(fmt::format("0.0.0.0:{}", port), grpc::InsecureServerCredentials());
            builder.RegisterService(&service);
            return builder.BuildAndStart();
        }()};
        fmt::print("{} listening on 0.0.0.0:{}\n", typeid(details::NotifierServiceImpl).name(), port);
        std::flush(std::cout);
        server->Wait();
    }} {}
}  // namespace shablov