#include "service/PrometheusService.hpp"

#include <array>
#include <thread>

#include "fmt/format.h"
#include "mailio/message.hpp"
#include "mailio/smtp.hpp"
#include "prometheus/counter.h"
#include "prometheus/exposer.h"
#include "prometheus/registry.h"
#include "spdlog/spdlog.h"

namespace shablov {

struct PrometheusService::Impl {
    Impl(std::string_view pass, std::uint16_t port) : pass{pass}, exposer{fmt::format("127.0.0.1:{}", port)} {
        exposer.RegisterCollectable(registry, "/metrics-calendar");
    }

    std::string pass;

    prometheus::Exposer exposer;
    std::shared_ptr<prometheus::Registry> registry = std::make_shared<prometheus::Registry>();

    prometheus::Family<prometheus::Counter>& commands_counter =
        prometheus::BuildCounter().Name("calendar_commands").Help("Number of observed commands").Register(*registry);

    prometheus::Counter& new_counter_correct   = commands_counter.Add({{"command", "new"}, {"correct", "true"}});
    prometheus::Counter& new_counter_incorrect = commands_counter.Add({{"command", "new"}, {"correct", "false"}});

    prometheus::Counter& remove_counter_correct   = commands_counter.Add({{"command", "remove"}, {"correct", "true"}});
    prometheus::Counter& remove_counter_incorrect = commands_counter.Add({{"command", "remove"}, {"correct", "false"}});

    prometheus::Counter& showall_counter_correct = commands_counter.Add({{"command", "showall"}, {"correct", "true"}});
    prometheus::Counter& showall_counter_incorrect =
        commands_counter.Add({{"command", "showall"}, {"correct", "false"}});

    prometheus::Family<prometheus::Counter>& incorrect_message_counter =
        prometheus::BuildCounter()
            .Name("calendar_incorrect_message")
            .Help("Number of incorrect messages from users")
            .Register(*registry);

    prometheus::Counter& incorrect_date         = incorrect_message_counter.Add({{"state", "date"}});
    prometheus::Counter& incorrect_time         = incorrect_message_counter.Add({{"state", "time"}});
    prometheus::Counter& incorrect_notification = incorrect_message_counter.Add({{"state", "notification"}});

    prometheus::Family<prometheus::Counter>& notifications_counter = prometheus::BuildCounter()
                                                                         .Name("calendar_notifications")
                                                                         .Help("Number of notifications sent")
                                                                         .Register(*registry);

    prometheus::Counter& notifications_sent = notifications_counter.Add({});

    prometheus::Family<prometheus::Counter>& critical_family =
        prometheus::BuildCounter().Name("calendar_critical_error").Help("Number of exceptions").Register(*registry);

    prometheus::Counter& critical_error = critical_family.Add({});
};

PrometheusService::PrometheusService(std::string_view pass, std::uint16_t port)
    : _impl{std::make_unique<Impl>(pass, port)} {}

void PrometheusService::add_new(bool correct) {
    (correct ? _impl->new_counter_correct : _impl->new_counter_incorrect).Increment();
}

void PrometheusService::add_remove(bool correct) {
    (correct ? _impl->remove_counter_correct : _impl->remove_counter_incorrect).Increment();
}

void PrometheusService::add_showall(bool correct) {
    (correct ? _impl->showall_counter_correct : _impl->showall_counter_incorrect).Increment();
}

void PrometheusService::add_error_date() {
    _impl->incorrect_date.Increment();
}

void PrometheusService::add_error_time() {
    _impl->incorrect_time.Increment();
}

void PrometheusService::add_error_notification() {
    _impl->incorrect_notification.Increment();
}

void PrometheusService::add_notification() {
    _impl->notifications_sent.Increment();
}
void PrometheusService::add_critical_error() {
    _impl->critical_error.Increment();

    try {
        // create mail message
        mailio::message msg;
        msg.from(
            mailio::mail_address("gmail", "anatoliishablov@gmail.com"));  // set the correct sender name and address
        msg.add_recipient(
            mailio::mail_address("gmail", "anatoliishablov@gmail.com"));  // set the correct recipent name and address
        msg.subject("Critical situation");
        msg.content("Check logs!!!");

        mailio::smtps conn("smtp.gmail.com", 587);

        mailio::dialog_ssl::ssl_options_t ssl_options;
        ssl_options.method      = boost::asio::ssl::context::tls_client;
        ssl_options.verify_mode = boost::asio::ssl::verify_none;
        conn.ssl_options(ssl_options);

        conn.authenticate("anatoliishablov@gmail.com", _impl->pass, mailio::smtps::auth_method_t::START_TLS);
        conn.submit(msg);
    } catch (mailio::smtp_error& exc) {
        spdlog::get("prometheus")->critical("Mailio problem: {}", exc.what());
    } catch (mailio::dialog_error& exc) {
        spdlog::get("prometheus")->critical("Mailio problem: {}", exc.what());

    } catch (...) {
        spdlog::get("prometheus")->critical("Mailio problem: unknown");
    }
}

PrometheusService::~PrometheusService() = default;

}  // namespace shablov