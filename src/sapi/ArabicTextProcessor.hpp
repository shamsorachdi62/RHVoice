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

    // The main entry point
    std::wstring normalize(const std::wstring& text);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace sapi
} // namespace RHVoice

#endif // ARABIC_TEXT_PROCESSOR_HPP
