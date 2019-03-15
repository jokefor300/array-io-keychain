//
// Created by user on 3/15/19.
//
#include "keydata_singleton.hpp"
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/pwdbased.h>
#include "hdkeys.h"
#include "wordlist.hpp"
#include <fc_light/crypto/sha256.hpp>

using namespace keychain_app;

keydata_singleton& keydata_singleton::instance()
{
    static keydata_singleton instance;
    return instance;
}

std::string keydata_singleton::seed(dev::bytes& user_entropy)
{
    CryptoPP::SecByteBlock ent(16);
    CryptoPP::OS_GenerateRandomBlock(false, ent, ent.size());

    auto sha256 = fc_light::sha256::hash( (const char *) ent.begin(), ent.SizeInBytes() );
    char cs = *sha256.data()&0x0f;

    std::vector<char> ent_cs(ent.begin(), ent.begin()+ent.SizeInBytes());
    ent_cs.push_back(cs);

    size_t ms_len = ent_cs.size()*8/11;

    std::vector<size_t> ms;
    auto pbyte = ent_cs.data();
    char bit = 0;
    for (auto i=0; i<ms_len; i++ )
    {
        size_t res=0;
        for(auto j=0; j<11; j++)
        {
            auto val = (*pbyte>>bit)&1;
            res = res | (val<<j) ;
            if (++bit==8)
            {
                ++pbyte;
                bit = 0;
            }
        }
        ms.push_back(res);
    }
    constexpr size_t wordlist_size =  sizeof(wordlist)/sizeof(wordlist[0]);
    std::vector<std::string> mnemonics;
    for(auto a : ms)
    {
        FC_LIGHT_ASSERT (a < wordlist_size);
        mnemonics.push_back(wordlist[a]);
    }

    std::string res;
    for (auto& a:mnemonics)
        res +=a + " ";
    return res;
}


std::vector<char> keydata_singleton::pbkdf2(std::string const& _pass)
{
    unsigned _iterations = 2048;
    unsigned _dkLen = 64;
    dev::bytes const _salt;
    std::vector<char> ret(_dkLen);
    if (CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256>().DeriveKey(
            (unsigned char *) ret.data(),
            _dkLen,
            0,
            reinterpret_cast<byte const*>(_pass.data()),
            _pass.size(),
            _salt.data(),
            _salt.size(),
            _iterations
    ) != _iterations)
        FC_LIGHT_THROW_EXCEPTION(fc_light::internal_error_exception, "Key derivation failed.");

    return ret;
}


void keydata_singleton::create_masterkey(std::string& mnemonics)
{
    std::vector<char> key = std::move(pbkdf2(mnemonics));
    dev::Secret master_key(dev::FixedHash<32>((byte * const)key.data(),    dev::FixedHash<32>::ConstructFromPointerType::ConstructFromPointer));
    dev::Secret chain_code(dev::FixedHash<32>((byte * const)key.data()+32, dev::FixedHash<32>::ConstructFromPointerType::ConstructFromPointer));

}


void keydata_singleton::create_privatekey()
{
//    Coin::HDkeychain hd(bytes_t(master_key.begin(), master_key.end()), bytes_t(chain_code.begin(), chain_code.end()));
/*
    std::string keyname = "";
    std::string pass = "blank";
    keyfiles.create(std::bind(create_new_keyfile,
                              keyname, keyname, true, keyfile_format::cipher_etype::aes256,
                              keyfile_format::curve_etype::secp256k1,
                              [&pass](const std::string& keyname)->byte_seq_t{
                                  byte_seq_t res;
                                  std::copy(pass.begin(), pass.end(), std::back_inserter(res));
                                  return res;
                              })
);
*/
}