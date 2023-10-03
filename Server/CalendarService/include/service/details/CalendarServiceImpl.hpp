#ifndef ITMOCALENDAR2023_CALENDARSERVICEIMPL_HPP
#define ITMOCALENDAR2023_CALENDARSERVICEIMPL_HPP

#include "fmt/chrono.h"
#include "fmt/format.h"
#include "grpcpp/server_builder.h"
#include "protocols/FrontAPI.grpc.pb.h"
#include "service/DBService.hpp"
#include "service/PrometheusService.hpp"

namespace shablov::details {

class CalendarServiceImpl final: public front_api::Calendar::Service {
public:
    CalendarServiceImpl(PrometheusService &prometheusservice, DBService &dbservice);

    grpc::Status ShowAll(grpc::ServerContext *context, const front_api::ShowAllRequest *request,
                         front_api::ShowAllResponse *response) override;

    grpc::Status StartNew(grpc::ServerContext *context, const front_api::StartNewRequest *request,
                          front_api::StartNewResponse *response) override;

    grpc::Status Remove(grpc::ServerContext *context, const front_api::RemoveRequest *request,
                        front_api::RemoveResponse *response) override;

    grpc::Status SetTimeZone(grpc::ServerContext *context, const front_api::SetTimeZoneRequest *request,
                             front_api::SetTimeZoneResponse *response) override;

    grpc::Status AddNextArgument(grpc::ServerContext *context, const front_api::AddNextArgumentRequest *request,
                                 front_api::AddNextArgumentResponse *response) override;

private:
    PrometheusService &prometheusservice;
    DBService &dbservice;
};

}  // namespace shablov::details

#endif  // ITMOCALENDAR2023_CALENDARSERVICEIMPL_HPP
