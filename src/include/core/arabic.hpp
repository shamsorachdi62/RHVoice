/* Copyright (C) 2026  Arabic RHVoice Module */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

#ifndef RHVOICE_ARABIC_HPP
#define RHVOICE_ARABIC_HPP

#include "str.hpp"
#include "fst.hpp"
#include "language.hpp"

namespace RHVoice
{
  class arabic_info: public language_info
  {
  public:
    arabic_info(const std::string& data_path, const std::string& userdict_path);

    #ifdef WIN32
    unsigned short get_id() const
    {
      return 0x0401; // ar-SA LCID
    }
    #endif

    bool has_unicase_alphabet() const
    {
      return true; // Arabic script has no upper/lower case
    }

  private:
    std::shared_ptr<language> create_instance() const;
  };

  class arabic: public language
  {
  public:
    explicit arabic(const arabic_info& info);

    const arabic_info& get_info() const
    {
      return info;
    }

    std::vector<std::string> get_word_transcription(const item& word) const;
    void decode_as_word(item& token, const std::string& token_name) const;
    void decode_as_letter_sequence(item& token, const std::string& token_name) const;

  private:
    void post_lex(utterance& u) const;

    const arabic_info& info;
    const fst g2p_fst;
  };
}
#endif
