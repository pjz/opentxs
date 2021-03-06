// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs
{
namespace server
{
class Notary
{
public:
    void NotarizeProcessInbox(
        ClientContext& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeProcessNymbox(
        ClientContext& context,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeTransaction(
        ClientContext& context,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);

private:
    friend class Server;

    class Finalize
    {
    public:
        Finalize(const Nym& signer, Item& item, Item& balanceItem);
        Finalize() = delete;

        ~Finalize();

    private:
        const Nym& signer_;
        Item& item_;
        Item& balance_item_;
    };

    Server& server_;
    const opentxs::api::server::Manager& manager_;
    OTZMQPushSocket notification_socket_;

    std::unique_ptr<Cheque> extract_cheque(
        const Identifier& serverID,
        const Identifier& unitID,
        const Item& item) const;
    void send_push_notification(
        const Account& account,
        const std::shared_ptr<const Ledger>& inbox,
        const std::shared_ptr<const Ledger>& outbox,
        const std::shared_ptr<const OTTransaction>& item) const;

    void cancel_cheque(
        const OTTransaction& input,
        const Cheque& cheque,
        const Item& depositItem,
        const String& serializedDepositItem,
        const Item& balanceItem,
        ClientContext& context,
        Account& account,
        Ledger& inbox,
        const Ledger& outbox,
        OTTransaction& output,
        bool& success,
        Item& responseItem,
        Item& responseBalanceItem);
    void deposit_cheque(
        const OTTransaction& input,
        const Item& depositItem,
        const String& serializedDepositItem,
        const Item& balanceItem,
        const Cheque& cheque,
        ClientContext& depositorContext,
        ExclusiveAccount& depositorAccount,
        Ledger& depositorInbox,
        const Ledger& depositorOutbox,
        bool& success,
        Item& responseItem,
        Item& responseBalanceItem);
    void deposit_cheque(
        const OTTransaction& input,
        const Item& depositItem,
        const String& serializedDepositItem,
        const Item& balanceItem,
        const Cheque& cheque,
        const bool isVoucher,
        const bool cancelling,
        const Identifier& senderNymID,
        ClientContext& senderContext,
        Account& senderAccount,
        Ledger& senderInbox,
        std::shared_ptr<OTTransaction>& inboxItem,
        Account& sourceAccount,
        const ClientContext& depositorContext,
        Account& depositorAccount,
        const Ledger& depositorInbox,
        const Ledger& depositorOutbox,
        bool& success,
        Item& responseItem,
        Item& responseBalanceItem);
    void NotarizeCancelCronItem(
        ClientContext& context,
        ExclusiveAccount& assetAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeDeposit(
        ClientContext& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeExchangeBasket(
        ClientContext& context,
        ExclusiveAccount& sourceAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeMarketOffer(
        ClientContext& context,
        ExclusiveAccount& assetAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizePayDividend(
        ClientContext& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizePaymentPlan(
        ClientContext& context,
        ExclusiveAccount& depositorAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeSmartContract(
        ClientContext& context,
        ExclusiveAccount& activatingAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeTransfer(
        ClientContext& context,
        ExclusiveAccount& fromAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeWithdrawal(
        ClientContext& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void process_cash_deposit(
        const OTTransaction& input,
        const Item& depositItem,
        const Item& balanceItem,
        ClientContext& context,
        ExclusiveAccount& depositorAccount,
        OTTransaction& output,
        bool& success,
        Item& responseItem,
        Item& responseBalanceItem);
    void process_cheque_deposit(
        const OTTransaction& input,
        const Item& depositItem,
        const Item& balanceItem,
        ClientContext& context,
        ExclusiveAccount& depositorAccount,
        OTTransaction& output,
        bool& success,
        Item& responseItem,
        Item& responseBalanceItem);

    explicit Notary(
        Server& server,
        const opentxs::api::server::Manager& manager);
    Notary() = delete;
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    Notary& operator=(const Notary&) = delete;
    Notary& operator=(Notary&&) = delete;
};
}  // namespace server
}  // namespace opentxs
