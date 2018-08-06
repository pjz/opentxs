// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REPLYSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REPLYSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/ReplySocket.hpp"

#include "CurveServer.hpp"
#include "Receiver.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class ReplySocket : virtual public zeromq::ReplySocket,
                    public Socket,
                    CurveServer,
                    Receiver
{
public:
    bool Start(const std::string& endpoint) const override;

    ~ReplySocket();

private:
    friend opentxs::network::zeromq::ReplySocket;
    typedef Socket ot_super;

    const ReplyCallback& callback_;

    const bool client_{false};

    ReplySocket* clone() const override;
    bool have_callback() const override;

    void process_incoming(const Lock& lock, Message& message) override;

    ReplySocket(
        const zeromq::Context& context,
        const bool client,
        const ReplyCallback& callback);
    ReplySocket() = delete;
    ReplySocket(const ReplySocket&) = delete;
    ReplySocket(ReplySocket&&) = delete;
    ReplySocket& operator=(const ReplySocket&) = delete;
    ReplySocket& operator=(ReplySocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REPLYSOCKET_HPP
