#include "service/details/NotifierServiceImpl.hpp"

#include "sapi/ServerInnerApi.hpp"
#include "spdlog/spdlog.h"

namespace shablov::details {

NotifierServiceImpl::NotifierServiceImpl(DBService &dbservice) : dbservice{dbservice} {}

grpc::Status NotifierServiceImpl::Subscribe(::grpc::ServerContext *context,
                                            const ::front_api::SubscribeRequest *request,
                                            ::grpc::ServerWriter< ::front_api::SubscribeResponse> *writer) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds{10});

        auto res = dbservice.getAndRemoveNotifications();

        for (auto const &event : res) {
            auto offset = dbservice.getUserInfo(event.userId).timezoneOffset;

            spdlog::get("notifier")->debug("Notifying user: userId: {}. eventId: {}", event.userId, event.eventId);

            front_api::SubscribeResponse response;
            response.mutable_user()->set_id(event.userId);
            response.set_text(fmt::format(
                "Напоминаю:\n"
                "\n"
                "Идентификатор: {}\n"
                "Описание:\n"
                "{}\n"
                "Запланированное время: {:%d.%m.%Y %H:%M}",
                event.eventId, event.description, std::chrono::utc_clock::time_point{event.timeFromZeroUTC + offset}));
            writer->Write(response);
        }
    }
}

}  // namespace shablov::details
