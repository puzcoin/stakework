// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * Why base-58 instead of standard base-64 encoding?
 * - Don't want 0OIl characters that look the same in some fonts and
 *      could be used to create visually identical looking data.
 * - A string with non-alphanumeric characters is not as easily accepted as input.
 * - E-mail usually won't line-break if there's no punctuation to break at.
 * - Double-clicking selects the whole string as one word if it's all alphanumeric.
 */
#ifndef STAKEWORK_BASE58_H
#define STAKEWORK_BASE58_H

#include <chainparams.h>
#include <key.h>
#include <pubkey.h>
#include <script/script.h>
#include <script/standard.h>
#include <support/allocators/zeroafterfree.h>

#include <string>
#include <vector>

/**
 * Encode a byte sequence as a base58-encoded string.
 * pbegin and pend cannot be NULL, unless both are.
 */
std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend);

/**
 * Encode a byte vector as a base58-encoded string
 */
std::string EncodeBase58(const std::vector<unsigned char>& vch);

/**
 * Decode a base58-encoded string (psz) into a byte vector (vchRet).
 * return true if decoding is successful.
 * psz cannot be NULL.
 */
bool DecodeBase58(const char* psz, std::vector<unsigned char>& vchRet);

/**
 * Decode a base58-encoded string (str) into a byte vector (vchRet).
 * return true if decoding is successful.
 */
bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet);

/**
 * Encode a byte vector into a base58-encoded string, including checksum
 */
std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn);

/**
 * Decode a base58-encoded string (psz) that includes a checksum into a byte
 * vector (vchRet), return true if decoding is successful
 */
inline bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet);

/**
 * Decode a base58-encoded string (str) that includes a checksum into a byte
 * vector (vchRet), return true if decoding is successful
 */
inline bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet);

/**
 * Base class for all base58-encoded data
 */
class CBase58Data
{
protected:
    //! the version byte(s)
    std::vector<unsigned char> vchVersion;

    //! the actually encoded data
    typedef std::vector<unsigned char, zero_after_free_allocator<unsigned char> > vector_uchar;
    vector_uchar vchData;

    CBase58Data();
    void SetData(const std::vector<unsigned char> &vchVersionIn, const void* pdata, size_t nSize);
    void SetData(const std::vector<unsigned char> &vchVersionIn, const void* pdata, size_t nSize, const void* pdata2, size_t nSize2);
    void SetData(const std::vector<unsigned char> &vchVersionIn, const unsigned char *pbegin, const unsigned char *pend);

public:
    bool SetString(const char* psz, unsigned int nVersionBytes = 1);
    bool SetString(const std::string& str);
    std::string ToString() const;
    int CompareTo(const CBase58Data& b58) const;

    bool operator==(const CBase58Data& b58) const { return CompareTo(b58) == 0; }
    bool operator<=(const CBase58Data& b58) const { return CompareTo(b58) <= 0; }
    bool operator>=(const CBase58Data& b58) const { return CompareTo(b58) >= 0; }
    bool operator< (const CBase58Data& b58) const { return CompareTo(b58) <  0; }
    bool operator> (const CBase58Data& b58) const { return CompareTo(b58) >  0; }
};

/** base58-encoded StakeWork addresses.
 * Public-key-hash-addresses have version 111 (or 20 testnet).
 * The data vector contains RIPEMD160(SHA256(pubkey)), where pubkey is the serialized public key.
 * Script-hash-addresses have version 28 (or 96 testnet).
 * The data vector contains RIPEMD160(SHA256(cscript)), where cscript is the serialized redemption script.
 * Cold-staking-addresses have version 196 (or 63 testnet).
 * The data vector contains RIPEMD160(SHA256(stakingkey)) || RIPEMD160(SHA256(spendingkey)), where stakingkey and spendingkey are the serialized public keys.
 */
class CStakeWorkAddress : public CBase58Data {
public:
    bool Set(const CKeyID &id);
    bool Set(const CKeyID &id, const CKeyID &id2);
    bool Set(const CScriptID &id);
    bool Set(const CScript &scriptIn);
    bool Set(const CTxDestination &dest);
    bool IsValid() const;
    bool IsValid(const CChainParams &params) const;
    bool IsColdStakingAddress(const CChainParams& params) const;
    bool IsRawScript() const;

    CStakeWorkAddress() {}
    CStakeWorkAddress(const CTxDestination &dest) { Set(dest); }
    CStakeWorkAddress(const CKeyID &id, const CKeyID &id2) { Set(id, id2); }
    CStakeWorkAddress(const std::string& strAddress) { SetString(strAddress); }
    CStakeWorkAddress(const char* pszAddress) { SetString(pszAddress); }
    CStakeWorkAddress(const CScript &scriptIn) { Set(scriptIn); }

    CTxDestination Get() const;
    bool GetKeyID(CKeyID &keyID) const;
    bool GetStakingKeyID(CKeyID &keyID) const;
    bool GetSpendingKeyID(CKeyID &keyID) const;
    bool GetIndexKey(uint160& hashBytes, int& type) const;
    bool IsScript() const;

    bool GetStakingAddress(CStakeWorkAddress &address) const;
    bool GetSpendingAddress(CStakeWorkAddress &address) const;

};

/**
 * A base58-encoded secret key
 */
class CStakeWorkSecret : public CBase58Data
{
public:
    void SetKey(const CKey& vchSecret);
    CKey GetKey();
    bool IsValid() const;
    bool SetString(const char* pszSecret);
    bool SetString(const std::string& strSecret);

    CStakeWorkSecret(const CKey& vchSecret) { SetKey(vchSecret); }
    CStakeWorkSecret() {}
};

template<typename K, int Size, CChainParams::Base58Type Type> class CStakeWorkExtKeyBase : public CBase58Data
{
public:
    void SetKey(const K &key) {
        unsigned char vch[Size];
        key.Encode(vch);
        SetData(Params().Base58Prefix(Type), vch, vch+Size);
    }

    K GetKey() {
        K ret;
        if (vchData.size() == Size) {
            //if base58 encouded data not holds a ext key, return a !IsValid() key
            ret.Decode(&vchData[0]);
        }
        return ret;
    }

    CStakeWorkExtKeyBase(const K &key) {
        SetKey(key);
    }

    CStakeWorkExtKeyBase(const std::string& strBase58c) {
        SetString(strBase58c.c_str(), Params().Base58Prefix(Type).size());
    }

    CStakeWorkExtKeyBase() {}
};

typedef CStakeWorkExtKeyBase<CExtKey, BIP32_EXTKEY_SIZE, CChainParams::EXT_SECRET_KEY> CStakeWorkExtKey;
typedef CStakeWorkExtKeyBase<CExtPubKey, BIP32_EXTKEY_SIZE, CChainParams::EXT_PUBLIC_KEY> CStakeWorkExtPubKey;

#endif // STAKEWORK_BASE58_H
