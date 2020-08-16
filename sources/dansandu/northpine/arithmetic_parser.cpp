#include "dansandu/northpine/arithmetic_parser.hpp"
#include "dansandu/ballotin/string.hpp"
#include "dansandu/glyph/node.hpp"
#include "dansandu/glyph/token.hpp"

#include <cmath>
#include <iostream>

using dansandu::ballotin::string::join;
using dansandu::glyph::node::Node;
using dansandu::glyph::token::Token;
using dansandu::northpine::function::IFunction;

namespace dansandu::northpine::arithmetic_parser
{

ArithmeticParser::ArithmeticParser()
    // clang-format off
    : parser_{/* 0*/ "Start -> Sums                                                                  \n"
              /* 1*/ "Sums  -> Sums plus Products                                                    \n"
              /* 2*/ "Sums  -> Sums minus Products                                                   \n"
              /* 3*/ "Sums  -> Products                                                              \n"
              /* 4*/ "Products -> Products multiply Exponentials                                     \n"
              /* 5*/ "Products -> Products divide Exponentials                                       \n"
              /* 6*/ "Products -> Exponentials                                                       \n"
              /* 7*/ "Exponentials -> Exponentials power SignedValue                                 \n"
              /* 8*/ "Exponentials -> SignedValue                                                    \n"
              /* 9*/ "SignedValue  -> Value                                                          \n"
              /*10*/ "SignedValue  -> plus Value                                                     \n"
              /*11*/ "SignedValue  -> minus Value                                                    \n"
              /*12*/ "Value -> identifier                                                            \n"
              /*13*/ "Value -> number                                                                \n"
              /*14*/ "Value -> parenthesesStart Sums parenthesesEnd                                  \n"
              /*15*/ "Value -> identifier parenthesesStart parenthesesEnd                            \n"
              /*16*/ "Value -> identifier parenthesesStart ParametersBegin Parameters parenthesesEnd \n"
              /*17*/ "ParametersBegin ->                                                             \n"
              /*18*/ "Parameters -> Parameters comma Sums                                            \n"
              /*19*/ "Parameters -> Sums                                                             \n"},
      tokenizer_{{{"plus", "\\+"},
                  {"minus", "\\-"},
                  {"multiply", "\\*"},
                  {"divide", "\\/"},
                  {"power", "\\^"},
                  {"identifier", "[a-zA-Z]\\w*"},
                  {"number", "([1-9]\\d*|0)(\\.\\d+)?"},
                  {"parenthesesStart", "\\("},
                  {"parenthesesEnd", "\\)"},
                  {"comma", ","},
                  {"whitespace", "\\s+"}},
                 {"whitespace"}}
// clang-format on
{
}

template<typename T>
auto pop(std::vector<T>& stack)
{
    auto value = std::move(stack.at(stack.size() - 1));
    stack.pop_back();
    return value;
}

double ArithmeticParser::evaluate(const std::map<std::string, std::unique_ptr<IFunction>>& functions,
                                  const std::map<std::string, double>& variables, std::string_view formula) const
{
    const auto identifier = parser_.getTerminalSymbol("identifier");
    const auto number = parser_.getTerminalSymbol("number");

    auto tokensStack = std::vector<Token>{};
    auto valuesStack = std::vector<double>{};
    auto argumentsBeginStack = std::vector<int>{};

    const auto visitor = [&](const Node& node) {
        if (node.isToken())
        {
            auto token = node.getToken();
            if (token.getSymbol() == identifier || token.getSymbol() == number)
            {
                tokensStack.push_back(token);
            }
        }
        else
        {
            switch (node.getRuleIndex())
            {
            case 0:
            case 3:
            case 6:
            case 8:
            case 9:
            case 10:
            case 14:
            case 18:
            case 19:
                break;
            case 1:
            {
                auto rhs = pop(valuesStack);
                auto lhs = pop(valuesStack);
                valuesStack.push_back(lhs + rhs);
                break;
            }
            case 2:
            {
                auto rhs = pop(valuesStack);
                auto lhs = pop(valuesStack);
                valuesStack.push_back(lhs - rhs);
                break;
            }
            case 4:
            {
                auto rhs = pop(valuesStack);
                auto lhs = pop(valuesStack);
                valuesStack.push_back(lhs * rhs);
                break;
            }
            case 5:
            {
                auto rhs = pop(valuesStack);
                auto lhs = pop(valuesStack);
                valuesStack.push_back(lhs / rhs);
                break;
            }
            case 7:
            {
                auto rhs = pop(valuesStack);
                auto lhs = pop(valuesStack);
                valuesStack.push_back(std::pow(lhs, rhs));
                break;
            }
            case 11:
            {
                auto value = pop(valuesStack);
                valuesStack.push_back(-value);
                break;
            }
            case 12:
            {
                auto token = pop(tokensStack);
                auto id = std::string{formula.begin() + token.begin(), formula.begin() + token.end()};
                valuesStack.push_back(variables.at(id));
                break;
            }
            case 13:
            {
                auto token = pop(tokensStack);
                auto nr = std::string{formula.begin() + token.begin(), formula.begin() + token.end()};
                valuesStack.push_back(std::stod(nr));
                break;
            }
            case 15:
            {
                auto token = pop(tokensStack);
                auto function = std::string{formula.begin() + token.begin(), formula.begin() + token.end()};
                auto result = functions.at(function)->invoke({});
                valuesStack.push_back(result);
                break;
            }
            case 16:
            {
                auto token = pop(tokensStack);
                auto function = std::string{formula.cbegin() + token.begin(), formula.cbegin() + token.end()};
                auto arguments =
                    std::vector<double>{valuesStack.cbegin() + argumentsBeginStack.back(), valuesStack.cend()};
                auto result = functions.at(function)->invoke(arguments);
                std::cout << std::endl << function << "(" << join(arguments, ", ") << ") -> " << result << std::endl;
                valuesStack.erase(valuesStack.cbegin() + argumentsBeginStack.back(), valuesStack.cend());
                argumentsBeginStack.pop_back();
                valuesStack.push_back(result);
                break;
            }
            case 17:
            {
                argumentsBeginStack.push_back(static_cast<int>(valuesStack.size()));
                break;
            }
            default:
                THROW(std::runtime_error, "unrecognized rule index: ", node.getRuleIndex());
            }
        }
    };

    const auto mapper = [this](auto id) { return parser_.getTerminalSymbol(id); };

    parser_.parse(tokenizer_(formula, mapper), visitor);

    if (!tokensStack.empty())
    {
        THROW(std::runtime_error, "tokens stack was not exhausted");
    }

    if (valuesStack.size() != 1)
    {
        THROW(std::runtime_error, "values stack must only contain the result");
    }

    return valuesStack.front();
}

std::string ArithmeticParser::dump() const
{
    return parser_.dump();
}

}
