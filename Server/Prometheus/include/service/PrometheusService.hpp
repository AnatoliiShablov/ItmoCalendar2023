#ifndef ITMOCALENDAR2023_PROMETHEUSSERVICE_HPP
#define ITMOCALENDAR2023_PROMETHEUSSERVICE_HPP

#include <memory>
#include <string_view>

namespace shablov {

class PrometheusService {
public:
    PrometheusService(std::string_view path, std::uint16_t port);

    void add_new(bool correct);
    void add_remove(bool correct);
    void add_showall(bool correct);

    void add_error_date();
    void add_error_time();
    void add_error_notification();

    void add_notification();

    void add_critical_error();

    ~PrometheusService();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}  // namespace shablov

#endif  // ITMOCALENDAR2023_PROMETHEUSSERVICE_HPP
