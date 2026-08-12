/* Copyright (C) 2026  Arabic RHVoice Module */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

#include <list>
#include <algorithm>
#include <iterator>
#include "core/path.hpp"
#include "core/utterance.hpp"
#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/arabic.hpp"

namespace RHVoice
{
  // ============================================================================
  // arabic_info — Language metadata and registration
  // ============================================================================

  arabic_info::arabic_info(const std::string& data_path, const std::string& userdict_path):
    language_info("Arabic", data_path, userdict_path)
  {
    set_alpha2_code("ar");
    set_alpha3_code("ara");

    // -----------------------------------------------------------------------
    // Register Arabic Unicode letter ranges
    // -----------------------------------------------------------------------

    // Basic Arabic block: U+0621 (Hamza) through U+063A (Ghain) = 26 chars
    register_letter_range(0x0621, 26);
    // U+0641 (Fa) through U+064A (Ya) = 10 chars
    register_letter_range(0x0641, 10);

    // Tatweel (kashida) — treat as letter for tokenization continuity
    register_letter(0x0640);

    // Alef variants that may appear as standalone letters
    register_letter(0x0622); // Alef with Madda
    register_letter(0x0623); // Alef with Hamza Above
    register_letter(0x0625); // Alef with Hamza Below
    register_letter(0x0671); // Alef Wasla

    // -----------------------------------------------------------------------
    // Register Arabic diacritical marks as "signs"
    // Signs are recognized but not treated as standalone letters
    // -----------------------------------------------------------------------
    // U+064B (Fathatan) through U+0652 (Sukun) = 8 diacritics
    for(utf8::uint32_t cp = 0x064B; cp <= 0x0652; ++cp)
      register_sign(cp);

    // Superscript Alef (used in some Quran typography)
    register_sign(0x0670);

    // -----------------------------------------------------------------------
    // Vowel letters — Arabic "matres lectionis"
    // These letters serve as long-vowel carriers
    // -----------------------------------------------------------------------
    register_vowel_letter(0x0627); // Alif (ا) — long /aa/
    register_vowel_letter(0x0648); // Waw  (و) — long /uu/
    register_vowel_letter(0x064A); // Ya   (ي) — long /ii/
    register_vowel_letter(0x0649); // Alif Maqsura (ى) — final /aa/
  }

  std::shared_ptr<language> arabic_info::create_instance() const
  {
    return std::shared_ptr<language>(new arabic(*this));
  }

  // ============================================================================
  // arabic — Runtime language implementation
  // ============================================================================

  arabic::arabic(const arabic_info& info_):
    language(info_),
    info(info_),
    g2p_fst(path::join(info_.get_data_path(), "g2p.fst"))
  {
    hts_labeller& labeller=get_hts_labeller();
    labeller.define_extra_phonetic_feature("emphatic");
  }

  // ---------------------------------------------------------------------------
  // decode_as_word: tokenizer calls this to split a token into word items.
  // Arabic is unicase, so no case conversion is needed — we pass through.
  // ---------------------------------------------------------------------------
  void arabic::decode_as_word(item& token, const std::string& token_name) const
  {
    item& word = token.append_child();
    word.set("name", token_name);
  }

  // ---------------------------------------------------------------------------
  // decode_as_letter_sequence: spell out individual letters.
  // Uses the spell.fst to map each Arabic letter to its spoken name.
  // ---------------------------------------------------------------------------
  void arabic::decode_as_letter_sequence(item& token, const std::string& token_name) const
  {
    auto cur = str::utf8_string_begin(token_name);
    auto end = str::utf8_string_end(token_name);
    while(cur != end)
    {
      auto next = cur;
      ++next;
      spell_fst.translate(cur, next, token.back_inserter());
      cur = next;
    }
  }

  // ---------------------------------------------------------------------------
  // get_word_transcription: THE CORE G2P FUNCTION
  // Takes an item representing a word and returns its phoneme sequence.
  // Delegates to g2p.fst which encodes the Mantoq phonetisation rules.
  // ---------------------------------------------------------------------------
  std::vector<std::string> arabic::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    const std::string& name = word.get("name").as<std::string>();

    // Explode UTF-8 word into individual grapheme strings for FST input
    std::vector<std::string> g2p_input;
    str::utf8explode(name, std::back_inserter(g2p_input));

    // Run G2P FST
    g2p_fst.translate(g2p_input.begin(), g2p_input.end(),
                      std::back_inserter(transcription));

    return transcription;
  }

  // ---------------------------------------------------------------------------
  // post_lex: post-lexical processing — assigns stress.
  // Arabic stress rules (simplified MSA):
  //   - If word has a superheavy final syllable (CVCC or CVVC), stress it
  //   - Else stress the penultimate if heavy (CVC or CVV)
  //   - Else stress the antepenultimate
  // For initial implementation, default to penultimate stress.
  // ---------------------------------------------------------------------------
  void arabic::post_lex(utterance& u) const
  {
    relation& sylstruct_rel = u.get_relation("SylStructure");
    for(relation::iterator word_iter(sylstruct_rel.begin());
        word_iter != sylstruct_rel.end(); ++word_iter)
    {
      // Count syllables
      unsigned int syl_count = 0;
      for(item::iterator syl_iter = word_iter->begin();
          syl_iter != word_iter->end(); ++syl_iter)
      {
        ++syl_count;
      }

      if(syl_count == 0)
        continue;

      if(syl_count == 1)
      {
        // Monosyllabic: stress the only syllable
        word_iter->first_child().set<std::string>("stress", "1");
      }
      else
      {
        // Polysyllabic: default to penultimate stress
        item& last_syl = word_iter->last_child();
        if(last_syl.has_prev())
          last_syl.prev().set<std::string>("stress", "1");
        else
          last_syl.set<std::string>("stress", "1");
      }
    }
  }
}
