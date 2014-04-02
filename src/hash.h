// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2014 Twecoin Project. http://www.twecoin.org
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_HASH_H
#define BITCOIN_HASH_H

#include "uint256.h"
#include "serialize.h"
#include "sph_fugue.h"
#include "sph_shavite.h"
#include "sph_hamsi.h"
#include "sph_panama.h"
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <vector>
#include <string>

template<typename T1>
inline uint256 Hashtwe(const T1 pbegin, const T1 pend)
{
    sph_fugue256_context     ctx_fugue;
	sph_shavite256_context	 ctx_shavite;
	sph_hamsi256_context	 ctx_hamsi;
	sph_panama_context	 ctx_panama;
    static unsigned char pblank[1];

//Twe Project http://www.twecoin.org : fugue -> shavite -> hamsi -> panama

    uint256 hash[4];

    sph_fugue256_init(&ctx_fugue);
    sph_fugue256 (&ctx_fugue, (pbegin == pend ? pblank : static_cast<const void*>(&pbegin[0])), (pend - pbegin) * sizeof(pbegin[0]));
    sph_fugue256_close(&ctx_fugue, static_cast<void*>(&hash[0]));
	
	sph_shavite256_init(&ctx_shavite);
    sph_shavite256(&ctx_shavite, static_cast<const void*>(&hash[0]), 64);
    sph_shavite256_close(&ctx_shavite, static_cast<void*>(&hash[1]));

	sph_hamsi256_init(&ctx_hamsi);
    sph_hamsi256(&ctx_hamsi, static_cast<const void*>(&hash[1]), 64);
    sph_hamsi256_close(&ctx_hamsi, static_cast<void*>(&hash[2]));

	sph_panama_init(&ctx_panama);
    sph_panama(&ctx_panama, static_cast<const void*>(&hash[2]), 64);
    sph_panama_close(&ctx_panama, static_cast<void*>(&hash[3]));

    return hash[3];
}

class CHashWriter
{
private:
    SHA256_CTX ctx;

public:
    int nType;
    int nVersion;

    void Init() {
        SHA256_Init(&ctx);
    }

    CHashWriter(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn) {
        Init();
    }

    CHashWriter& write(const char *pch, size_t size) {
        SHA256_Update(&ctx, pch, size);
        return (*this);
    }

    // invalidates the object
    uint256 GetHash() {
        uint256 hash1;
        SHA256_Final((unsigned char*)&hash1, &ctx);
        return hash1;
    }

    template<typename T>
    CHashWriter& operator<<(const T& obj) {
        // Serialize to this stream
        ::Serialize(*this, obj, nType, nVersion);
        return (*this);
    }
};

template<typename T1, typename T2>
inline uint256 Hash4(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end)
{
    static unsigned char pblank[1];
    uint256 hash1;
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
    SHA256_Update(&ctx, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
    SHA256_Final((unsigned char*)&hash1, &ctx);
    uint256 hash2;
    SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

template<typename T>
uint256 SerializeHash(const T& obj, int nType=SER_GETHASH, int nVersion=PROTOCOL_VERSION)
{
    CHashWriter ss(nType, nVersion);
    ss << obj;
    return ss.GetHash();
}

inline uint160 Hash160(const std::vector<unsigned char>& vch)
{
    uint256 hash1;
    SHA256(&vch[0], vch.size(), (unsigned char*)&hash1);
    uint160 hash2;
    RIPEMD160((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

unsigned int MurmurHash3(unsigned int nHashSeed, const std::vector<unsigned char>& vDataToHash);

#endif