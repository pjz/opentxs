/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_OTPAYMENTPLAN_HPP
#define OPENTXS_CORE_OTPAYMENTPLAN_HPP

#include <opentxs/core/recurring/OTAgreement.hpp>

namespace opentxs
{

#define PLAN_PROCESS_INTERVAL OTTimeGetTimeFromSeconds(10)

/*
 OTPaymentPlan

 This instrument is signed by two parties or more (the first one, I think...)

 While processing payment, BOTH parties to a payment plan will be loaded up and
 their signatures will be checked against the original plan, which is saved as a
 cron receipt.

 There is also a "current version" of the payment plan, which contains updated
 info
 from processing, and is signed by the server.

 BOTH the original version, and the updated version, are sent to EACH user
 whenever
 a payment is processed, as his receipt. This way you have the user's signature
 on
 the terms, and the server's signature whenever it carries out the terms. A
 receipt
 with both is placed in the inbox of both users after any action.

 As with cheques, the server can use the receipts in the inboxes, plus the last
 agreed
 balance, to prove the current balance of any account. The user removes the
 receipt from
 his inbox by accepting it and, in the process, performing a new balance
 agreement.

 THIS MEANS that the OT server can carry out the terms of contracts!  So far, at
 least,
 cheques, trades, payment plans... as long as everything is signed off, we're
 free and
 clear under the same triple-signed system that account transfer uses. (The
 Users cannot
 repudiate their own signatures later, and the server can prove all balances
 with the
 user's own signature.)

 Of course, either side is free to CANCEL a payment plan, or to leave their
 account bereft
 of funds and prone to failed payments. But if they cancel, their signature will
 appear
 on the cancellation request, and the recipient gets a copy of it in his inbox.
 And if
 the funds are insufficient, the plan will keep trying to charge, leaving
 failure notices
 in both inboxes when such things occur.

 You could even have the server manage an issuer account, backed in payment plan
 revenue,
 that would form a new instrument definition that can then be traded on markets.
 (The same
 as you can
 have the server manage the issuer account for a basket currency now, which is
 backed with
 reserve accounts managed by the server, and you can then trade the basket
 currency on markets.)
 */
class OTPaymentPlan : public OTAgreement
{
private: // Private prevents erroneous use by other classes.
    typedef OTAgreement ot_super;

    // *********** Methods for generating a payment plan: ****************

public:
    // From parent:  (This must be called first, before the other two methods
    // below can be called.)
    //
    //    bool        SetAgreement(const int64_t& lTransactionNum,    const
    // OTString& strConsideration,
    //                             const time64_t& VALID_FROM=0,    const
    // time64_t& VALID_TO=0);

    // Then call one (or both) of these:

    EXPORT bool SetInitialPayment(
        const int64_t& lAmount,
        time64_t tTimeUntilInitialPayment = OT_TIME_ZERO); // default: now.

    // These two can be called independent of each other. You can
    // have an initial payment, AND/OR a payment plan.

    EXPORT bool SetPaymentPlan(
        const int64_t& lPaymentAmount,
        time64_t tTimeUntilPlanStart = OT_TIME_MONTH_IN_SECONDS,
        time64_t tBetweenPayments = OT_TIME_MONTH_IN_SECONDS, // Default: 30
                                                              // days.
        time64_t tPlanLength = OT_TIME_ZERO, int32_t nMaxPayments = 0);

    // VerifyAgreement()
    // This function verifies both Nyms and both signatures.
    // Due to the peculiar nature of how OTAgreement/OTPaymentPlan works, there
    // are two signed
    // copies stored. The merchant signs first, adding his transaction numbers
    // (2), and then he
    // sends it to the customer, who also adds two numbers and signs. (Also
    // resetting the creation date.)
    // The problem is, adding the additional transaction numbers invalidates the
    // first (merchant's)
    // signature.
    // The solution is, when the customer confirms the agreement, he stores an
    // internal copy of the
    // merchant's signed version.  This way later, in VERIFY AGREEMENT, the
    // internal copy can be loaded,
    // and BOTH Nyms can be checked to verify that BOTH transaction numbers are
    // valid for each.
    // The two versions of the contract can also be compared to each other, to
    // make sure that none of
    // the vital terms, values, clauses, etc are different between the two.
    //
    virtual bool VerifyAgreement(Nym& RECIPIENT_NYM, Nym& SENDER_NYM) const;
    virtual bool CompareAgreement(const OTAgreement& rh) const;

    // ************ "INITIAL PAYMENT" public GET METHODS **************
public:
    inline bool HasInitialPayment() const
    {
        return m_bInitialPayment;
    }
    inline const time64_t& GetInitialPaymentDate() const
    {
        return m_tInitialPaymentDate;
    }
    inline const int64_t& GetInitialPaymentAmount() const
    {
        return m_lInitialPaymentAmount;
    }
    inline bool IsInitialPaymentDone() const
    {
        return m_bInitialPaymentDone;
    }

    inline const time64_t& GetInitialPaymentCompletedDate() const
    {
        return m_tInitialPaymentCompletedDate;
    }
    inline const time64_t& GetLastFailedInitialPaymentDate() const
    {
        return m_tFailedInitialPaymentDate;
    }
    inline int32_t GetNoInitialFailures() const
    {
        return m_nNumberInitialFailures;
    }

    // "INITIAL PAYMENT" private MEMBERS
private:
    bool m_bInitialPayment;         // Will there be an initial payment?
    time64_t m_tInitialPaymentDate; // Date of the initial payment, measured
                                    // seconds after creation.
    time64_t m_tInitialPaymentCompletedDate; // Date the initial payment was
                                             // finally transacted.
    time64_t m_tFailedInitialPaymentDate;    // Date of the last failed payment,
                                             // measured seconds after creation.
    int64_t m_lInitialPaymentAmount;         // Amount of the initial payment.
    bool m_bInitialPaymentDone;       // Has the initial payment been made?
    int32_t m_nNumberInitialFailures; // If we've tried to process this multiple
                                      // times, we'll know.

    // "INITIAL PAYMENT" protected SET METHODS
protected:
    inline void SetInitialPaymentDate(const time64_t& tInitialPaymentDate)
    {
        m_tInitialPaymentDate = tInitialPaymentDate;
    }
    inline void SetInitialPaymentAmount(const int64_t& lAmount)
    {
        m_lInitialPaymentAmount = lAmount;
    }

    // Sets the bool that officially the initial payment has been done. (Checks
    // first to make sure not already done.)
    bool SetInitialPaymentDone();

    inline void SetInitialPaymentCompletedDate(
        const time64_t& tInitialPaymentDate)
    {
        m_tInitialPaymentCompletedDate = tInitialPaymentDate;
    }
    inline void SetLastFailedInitialPaymentDate(
        const time64_t& tFailedInitialPaymentDate)
    {
        m_tFailedInitialPaymentDate = tFailedInitialPaymentDate;
    }

    inline void SetNoInitialFailures(const int32_t& nNoFailures)
    {
        m_nNumberInitialFailures = nNoFailures;
    }
    inline void IncrementNoInitialFailures()
    {
        m_nNumberInitialFailures++;
    }

    // ************ "PAYMENT PLAN" public GET METHODS ****************
public:
    inline bool HasPaymentPlan() const
    {
        return m_bPaymentPlan;
    }
    inline const int64_t& GetPaymentPlanAmount() const
    {
        return m_lPaymentPlanAmount;
    }
    inline const time64_t& GetTimeBetweenPayments() const
    {
        return m_tTimeBetweenPayments;
    }
    inline const time64_t& GetPaymentPlanStartDate() const
    {
        return m_tPaymentPlanStartDate;
    }
    inline const time64_t& GetPaymentPlanLength() const
    {
        return m_tPaymentPlanLength;
    }
    inline int32_t GetMaximumNoPayments() const
    {
        return m_nMaximumNoPayments;
    }

    inline const time64_t& GetDateOfLastPayment() const
    {
        return m_tDateOfLastPayment;
    }
    inline const time64_t& GetDateOfLastFailedPayment() const
    {
        return m_tDateOfLastFailedPayment;
    }

    inline int32_t GetNoPaymentsDone() const
    {
        return m_nNoPaymentsDone;
    }
    inline int32_t GetNoFailedPayments() const
    {
        return m_nNoFailedPayments;
    }

    // "PAYMENT PLAN" private MEMBERS
private:
    bool m_bPaymentPlan;              // Will there be a payment plan?
    int64_t m_lPaymentPlanAmount;     // Amount of each payment.
    time64_t m_tTimeBetweenPayments;  // How much time between each payment?
    time64_t m_tPaymentPlanStartDate; // Date for the first payment plan
                                      // payment.
    time64_t m_tPaymentPlanLength; // Optional. Plan length measured in seconds
                                   // since plan start.
    int32_t m_nMaximumNoPayments;  // Optional. The most number of payments that
                                   // are authorized.

    time64_t m_tDateOfLastPayment; // Recording of date of the last payment.
    time64_t m_tDateOfLastFailedPayment; // Recording of date of the last failed
                                         // payment.
    int32_t m_nNoPaymentsDone;   // Recording of the number of payments already
                                 // processed.
    int32_t m_nNoFailedPayments; // Every time a payment fails, we record that
                                 // here.

    // "PAYMENT PLAN" protected SET METHODS
protected:
    inline void SetPaymentPlanAmount(const int64_t& lAmount)
    {
        m_lPaymentPlanAmount = lAmount;
    }
    inline void SetTimeBetweenPayments(const time64_t& tTimeBetween)
    {
        m_tTimeBetweenPayments = tTimeBetween;
    }
    inline void SetPaymentPlanStartDate(const time64_t& tPlanStartDate)
    {
        m_tPaymentPlanStartDate = tPlanStartDate;
    }
    inline void SetPaymentPlanLength(const time64_t& tPlanLength)
    {
        m_tPaymentPlanLength = tPlanLength;
    }
    inline void SetMaximumNoPayments(int32_t nMaxNoPayments)
    {
        m_nMaximumNoPayments = nMaxNoPayments;
    }

    inline void SetDateOfLastPayment(const time64_t& tDateOfLast)
    {
        m_tDateOfLastPayment = tDateOfLast;
    }
    inline void SetDateOfLastFailedPayment(const time64_t& tDateOfLast)
    {
        m_tDateOfLastFailedPayment = tDateOfLast;
    }

    inline void SetNoPaymentsDone(int32_t nNoPaymentsDone)
    {
        m_nNoPaymentsDone = nNoPaymentsDone;
    }
    inline void SetNoFailedPayments(int32_t nNoFailed)
    {
        m_nNoFailedPayments = nNoFailed;
    }

    inline void IncrementNoPaymentsDone()
    {
        m_nNoPaymentsDone++;
    }
    inline void IncrementNoFailedPayments()
    {
        m_nNoFailedPayments++;
    }

private: // These are NOT stored as part of the payment plan. They are merely
         // used during execution.
    bool m_bProcessingInitialPayment;
    bool m_bProcessingPaymentPlan;

public:
    // From OTAgreement (parent class of this)
    /*
     inline OTIdentifier&    GetRecipientAcctID()        { return
     m_RECIPIENT_ACCT_ID; }
     inline OTIdentifier&    GetRecipientNymID()        { return
     m_RECIPIENT_NYM_ID; }
     inline void            SetRecipientAcctID(OTIdentifier& ACCT_ID)    {
     m_RECIPIENT_ACCT_ID = ACCT_ID; }
     inline void            SetRecipientNymID(OTIdentifier& NYM_ID)    {
     m_RECIPIENT_NYM_ID = NYM_ID; }

     const OTString&  GetConsideration() const { return m_strConsideration; }
     */
    // Return True if should stay on OTCron's list for more processing.
    // Return False if expired or otherwise should be removed.
    virtual bool ProcessCron(); // OTCron calls this regularly, which is my
                                // chance to expire, etc.

    // From OTCronItem (parent class of OTAgreement, parent class of this)

    /*
     inline void SetCronPointer(OTCron& theCron) { m_pCron = &theCron; }

     inline void SetCreationDate(const time64_t& CREATION_DATE) {
     m_CREATION_DATE = CREATION_DATE; }
     inline const time64_t& GetCreationDate() const { return m_CREATION_DATE; }
     */

    // From OTTrackable (parent class of OTCronItem, parent class of
    // OTAgreement, parent of this)
    /*
     inline int64_t GetTransactionNum() const { return m_lTransactionNum; }
     inline void SetTransactionNum(int64_t lTransactionNum) { m_lTransactionNum
     = lTransactionNum; }

     inline const   OTIdentifier&    GetSenderAcctID()               { return
     m_SENDER_ACCT_ID; }
     inline const   OTIdentifier&    GetSenderNymID()               { return
     m_SENDER_NYM_ID; }
     inline void    SetSenderAcctID(const OTIdentifier& ACCT_ID)    {
     m_SENDER_ACCT_ID = ACCT_ID; }
     inline void    SetSenderNymID(const OTIdentifier& NYM_ID)    {
     m_SENDER_NYM_ID = NYM_ID; }
     */

    // From OTInstrument (parent of OTTrackable, parent of OTCronItem, parent of
    // OTAgreement, parent of this)
    /*
     OTInstrument(const OTIdentifier& NOTARY_ID, const OTIdentifier&
     INSTRUMENT_DEFINITION_ID)
     : Contract()

     inline const OTIdentifier& GetInstrumentDefinitionID() const { return
     m_InstrumentDefinitionID; }
     inline const OTIdentifier& GetNotaryID() const { return m_NotaryID; }

     inline void SetInstrumentDefinitionID(const OTIdentifier&
     INSTRUMENT_DEFINITION_ID)  {
     m_InstrumentDefinitionID    =
     INSTRUMENT_DEFINITION_ID; }
     inline void SetNotaryID(const OTIdentifier& NOTARY_ID) { m_NotaryID    =
     NOTARY_ID; }

     inline time64_t GetValidFrom()    const { return m_VALID_FROM; }
     inline time64_t GetValidTo()        const { return m_VALID_TO; }

     inline void SetValidFrom(time64_t TIME_FROM)    { m_VALID_FROM    =
     TIME_FROM; }
     inline void SetValidTo(time64_t TIME_TO)        { m_VALID_TO    = TIME_TO;
     }

     bool VerifyCurrentDate(); // Verify the current date against the VALID FROM
     / TO dates.
     bool IsExpired()
     */
protected:
    //  virtual void onFinalReceipt();        // Now handled in the parent
    // class.
    //  virtual void onRemovalFromCron();     // Now handled in the parent
    // class.

    bool ProcessPayment(const int64_t& lAmount);
    void ProcessInitialPayment();
    void ProcessPaymentPlan();

public:
    EXPORT OTPaymentPlan();
    EXPORT OTPaymentPlan(const Identifier& NOTARY_ID,
                         const Identifier& INSTRUMENT_DEFINITION_ID);
    EXPORT OTPaymentPlan(const Identifier& NOTARY_ID,
                         const Identifier& INSTRUMENT_DEFINITION_ID,
                         const Identifier& SENDER_ACCT_ID,
                         const Identifier& SENDER_NYM_ID,
                         const Identifier& RECIPIENT_ACCT_ID,
                         const Identifier& RECIPIENT_NYM_ID);
    EXPORT virtual ~OTPaymentPlan();
    void InitPaymentPlan();
    virtual void Release();
    void Release_PaymentPlan();
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    virtual int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml);
    virtual void UpdateContents(); // Before transmission or serialization, this
                                   // is where the ledger saves its contents
};

} // namespace opentxs

#endif // OPENTXS_CORE_OTPAYMENTPLAN_HPP
