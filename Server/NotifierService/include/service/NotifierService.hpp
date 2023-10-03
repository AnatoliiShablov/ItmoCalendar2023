#ifndef ITMOCALENDAR2023_NOTIFIERSERVICE_HPP
#define ITMOCALENDAR2023_NOTIFIERSERVICE_HPP

#include <thread>

#include "service/DBService.hpp"
#include "service/PrometheusService.hpp"

namespace shablov {

class NotifierService {
public:
    NotifierService(PrometheusService& prometheusservice, DBService& dbservice, std::uint16_t port);

private:
    std::jthread workingThread;
};

}  // namespace shablov

#endif  // ITMOCALENDAR2023_NOTIFIERSERVICE_HPP
