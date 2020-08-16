#pragma once

#include "dansandu/ballotin/exception.hpp"

#include <utility>
#include <vector>

namespace dansandu::northpine::function
{

class IFunction
{
public:
    virtual ~IFunction()
    {
    }

    virtual double invoke(const std::vector<double>& arguments) = 0;

    virtual int getParametersCount() const = 0;
};

template<typename T>
class SimpleFunction;

template<typename... Parameters>
class SimpleFunction<double(Parameters...)> : public IFunction
{
public:
    explicit SimpleFunction(double (*function)(Parameters...)) : function_{function}
    {
    }

    double invoke(const std::vector<double>& arguments) override
    {
        if (static_cast<int>(arguments.size()) == getParametersCount())
        {
            return invokeImpl(arguments, std::make_index_sequence<sizeof...(Parameters)>{});
        }
        THROW(std::runtime_error, "function takes ", getParametersCount(), " arguments but ", arguments.size(),
              " were supplied");
    }

    int getParametersCount() const override
    {
        return static_cast<int>(sizeof...(Parameters));
    }

private:
    template<std::size_t... I>
    double invokeImpl(const std::vector<double>& arguments, std::index_sequence<I...>)
    {
        return function_(arguments[I]...);
    }

    double (*function_)(Parameters...);
};

template<typename... Parameters>
auto makeSimpleFunction(double (*function)(Parameters...))
{
    return std::make_unique<SimpleFunction<double(Parameters...)>>(function);
}

}
