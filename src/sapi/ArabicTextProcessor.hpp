#ifndef ARABIC_TEXT_PROCESSOR_HPP
#define ARABIC_TEXT_PROCESSOR_HPP

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <memory>

namespace RHVoice {
namespace sapi {

class ArabicTextProcessor {
public:
    ArabicTextProcessor();
    ~ArabicTextProcessor();

    // Initialize ONNX runtime with the model and vocab directory
    void initialize(const std::string& model_dir);

    // The main entry point that applies pre-processing, ONNX diacritization, and post-processing
    std::wstring process(const std::wstring& text);

    // Expose normalization step (useful for testing or if ONNX is not initialized)
    std::wstring normalize(const std::wstring& text);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace sapi
} // namespace RHVoice

#endif // ARABIC_TEXT_PROCESSOR_HPP
