#ifndef ARABIC_NUM_2_WORDS_HPP
#define ARABIC_NUM_2_WORDS_HPP

#include <string>
#include <vector>

namespace RHVoice {
namespace sapi {

class ArabicNum2Words {
public:
    // Converts a number string like "1234" to Arabic words "ألف ومائتان وأربعة وثلاثون"
    static std::wstring convert(long long number);
    
    // Parses text and replaces all digits with Arabic words
    static std::wstring replace_numbers_in_text(const std::wstring& text);

private:
    static std::wstring get_group_string(int group, int group_index);
    static std::wstring get_tens_string(int tens);
    static std::wstring get_ones_string(int ones);
    static std::wstring get_hundreds_string(int hundreds);
};

} // namespace sapi
} // namespace RHVoice

#endif // ARABIC_NUM_2_WORDS_HPP
