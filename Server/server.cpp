#include <iostream>

#include "fmt/format.h"
#include "grpc/grpc.h"
#include "grpcpp/server_builder.h"
#include "protocols/FrontAPI.grpc.pb.h"

class BackendServer final: public front_api::Calendar::Service {
public:
    grpc::Status ShowAll(grpc::ServerContext *context, const front_api::ShowAllRequest *request,
                         front_api::ShowAllResponse *response) override;
    grpc::Status StartNew(grpc::ServerContext *context, const front_api::StartNewRequest *request,
                          front_api::StartNewResponse *response) override;
    grpc::Status StartEdit(grpc::ServerContext *context, const front_api::StartEditRequest *request,
                           front_api::StartEditResponse *response) override;
    grpc::Status StartRemove(grpc::ServerContext *context, const front_api::StartRemoveRequest *request,
                             front_api::StartRemoveResponse *response) override;
    grpc::Status SetLanguage(grpc::ServerContext *context, const front_api::SetLanguageRequest *request,
                             front_api::SetLanguageResponse *response) override;
    grpc::Status SetTimeZone(grpc::ServerContext *context, const front_api::SetTimeZoneRequest *request,
                             front_api::SetTimeZoneResponse *response) override;
    grpc::Status Subscribe(grpc::ServerContext *context, const front_api::SubscribeRequest *request,
                           ::grpc::ServerWriter<front_api::SubscribeResponse> *writer) override;
};

int main() {
    BackendServer service;
    auto server{[&service]() {
        grpc::ServerBuilder builder;
        builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        return builder.BuildAndStart();
    }()};

    fmt::print("Server listening on 0.0.0.0:50051");
    std::flush(std::cout);
    server->Wait();
    return 0;
}
