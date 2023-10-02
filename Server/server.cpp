#include <iostream>

#include "fmt/chrono.h"
#include "fmt/format.h"
#include "grpcpp/server_builder.h"
#include "protocols/FrontAPI.grpc.pb.h"
#include "utils/ParseUtils.hpp"

struct UserInfo {
    struct Event {
        std::chrono::utc_clock::time_point tp;
        std::string description;
        std::optional<std::chrono::utc_clock::time_point> notification;
    };

    enum class State {
        //
        Empty,
        // New
        AddingNewDate,
        AddingNewTime,
        AddingNewDescription,
        AddingNewNotification,
    };

    std::uint64_t maxId{0};
    State state{State::Empty};
    std::chrono::minutes utcOffset{180};

    Event inprocessEvent;
    std::map<std::uint64_t, Event> events;
};

std::unordered_map<std::int64_t, UserInfo> usersEvents;

class CalendarService final: public front_api::Calendar::Service {
public:
    grpc::Status ShowAll(grpc::ServerContext *context, const front_api::ShowAllRequest *request,
                         front_api::ShowAllResponse *response) override {
        auto const &currentUser = usersEvents[request->user().id()];

        if (currentUser.state != UserInfo::State::Empty) {
            return nonEmptyErrorHandling(response->mutable_status(), currentUser.state);
        }

        if (currentUser.events.empty()) {
            response->mutable_status()->mutable_ok()->set_text("Нет текущих событий");
            return grpc::Status::OK;
        }

        for (auto const &[eventId, event] : currentUser.events) {
            try {
                response->mutable_status()->mutable_ok()->mutable_text()->append(
                    fmt::format("Идентификатор: {}\n"
                                "Описание:\n"
                                "{}\n"
                                "Запланированное время: {:%d.%m.%Y %H:%M}\n"
                                "\n",
                                eventId, event.description, event.tp + currentUser.utcOffset));
            } catch (...) {
            }
        }

        return grpc::Status::OK;
    }

    grpc::Status StartNew(grpc::ServerContext *context, const front_api::StartNewRequest *request,
                          front_api::StartNewResponse *response) override {
        auto &currentUser = usersEvents[request->user().id()];

        if (currentUser.state != UserInfo::State::Empty) {
            return nonEmptyErrorHandling(response->mutable_status(), currentUser.state);
        }

        response->mutable_status()->mutable_ok()->set_text(fmt::format("{}", popupDateHelp));

        currentUser.state = UserInfo::State::AddingNewDate;
        return grpc::Status::OK;
    }

    grpc::Status Remove(grpc::ServerContext *context, const front_api::RemoveRequest *request,
                        front_api::RemoveResponse *response) override {
        auto &currentUser = usersEvents[request->user().id()];

        if (currentUser.state != UserInfo::State::Empty) {
            return nonEmptyErrorHandling(response->mutable_status(), currentUser.state);
        }

        if (currentUser.events.empty()) {
            response->mutable_status()->mutable_ok()->set_text("Нет текущих событий. Удаление невозможно");
            return grpc::Status::OK;
        }

        for (auto eventIt{currentUser.events.begin()}; eventIt != currentUser.events.end(); ++eventIt) {
            if (eventIt->first == request->id()) {
                currentUser.events.erase(eventIt);

                response->mutable_status()->mutable_ok()->set_text("Удаление произведено успешно");
                return grpc::Status::OK;
            }
        }

        response->mutable_status()->mutable_ok()->set_text("Данного события не существует. Удаление невозможно");
        return grpc::Status::OK;
    }

    grpc::Status SetTimeZone(grpc::ServerContext *context, const front_api::SetTimeZoneRequest *request,
                             front_api::SetTimeZoneResponse *response) override {
        auto &currentUser = usersEvents[request->user().id()];

        if (currentUser.state != UserInfo::State::Empty) {
            return nonEmptyErrorHandling(response->mutable_status(), currentUser.state);
        }

        currentUser.utcOffset = std::chrono::minutes{request->utcoffset().value()};

        response->mutable_status()->mutable_ok()->set_text("Зона установлена");

        return grpc::Status::OK;
    }

    grpc::Status AddNextArgument(grpc::ServerContext *context, const front_api::AddNextArgumentRequest *request,
                                 front_api::AddNextArgumentResponse *response) override {
        auto &currentUser = usersEvents[request->user().id()];

        switch (currentUser.state) {
        case UserInfo::State::Empty:
            return printHelpAndReturn(response->mutable_status());

        case UserInfo::State::AddingNewDate:
            if (auto res = getDate(request->text()); res) {
                currentUser.inprocessEvent.tp = std::chrono::utc_clock::from_sys(std::chrono::sys_days(res.value()));
                response->mutable_status()->mutable_ok()->set_text(fmt::format("{}", popupTimeHelp));
                currentUser.state = UserInfo::State::AddingNewTime;
                return grpc::Status::OK;
            }
            response->mutable_status()->mutable_ok()->set_text(
                fmt::format("Введена некорректная дата.\n"
                            "{}",
                            popupDateHelp));
            return grpc::Status::OK;
        case UserInfo::State::AddingNewTime:
            if (auto res = getTime(request->text()); res) {
                currentUser.inprocessEvent.tp += res.value();
                currentUser.inprocessEvent.tp -= currentUser.utcOffset;
                response->mutable_status()->mutable_ok()->set_text(fmt::format("{}", popupDescriptionHelp));
                currentUser.state = UserInfo::State::AddingNewDescription;
                return grpc::Status::OK;
            }
            response->mutable_status()->mutable_ok()->set_text(
                fmt::format("Введено некорректное время.\n"
                            "{}",
                            popupTimeHelp));
            return grpc::Status::OK;
        case UserInfo::State::AddingNewDescription:
            currentUser.inprocessEvent.description = request->text();
            response->mutable_status()->mutable_ok()->set_text(fmt::format("{}", popupNotificationHelp));
            currentUser.state = UserInfo::State::AddingNewNotification;
            return grpc::Status::OK;
        case UserInfo::State::AddingNewNotification:
            if (auto res = getOffset(request->text()); res) {
                if (res.value()) {
                    currentUser.inprocessEvent.notification =
                        currentUser.inprocessEvent.tp - std::chrono::minutes{res.value().value()};
                }
                currentUser.events.emplace(++currentUser.maxId, std::move(currentUser.inprocessEvent));
                response->mutable_status()->mutable_ok()->set_text("Новая запись добавлена");
                currentUser.state = UserInfo::State::Empty;
                return grpc::Status::OK;
            }
            response->mutable_status()->mutable_ok()->set_text(
                fmt::format("Введен некорректный период для уведомления.\n"
                            "{}",
                            popupNotificationHelp));
            return grpc::Status::OK;
        }
        return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, "Server version is out-of-date"};
    }

private:
    static grpc::Status printHelpAndReturn(front_api::Status *status) {
        status->mutable_ok()->set_text(
            "/help -- для вывода этого сообщения\n"
            "\n"
            "/showall --для вывода существующих событий"
            "\n"
            "/new -- для создания новой записи\n"
            "/remove (id) -- для удаления записи\n"
            "\n"
            "/settimezone (minutes) -- смещение от UTC+00.00 в минутах. По умолчанию UTC+03.00");
        return grpc::Status::OK;
    }

    static grpc::Status printErrorStateAndReturn(front_api::Status *status, std::string_view error) {
        std::string concat{popupStateError};
        concat += '\n';
        concat += error;
        status->mutable_ok()->set_text(concat);
        return grpc::Status::OK;
    }

    constexpr static std::string_view popupStateError{"Невозможно выполнить данную комманду сейчас"};
    constexpr static std::string_view popupDateHelp{"Введите дату (DD.MM.YYYY). Принимаются года, начиная с 1971"};
    constexpr static std::string_view popupTimeHelp{"Введите время (HH:MM)"};
    constexpr static std::string_view popupDescriptionHelp{"Введите описание"};
    constexpr static std::string_view popupNotificationHelp{
        "Если требуется уведомление, напишите количество минут, за которое его прислать\n"
        "Если нет, напишите 'нет'"};

    static grpc::Status nonEmptyErrorHandling(front_api::Status *status, UserInfo::State state) {
        switch (state) {
        case UserInfo::State::Empty:
            throw std::exception("Non empty have to be passed");
        case UserInfo::State::AddingNewDate:
            return printErrorStateAndReturn(status, popupDateHelp);
        case UserInfo::State::AddingNewTime:
            return printErrorStateAndReturn(status, popupTimeHelp);
        case UserInfo::State::AddingNewDescription:
            return printErrorStateAndReturn(status, popupDescriptionHelp);
        case UserInfo::State::AddingNewNotification:
            return printErrorStateAndReturn(status, popupNotificationHelp);
        }

        return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, "Server version is out-of-date"};
    }
};

class NotifierService final: public front_api::Notifier::Service {
public:
    grpc::Status Subscribe(::grpc::ServerContext *context, const ::front_api::SubscribeRequest *request,
                           ::grpc::ServerWriter< ::front_api::SubscribeResponse> *writer) override {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds{1});

            for (auto &[userId, userInfo] : usersEvents) {
                for (auto &[taskId, event] : userInfo.events) {
                    if (event.notification) {
                        if (std::chrono::utc_clock::now() >= event.notification) {
                            front_api::SubscribeResponse response;
                            response.mutable_user()->set_id(userId);
                            response.set_text(
                                fmt::format("Напоминаю:\n"
                                            "\n"
                                            "Идентификатор: {}\n"
                                            "Описание:\n"
                                            "{}\n"
                                            "Запланированное время: {:%d.%m.%Y %H:%M}",
                                            taskId, event.description, event.tp + userInfo.utcOffset));
                            writer->Write(response);
                            event.notification = std::nullopt;
                        }
                    }
                }
            }
        }
    }
};

template <typename Service>
std::jthread startService(std::uint16_t port) {
    return std::jthread{[port]() {
        Service service;
        auto server{[&service, port]() {
            grpc::ServerBuilder builder;
            builder.AddListeningPort(fmt::format("0.0.0.0:{}", port), grpc::InsecureServerCredentials());
            builder.RegisterService(&service);
            return builder.BuildAndStart();
        }()};
        fmt::print("{} listening on 0.0.0.0:{}", typeid(Service).name(), port);
        std::flush(std::cout);
        server->Wait();
    }};
}

int main() {
    auto calendar{startService<CalendarService>(50051)};
    auto notifier{startService<NotifierService>(50052)};

    return 0;
}
