#ifndef ITMOCALENDAR2023_CALENDARSERVICE_HPP
#define ITMOCALENDAR2023_CALENDARSERVICE_HPP

#include <thread>

#include "service/DBService.hpp"

namespace shablov {

class CalendarService {
public:
    CalendarService(DBService& dbservice, std::uint16_t port);

private:
    std::jthread workingThread;
};

}  // namespace shablov

#endif  // ITMOCALENDAR2023_CALENDARSERVICE_HPP
