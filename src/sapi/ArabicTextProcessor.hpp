#ifndef ARABIC_TEXT_PROCESSOR_HPP
#define ARABIC_TEXT_PROCESSOR_HPP

#include <string>
#include <memory>

namespace RHVoice {
namespace sapi {

class ArabicTextProcessor {
public:
    ArabicTextProcessor();
    ~ArabicTextProcessor();

    // Initializes the ONNX model (loads from a path or hardcoded if necessary)
    bool initialize(const std::string& model_path, const std::string& vocab_path);

    // Normalizes numbers, dates, times, symbols (equivalent to ar_textnorm.py)
    std::wstring normalize_text(const std::wstring& input);

    // Diacritizes the text using ONNX model (equivalent to text_processing.py)
    std::wstring process_and_diacritize(const std::wstring& input);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace sapi
} // namespace RHVoice

#endif // ARABIC_TEXT_PROCESSOR_HPP
