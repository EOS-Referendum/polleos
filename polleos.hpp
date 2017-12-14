#pragma once

#include <eoslib/eos.hpp>
#include <eoslib/db.hpp>
#include <eoslib/string.hpp>

#define CONTRACT_NAME polleos
#define CONTRACT_NAME_UINT64 N(polleos)

namespace CONTRACT_NAME {

  const size_t max_options = 32;

  struct PACKED (option) {
    eosio::string name;

    option(eosio::string name) : name(name) {}
    option() {}
  };

  struct PACKED (option_result) : option {
    uint64_t votes = 0;

    option_result(const eosio::string& name, uint64_t votes) : option(name), votes(votes ) {}
    option_result(const eosio::string& name) : option(name), votes(0) {}
    option_result(const option& opt) : option(opt), votes(0) {}
    option_result() {}
  };

  typedef option_result* results_array;

  //@abi action newmultiopt
  struct PACKED (multi_opt_poll_msg) {
    eosio::string question;
    uint8_t options_len;
    option options[max_options];

    bool is_valid() const {
      //Check if question and all options have descriptions
      if ( question.get_size() == 0 )
        return false;
      for (int i = 0; i < options_len; i++) {
        if ( options[i].name.get_size() == 0 )
          return false;
      }
      return true;
    }
  };

  //@abi table
  struct PACKED (multi_opt_poll) {
    eosio::string question;
    uint8_t results_len;
    option_result results[max_options];

    multi_opt_poll() {}
    multi_opt_poll(const multi_opt_poll_msg& msg) : question(msg.question) {
      for (int i = 0; i < msg.options_len; i++) {
        results[i] = option(msg.options[i]);
      }
      results_len = msg.options_len;
    }

    bool has_option(uint32_t option_num) const {
      //TODO: Adapt for dynamic arrays
      eosio::print("option_num: ", option_num);
      return (option_num >= 1 && option_num <= 4 && results[option_num-1].name.get_size() > 0);
      //return (option_num >= 1 && option_num <= results_len);
    }

    // Returns true if vote is successfully added
    bool add_vote(uint32_t option_num) {
      eosio::print("option_num: ", option_num);
      if ( has_option(option_num) ) {
        results[option_num - 1].votes++;
        return true;
      } else return false;
    }
  };

  //@abi action vote
  struct multi_opt_vote {
    eosio::string question;
    account_name  voter;
    uint32_t      option;
  };

  inline int32_t load_poll(const eosio::string& question, table_name table, char* buffer, uint32_t size) {
    return load_str(CONTRACT_NAME_UINT64, CONTRACT_NAME_UINT64, table,
             (char*)question.get_data(), question.get_size(), buffer, size);
  }

  // Returns true if finds a poll with specified key (question)
  bool get_poll(const eosio::string& question, multi_opt_poll& poll);

  bool poll_exists(const eosio::string& question, table_name poll_type) {
    char buff[1];
    int r = load_poll(question, poll_type, buff, 1);
    return r > -1;
  }

  inline bool poll_exists(const multi_opt_poll_msg& poll_msg) {
    return poll_exists(poll_msg.question, N(multioptpoll));
  }

  inline bool poll_exists(const multi_opt_poll& poll) {
    return poll_exists(poll.question, N(multioptpoll));
  }

  bool has_voted(const eosio::string& question, account_name account, table_name poll_type) {
    int32_t r = load_str(account, CONTRACT_NAME_UINT64, poll_type,
                         (char*)question.get_data(), question.get_size(), nullptr, 0);
    return r >= 0;
  }

  inline bool has_voted(const multi_opt_vote& vote) {
    return has_voted(vote.question, vote.voter, N(optvotes));
  }
}

