#ifndef ITMOCALENDAR2023_NOTIFIERSERVICEIMPL_HPP
#define ITMOCALENDAR2023_NOTIFIERSERVICEIMPL_HPP

#include "fmt/chrono.h"
#include "fmt/format.h"
#include "grpcpp/server_builder.h"
#include "protocols/FrontAPI.grpc.pb.h"
#include "service/DBService.hpp"

namespace shablov::details {

class NotifierServiceImpl final: public front_api::Notifier::Service {
public:
    NotifierServiceImpl(DBService &dbservice);

    grpc::Status Subscribe(::grpc::ServerContext *context, const ::front_api::SubscribeRequest *request,
                           ::grpc::ServerWriter< ::front_api::SubscribeResponse> *writer) override;

private:
    DBService &dbservice;
};

}  // namespace shablov::details

#endif  // ITMOCALENDAR2023_NOTIFIERSERVICEIMPL_HPP
