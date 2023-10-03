#ifndef ITMOCALENDAR2023_CALENDARSERVICE_HPP
#define ITMOCALENDAR2023_CALENDARSERVICE_HPP

#include <thread>

#include "service/DBService.hpp"
#include "service/PrometheusService.hpp"

namespace shablov {

class CalendarService {
public:
    CalendarService(PrometheusService& prometheusservice, DBService& dbservice, std::uint16_t port);

private:
    std::jthread workingThread;
};

}  // namespace shablov

#endif  // ITMOCALENDAR2023_CALENDARSERVICE_HPP
