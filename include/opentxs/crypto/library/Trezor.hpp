// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_TREZOR_HPP
#define OPENTXS_CRYPTO_LIBRARY_TREZOR_HPP

#include "Internal.hpp"

#if OT_CRYPTO_USING_TREZOR
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EncodingProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/crypto/library/Ripemd160.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/crypto/Bip32.hpp"
#endif
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/crypto/Bip39.hpp"
#endif

namespace opentxs::crypto
{
class Trezor : virtual public EncodingProvider,
               virtual public Ripemd160
#if OT_CRYPTO_WITH_BIP39
    ,
               virtual public Bip39
#endif
#if OT_CRYPTO_WITH_BIP32
    ,
               virtual public Bip32
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    ,
               virtual public AsymmetricProvider,
               virtual public EcdsaProvider
#endif
{
public:
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    EXPORT virtual bool ECDH(
        const Data& publicKey,
        const OTPassword& privateKey,
        OTPassword& secret) const = 0;
#endif

    EXPORT virtual ~Trezor() = default;

protected:
    Trezor() = default;

private:
    Trezor(const Trezor&) = delete;
    Trezor(Trezor&&) = delete;
    Trezor& operator=(const Trezor&) = delete;
    Trezor& operator=(Trezor&&) = delete;
};
}  // namespace opentxs::crypto
#endif  // OT_CRYPTO_USING_TREZOR
#endif
