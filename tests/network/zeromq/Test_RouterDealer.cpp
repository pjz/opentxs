// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;
using namespace opentxs::network;

namespace zmq = opentxs::network::zeromq;

namespace
{

class Test_RouterDealer : public ::testing::Test
{
public:
    static OTZMQContext context_;

    OTZMQRouterSocket* routerSocket_;
    OTZMQDealerSocket* dealerSocket_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};
    const std::string testMessage3_{"zeromq test message 3"};

    const std::string routerEndpoint_{"inproc://opentxs/test/router"};
    const std::string dealerEndpoint_{"inproc://opentxs/test/dealer"};

    std::atomic_int callbackFinishedCount_{0};

    void requestSocketThread(
        const std::string& endpoint,
        const std::string& msg);

    void dealerSocketThread(
        const std::string& endpoint,
        const std::string& msg);
};

OTZMQContext Test_RouterDealer::context_{zeromq::Context::Factory()};

void Test_RouterDealer::requestSocketThread(
    const std::string& endpoint,
    const std::string& msg)
{
    ASSERT_NE(nullptr, &Test_RouterDealer::context_.get());

    auto requestSocket =
        network::zeromq::RequestSocket::Factory(Test_RouterDealer::context_);

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(SocketType::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint);

    auto [result, message] = requestSocket->SendRequest(msg);

    ASSERT_EQ(result, SendResult::VALID_REPLY);

    const std::string& messageString = *message->Body().begin();
    ASSERT_EQ(msg, messageString);
}

void Test_RouterDealer::dealerSocketThread(
    const std::string& endpoint,
    const std::string& msg)
{
    ASSERT_NE(nullptr, &Test_RouterDealer::context_.get());

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed, msg](network::zeromq::Message& input) -> void {
            EXPECT_EQ(2, input.size());

            const std::string& inputString = *input.Body().begin();

            EXPECT_EQ(msg, inputString);

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = zmq::DealerSocket::Factory(
        Test_RouterDealer::context_,
        zmq::Socket::Direction::Connect,
        dealerCallback);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    dealerSocket->Start(endpoint);

    auto sent = dealerSocket->Send(msg);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) {
        Log::Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(replyProcessed);
}
}  // namespace

TEST_F(Test_RouterDealer, Router_Dealer)
{
    ASSERT_NE(nullptr, &Test_RouterDealer::context_.get());

    auto dealerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message& input) -> void {
            EXPECT_EQ(3, input.size());
            const std::string& inputString = *input.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            auto sent = routerSocket_->get().Send(input);

            EXPECT_TRUE(sent);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = zmq::DealerSocket::Factory(
        Test_RouterDealer::context_,
        zmq::Socket::Direction::Bind,
        dealerCallback);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    dealerSocket->Start(dealerEndpoint_);

    dealerSocket_ = &dealerSocket;

    auto routerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message& input) -> void {
            EXPECT_EQ(3, input.size());
            const std::string& inputString = *input.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            auto sent = dealerSocket_->get().Send(input);

            EXPECT_TRUE(sent);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = zmq::RouterSocket::Factory(
        Test_RouterDealer::context_,
        zmq::Socket::Direction::Bind,
        routerCallback);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(SocketType::Router, routerSocket->Type());

    routerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    routerSocket->Start(routerEndpoint_);

    routerSocket_ = &routerSocket;

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const network::zeromq::Message& input) -> OTZMQMessage {
            const std::string& inputString = *input.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            auto reply = network::zeromq::Message::ReplyFactory(input);
            reply->AddFrame(inputString);

            ++callbackFinishedCount_;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = zmq::ReplySocket::Factory(
        Test_RouterDealer::context_,
        zmq::Socket::Direction::Connect,
        replyCallback);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(dealerEndpoint_);

    // Send the request on a separate thread so this thread can continue and
    // wait for replyCallback to finish.
    std::thread requestSocketThread1(
        &Test_RouterDealer::requestSocketThread,
        this,
        routerEndpoint_,
        testMessage_);

    auto end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < 3 && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(3, callbackFinishedCount_);

    requestSocketThread1.join();

    routerSocket_ = nullptr;
    dealerSocket_ = nullptr;
}

TEST_F(Test_RouterDealer, Dealer_3_Router_Dealer_Reply)
{
    ASSERT_NE(nullptr, &Test_RouterDealer::context_.get());

    auto dealerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message& input) -> void {
            EXPECT_EQ(3, input.size());
            const std::string& inputString = *input.Body().begin();
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto sent = routerSocket_->get().Send(input);

            EXPECT_TRUE(sent);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = zmq::DealerSocket::Factory(
        Test_RouterDealer::context_,
        zmq::Socket::Direction::Bind,
        dealerCallback);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    dealerSocket->Start(dealerEndpoint_);

    dealerSocket_ = &dealerSocket;

    auto routerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message& input) -> void {
            EXPECT_EQ(3, input.size());
            const std::string& inputString = *input.Body().begin();
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto sent = dealerSocket_->get().Send(input);

            EXPECT_TRUE(sent);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = zmq::RouterSocket::Factory(
        Test_RouterDealer::context_,
        zmq::Socket::Direction::Bind,
        routerCallback);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(SocketType::Router, routerSocket->Type());

    routerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    routerSocket->Start(routerEndpoint_);

    routerSocket_ = &routerSocket;

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const network::zeromq::Message& input) -> OTZMQMessage {
            const std::string& inputString = *input.Body().begin();
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto reply = network::zeromq::Message::ReplyFactory(input);
            reply->AddFrame(inputString);

            ++callbackFinishedCount_;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = zmq::ReplySocket::Factory(
        Test_RouterDealer::context_,
        zmq::Socket::Direction::Connect,
        replyCallback);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(dealerEndpoint_);

    // Send the requests on separate threads so this thread can continue and
    // wait for clientRouterCallback to finish.
    std::thread dealerSocketThread1(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage_);

    std::thread dealerSocketThread2(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage2_);

    std::thread dealerSocketThread3(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage3_);

    auto end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < 9 && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(9, callbackFinishedCount_);

    dealerSocketThread1.join();
    dealerSocketThread2.join();
    dealerSocketThread3.join();

    routerSocket_ = nullptr;
    dealerSocket_ = nullptr;
}

TEST_F(Test_RouterDealer, Dealer_3_Router_Dealer_Router)
{
    ASSERT_NE(nullptr, &Test_RouterDealer::context_.get());

    auto dealerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message& input) -> void {
            EXPECT_EQ(3, input.size());
            const std::string& inputString = *input.Body().begin();
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto sent = routerSocket_->get().Send(input);

            EXPECT_TRUE(sent);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = zmq::DealerSocket::Factory(
        Test_RouterDealer::context_,
        zmq::Socket::Direction::Bind,
        dealerCallback);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    dealerSocket->Start(dealerEndpoint_);

    dealerSocket_ = &dealerSocket;

    auto routerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message& input) -> void {
            EXPECT_EQ(3, input.size());
            const std::string& inputString = *input.Body().begin();
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto sent = dealerSocket_->get().Send(input);

            EXPECT_TRUE(sent);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = zmq::RouterSocket::Factory(
        Test_RouterDealer::context_,
        zmq::Socket::Direction::Bind,
        routerCallback);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(SocketType::Router, routerSocket->Type());

    routerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    routerSocket->Start(routerEndpoint_);

    routerSocket_ = &routerSocket;

    std::vector<OTZMQMessage> replyMessages;

    auto clientRouterCallback = zmq::ListenCallback::Factory(
        [this, &replyMessages](const network::zeromq::Message& input) -> void {
            const std::string& inputString = *input.Body().begin();
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto replyMessage = zeromq::Message::Factory();
            for (auto it = input.begin(); it != input.end(); ++it) {
                auto& frame = *it;
                if (0 < frame.size()) {
                    OTData data = Data::Factory(frame.data(), frame.size());
                    replyMessage->AddFrame(data.get());
                } else {
                    replyMessage->AddFrame();
                }
            }
            replyMessages.push_back(replyMessage);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &clientRouterCallback.get());

    auto clientRouterSocket = zmq::RouterSocket::Factory(
        Test_RouterDealer::context_,
        zmq::Socket::Direction::Connect,
        clientRouterCallback);

    ASSERT_NE(nullptr, &clientRouterSocket.get());
    ASSERT_EQ(SocketType::Router, clientRouterSocket->Type());

    clientRouterSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    clientRouterSocket->Start(dealerEndpoint_);

    // Send the requests on separate threads so this thread can continue and
    // wait for clientRouterCallback to finish.
    std::thread dealerSocketThread1(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage_);

    std::thread dealerSocketThread2(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage2_);

    std::thread dealerSocketThread3(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage3_);

    auto end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < 6 && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(6, callbackFinishedCount_);

    for (auto replyMessage : replyMessages) {
        clientRouterSocket->Send(replyMessage.get());
    }

    end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < 9 && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(9, callbackFinishedCount_);

    dealerSocketThread1.join();
    dealerSocketThread2.join();
    dealerSocketThread3.join();

    routerSocket_ = nullptr;
    dealerSocket_ = nullptr;
}
