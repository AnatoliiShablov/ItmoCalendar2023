#include <fmt/core.h>

#include "grpc/grpc.h"
#include "grpcpp/channel.h"
#include "grpcpp/client_context.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/security/credentials.h"
#include "gtest/gtest.h"
#include "protocols/FrontAPI.grpc.pb.h"

namespace shablov {

std::string gGmailKey;

struct Runner {
    explicit Runner(std::string const& filename = ":memory:") {
        std::system(("start /b Server.exe \"" + filename + "\" " + gGmailKey).c_str());
    }

    ~Runner() { std::system("taskkill /f /im Server.exe"); }

    std::shared_ptr<grpc::Channel> channel{grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials())};
    std::unique_ptr<front_api::Calendar::Stub> stub{front_api::Calendar::NewStub(channel)};

    std::shared_ptr<grpc::Channel> channelN{grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials())};
    std::unique_ptr<front_api::Notifier::Stub> stubN{front_api::Notifier::NewStub(channelN)};
};

TEST(OneRun, ShowAllEmpty) {
    Runner runner;

    {
        grpc::ClientContext context;
        front_api::ShowAllRequest request;
        request.mutable_user()->set_id(1);
        front_api::ShowAllResponse response;
        runner.stub->ShowAll(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Нет текущих событий");
    }
}

TEST(OneRun, DoubleStart) {
    Runner runner;

    {
        grpc::ClientContext context;
        front_api::StartNewRequest request;
        request.mutable_user()->set_id(1);
        front_api::StartNewResponse response;
        runner.stub->StartNew(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Введите дату (DD.MM.YYYY). Принимаются года, начиная с 1971");
    }

    {
        grpc::ClientContext context;
        front_api::StartNewRequest request;
        request.mutable_user()->set_id(1);
        front_api::StartNewResponse response;
        runner.stub->StartNew(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(),
                  "Невозможно выполнить данную комманду сейчас\n"
                  "Введите дату (DD.MM.YYYY). Принимаются года, начиная с 1971");
    }
}

TEST(OneRun, FullTwoEvents) {
    Runner runner;

    {
        grpc::ClientContext context;
        front_api::StartNewRequest request;
        request.mutable_user()->set_id(1);
        front_api::StartNewResponse response;
        runner.stub->StartNew(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Введите дату (DD.MM.YYYY). Принимаются года, начиная с 1971");
    }

    {
        grpc::ClientContext context;
        front_api::StartNewRequest request;
        request.mutable_user()->set_id(2);
        front_api::StartNewResponse response;
        runner.stub->StartNew(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Введите дату (DD.MM.YYYY). Принимаются года, начиная с 1971");
    }

    {
        grpc::ClientContext context;
        front_api::AddNextArgumentRequest request;
        request.mutable_user()->set_id(1);
        request.set_text("01.01.2000");
        front_api::AddNextArgumentResponse response;
        runner.stub->AddNextArgument(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Введите время (HH:MM)");
    }

    {
        grpc::ClientContext context;
        front_api::AddNextArgumentRequest request;
        request.mutable_user()->set_id(2);
        request.set_text("01.01.2002");
        front_api::AddNextArgumentResponse response;
        runner.stub->AddNextArgument(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Введите время (HH:MM)");
    }

    {
        grpc::ClientContext context;
        front_api::AddNextArgumentRequest request;
        request.mutable_user()->set_id(1);
        request.set_text("10:15");
        front_api::AddNextArgumentResponse response;
        runner.stub->AddNextArgument(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Введите описание");
    }

    {
        grpc::ClientContext context;
        front_api::AddNextArgumentRequest request;
        request.mutable_user()->set_id(2);
        request.set_text("14:20");
        front_api::AddNextArgumentResponse response;
        runner.stub->AddNextArgument(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Введите описание");
    }

    {
        grpc::ClientContext context;
        front_api::AddNextArgumentRequest request;
        request.mutable_user()->set_id(1);
        request.set_text("Описание 1");
        front_api::AddNextArgumentResponse response;
        runner.stub->AddNextArgument(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(),
                  "Если требуется уведомление, напишите количество минут, за которое его прислать\n"
                  "Если нет, напишите 'нет'");
    }

    {
        grpc::ClientContext context;
        front_api::AddNextArgumentRequest request;
        request.mutable_user()->set_id(2);
        request.set_text("Описание 2");
        front_api::AddNextArgumentResponse response;
        runner.stub->AddNextArgument(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(),
                  "Если требуется уведомление, напишите количество минут, за которое его прислать\n"
                  "Если нет, напишите 'нет'");
    }

    {
        grpc::ClientContext context;
        front_api::AddNextArgumentRequest request;
        request.mutable_user()->set_id(1);
        request.set_text("10");
        front_api::AddNextArgumentResponse response;
        runner.stub->AddNextArgument(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Новая запись добавлена");
    }

    {
        grpc::ClientContext context;
        front_api::AddNextArgumentRequest request;
        request.mutable_user()->set_id(2);
        request.set_text("нет");
        front_api::AddNextArgumentResponse response;
        runner.stub->AddNextArgument(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Новая запись добавлена");
    }

    {
        grpc::ClientContext context;
        front_api::ShowAllRequest request;
        request.mutable_user()->set_id(1);
        front_api::ShowAllResponse response;
        runner.stub->ShowAll(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(),
                  "Идентификатор: 1\n"
                  "Описание:\n"
                  "Описание 1\n"
                  "Запланированное время: 01.01.2000 10:15\n"
                  "\n");
    }

    {
        grpc::ClientContext context;
        front_api::ShowAllRequest request;
        request.mutable_user()->set_id(2);
        front_api::ShowAllResponse response;
        runner.stub->ShowAll(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(),
                  "Идентификатор: 2\n"
                  "Описание:\n"
                  "Описание 2\n"
                  "Запланированное время: 01.01.2002 14:20\n"
                  "\n");
    }

    {
        grpc::ClientContext context;
        front_api::SubscribeRequest request;
        front_api::SubscribeResponse response;
        auto reader = runner.stubN->Subscribe(&context, request);
        reader->Read(&response);
        ASSERT_EQ(response.text(),
                  "Напоминаю:\n"
                  "\n"
                  "Идентификатор: 1\n"
                  "Описание:\n"
                  "Описание 1\n"
                  "Запланированное время: 01.01.2000 10:15");
    }

    {
        grpc::ClientContext context;
        front_api::RemoveRequest request;
        request.mutable_user()->set_id(1);
        request.set_id(1);
        front_api::RemoveResponse response;
        runner.stub->Remove(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Удаление произведено успешно");
    }

    {
        grpc::ClientContext context;
        front_api::RemoveRequest request;
        request.mutable_user()->set_id(1);
        request.set_id(1);
        front_api::RemoveResponse response;
        runner.stub->Remove(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Данного события не существует. Удаление невозможно");
    }

    {
        grpc::ClientContext context;
        front_api::RemoveRequest request;
        request.mutable_user()->set_id(1);
        request.set_id(2);
        front_api::RemoveResponse response;
        runner.stub->Remove(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Данного события не существует. Удаление невозможно");
    }

    {
        grpc::ClientContext context;
        front_api::ShowAllRequest request;
        request.mutable_user()->set_id(1);
        front_api::ShowAllResponse response;
        runner.stub->ShowAll(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(), "Нет текущих событий");
    }

    {
        grpc::ClientContext context;
        front_api::ShowAllRequest request;
        request.mutable_user()->set_id(2);
        front_api::ShowAllResponse response;
        runner.stub->ShowAll(&context, request, &response);
        ASSERT_EQ(response.status().ok().text(),
                  "Идентификатор: 2\n"
                  "Описание:\n"
                  "Описание 2\n"
                  "Запланированное время: 01.01.2002 14:20\n"
                  "\n");
    }
}

}  // namespace shablov

int main(int argc, char** argv) {
    if (argc != 2) {
        fmt::print("./interests (gmail key)");
        return -1;
    }

    shablov::gGmailKey = argv[1];

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}