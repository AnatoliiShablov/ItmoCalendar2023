#include "gtest/gtest.h"
#include "sapi/ServerInnerApi.hpp"
#include "service/DBService.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace shablov {

TEST(OneRun, GetUser) {
    DBService db(":memory:");
    ASSERT_EQ(db.getUserInfo(1).state, sapi::UserInfo::State::Empty);
    ASSERT_EQ(db.getUserInfo(2).state, sapi::UserInfo::State::Empty);
}

TEST(OneRun, StateChange) {
    DBService db(":memory:");
    ASSERT_EQ(db.getUserInfo(1).state, sapi::UserInfo::State::Empty);
    ASSERT_EQ(db.getUserInfo(2).state, sapi::UserInfo::State::Empty);

    db.setState(1, sapi::UserInfo::State::AddingNewDescription);

    ASSERT_EQ(db.getUserInfo(1).state, sapi::UserInfo::State::AddingNewDescription);
    ASSERT_EQ(db.getUserInfo(2).state, sapi::UserInfo::State::Empty);
}

TEST(OneRun, Event) {
    DBService db(":memory:");
    ASSERT_TRUE(db.getAllByUserId(1).empty());
    ASSERT_TRUE(db.getAllByUserId(2).empty());

    db.setState(1, sapi::UserInfo::State::AddingNewDate);
    db.setDateWorkingEvent(1, std::chrono::seconds{2000000});
    db.setTimeWorkingEvent(1, std::chrono::seconds{1000000});
    db.setDescriptionWorkingEvent(1, "123");
    db.setNotificationWorkingEvent(1, std::nullopt);

    ASSERT_EQ(db.getAllByUserId(1).size(), 1);
    ASSERT_EQ(db.getAllByUserId(1).front().userId, 1);
    ASSERT_EQ(db.getAllByUserId(1).front().timeFromZeroUTC, std::chrono::seconds{2000000 + 1000000});
    ASSERT_EQ(db.getAllByUserId(1).front().description, "123");

    ASSERT_TRUE(db.getAllByUserId(2).empty());
}

}  // namespace shablov

int main(int argc, char** argv) {
    spdlog::register_logger(
        std::make_shared<spdlog::logger>("db", std::make_shared<spdlog::sinks::stdout_color_sink_st>()));

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}