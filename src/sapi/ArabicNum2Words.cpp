#include "ArabicNum2Words.hpp"
#include <regex>

namespace RHVoice {
namespace sapi {

static const std::wstring ones[] = {
    L"", L"واحد", L"اثنان", L"ثلاثة", L"أربعة", L"خمسة", L"ستة", L"سبعة", L"ثمانية", L"تسعة"
};

static const std::wstring tens[] = {
    L"", L"عشرة", L"عشرون", L"ثلاثون", L"أربعون", L"خمسون", L"ستون", L"سبعون", L"ثمانون", L"تسعون"
};

static const std::wstring hundreds[] = {
    L"", L"مائة", L"مائتان", L"ثلاثمائة", L"أربعمائة", L"خمسمائة", L"ستمائة", L"سبعمائة", L"ثمانمائة", L"تسعمائة"
};

static const std::wstring groups[] = {
    L"", L"ألف", L"مليون", L"مليار", L"تريليون"
};

static const std::wstring groups_plural[] = {
    L"", L"آلاف", L"ملايين", L"مليارات", L"تريليونات"
};

static const std::wstring groups_dual[] = {
    L"", L"ألفان", L"مليونان", L"ملياران", L"تريليونان"
};

std::wstring ArabicNum2Words::get_group_string(int group_val, int group_index) {
    if (group_val == 0) return L"";

    std::wstring result = L"";
    int h = group_val / 100;
    int t = (group_val % 100) / 10;
    int o = group_val % 10;

    if (h > 0) {
        result += hundreds[h];
    }

    if (t == 1) {
        if (!result.empty()) result += L" و";
        if (o == 0) result += L"عشرة";
        else if (o == 1) result += L"أحد عشر";
        else if (o == 2) result += L"اثنا عشر";
        else result += ones[o] + L" عشر";
    } else {
        if (o > 0) {
            if (!result.empty()) result += L" و";
            result += ones[o];
        }
        if (t > 1) {
            if (!result.empty()) result += L" و";
            result += tens[t];
        }
    }

    if (group_index > 0) {
        if (group_val == 1) {
            result = groups[group_index];
        } else if (group_val == 2) {
            result = groups_dual[group_index];
        } else if (group_val >= 3 && group_val <= 10) {
            result += L" " + groups_plural[group_index];
        } else {
            result += L" " + groups[group_index];
        }
    }

    return result;
}

std::wstring ArabicNum2Words::convert(long long number) {
    if (number == 0) return L"صفر";
    if (number < 0) return L"سالب " + convert(-number);

    std::wstring result = L"";
    int group_index = 0;

    while (number > 0) {
        int group_val = number % 1000;
        std::wstring group_str = get_group_string(group_val, group_index);
        
        if (!group_str.empty()) {
            if (!result.empty()) {
                result = group_str + L" و" + result;
            } else {
                result = group_str;
            }
        }
        
        number /= 1000;
        group_index++;
    }

    return result;
}

std::wstring ArabicNum2Words::replace_numbers_in_text(const std::wstring& text) {
    std::wstring result = text;
    std::wregex number_regex(L"\\b\\d+\\b");
    
    std::wsmatch match;
    std::wstring::const_iterator searchStart(text.cbegin());
    std::wstring replaced_text;

    while (std::regex_search(searchStart, text.cend(), match, number_regex)) {
        replaced_text += match.prefix().str();
        long long num = std::stoll(match.str());
        replaced_text += convert(num);
        searchStart = match.suffix().first;
    }
    replaced_text += std::wstring(searchStart, text.cend());

    return replaced_text;
}

} // namespace sapi
} // namespace RHVoice
