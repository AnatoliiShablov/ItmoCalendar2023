#include "service/DBService.hpp"

#include <chrono>
#include <mutex>
#include <source_location>

#include "fmt/chrono.h"
#include "fmt/format.h"
#include "spdlog/spdlog.h"
#include "sqlite_orm/sqlite_orm.h"

namespace shablov {

namespace sql = sqlite_orm;

#define STORAGE_SCHEME                                                                                          \
    sql::make_table(                                                                                            \
                                                                                                                \
        "events",                                                                                               \
                                                                                                                \
        sql::make_column("eventId", &sapi::Event::eventId, sql::primary_key().autoincrement()),                 \
                                                                                                                \
        sql::make_column("userId", &sapi::Event::userId),                                                       \
                                                                                                                \
        sql::make_column("description", &sapi::Event::description),                                             \
                                                                                                                \
        sql::make_column("timeFromZeroUTC", &sapi::Event::getTimeFromZeroUTC, &sapi::Event::setTimeFromZeroUTC) \
                                                                                                                \
            ),                                                                                                  \
                                                                                                                \
        sql::make_table(                                                                                        \
                                                                                                                \
            "notifications",                                                                                    \
                                                                                                                \
            sql::make_column("eventId", &sapi::NotificationEvent::eventId, sql::unique(), sql::primary_key()),  \
                                                                                                                \
            sql::make_column("secondsFromZeroUTC", &sapi::NotificationEvent::secondsFromZeroUTC)                \
                                                                                                                \
                ),                                                                                              \
                                                                                                                \
        sql::make_table(                                                                                        \
                                                                                                                \
            "user_info",                                                                                        \
                                                                                                                \
            sql::make_column("userId", &sapi::UserInfo::userId, sql::unique(), sql::primary_key()),             \
                                                                                                                \
            sql::make_column("timezoneOffset", &sapi::UserInfo::getTimezoneOffset,                              \
                             &sapi::UserInfo::setTimezoneOffset),                                               \
                                                                                                                \
            sql::make_column("state", &sapi::UserInfo::getState, &sapi::UserInfo::setState),                    \
                                                                                                                \
            sql::make_column("workingDescription", &sapi::UserInfo::workingDescription),                        \
                                                                                                                \
            sql::make_column("workingTimeFromZeroUTC", &sapi::UserInfo::getWorkingTimeFromZeroUTC,              \
                             &sapi::UserInfo::setWorkingTimeFromZeroUTC)                                        \
                                                                                                                \
        )

struct DBService::Impl {
    Impl(std::string_view path)

        : storage{sql::make_storage(path.data(), STORAGE_SCHEME)} {
        storage.sync_schema();
        spdlog::get("db")->info("Schema synced");
    }

    decltype(sql::make_storage(std::declval<char const*>(), STORAGE_SCHEME)) storage;
    std::recursive_mutex storageMutex;
};

#undef STORAGE_SCHEME

DBService::DBService(std::string_view path) : _impl{std::make_unique<Impl>(path)} {}

std::vector<sapi::Event> DBService::getAllByUserId(sapi::UserId userId) {
    using namespace sql;
    spdlog::get("db")->debug("{} entered. userId: {}", std::source_location().function_name(), userId);
    std::lock_guard lock{_impl->storageMutex};

    auto result = _impl->storage.get_all<sapi::Event>(where(c(&sapi::Event::userId) == userId));
    spdlog::get("db")->debug("{} exited", std::source_location().function_name());

    return result;
}

sapi::UserInfo DBService::getUserInfo(sapi::UserId userId) {
    using namespace sql;
    spdlog::get("db")->debug("{} entered. userId: {}", std::source_location().function_name(), userId);
    std::lock_guard lock{_impl->storageMutex};
    if (_impl->storage.count<sapi::UserInfo>(where(c(&sapi::UserInfo::userId) == userId)) == 0) {
        spdlog::get("db")->info("{} new user in DB", std::source_location().function_name());
        _impl->storage.replace(sapi::UserInfo{
            .userId = userId, .timezoneOffset = std::chrono::hours{3}, .state = sapi::UserInfo::State::Empty});
    }

    auto result = _impl->storage.get<sapi::UserInfo>(userId);

    spdlog::get("db")->debug("{} exited", std::source_location().function_name());

    return result;
}

void DBService::setState(sapi::UserId userId, sapi::UserInfo::State state) {
    spdlog::get("db")->debug("{} entered. userId: {}, state: {}", std::source_location().function_name(), userId,
                             static_cast<std::uint32_t>(state));

    std::lock_guard lock{_impl->storageMutex};
    auto userInfo  = getUserInfo(userId);
    userInfo.state = state;
    _impl->storage.replace(userInfo);

    spdlog::get("db")->debug("{} exited", std::source_location().function_name());
}

bool DBService::removeEvent(sapi::UserId userId, sapi::EventId eventId) {
    spdlog::get("db")->debug("{} entered. userId: {}, eventId: {}", std::source_location().function_name(), userId,
                             eventId);
    using namespace sql;

    std::lock_guard lock{_impl->storageMutex};

    if (_impl->storage.count<sapi::Event>(
            where((c(&sapi::Event::userId) == userId && c(&sapi::Event::eventId) == eventId))) == 0) {
        spdlog::get("db")->debug("{}. No such event", std::source_location().function_name());
        return false;
    }

    _impl->storage.remove<sapi::Event>(eventId);
    _impl->storage.remove<sapi::NotificationEvent>(eventId);

    spdlog::get("db")->debug("{}. Event removed", std::source_location().function_name());

    return true;
}

void DBService::setTimeZone(sapi::UserId userId, std::int64_t minutes) {
    spdlog::get("db")->debug("{} entered. minutes: {}", std::source_location().function_name(), minutes);
    std::lock_guard lock{_impl->storageMutex};

    auto userInfo = getUserInfo(userId);
    userInfo.setTimezoneOffset(minutes);
    _impl->storage.replace(userInfo);

    spdlog::get("db")->debug("{} exited", std::source_location().function_name());
}

void DBService::setDateWorkingEvent(sapi::UserId userId, std::chrono::seconds timeFromZeroUtc) {
    spdlog::get("db")->debug("{} entered. timeFromZeroUtc: {}", std::source_location().function_name(),
                             timeFromZeroUtc);
    std::lock_guard lock{_impl->storageMutex};

    auto userInfo = getUserInfo(userId);

    userInfo.state                  = sapi::UserInfo::State::AddingNewTime;
    userInfo.workingTimeFromZeroUTC = timeFromZeroUtc;

    _impl->storage.replace(userInfo);
    spdlog::get("db")->debug("{} exited", std::source_location().function_name());
}

void DBService::setTimeWorkingEvent(sapi::UserId userId, std::chrono::seconds timeOfDay) {
    spdlog::get("db")->debug("{} entered. timeOfDay: {}", std::source_location().function_name(), timeOfDay);
    std::lock_guard lock{_impl->storageMutex};

    auto userInfo = getUserInfo(userId);

    userInfo.state = sapi::UserInfo::State::AddingNewDescription;
    userInfo.workingTimeFromZeroUTC += timeOfDay;

    _impl->storage.replace(userInfo);
    spdlog::get("db")->debug("{} exited", std::source_location().function_name());
}

void DBService::setDescriptionWorkingEvent(sapi::UserId userId, std::string description) {
    spdlog::get("db")->debug("{} entered. description: {}", std::source_location().function_name(), description);
    std::lock_guard lock{_impl->storageMutex};

    auto userInfo = getUserInfo(userId);

    userInfo.state              = sapi::UserInfo::State::AddingNewNotification;
    userInfo.workingDescription = std::move(description);

    _impl->storage.replace(userInfo);
    spdlog::get("db")->debug("{} exited", std::source_location().function_name());
}

void DBService::setNotificationWorkingEvent(sapi::UserId userId, std::optional<std::uint64_t> offset) {
    spdlog::get("db")->debug("{} entered", std::source_location().function_name());
    std::lock_guard lock{_impl->storageMutex};

    auto userInfo = getUserInfo(userId);

    sapi::EventId eventId =
        _impl->storage.insert<sapi::Event>(sapi::Event{.userId          = userId,
                                                       .description     = userInfo.workingDescription,
                                                       .timeFromZeroUTC = userInfo.workingTimeFromZeroUTC});
    if (offset) {
        _impl->storage.insert<sapi::NotificationEvent>(sapi::NotificationEvent{
            .eventId            = eventId,
            .secondsFromZeroUTC = static_cast<std::uint64_t>(
                std::chrono::seconds{userInfo.workingTimeFromZeroUTC - std::chrono::minutes{offset.value()}}.count())});
        spdlog::get("db")->debug("{}. Notification event added. Offset: {} minutes",
                                 std::source_location().function_name(), offset.value());
    }

    userInfo.state = sapi::UserInfo::State::Empty;

    _impl->storage.replace(userInfo);
    spdlog::get("db")->debug("{} exited", std::source_location().function_name());
}

std::vector<sapi::Event> DBService::getAndRemoveNotifications() {
    spdlog::get("db")->debug("{} entered", std::source_location().function_name());
    using namespace sql;

    std::lock_guard lock{_impl->storageMutex};

    std::vector<sapi::Event> events;

    for (auto const& ev :
         _impl->storage.iterate<sapi::NotificationEvent>(order_by(&sapi::NotificationEvent::secondsFromZeroUTC))) {
        if (std::chrono::utc_clock::time_point(std::chrono::seconds{ev.secondsFromZeroUTC}) <=
            std::chrono::utc_clock::now()) {
            spdlog::get("db")->debug("{}. Event added", std::source_location().function_name());
            events.emplace_back(_impl->storage.get<sapi::Event>(ev.eventId));
        } else {
            break;
        }
    }

    for (auto const& ev : events) {
        _impl->storage.remove<sapi::NotificationEvent>(ev.eventId);
    }
    spdlog::get("db")->debug("{} exited", std::source_location().function_name());
    return events;
}

DBService::~DBService() = default;

}  // namespace shablov