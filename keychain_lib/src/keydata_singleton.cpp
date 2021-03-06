//
// Created by user on 3/15/19.
//
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/pwdbased.h>
#include "hdkeys.h"
#include <wordlist.hpp>
#include <fc_light/crypto/sha256.hpp>
#include <keydata_singleton.hpp>
#include <regex>
#include <key_encryptor.hpp>
#include "eth_types_conversion.hpp"
#include "sql_singleton.hpp"
#include "keychain_logger.hpp"

using namespace keychain_app;

std::string keydata::seed(dev::bytes& user_entropy)
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


std::vector<char> keydata::pbkdf2(std::string const& _pass)
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


bool keydata::derive_masterkey(std::string& mnemonics, std::string& pass)
{
    auto& log = logger_singleton::instance();
    auto& keyfiles = keyfile_singleton::instance();
    if (keyfiles.is_exist("master_key"))
    {
        BOOST_LOG_SEV(log.lg, info) << "error create master_key: master_key already exist";
        return false;
    }

    std::regex re(" +");
    std::string mnemonics_ = std::regex_replace(mnemonics, re, "");

    std::vector<char> key = std::move(pbkdf2(mnemonics_));
    dev::bytes priv_key(key.begin(), key.begin()+32);
    dev::bytes chain_code(key.begin()+32, key.end());

    auto res = keyfiles.create(std::bind(create_new_keyfile,
                              "master_key", "master_key", true, keyfile_format::cipher_etype::aes256,
                              keyfile_format::curve_etype::secp256k1,
                              [&pass](const std::string& keyname)->byte_seq_t{
                                  byte_seq_t res;
                                  std::copy(pass.begin(), pass.end(), std::back_inserter(res));
                                  return res;
                              },
                              priv_key,
                              chain_code
                              ));
    if (res)
        BOOST_LOG_SEV(log.lg, info) << "create master key";
    else
        BOOST_LOG_SEV(log.lg, info) << "error to create master key";

    return res;
}

bool keydata::derive_key(std::string& masterkey_pass, std::string& json)
{
    using namespace keydata;
    auto& log = logger_singleton::instance();
    bool res = false;

    auto& keyfiles = keyfile_singleton::instance();
    auto count = keyfiles.count("master_key");
    if (count >1)
    {
        {BOOST_LOG_SEV(log.lg, info) << "error derive keys:  master key is not the only one" ;}
        return false;
    }
    else if (count <1)
    {
        {BOOST_LOG_SEV(log.lg, info) << "error derive keys:  master key not found" ;}
        return false;
    }

    auto password = [&masterkey_pass](const std::string& keyname)->byte_seq_t{
        byte_seq_t res;
        std::copy(masterkey_pass.begin(), masterkey_pass.end(), std::back_inserter(res));
        return res;
    };

    auto params = fc_light::json::from_string(json);
    create_t params_;
    path_levels_t path;
    try {
        params_ = params.as<create_t>();
        path = params_.path.as<path_levels_t>();
    }
    catch (const std::exception &e) {throw std::runtime_error(e.what());}
    catch (const fc_light::exception &e) {throw std::runtime_error(e.what());}

    if (params_.keyname == "master_key")
    {
        BOOST_LOG_SEV(log.lg, info) << "keyname \"master_key\"is not may be used for private key";
        return false;
    }

    FC_LIGHT_ASSERT(path.root == "m");

    auto secret = get_master_key(password);
    dev::bytes priv_key(secret.first.data(), secret.first.data()+32);

    Coin::HDKeychain hd(priv_key, secret.second);
    boost::hana::for_each(  level_static_list,
                     [&](auto a)
                     {
                         using a_type = decltype(a);
                         constexpr auto level = static_cast<levels_te>(a_type::value);
                         int value = 0;
                         switch (level)
                         {
                             case(levels_te::purpose):      {value=path.purpose; break;}
                             case(levels_te::coin_type):    {value=path.coin_type; break;}
                             case(levels_te::change):       {value=path.change; break;}
                             case(levels_te::address_index):{value=path.address_index;}
                         }
                         hd = hd.getChild(0x80000000|value);
                         if (level == levels_te::address_index) {
                             res = keyfiles.create(std::bind(create_new_keyfile,
                                                       params_.keyname, params_.description, params_.encrypted,
                                                       params_.cipher, params_.curve,
                                                       password,
                                                       hd.privkey(),
                                                       hd.chain_code()
                             ));
                             auto& sql = sql_singleton::instance();
                             backup_t backup(params_.keyname, params_.path);
                             sql.insert_path(backup);
                             auto json = fc_light::json::to_string(backup);
                             if (res)
                                {BOOST_LOG_SEV(log.lg, info) << "derive key: " << json;}
                             else
                                {BOOST_LOG_SEV(log.lg, info) << "error to derive key: " << json;}
                         }
                     });
    return res;
}


std::pair<dev::Secret, dev::bytes> keydata::get_master_key( get_password_create_f&& get_passwd)
{
    dev::Secret priv_key;
    dev::bytes chain_code;
    std::string keyname= "master_key";

    auto& keyfiles = keyfile_singleton::instance();
    auto& keyfile = keyfiles[keyname];

    if (keyfile.keyinfo.encrypted)
    {
        auto passwd = get_passwd(keyname);//operation canceled exception need to be thrown into get_password functor
        try {
            auto encrypted_data = keyfile.keyinfo.priv_key_data.as<keyfile_format::encrypted_data>();
            auto encrypted_chain_code = keyfile.keyinfo.chain_code_data.as<keyfile_format::encrypted_data>();
            auto& encryptor = encryptor_singleton::instance();
            priv_key = encryptor.decrypt_private_key(passwd, encrypted_data);
            if (encrypted_chain_code.enc_data != "")
            {
                auto secret = encryptor.decrypt_private_key(passwd, encrypted_chain_code);
                chain_code.assign(secret.data(), secret.data()+32);
            }
        }
        catch (const std::exception &e) {throw std::runtime_error(e.what());}
        catch (const fc_light::exception &e) {throw std::runtime_error(e.what());}
    }
    else
    {
        try {
            priv_key = keyfile.keyinfo.priv_key_data.as<dev::Secret>();
            chain_code = keyfile.keyinfo.chain_code_data.as<dev::bytes>();
        }
        catch (const std::exception &e) {throw std::runtime_error(e.what());}
        catch (const fc_light::exception &e) {throw std::runtime_error(e.what());}
    }
    return std::make_pair(priv_key, chain_code);
}


int keydata::restore(const char * filename, std::string& mnemonics, std::string& masterkey_pass)
{
    using namespace keydata;

    auto& log = logger_singleton::instance();
    auto& keyfiles = keyfile_singleton::instance();
    if (keyfiles.is_exist("master_key"))
    {
        BOOST_LOG_SEV(log.lg, info) << "error restore: master_key already exist";
        return false;
    }

    int count = 0;
    auto file = std::ifstream(filename);
    if(!file.is_open())
        FC_LIGHT_THROW_EXCEPTION(fc_light::internal_error_exception,
                                 "Cannot open restore file ${filename}", ("filename", filename));

    BOOST_LOG_SEV(log.lg, info) << "restore keydata";

    const int buf_size = 1000;
    char buf[buf_size];
    std::vector<std::string> json;
    while(true) {
        file.getline(buf, buf_size);
        if (file.eof() || !file.good())
            break;
        json.push_back(std::string(buf, file.gcount()-1));
        BOOST_LOG_SEV(log.lg, info) << "restore path: " << json.back();
    }

    auto res = derive_masterkey(mnemonics, masterkey_pass);
    if (!res)
    {
        BOOST_LOG_SEV(log.lg, info) << "restore error ";
        return count;
    }

    for (auto &a: json)
    {
        auto variant  = fc_light::json::from_string(a);
        backup_t backup;
        try {
            backup = variant.as<backup_t>();
        }
        catch (const std::exception &e) {throw std::runtime_error(e.what());}
        catch (const fc_light::exception &e) {throw std::runtime_error(e.what());}

        create_t params;
        params.keyname = backup.keyname;
        params.description = "";
        params.encrypted = false;
        params.cipher = keyfile_format::cipher_etype::aes256;
        params.curve = keyfile_format::curve_etype::secp256k1;
        params.path = backup.path;

        auto params_json = fc_light::json::to_string(params);
        if (derive_key(masterkey_pass, params_json))
            count++;
    }
    return count;
}


int keydata::backup(const char * filename)
{
    int count = 0;
    auto file = std::ofstream(filename);
    if (!file.is_open())
        FC_LIGHT_THROW_EXCEPTION(fc_light::internal_error_exception, "Cannot open backup file (${filename})", ("filename", filename));

    auto& log = logger_singleton::instance();
    BOOST_LOG_SEV(log.lg, info) << "backup keydata";

    auto& sql = sql_singleton::instance();
    auto backup_list = std::move(sql.select_path());
    for (auto& a : backup_list)
    {
        auto json = fc_light::json::to_string(a);
        BOOST_LOG_SEV(log.lg, info) << json;
        file << json << std::endl;
        count++;
    }
    BOOST_LOG_SEV(log.lg, info) << "backup "+std::to_string(count)+" keys";

    return count;
}