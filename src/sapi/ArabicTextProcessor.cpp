#include "ArabicTextProcessor.hpp"
#include "ArabicNum2Words.hpp"
#include <regex>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <cwctype>
#include <onnxruntime_cxx_api.h>
#include "vocab.hpp"

namespace RHVoice {
namespace sapi {

struct ArabicTextProcessor::Impl {
    bool initialized = false;
    
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::Session> session;
    Ort::SessionOptions session_options;
    
    std::unordered_map<std::wstring, std::wstring> char_names;
    std::unordered_map<std::wstring, std::wstring> diacritic_names;
    std::unordered_map<std::wstring, std::wstring> symbols;
    std::unordered_map<std::wstring, std::wstring> abbrev;
    std::unordered_map<std::wstring, std::wstring> currency;
    
    std::vector<std::wstring> months;
    std::vector<std::wstring> hijri_months;
    std::vector<std::pair<std::wstring, int>> roman_numerals;

    void init_dictionaries() {
        if (initialized) return;

        char_names = {
            {L"أ", L"أَلِف هَمْزَة"}, {L"إ", L"أَلِف هَمْزَة تَحْت"}, {L"آ", L"أَلِف مَدَّة"}, {L"ا", L"أَلِف"},
            {L"ب", L"بَاء"}, {L"ت", L"تَاء"}, {L"ث", L"ثَاء"}, {L"ج", L"جِيم"}, {L"ح", L"حَاء"}, {L"خ", L"خَاء"},
            {L"د", L"دَال"}, {L"ذ", L"ذَال"}, {L"ر", L"رَاء"}, {L"ز", L"زَاي"}, {L"س", L"سِين"}, {L"ش", L"شِين"},
            {L"ص", L"صَاد"}, {L"ض", L"ضَاد"}, {L"ط", L"طَاء"}, {L"ظ", L"ظَاء"}, {L"ع", L"عَيْن"}, {L"غ", L"غَيْن"},
            {L"ف", L"فَاء"}, {L"ق", L"قَاف"}, {L"ك", L"كَاف"}, {L"ل", L"لاَم"}, {L"م", L"مِيم"}, {L"ن", L"نُون"},
            {L"ه", L"هَاء"}, {L"هـ", L"هَاء"}, {L"و", L"وَاو"}, {L"ي", L"يَاء"}, {L"ء", L"هَمْزَة"},
            {L"ؤ", L"وَاو هَمْزَة"}, {L"ئ", L"يَاء هَمْزَة"}, {L"ة", L"تَاء مَرْبُوطَة"}, {L"ى", L"أَلِف مَقْصُورَة"},
            {L"لا", L"لاَم أَلِف"}, {L"ٱ", L"أَلِف وَصْل"},
            // English Letters
            {L"a", L"إِيه"}, {L"A", L"إِيه"}, {L"b", L"بِي"}, {L"B", L"بِي"}, {L"c", L"سِي"}, {L"C", L"سِي"},
            {L"d", L"دِي"}, {L"D", L"دِي"}, {L"e", L"إِي"}, {L"E", L"إِي"}, {L"f", L"إِف"}, {L"F", L"إِف"},
            {L"g", L"جِي"}, {L"G", L"جِي"}, {L"h", L"إِيتْش"}, {L"H", L"إِيتْش"}, {L"i", L"آي"}, {L"I", L"آي"},
            {L"j", L"جَي"}, {L"J", L"جَي"}, {L"k", L"كَي"}, {L"K", L"كَي"}, {L"l", L"إِل"}, {L"L", L"إِل"},
            {L"m", L"إِم"}, {L"M", L"إِم"}, {L"n", L"إِن"}, {L"N", L"إِن"}, {L"o", L"أُو"}, {L"O", L"أُو"},
            {L"p", L"بِي"}, {L"P", L"بِي"}, {L"q", L"كِيُو"}, {L"Q", L"كِيُو"}, {L"r", L"آر"}, {L"R", L"آر"},
            {L"s", L"إِس"}, {L"S", L"إِس"}, {L"t", L"تِي"}, {L"T", L"تِي"}, {L"u", L"يُو"}, {L"U", L"يُو"},
            {L"v", L"فِي"}, {L"V", L"فِي"}, {L"w", L"دَبْلِيُو"}, {L"W", L"دَبْلِيُو"}, {L"x", L"إِكْس"}, {L"X", L"إِكْس"},
            {L"y", L"وَاي"}, {L"Y", L"وَاي"}, {L"z", L"زِد"}, {L"Z", L"زِد"}
        };

        diacritic_names = {
            {L"\u064B", L"تَنْوِين فَتْح"}, {L"\u064C", L"تَنْوِين ضَمّ"}, {L"\u064D", L"تَنْوِين كَسْر"},
            {L"\u064E", L"فَتْحَة"}, {L"\u064F", L"ضَمَّة"}, {L"\u0650", L"كَسْرَة"},
            {L"\u0651", L"شَدَّة"}, {L"\u0652", L"سُكُون"}, {L"\u0653", L"مَدَّة"},
            {L"\u0654", L"هَمْزَة"}, {L"\u0655", L"هَمْزَة تَحْت"}, {L"\u0670", L"أَلِف خَنْجَرِيَّة"},
            {L"\u0640", L"تَطْوِيل"}
        };

        symbols = {
            {L"&", L"وَ"}, {L"@", L"آت"}, {L"#", L"رَقْم"}, {L"%", L"بِالْمِئَة"}, {L"‰", L"بِالْأَلْف"},
            {L"+", L"زَائِد"}, {L"=", L"يُسَاوِي"}, {L"≠", L"لَا يُسَاوِي"}, {L"<", L"أَصْغَرُ مِنْ"},
            {L">", L"أَكْبَرُ مِنْ"}, {L"≤", L"أَصْغَرُ أَوْ يُسَاوِي"}, {L"≥", L"أَكْبَرُ أَوْ يُسَاوِي"},
            {L"±", L"زَائِدٌ أَوْ نَاقِص"}, {L"×", L"ضَرْب"}, {L"÷", L"قِسْمَة"}, {L"√", L"جَذْر"},
            {L"∞", L"مَا لَا نِهَايَة"}, {L"π", L"بَاي"}, {L"°", L"دَرَجَة"}, {L"µ", L"مِيكْرُو"},
            {L"©", L"حُقُوقُ النَّشْر"}, {L"®", L"عَلَامَةٌ مُسَجَّلَة"}, {L"™", L"عَلَامَةٌ تِجَارِيَّة"},
            {L"§", L"فِقْرَة"}, {L"¶", L"عَلَامَةُ فِقْرَة"}, {L"†", L"عَلَامَةُ إِحَالَة"}, {L"•", L"نُقْطَةُ تَعْدَاد"},
            {L"★", L"نَجْمَة"}, {L"☆", L"نَجْمَة"}, {L"→", L"يُؤَدِّي إِلَى"}, {L"←", L"مِنْ"},
            {L"↔", L"ذَهَابًا وَإِيَابًا"}, {L"⇒", L"إِذَنْ"}, {L"~", L"تَقْرِيبًا"}, {L"|", L"خَطٌّ عَمُودِيّ"}
        };

        abbrev = {
            {L"أ.د", L"أستاذ دكتور"}, {L"ص.ب", L"صندوق بريد"}, {L"ق.م", L"قبل الميلاد"}, {L"ب.م", L"بعد الميلاد"},
            {L"إلخ", L"إلى آخره"}, {L"ا.هـ", L"انتهى"}, {L"د.", L"دكتور"}, {L"م.", L"مهندس"}, {L"أ.", L"أستاذ"},
            {L"ت.", L"توفي"}, {L"ولد.", L"ولد"}
        };

        currency = {
            {L"$", L"دولار"}, {L"USD", L"دولار"}, {L"£", L"جنيه"}, {L"GBP", L"جنيه"},
            {L"€", L"يورو"}, {L"EUR", L"يورو"}, {L"¥", L"ين"}, {L"ر.س", L"ريال"}, {L"ريال", L"ريال"}, {L"SAR", L"ريال"},
            {L"د.إ", L"درهم"}, {L"درهم", L"درهم"}, {L"AED", L"درهم"}, {L"ج.م", L"جنيه مصري"}, {L"د.ك", L"دينار كويتي"},
            {L"د.ع", L"دينار عراقي"}, {L"ل.ل", L"ليرة لبنانية"}, {L"ل.س", L"ليرة سورية"}, {L"د.أ", L"دينار أردني"},
            {L"ر.ق", L"ريال قطري"}, {L"د.ب", L"دينار بحريني"}, {L"ر.ع", L"ريال عماني"}
        };

        months = {L"", L"يناير", L"فبراير", L"مارس", L"أبريل", L"مايو", L"يونيو",
                  L"يوليو", L"أغسطس", L"سبتمبر", L"أكتوبر", L"نوفمبر", L"ديسمبر"};

        hijri_months = {L"", L"محرم", L"صفر", L"ربيع الأول", L"ربيع الآخر", L"جمادى الأولى",
                        L"جمادى الآخرة", L"رجب", L"شعبان", L"رمضان", L"شوال", L"ذو القعدة", L"ذو الحجة"};

        roman_numerals = {
            {L"XXI", 21}, {L"XIX", 19}, {L"XVIII", 18}, {L"XVII", 17}, {L"XVI", 16}, {L"XIV", 14},
            {L"XIII", 13}, {L"XII", 12}, {L"XI", 11}, {L"VIII", 8}, {L"VII", 7}, {L"VI", 6},
            {L"XX", 20}, {L"XV", 15}, {L"IX", 9}, {L"IV", 4}, {L"X", 10}, {L"V", 5}, {L"III", 3},
            {L"II", 2}, {L"I", 1}
        };

        initialized = true;
    }

    std::wstring normalize_abbrev(std::wstring t) {
        for (const auto& pair : abbrev) {
            std::wregex e(L"(?<![\\x{0621}-\\x{064A}\\x{0670}\\x{0671}])" + pair.first + L"(?![\\x{0621}-\\x{064A}\\x{0670}\\x{0671}])");
            t = std::regex_replace(t, e, pair.second);
        }
        return t;
    }

    std::wstring normalize_symbols(std::wstring t) {
        for (const auto& pair : symbols) {
            size_t pos = 0;
            while ((pos = t.find(pair.first, pos)) != std::wstring::npos) {
                t.replace(pos, pair.first.length(), L" " + pair.second + L" ");
                pos += pair.second.length() + 2;
            }
        }
        return t;
    }

    std::wstring normalize_math(std::wstring t) {
        t = std::regex_replace(t, std::wregex(L"(?<=\\d)\\s*\\+\\s*(?=\\d)"), L" زائد ");
        t = std::regex_replace(t, std::wregex(L"(?<=\\d)\\s*[-−]\\s*(?=\\d)"), L" ناقص ");
        t = std::regex_replace(t, std::wregex(L"(?<=\\d)\\s*[×x]\\s*(?=\\d)"), L" ضرب ");
        t = std::regex_replace(t, std::wregex(L"(?<=\\d)\\s*[÷/]\\s*(?=\\d)"), L" قسمة ");
        t = std::regex_replace(t, std::wregex(L"(?<=\\d)\\s*=\\s*"), L" يساوي ");
        return t;
    }
};

ArabicTextProcessor::ArabicTextProcessor() : pImpl(std::make_unique<Impl>()) {
    pImpl->init_dictionaries();
}
ArabicTextProcessor::~ArabicTextProcessor() = default;

std::wstring ArabicTextProcessor::normalize(const std::wstring& text) {
    if (text.empty()) return text;
    
    std::wstring t = text;

    try {
        // 1. Fold digits to ASCII
        for (auto& c : t) {
            if (c >= L'٠' && c <= L'٩') c = c - L'٠' + L'0';
            else if (c >= L'۰' && c <= L'۹') c = c - L'۰' + L'0';
        }

        // 2. Remove double "الساعة" before time
        std::wregex time_regex(L"الساع[ةه][\\s\\x{200E}\\x{200F}\\x{202A}-\\x{202E}\\x{2066}-\\x{2069}]*(?=\\d{1,2}:\\d{2})");
        t = std::regex_replace(t, time_regex, L"");

        // 3. Abbreviations
        t = pImpl->normalize_abbrev(t);
        
        // 4. Math symbols
        t = pImpl->normalize_math(t);
        
        // 5. Symbols
        t = pImpl->normalize_symbols(t);

        // 6. Convert numbers
        t = ArabicNum2Words::replace_numbers_in_text(t);
    } catch (const std::regex_error&) {}
    
    return t;
}

void ArabicTextProcessor::initialize(const std::string& model_dir) {
    if (pImpl->env) return;
    try {
        pImpl->env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "ArabicDiacritizer");
        pImpl->session_options.SetIntraOpNumThreads(1);
        pImpl->session_options.SetInterOpNumThreads(1);
        
        std::string model_path = model_dir + "\\model.onnx";
        std::wstring w_model_path(model_path.begin(), model_path.end());
        pImpl->session = std::make_unique<Ort::Session>(*(pImpl->env), w_model_path.c_str(), pImpl->session_options);
    } catch (...) {
        pImpl->session.reset();
    }
}

std::wstring ArabicTextProcessor::process(const std::wstring& text) {
    std::wstring normalized = normalize(text);
    if (normalized.empty() || !pImpl->session) return normalized;

    std::vector<int64_t> input_ids;
    input_ids.reserve(normalized.length());
    for (wchar_t c : normalized) {
        std::wstring s(1, c);
        auto it = char_to_idx.find(s);
        if (it != char_to_idx.end()) {
            input_ids.push_back(it->second);
        } else {
            input_ids.push_back(1); // <UNK>
        }
    }

    if (input_ids.empty()) return normalized;

    std::vector<int64_t> input_shape = {1, static_cast<int64_t>(input_ids.size())};
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<int64_t>(
        memory_info, input_ids.data(), input_ids.size(), input_shape.data(), input_shape.size()
    );

    const char* input_names[] = {"input"};
    const char* output_names[] = {"gated_cls"};

    try {
        auto output_tensors = pImpl->session->Run(
            Ort::RunOptions{nullptr}, input_names, &input_tensor, 1, output_names, 1
        );

        int64_t* output_data = output_tensors.front().GetTensorMutableData<int64_t>();
        size_t count = output_tensors.front().GetTensorTypeAndShapeInfo().GetElementCount();

        std::wstring diacritized;
        diacritized.reserve(normalized.length() * 2);
        
        for (size_t i = 0; i < count && i < normalized.length(); ++i) {
            wchar_t base_char = normalized[i];
            diacritized += base_char;
            
            bool is_letter = (base_char >= L'ء' && base_char <= L'ي') || base_char == L'ٱ' || base_char == L'ـ';
            if (is_letter) {
                int64_t diac_id = output_data[i];
                auto it = idx_to_diac.find(diac_id);
                if (it != idx_to_diac.end() && !it->second.empty()) {
                    diacritized += it->second;
                }
            }
        }
        return diacritized;
    } catch (...) {
        return normalized;
    }
}

} // namespace sapi
} // namespace RHVoice
