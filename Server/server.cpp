#include <iostream>

#include "fmt/chrono.h"
#include "fmt/format.h"
#include "grpc/grpc.h"
#include "grpcpp/server_builder.h"
#include "protocols/FrontAPI.grpc.pb.h"

class BackendServer final: public front_api::Calendar::Service {
public:
    grpc::Status ShowAll(grpc::ServerContext *context, const front_api::ShowAllRequest *request,
                         front_api::ShowAllResponse *response) override {
        auto const &currentUser = usersEvents[request->user().id()];

        switch (currentUser.state) {
        case UserInfo::State::Empty:
            break;
        case UserInfo::State::AddingNewDate:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Введите дату");
            return grpc::Status::OK;
        case UserInfo::State::AddingNewTime:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Введите время");
            return grpc::Status::OK;
        case UserInfo::State::AddingNewDescription:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Введите описание");
            return grpc::Status::OK;
        case UserInfo::State::AddingNewNotification:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Если требуется уведомление, напишите количество минут, за которое его прислать\n"
                "Если нет, напишите 'нет'");
            return grpc::Status::OK;
        }

        if (currentUser.events.empty()) {
            response->mutable_status()->mutable_ok()->set_text("Нет текущих событий");
            return grpc::Status::OK;
        }

        for (auto const &event : currentUser.events) {
            response->mutable_status()->mutable_ok()->mutable_text()->append(
                fmt::format("{}. {}\n"
                            "    {}\n"
                            "\n",
                            event.first, event.second.description,
                            std::chrono::clock_cast<std::chrono::system_clock>(event.second.tp)));
        }

        return grpc::Status::OK;
    }

    grpc::Status StartNew(grpc::ServerContext *context, const front_api::StartNewRequest *request,
                          front_api::StartNewResponse *response) override {
        auto &currentUser = usersEvents[request->user().id()];

        switch (currentUser.state) {
        case UserInfo::State::Empty:
            break;
        case UserInfo::State::AddingNewDate:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Введите дату");
            return grpc::Status::OK;
        case UserInfo::State::AddingNewTime:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Введите время");
            return grpc::Status::OK;
        case UserInfo::State::AddingNewDescription:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Введите описание");
            return grpc::Status::OK;
        case UserInfo::State::AddingNewNotification:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Если требуется уведомление, напишите количество минут, за которое его прислать\n"
                "Если нет, напишите 'нет'");
            return grpc::Status::OK;
        }

        response->mutable_status()->mutable_ok()->set_text("Введите дату");

        currentUser.state = UserInfo::State::AddingNewDate;
        return grpc::Status::OK;
    }

    grpc::Status StartEdit(grpc::ServerContext *context, const front_api::StartEditRequest *request,
                           front_api::StartEditResponse *response) override {
        return grpc::Status::OK;
    }

    grpc::Status StartRemove(grpc::ServerContext *context, const front_api::StartRemoveRequest *request,
                             front_api::StartRemoveResponse *response) override {
        auto &currentUser = usersEvents[request->user().id()];

        switch (currentUser.state) {
        case UserInfo::State::Empty:
            break;
        case UserInfo::State::AddingNewDate:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Введите дату");
            return grpc::Status::OK;
        case UserInfo::State::AddingNewTime:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Введите время");
            return grpc::Status::OK;
        case UserInfo::State::AddingNewDescription:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Введите описание");
            return grpc::Status::OK;
        case UserInfo::State::AddingNewNotification:
            response->mutable_status()->mutable_ok()->set_text(
                "Невозможно выполнить данную комманду сейчас.\n"
                "Если требуется уведомление, напишите количество минут, за которое его прислать\n"
                "Если нет, напишите 'нет'");
            return grpc::Status::OK;
        }

        if (currentUser.events.empty()) {
            response->mutable_status()->mutable_ok()->set_text("Нет текущих событий. Удаление невозможно");
            return grpc::Status::OK;
        }

        for (auto eventIt{currentUser.events.begin()}; eventIt != currentUser.events.end(); ++eventIt) {
            if (eventIt->first == request->id()) {
                response->mutable_status()->mutable_ok()->set_text("Удаление произведено успешно");
                currentUser.events.erase(eventIt);
                return grpc::Status::OK;
            }
        }

        response->mutable_status()->mutable_ok()->set_text("Данного события не существует. Удаление невозможно");
        return grpc::Status::OK;
    }

    grpc::Status SetLanguage(grpc::ServerContext *context, const front_api::SetLanguageRequest *request,
                             front_api::SetLanguageResponse *response) override {
        return grpc::Status::OK;
    }

    grpc::Status SetTimeZone(grpc::ServerContext *context, const front_api::SetTimeZoneRequest *request,
                             front_api::SetTimeZoneResponse *response) override {
        return grpc::Status::OK;
    }

    grpc::Status AddNextArgument(grpc::ServerContext *context, const front_api::AddNextArgumentRequest *request,
                                 front_api::AddNextArgumentResponse *response) override {
        auto &currentUser = usersEvents[request->user().id()];

        switch (currentUser.state) {
        case UserInfo::State::Empty:
            response->mutable_status()->mutable_ok()->set_text(
                "/help -- для вывода этого сообщения\n"
                "\n"
                "/showall --для вывода существующих событий"
                "\n"
                "/new -- для создания новой записи\n"
                "/remove [id] -- для удаления записи\n"
                "");
            return grpc::Status::OK;
        case UserInfo::State::AddingNewDate:
            response->mutable_status()->mutable_ok()->set_text(
                "Здесь будет парсинг текста. Пока что 01.10.2023\n"
                "Пожалуйста введите время");
            currentUser.inprocessEvent.tp = std::chrono::utc_clock::from_sys(std::chrono::sys_days(
                std::chrono::year_month_day{std::chrono::year{2023}, std::chrono::month{1}, std::chrono::day{10}}));
            currentUser.state             = UserInfo::State::AddingNewTime;
            return grpc::Status::OK;
        case UserInfo::State::AddingNewTime:
            response->mutable_status()->mutable_ok()->set_text(
                "Здесь будет парсинг текста. Пока что 14:00\n"
                "Пожалуйста введите описание");
            currentUser.inprocessEvent.tp += std::chrono::hours(14);
            currentUser.inprocessEvent.tp += std::chrono::minutes{0};
            currentUser.inprocessEvent.tp -= std::chrono::hours{3};

            currentUser.state = UserInfo::State::AddingNewDescription;
            return grpc::Status::OK;
        case UserInfo::State::AddingNewDescription:
            response->mutable_status()->mutable_ok()->set_text(
                "Если требуется уведомление, напишите количество минут, за которое его прислать\n"
                "Если нет, напишите 'нет'");
            currentUser.inprocessEvent.description = request->text();

            currentUser.state = UserInfo::State::AddingNewNotification;
            return grpc::Status::OK;
        case UserInfo::State::AddingNewNotification:
            std::int64_t value;
            if (request->text() != "нет") {
                if (auto res = std::from_chars(request->text().c_str(),
                                               request->text().c_str() + request->text().size(), value);
                    res.ec != std::errc{} || res.ptr != request->text().c_str() + request->text().size() || value < 0) {
                    response->mutable_status()->mutable_ok()->set_text(
                        "Не смог прочитать необходимое количество минут. Повторите пожалуйста");
                    return grpc::Status::OK;
                }

                currentUser.inprocessEvent.notification = currentUser.inprocessEvent.tp - std::chrono::minutes{value};
            }
            response->mutable_status()->mutable_ok()->set_text("Новая запись добавлена");
            currentUser.events.emplace(++currentUser.maxId, std::move(currentUser.inprocessEvent));
            currentUser.state = UserInfo::State::Empty;
            return grpc::Status::OK;
        }
        return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, "Server version is out-of-date"};
    }

    grpc::Status Subscribe(grpc::ServerContext *context, const front_api::SubscribeRequest *request,
                           ::grpc::ServerWriter<front_api::SubscribeResponse> *writer) override {
        return grpc::Status::OK;
    }

private:
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

        Event inprocessEvent;
        std::map<std::uint64_t, Event> events;
    };

    std::unordered_map<std::int64_t, UserInfo> usersEvents;
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
