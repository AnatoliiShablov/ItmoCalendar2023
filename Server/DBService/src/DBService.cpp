#include "service/DBService.hpp"

#include <chrono>
#include <mutex>

#include "fmt/format.h"
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
    }

    decltype(sql::make_storage(std::declval<char const*>(), STORAGE_SCHEME)) storage;
    std::recursive_mutex storageMutex;
};

#undef STORAGE_SCHEME

DBService::DBService(std::string_view path) : _impl{std::make_unique<Impl>(path)} {}

std::vector<sapi::Event> DBService::getAllByUserId(sapi::UserId userId) {
    using namespace sql;
    std::lock_guard lock{_impl->storageMutex};
    return _impl->storage.get_all<sapi::Event>(where(c(&sapi::Event::userId) == userId));
}

sapi::UserInfo DBService::getUserInfo(sapi::UserId userId) {
    using namespace sql;
    std::lock_guard lock{_impl->storageMutex};
    if (_impl->storage.count<sapi::UserInfo>(where(c(&sapi::UserInfo::userId) == userId)) == 0) {
        _impl->storage.replace(sapi::UserInfo{
            .userId = userId, .timezoneOffset = std::chrono::hours{3}, .state = sapi::UserInfo::State::Empty});
    }

    return _impl->storage.get<sapi::UserInfo>(userId);
}

void DBService::setState(sapi::UserId userId, sapi::UserInfo::State state) {
    std::lock_guard lock{_impl->storageMutex};
    auto userInfo  = getUserInfo(userId);
    userInfo.state = state;
    _impl->storage.replace(userInfo);
}

bool DBService::removeEvent(sapi::UserId userId, sapi::EventId eventId) {
    using namespace sql;

    std::lock_guard lock{_impl->storageMutex};

    if (_impl->storage.count<sapi::Event>(
            where((c(&sapi::Event::userId) == userId && c(&sapi::Event::eventId) == eventId))) == 0) {
        return false;
    }

    _impl->storage.remove<sapi::Event>(eventId);
    _impl->storage.remove<sapi::NotificationEvent>(eventId);

    return true;
}

void DBService::setTimeZone(sapi::UserId userId, std::int64_t minutes) {
    std::lock_guard lock{_impl->storageMutex};

    auto userInfo = getUserInfo(userId);
    userInfo.setTimezoneOffset(minutes);
    _impl->storage.replace(userInfo);
}

void DBService::setDateWorkingEvent(sapi::UserId userId, std::chrono::seconds timeFromZeroUtc) {
    std::lock_guard lock{_impl->storageMutex};

    auto userInfo = getUserInfo(userId);

    userInfo.state                  = sapi::UserInfo::State::AddingNewTime;
    userInfo.workingTimeFromZeroUTC = timeFromZeroUtc;

    _impl->storage.replace(userInfo);
}

void DBService::setTimeWorkingEvent(sapi::UserId userId, std::chrono::seconds timeOfDay) {
    std::lock_guard lock{_impl->storageMutex};

    auto userInfo = getUserInfo(userId);

    userInfo.state = sapi::UserInfo::State::AddingNewDescription;
    userInfo.workingTimeFromZeroUTC += timeOfDay;

    _impl->storage.replace(userInfo);
}

void DBService::setDescriptionWorkingEvent(sapi::UserId userId, std::string description) {
    std::lock_guard lock{_impl->storageMutex};

    auto userInfo = getUserInfo(userId);

    userInfo.state              = sapi::UserInfo::State::AddingNewNotification;
    userInfo.workingDescription = std::move(description);

    _impl->storage.replace(userInfo);
}

void DBService::setNotificationWorkingEvent(sapi::UserId userId, std::optional<std::uint64_t> offset) {
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
    }

    userInfo.state = sapi::UserInfo::State::Empty;

    _impl->storage.replace(userInfo);
}

std::vector<sapi::Event> DBService::getAndRemoveNotifications() {
    using namespace sql;

    std::lock_guard lock{_impl->storageMutex};

    std::vector<sapi::Event> events;

    for (auto const& ev :
         _impl->storage.iterate<sapi::NotificationEvent>(order_by(&sapi::NotificationEvent::secondsFromZeroUTC))) {
        if (std::chrono::utc_clock::time_point(std::chrono::seconds{ev.secondsFromZeroUTC}) <=
            std::chrono::utc_clock::now()) {
            events.emplace_back(_impl->storage.get<sapi::Event>(ev.eventId));
        } else {
            break;
        }
    }

    for (auto const& ev : events) {
        _impl->storage.remove<sapi::NotificationEvent>(ev.eventId);
    }

    return events;
}

DBService::~DBService() = default;

}  // namespace shablov