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
                            event.first.id, event.second.description, event.first.tp));
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

        currentUser.newTP = std::chrono::system_clock::time_point{};
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
            if (eventIt->first.id == request->id()) {
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
                                 front_api::AddNextArgumentResponse *response) {
        auto &currentUser = usersEvents[request->user().id()];

        switch (currentUser.state) {
        case UserInfo::State::Empty:
            response->mutable_status()->mutable_ok()->set_text(
                "/help   -- для вывода этого сообщения\n"
                "\n"
                "/new    -- для создания новой записи\n"
                "/remove -- для удаления записи\n"
                "");
            break;
        case UserInfo::State::AddingNewDate:
            response->mutable_status()->mutable_ok()->set_text("Здесь будет парсинг текста. Пока что 01.10.2023");

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
    }

    grpc::Status Subscribe(grpc::ServerContext *context, const front_api::SubscribeRequest *request,
                           ::grpc::ServerWriter<front_api::SubscribeResponse> *writer) override {
        return grpc::Status::OK;
    }

private:
    struct UserInfo {
        struct Identifier {
            std::chrono::system_clock::time_point tp;
            std::uint64_t id;
        };

        struct Event {
            std::string description;
            std::optional<std::chrono::seconds> beforeNotification;
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
        std::chrono::system_clock::time_point newTP;
        std::map<Identifier, Event> events;
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
