#include "dansandu/northpine/arithmetic_parser.hpp"
#include "dansandu/northpine/function.hpp"

#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <string>

using dansandu::northpine::arithmetic_parser::ArithmeticParser;
using dansandu::northpine::function::IFunction;
using dansandu::northpine::function::makeSimpleFunction;
using dansandu::northpine::function::SimpleFunction;

static double quadratic(double x, double a, double b, double c)
{
    return a * x * x + b * x + c;
}

static double sigmoid(double x)
{
    return 1.0 / (std::exp(-x) + 1.0);
}

static auto variables = std::map<std::string, double>{};

static auto functions = std::map<std::string, std::unique_ptr<IFunction>>{};

int main(int argc, char** argv)
{
    auto parser = ArithmeticParser{};
    if (argc > 1)
    {
        functions.insert({"quadratic", makeSimpleFunction(&quadratic)});
        functions.insert({"sigmoid", makeSimpleFunction(&sigmoid)});
        auto result = parser.evaluate(functions, variables, argv[1]);
        std::cout << argv[1] << " = " << result << std::endl;
    }
    else
    {
        std::cout << parser.dump();
    }
    return 0;
}
