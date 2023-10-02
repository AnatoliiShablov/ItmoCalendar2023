#ifndef ITMOCALENDAR2023_EVENT_HPP
#define ITMOCALENDAR2023_EVENT_HPP

#include <cstdint>
#include <string>

namespace shablov::sapi {

using UserId  = std::int64_t;
using EventId = std::uint64_t;

struct Event {
    EventId eventId;
    UserId userId;

    std::string description;
    std::chrono::seconds timeFromZeroUTC;

    [[nodiscard]] constexpr std::uint64_t getTimeFromZeroUTC() const noexcept { return timeFromZeroUTC.count(); }

    constexpr void setTimeFromZeroUTC(std::uint64_t newTimeFromZeroUTC) noexcept {
        timeFromZeroUTC = std::chrono::seconds{newTimeFromZeroUTC};
    }
};

struct NotificationEvent {
    EventId eventId;

    std::uint64_t secondsFromZeroUTC;
};

struct UserInfo {
    UserId userId;

    std::chrono::minutes timezoneOffset;

    enum class State : std::uint32_t {
        //
        Empty = 0x1000,
        // New
        AddingNewDate         = 0x2000,
        AddingNewTime         = 0x2001,
        AddingNewDescription  = 0x2002,
        AddingNewNotification = 0x2003,
    } state;

    std::string workingDescription;
    std::chrono::seconds workingTimeFromZeroUTC;

    [[nodiscard]] constexpr std::int64_t getTimezoneOffset() const noexcept { return timezoneOffset.count(); }

    constexpr void setTimezoneOffset(std::int64_t newTimezoneOffset) noexcept {
        timezoneOffset = std::chrono::minutes{newTimezoneOffset};
    }

    [[nodiscard]] constexpr std::uint32_t getState() const noexcept { return static_cast<std::uint32_t>(state); }

    constexpr void setState(std::uint32_t newState) noexcept { state = static_cast<State>(newState); };

    [[nodiscard]] constexpr std::uint64_t getWorkingTimeFromZeroUTC() const noexcept {
        return workingTimeFromZeroUTC.count();
    }

    constexpr void setWorkingTimeFromZeroUTC(std::uint64_t newWorkingTimeFromZeroUTC) noexcept {
        workingTimeFromZeroUTC = std::chrono::seconds{newWorkingTimeFromZeroUTC};
    }
};

}  // namespace shablov::sapi

#endif  // ITMOCALENDAR2023_EVENT_HPP
