//
// Created by roman on 4/6/18.
//

#ifndef KEYCHAINAPP_KEYCHAIN_HPP
#define KEYCHAINAPP_KEYCHAIN_HPP

#include <functional>
#include <vector>
#include <iostream>
#include <fstream>

#include <fc_light/crypto/hex.hpp>
#include <fc_light/variant.hpp>

#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>

#include "keychain_commands.hpp"

#ifdef __linux__
#define KEY_DEFAULT_PATH  "/var/keychain/key_data"
#define LOG_DEFAULT_PATH  "/var/keychain/logs"
#else

    #if defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
    //#error "Need to define path to KEYCHAIN_DATA"
        #define KEY_DEFAULT_PATH  "/var/keychain/key_data"
        #define LOG_DEFAULT_PATH  "/var/keychain/logs"
//#define KEY_DEFAULT_PATH  "var/keychain/key_data"
//#define LOG_DEFAULT_PATH  "var/keychain/logs"
    #else
        #error "Need to define path to KEYCHAIN_DATA"
    #endif
#endif


namespace keychain_app
{

namespace bfs = boost::filesystem;


class secure_dlg_mod_base
{
public:
    using string_list = std::list<std::wstring>;

    virtual ~secure_dlg_mod_base(){}
    virtual byte_seq_t get_passwd_trx_raw(const std::string& raw_trx,  std::string binary_dir) const = 0;
//  virtual std::wstring get_passwd_trx(const graphene::chain::transaction& trx) const = 0;
    virtual byte_seq_t get_passwd_on_create(std::string binary_dir) const = 0;
    virtual void print_mnemonic(const string_list& mnemonic) const = 0;
};


class keychain : public keychain_base
{
public:
  static keychain& instance(const secure_dlg_mod_base* );
  virtual ~keychain();
  virtual std::string operator()(const fc_light::variant& command) override;
private:
  bfs::path m_init_path;
  keychain(const secure_dlg_mod_base* , const char* default_key_dir = KEY_DEFAULT_PATH);
};

struct keychain_commands_singletone
{
    using command_ptr = std::shared_ptr<keychain_command_base>;
    static const keychain_commands_singletone& instance();
    const command_ptr operator[](command_te cmd_type) const;
private:
    std::vector<command_ptr> m_command_list;
    keychain_commands_singletone();
};

}

#endif //KEYCHAINAPP_KEYCHAIN_HPP
