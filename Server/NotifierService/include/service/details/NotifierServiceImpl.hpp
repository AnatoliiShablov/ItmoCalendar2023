#ifndef ITMOCALENDAR2023_NOTIFIERSERVICEIMPL_HPP
#define ITMOCALENDAR2023_NOTIFIERSERVICEIMPL_HPP

#include "fmt/chrono.h"
#include "fmt/format.h"
#include "grpcpp/server_builder.h"
#include "protocols/FrontAPI.grpc.pb.h"
#include "service/DBService.hpp"
#include "service/PrometheusService.hpp"

namespace shablov::details {

class NotifierServiceImpl final: public front_api::Notifier::Service {
public:
    NotifierServiceImpl(PrometheusService &prometheusservice, DBService &dbservice);

    grpc::Status Subscribe(::grpc::ServerContext *context, const ::front_api::SubscribeRequest *request,
                           ::grpc::ServerWriter< ::front_api::SubscribeResponse> *writer) override;

private:
    PrometheusService &prometheusservice;
    DBService &dbservice;
};

}  // namespace shablov::details

#endif  // ITMOCALENDAR2023_NOTIFIERSERVICEIMPL_HPP
