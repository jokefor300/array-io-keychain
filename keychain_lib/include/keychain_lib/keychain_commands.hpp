//
// Created by roman on 4/20/18.
//

#ifndef KEYCHAINAPP_KEYCHAIN_COMMANDS_HPP
#define KEYCHAINAPP_KEYCHAIN_COMMANDS_HPP

#include <string.h>
#include <iostream>

#include <type_traits>
#include <string>
#include <fc_light/reflect/reflect.hpp>
#include <fc_light/variant.hpp>
#include <boost/hana/range.hpp>
#include <boost/filesystem.hpp>

#include <fc_light/variant.hpp>
#include <fc_light/io/json.hpp>
#include <fc_light/exception/exception.hpp>
#include <fc/crypto/hex.hpp>

#include <fc_light/reflect/variant.hpp>

#include <graphene/utilities/key_conversion.hpp>
#include <boost/signals2.hpp>

#include "key_file_parser.hpp"
#include "key_encryptor.hpp"
#include "sign_define.hpp"
#include "sig.hpp"
#include <ethereum/core/FixedHash.h>
#include <ethereum/crypto/Common.h>

namespace keychain_app {

using byte_seq_t = std::vector<char>;

struct keychain_error: std::runtime_error
{
  keychain_error(int id_, const char* errmsg): std::runtime_error(errmsg), id(id_){}
  int id;
};

class keychain_base
{
public:
    using string_list = std::list<std::wstring>;
    keychain_base(std::string&& uid_hash_);
    virtual ~keychain_base();
    virtual std::string operator()(const fc_light::variant& command) = 0;
    boost::signals2::signal<byte_seq_t(const std::string&)> get_passwd_trx_raw;
    boost::signals2::signal<byte_seq_t(void)> get_passwd_on_create;
    boost::signals2::signal<void(const string_list&)> print_mnemonic;
    std::string uid_hash;
};

template <typename char_t>
fc_light::variant open_keyfile(const char_t* filename)
{
  std::ifstream fin = std::ifstream(filename);
  if(!fin.is_open())
    throw std::runtime_error("Error: cannot open keyfile");
  std::array<char, 1024> read_buf;
  memset(read_buf.data(), 0x00, read_buf.size());
  auto pbuf = read_buf.data();
  auto it = read_buf.begin();
  size_t read_count = 0;
  while(true)
  {
    fin.getline(pbuf, std::distance(it, read_buf.end()));
    if (fin.eof() || !fin.good())
        break;
    pbuf += fin.gcount() - 1;
    it += fin.gcount() - 1;
    read_count += fin.gcount() - 1;
  }
  if(!fin.good()&&read_count==0)
    throw std::runtime_error("Error: cannot read keyfile");
  return fc_light::json::from_string(std::string(read_buf.begin(), read_buf.end()), fc_light::json::strict_parser);
}

void create_keyfile(const char* filename, const fc_light::variant& keyfile_var);
secp256_private_key get_priv_key_from_str(const std::string& str);
fc::sha256 get_hash(const keychain_app::unit_list_t &list);
size_t from_hex(const std::string& hex_str, unsigned char* out_data, size_t out_data_len );
std::string to_hex(const uint8_t* data, size_t length);
/*{
  using out_map = std::map<std::string, nlohmann::json>;
  using out_map_val = out_map::value_type;
  out_map result;
  result.insert(out_map_val(json_parser::json_keys::RESULT,to_hex(signature.begin(),signature.size())));
  return result;
}*/

struct json_response
{
    json_response(){}
    json_response(const fc_light::variant& var, int id_): id(id_), result(var){}
    json_response(const char* result_, int id_): id(id_), result(result_){}
    json_response(const fc_light::variants& var, int id_): id(id_), result(var){}
    int id;
    fc_light::variant result;
};

struct json_error
{
  json_error(int id_, const char* str): id(id_), error(str){}
  int id;
  std::string error;
};

namespace hana = boost::hana;
namespace bfs = boost::filesystem;

enum struct command_te {
    null = 0,
    help,
    list,
    sign,
    create,
    import_cmd,
    export_cmd,
    remove,
    restore,
    seed,
    public_key,
    last
};

struct find_keyfile_by_username
{
  find_keyfile_by_username(const char* keyname, keyfile_format::keyfile_t* keyfile = nullptr)
    : m_keyname(keyname)
    , m_keyfile(keyfile)
  {
  }

  bool operator()(bfs::directory_entry &unit)
  {
    if (!bfs::is_regular_file(unit.status()))
      return false;
    const auto &file_path = unit.path().filename();
    
    auto j_keyfile = open_keyfile(file_path.c_str());
    auto keyfile = j_keyfile.as<keyfile_format::keyfile_t>();
    if(m_keyfile)
      *m_keyfile = keyfile;//NOTE: move semantic is not implemented in fc_light::variant in fact
    return strcmp(m_keyname, keyfile.keyname.c_str()) == 0;
  }
  const char* m_keyname;
  keychain_base* m_pkeychain;
  keyfile_format::keyfile_t* m_keyfile;
};

struct keychain_command_common {
  keychain_command_common (command_te etype = command_te::null, int id_ = 0)
    : command(etype)
    , id(id_){}
  command_te command;
  int id;
  fc_light::variant params;
};

struct keychain_command_base {
    keychain_command_base(command_te type): e_type(type){}
    virtual ~keychain_command_base(){}
    command_te e_type;
    virtual std::string operator()(keychain_base* keychain, const fc_light::variant& params_variant, int id) const = 0;
};

template<command_te cmd>
struct keychain_command: keychain_command_base
{
    keychain_command():keychain_command_base(cmd){}
    virtual ~keychain_command(){}
    virtual std::string operator()(keychain_base* keychain, const fc_light::variant& params_variant, int id) const override
    {
      return fc_light::json::to_pretty_string(fc_light::variant(json_error(id, "method is not implemented")));
    }
    using params_t = void;
};

template<>
struct keychain_command<command_te::sign> : keychain_command_base
{
  keychain_command():keychain_command_base(command_te::sign){}
  virtual ~keychain_command(){}
  struct params
  {
      std::string chainid;
      std::string transaction;
      std::string keyname;
  };

  using params_t = params;

  virtual std::string operator()(keychain_base* keychain, const fc_light::variant& params_variant, int id) const override
  {
    try {
      auto params = params_variant.as<params_t>();
      unit_list_t unit_list;
      fc::ecc::private_key private_key;
      if (!params.chainid.empty())
        unit_list.push_back(fc::sha256(params.chainid));

      //NOTE: using vector instead array because move semantic is implemented in the vector
      std::vector<char> buf(1024);
      auto trans_len = fc::from_hex(params.transaction, buf.data(), buf.size());
      buf.resize(trans_len);

      keyfile_format::keyfile_t keyfile;

      unit_list.push_back(buf);
      if (params.keyname.empty())
        std::runtime_error("Error: keyname is not specified");
      
      auto curdir = bfs::current_path();
      auto first = bfs::directory_iterator(bfs::path("./"));
      auto it = std::find_if(first, bfs::directory_iterator(),find_keyfile_by_username(params.keyname.c_str(), &keyfile));
      if (it == bfs::directory_iterator())
        throw std::runtime_error("Error: keyfile could not found by keyname");
      
      if(keyfile.uid_hash != keychain->uid_hash)
        std::runtime_error("Error: user is not keyfile owner");

      std::string key_data;
      if(keyfile.keyinfo.encrypted)
      {
        auto encrypted_data = keyfile.keyinfo.priv_key_data.as<keyfile_format::encrypted_data>();
        auto& encryptor = encryptor_singletone::instance();
        //TODO: need to try to parse transaction.
        // If we can parse transaction we need to use get_passwd_trx function
        // else use get_passwd_trx_raw()
        // At this moment parsing of transaction is not implemented
        byte_seq_t passwd = *(keychain->get_passwd_trx_raw(params.transaction));
        if (passwd.empty())
          throw std::runtime_error("Error: can't get password");
        key_data = std::move(encryptor.decrypt_keydata(passwd, encrypted_data));
      }
      else
      {
        key_data = std::move(keyfile.keyinfo.priv_key_data.as<std::string>());
      }
      private_key = get_priv_key_from_str(key_data);
//      auto signature = private_key.sign_compact(get_hash(unit_list));

      std::vector<byte> hash (private_key.get_secret().data(),
                           private_key.get_secret().data() + private_key.get_secret().data_size());
      auto signature = dev::sign(
              //dev::SecureFixedHash<32>( ((dev::bytes const*) &hash), dev::SecureFixedHash<32>::ConstructFromPointerType::ConstructFromPointer ),
              dev::SecureFixedHash<32>(
                      dev::FixedHash<32>(((byte const*) private_key.get_secret().data()),
                                         dev::FixedHash<32>::ConstructFromPointerType::ConstructFromPointer)
                      ),
              dev::FixedHash<32>(((byte const*) get_hash(unit_list).data()),
                                 dev::FixedHash<32>::ConstructFromPointerType::ConstructFromPointer)
                                );

      json_response response(to_hex(signature.begin(), signature.size).c_str(), id);
  //    json_response response(to_hex(signature.begin(), signature.size()).c_str(), id);
      fc_light::variant res(response);
      return fc_light::json::to_pretty_string(res);
    }
    catch (const std::exception &exc)
    {
      std::cerr << fc_light::json::to_pretty_string(fc_light::variant(json_error(id, exc.what()))) << std::endl;
      return fc_light::json::to_pretty_string(fc_light::variant(json_error(id, exc.what())));
    }
    catch (const fc_light::exception& exc)
    {
      std::cerr << fc_light::json::to_pretty_string(fc_light::variant(json_error(0, exc.to_detail_string().c_str()))) << std::endl;
      return fc_light::json::to_pretty_string(fc_light::variant(json_error(0, exc.what())));
    }
  }
};

template <>
struct keychain_command<command_te::create>: keychain_command_base
{
    keychain_command():keychain_command_base(command_te::create){}
    virtual ~keychain_command(){}
    struct params
    {
      std::string keyname;
      bool encrypted;
      keyfile_format::cipher_etype cipher;
      keyfile_format::keyfile_t::keyinfo_t::curve_etype curve;
    };
    using params_t = params;
    virtual std::string operator()(keychain_base* keychain, const fc_light::variant& params_variant, int id) const override
    {
      try
      {
        auto params = params_variant.as<params_t>();
        keyfile_format::keyfile_t keyfile;
        std::string wif_key;
        fc::ecc::public_key_data public_key_data;
        switch (params.curve)
        {
          case keyfile_format::keyfile_t::keyinfo_t::curve_etype::secp256k1:
          {
            auto priv_key = fc::ecc::private_key::generate();
            public_key_data = priv_key.get_public_key().serialize();
            wif_key = std::move(graphene::utilities::key_to_wif(priv_key));
          }
            break;
          default:
          {
            throw std::runtime_error("Error: unsupported curve format");
          }
        }
        if (params.encrypted)
        {
          auto passwd = *keychain->get_passwd_on_create();
          if (passwd.empty())
            throw std::runtime_error("Error: can't get password");
          auto& encryptor = encryptor_singletone::instance();
          auto enc_data = encryptor.encrypt_keydata(params.cipher, passwd, wif_key);
          keyfile.keyinfo.priv_key_data = fc_light::variant(enc_data);
          keyfile.keyinfo.encrypted = true;
        }
        else{
          keyfile.keyinfo.priv_key_data = std::move(wif_key);
          keyfile.keyinfo.encrypted = false;
        }
        keyfile.keyinfo.public_key = to_hex(reinterpret_cast<const uint8_t *>(public_key_data.begin()), public_key_data.size());
        keyfile.keyname = params.keyname;
        keyfile.uid_hash = keychain->uid_hash;
        keyfile.filetype = keyfile_format::TYPE_KEY;
        keyfile.keyinfo.format = keyfile_format::keyfile_t::keyinfo_t::FORMAT_ARRAYIO;
        keyfile.keyinfo.curve_type = params.curve;
        std::string filename(keyfile.keyname);
        if(filename.empty())
          throw std::runtime_error("Error: keyname (filename) is empty");
        filename += ".json";
        auto first = bfs::directory_iterator(bfs::path("./"));
        auto it = std::find_if(first, bfs::directory_iterator(),find_keyfile_by_username(keyfile.keyname.c_str()));
        if(it != bfs::directory_iterator())
          throw std::runtime_error("Error: keyfile for this user is already exist");
        create_keyfile(filename.c_str(), fc_light::variant(keyfile));

        json_response response(true, id);
        return fc_light::json::to_pretty_string(fc_light::variant(response));
      }
      catch (const std::exception &exc)
      {
        std::cerr << fc_light::json::to_pretty_string(fc_light::variant(json_error(id, exc.what()))) << std::endl;
        return fc_light::json::to_pretty_string(fc_light::variant(json_error(id, exc.what())));
      }
      catch (const fc_light::exception& exc)
      {
        std::cerr << fc_light::json::to_pretty_string(fc_light::variant(json_error(0, exc.to_detail_string().c_str()))) << std::endl;
        return fc_light::json::to_pretty_string(fc_light::variant(json_error(0, exc.what())));
      }
    }
};

template <>
struct keychain_command<command_te::list>: keychain_command_base {
  keychain_command() : keychain_command_base(command_te::list) {}
  virtual ~keychain_command() {}
  
  using params_t = void;
  
  virtual std::string operator()(keychain_base *keychain, const fc_light::variant &params_variant, int id) const override
  {
    try {
      fc_light::variants keyname_list;
      keyname_list.reserve(128);
      auto first = bfs::directory_iterator(bfs::path("./"));
      std::for_each(first, bfs::directory_iterator(), [&keyname_list](bfs::directory_entry &unit){
        if (!bfs::is_regular_file(unit.status()))
          return;
        const auto &file_path = unit.path().filename();
  
        auto j_keyfile = open_keyfile(file_path.c_str());
        auto keyfile = j_keyfile.as<keyfile_format::keyfile_t>();
        keyname_list.push_back(fc_light::variant(std::move(keyfile.keyname)));
        return;
      });
	  json_response response(keyname_list, id);
	  return fc_light::json::to_pretty_string(fc_light::variant(response));
	}
    catch (const std::exception &exc)
    {
      std::cerr << fc_light::json::to_pretty_string(fc_light::variant(json_error(id, exc.what()))) << std::endl;
	  return fc_light::json::to_pretty_string(fc_light::variant(json_error(id, exc.what())));
    }
    catch (const fc_light::exception& exc)
    {
      std::cerr << fc_light::json::to_pretty_string(fc_light::variant(json_error(0, exc.to_detail_string().c_str()))) << std::endl;
	  return fc_light::json::to_pretty_string(fc_light::variant(json_error(0, exc.what())));
    }
  }
};

template <>
struct keychain_command<command_te::remove>: keychain_command_base
{
  keychain_command():keychain_command_base(command_te::remove){}
  virtual ~keychain_command(){}
  struct params
  {
    std::string keyname;
  };
  using params_t = params;
  virtual std::string operator()(keychain_base* keychain, const fc_light::variant& params_variant, int id) const override {
    try {
      auto params = params_variant.as<params_t>();
      keyfile_format::keyfile_t keyfile;
      auto first = bfs::directory_iterator(bfs::path("./"));
      auto it = std::find_if(first, bfs::directory_iterator(),find_keyfile_by_username(params.keyname.c_str(), &keyfile));
      if(it != bfs::directory_iterator())
      {
        if(keyfile.uid_hash != keychain->uid_hash)
          throw std::runtime_error("Error: can't remove keyfile because of it is owned by different user");
        bfs::remove(*it);
      }
	    json_response response(true, id);
	    return fc_light::json::to_pretty_string(fc_light::variant(response));
	  }
    catch (const std::exception &exc)
    {
	  std::cerr << fc_light::json::to_pretty_string(fc_light::variant(json_error(id, exc.what()))) << std::endl;
	  return fc_light::json::to_pretty_string(fc_light::variant(json_error(id, exc.what())));
    }
    catch (const fc_light::exception& exc)
    {
      std::cerr << fc_light::json::to_pretty_string(fc_light::variant(json_error(0, exc.to_detail_string().c_str()))) << std::endl;
	  return fc_light::json::to_pretty_string(fc_light::variant(json_error(0, exc.what())));
    }
  }
};

template<>
struct keychain_command<command_te::public_key>: keychain_command_base
{
  keychain_command(): keychain_command_base(command_te::public_key){}
  virtual ~keychain_command(){}
  struct params
  {
    std::string keyname;
  };
  using params_t = params;
  virtual std::string operator()(keychain_base* keychain, const fc_light::variant& params_variant, int id) const override {
    try {
      auto params = params_variant.as<params_t>();
      keyfile_format::keyfile_t keyfile;
  
      if (params.keyname.empty())
        std::runtime_error("Error: keyname is not specified");
  
      auto curdir = bfs::current_path();
      auto first = bfs::directory_iterator(bfs::path("./"));
      auto it = std::find_if(first, bfs::directory_iterator(),find_keyfile_by_username(params.keyname.c_str(), &keyfile));
      if (it == bfs::directory_iterator())
        throw std::runtime_error("Error: keyfile could not found by keyname");
      
      json_response response(keyfile.keyinfo.public_key.c_str(), id);
      return fc_light::json::to_pretty_string(fc_light::variant(response));
    }
    catch (const std::exception &exc)
    {
      std::cerr << fc_light::json::to_pretty_string(fc_light::variant(json_error(id, exc.what()))) << std::endl;
      return fc_light::json::to_pretty_string(fc_light::variant(json_error(id, exc.what())));
    }
    catch (const fc_light::exception& exc)
    {
      std::cerr << fc_light::json::to_pretty_string(fc_light::variant(json_error(0, exc.to_detail_string().c_str()))) << std::endl;
      return fc_light::json::to_pretty_string(fc_light::variant(json_error(0, exc.what())));
    }
  }
};

constexpr auto cmd_static_list =
    hana::make_range(
        hana::int_c<static_cast<int>(command_te::null)>,
        hana::int_c<static_cast<int>(command_te::last)>);

}

FC_LIGHT_REFLECT_ENUM(
  keychain_app::command_te,
    (null)
    (help)
    (list)
    (sign)
    (create)
    (import_cmd)
    (export_cmd)
    (remove)
    (restore)
    (seed)
    (public_key)
    (last))

FC_LIGHT_REFLECT(keychain_app::keychain_command<keychain_app::command_te::sign>::params_t, (chainid)(transaction)(keyname))
FC_LIGHT_REFLECT(keychain_app::keychain_command<keychain_app::command_te::create>::params_t, (keyname)(encrypted)(cipher)(curve))
FC_LIGHT_REFLECT(keychain_app::keychain_command<keychain_app::command_te::remove>::params_t, (keyname))
FC_LIGHT_REFLECT(keychain_app::keychain_command<keychain_app::command_te::public_key>::params_t, (keyname))
FC_LIGHT_REFLECT(keychain_app::keychain_command_common, (command)(id)(params))
FC_LIGHT_REFLECT(keychain_app::json_response, (id)(result))
FC_LIGHT_REFLECT(keychain_app::json_error, (id)(error))

#endif //KEYCHAINAPP_KEYCHAIN_COMMANDS_HPP
