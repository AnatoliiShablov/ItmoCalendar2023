#include "service/details/CalendarServiceImpl.hpp"

#include "sapi/ServerInnerApi.hpp"
#include "utils/ParseUtils.hpp"

namespace shablov::details {

namespace {

constexpr static std::string_view popupStateError{"Невозможно выполнить данную комманду сейчас"};
constexpr static std::string_view popupDateHelp{"Введите дату (DD.MM.YYYY). Принимаются года, начиная с 1971"};
constexpr static std::string_view popupTimeHelp{"Введите время (HH:MM)"};
constexpr static std::string_view popupDescriptionHelp{"Введите описание"};
constexpr static std::string_view popupNotificationHelp{
    "Если требуется уведомление, напишите количество минут, за которое его прислать\n"
    "Если нет, напишите 'нет'"};

grpc::Status printHelpAndReturn(front_api::Status *status) {
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

grpc::Status printErrorStateAndReturn(front_api::Status *status, std::string_view error) {
    std::string concat{popupStateError};
    concat += '\n';
    concat += error;
    status->mutable_ok()->set_text(concat);
    return grpc::Status::OK;
}

grpc::Status nonEmptyErrorHandling(front_api::Status *status, sapi::UserInfo::State state) {
    using State = sapi::UserInfo::State;
    switch (state) {
    case State::Empty:
        throw std::exception("Non empty have to be passed");
    case State::AddingNewDate:
        return printErrorStateAndReturn(status, popupDateHelp);
    case State::AddingNewTime:
        return printErrorStateAndReturn(status, popupTimeHelp);
    case State::AddingNewDescription:
        return printErrorStateAndReturn(status, popupDescriptionHelp);
    case State::AddingNewNotification:
        return printErrorStateAndReturn(status, popupNotificationHelp);
    }

    return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, "Server version is out-of-date"};
}

}  // namespace

grpc::Status CalendarServiceImpl::ShowAll(grpc::ServerContext *context, const front_api::ShowAllRequest *request,
                                          front_api::ShowAllResponse *response) {
    try {
        auto const &currentUser = dbservice.getUserInfo(request->user().id());

        if (currentUser.state != sapi::UserInfo::State::Empty) {
            return nonEmptyErrorHandling(response->mutable_status(), currentUser.state);
        }

        auto events = dbservice.getAllByUserId(request->user().id());

        if (events.empty()) {
            response->mutable_status()->mutable_ok()->set_text("Нет текущих событий");
            return grpc::Status::OK;
        }

        for (auto const &event : events) {
            try {
                response->mutable_status()->mutable_ok()->mutable_text()->append(fmt::format(
                    "Идентификатор: {}\n"
                    "Описание:\n"
                    "{}\n"
                    "Запланированное время: {:%d.%m.%Y %H:%M}\n"
                    "\n",
                    event.eventId, event.description,
                    std::chrono::utc_clock::time_point{event.timeFromZeroUTC + currentUser.timezoneOffset}));
            } catch (...) {
            }
        }

        return grpc::Status::OK;
    } catch (std::exception const &exception) {
        fmt::print("{}\n", exception.what());
        std::flush(std::cout);
        throw;
    }
}
grpc::Status CalendarServiceImpl::StartNew(grpc::ServerContext *context, const front_api::StartNewRequest *request,
                                           front_api::StartNewResponse *response) {
    try {
        auto const &currentUser = dbservice.getUserInfo(request->user().id());

        if (currentUser.state != sapi::UserInfo::State::Empty) {
            return nonEmptyErrorHandling(response->mutable_status(), currentUser.state);
        }

        dbservice.setState(request->user().id(), sapi::UserInfo::State::AddingNewDate);

        response->mutable_status()->mutable_ok()->set_text(fmt::format("{}", popupDateHelp));

        return grpc::Status::OK;
    } catch (std::exception const &exception) {
        fmt::print("{}\n", exception.what());
        std::flush(std::cout);
        throw;
    }
}
grpc::Status CalendarServiceImpl::Remove(grpc::ServerContext *context, const front_api::RemoveRequest *request,
                                         front_api::RemoveResponse *response) {
    try {
        auto const &currentUser = dbservice.getUserInfo(request->user().id());

        if (currentUser.state != sapi::UserInfo::State::Empty) {
            return nonEmptyErrorHandling(response->mutable_status(), currentUser.state);
        }

        if (dbservice.removeEvent(request->user().id(), request->id())) {
            response->mutable_status()->mutable_ok()->set_text("Удаление произведено успешно");
            return grpc::Status::OK;
        }

        response->mutable_status()->mutable_ok()->set_text("Данного события не существует. Удаление невозможно");
        return grpc::Status::OK;
    } catch (std::exception const &exception) {
        fmt::print("{}\n", exception.what());
        std::flush(std::cout);
        throw;
    }
}
grpc::Status CalendarServiceImpl::SetTimeZone(grpc::ServerContext *context,
                                              const front_api::SetTimeZoneRequest *request,
                                              front_api::SetTimeZoneResponse *response) {
    try {
        auto const &currentUser = dbservice.getUserInfo(request->user().id());

        if (currentUser.state != sapi::UserInfo::State::Empty) {
            return nonEmptyErrorHandling(response->mutable_status(), currentUser.state);
        }

        dbservice.setTimeZone(request->user().id(), request->utcoffset().value());

        response->mutable_status()->mutable_ok()->set_text("Зона установлена");

        return grpc::Status::OK;
    } catch (std::exception const &exception) {
        fmt::print("{}\n", exception.what());
        std::flush(std::cout);
        throw;
    }
}
grpc::Status CalendarServiceImpl::AddNextArgument(grpc::ServerContext *context,
                                                  const front_api::AddNextArgumentRequest *request,
                                                  front_api::AddNextArgumentResponse *response) {
    try {
        auto const &currentUser = dbservice.getUserInfo(request->user().id());

        switch (currentUser.state) {
        case sapi::UserInfo::State::Empty:
            return printHelpAndReturn(response->mutable_status());

        case sapi::UserInfo::State::AddingNewDate:
            if (auto res = getDate(request->text()); res) {
                dbservice.setDateWorkingEvent(
                    request->user().id(),
                    std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::utc_clock::from_sys(std::chrono::sys_days(res.value())).time_since_epoch()));
                response->mutable_status()->mutable_ok()->set_text(fmt::format("{}", popupTimeHelp));

                return grpc::Status::OK;
            }
            response->mutable_status()->mutable_ok()->set_text(
                fmt::format("Введена некорректная дата.\n"
                            "{}",
                            popupDateHelp));
            return grpc::Status::OK;
        case sapi::UserInfo::State::AddingNewTime:
            if (auto res = getTime(request->text()); res) {
                dbservice.setTimeWorkingEvent(request->user().id(),
                                              std::chrono::minutes{res.value()} - currentUser.timezoneOffset);

                response->mutable_status()->mutable_ok()->set_text(fmt::format("{}", popupDescriptionHelp));
                return grpc::Status::OK;
            }
            response->mutable_status()->mutable_ok()->set_text(
                fmt::format("Введено некорректное время.\n"
                            "{}",
                            popupTimeHelp));
            return grpc::Status::OK;
        case sapi::UserInfo::State::AddingNewDescription:
            dbservice.setDescriptionWorkingEvent(request->user().id(), request->text());
            response->mutable_status()->mutable_ok()->set_text(fmt::format("{}", popupNotificationHelp));
            return grpc::Status::OK;
        case sapi::UserInfo::State::AddingNewNotification:
            if (auto res = getOffset(request->text()); res) {
                std::optional<std::chrono::seconds> value;
                dbservice.setNotificationWorkingEvent(request->user().id(), res.value());
                response->mutable_status()->mutable_ok()->set_text("Новая запись добавлена");
                return grpc::Status::OK;
            }
            response->mutable_status()->mutable_ok()->set_text(
                fmt::format("Введен некорректный период для уведомления.\n"
                            "{}",
                            popupNotificationHelp));
            return grpc::Status::OK;
        }
        return grpc::Status{grpc::StatusCode::UNIMPLEMENTED, "Server version is out-of-date"};
    } catch (std::exception const &exception) {
        fmt::print("{}\n", exception.what());
        std::flush(std::cout);
        throw;
    }
}

CalendarServiceImpl::CalendarServiceImpl(DBService &dbservice) : dbservice{dbservice} {}

}  // namespace shablov::details