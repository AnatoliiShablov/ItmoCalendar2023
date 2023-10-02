#ifndef ITMOCALENDAR2023_DBSERVICE_HPP
#define ITMOCALENDAR2023_DBSERVICE_HPP

#include <optional>
#include <string_view>
#include <thread>
#include <vector>

#include "sapi/ServerInnerApi.hpp"

namespace shablov {

class DBService {
public:
    DBService(std::string_view path);

    sapi::UserInfo getUserInfo(sapi::UserId userId);
    std::vector<sapi::Event> getAllByUserId(sapi::UserId userId);

    void setState(sapi::UserId userId, sapi::UserInfo::State state);
    bool removeEvent(sapi::UserId userId, sapi::EventId eventId);

    ~DBService();

    void setTimeZone(sapi::UserId userId, std::int64_t minutes);
    void setDateWorkingEvent(sapi::UserId userId, std::chrono::seconds timeFromZeroUtc);
    void setTimeWorkingEvent(sapi::UserId userId, std::chrono::seconds timeOfDay);
    void setDescriptionWorkingEvent(sapi::UserId userId, std::string description);
    void setNotificationWorkingEvent(sapi::UserId userId, std::optional<std::uint64_t> offset);

    std::vector<sapi::Event> getAndRemoveNotifications();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}  // namespace shablov

#endif  // ITMOCALENDAR2023_DBSERVICE_HPP
