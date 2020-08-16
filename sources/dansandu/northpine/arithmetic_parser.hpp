#pragma once

#include "dansandu/glyph/node.hpp"
#include "dansandu/glyph/parser.hpp"
#include "dansandu/glyph/regex_tokenizer.hpp"
#include "dansandu/northpine/function.hpp"

#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace dansandu::northpine::arithmetic_parser
{

class ArithmeticParser
{
public:
    ArithmeticParser();

    double evaluate(const std::map<std::string, std::unique_ptr<dansandu::northpine::function::IFunction>>& functions,
                    const std::map<std::string, double>& variables, std::string_view formula) const;

    std::string dump() const;

private:
    dansandu::glyph::parser::Parser parser_;
    dansandu::glyph::regex_tokenizer::RegexTokenizer tokenizer_;
};

}
