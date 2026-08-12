import json

with open(r"C:\arabic-src\RHVOICE ARABIC\nvda-addon\synthDrivers\rhvoice-arabic\model\vocab.json", "r", encoding="utf-8") as f:
    v = json.load(f)

with open(r"C:\arabic-src\RHVoice\src\sapi\vocab.hpp", "w", encoding="utf-8") as f:
    f.write("#pragma once\n")
    f.write("#include <unordered_map>\n")
    f.write("#include <string>\n\n")
    f.write("namespace RHVoice {\n")
    f.write("namespace sapi {\n\n")
    
    # char_to_idx
    f.write("static const std::unordered_map<std::wstring, int64_t> char_to_idx = {\n")
    for key, val in v["char_to_idx"].items():
        # Escape characters
        escaped_key = key.replace("\\", "\\\\").replace('"', '\\"')
        f.write(f'    {{L"{escaped_key}", {val}}},\n')
    f.write("};\n\n")
    
    # diac_to_idx
    f.write("static const std::unordered_map<int64_t, std::wstring> idx_to_diac = {\n")
    for key, val in v["diac_to_idx"].items():
        escaped_key = key.replace("\\", "\\\\").replace('"', '\\"')
        f.write(f'    {{{val}, L"{escaped_key}"}},\n')
    f.write("};\n\n")
    
    f.write("} // namespace sapi\n")
    f.write("} // namespace RHVoice\n")

print("Generated vocab.hpp")
