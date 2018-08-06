// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REQUESTSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REQUESTSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/RequestSocket.hpp"

#include "CurveClient.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class RequestSocket : virtual public zeromq::RequestSocket,
                      public Socket,
                      CurveClient
{
public:
    MultipartSendResult SendRequest(opentxs::Data& message) const override;
    MultipartSendResult SendRequest(const std::string& message) const override;
    MultipartSendResult SendRequest(zeromq::Message& message) const override;
    bool SetSocksProxy(const std::string& proxy) const override;
    bool Start(const std::string& endpoint) const override;

    ~RequestSocket() = default;

private:
    friend opentxs::network::zeromq::RequestSocket;
    typedef Socket ot_super;

    const Flag& running_;

    RequestSocket* clone() const override;
    bool wait(const Lock& lock) const;

    RequestSocket(const zeromq::Context& context, const Flag& running);
    RequestSocket() = delete;
    RequestSocket(const RequestSocket&) = delete;
    RequestSocket(RequestSocket&&) = delete;
    RequestSocket& operator=(const RequestSocket&) = delete;
    RequestSocket& operator=(RequestSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REQUESTSOCKET_HPP
