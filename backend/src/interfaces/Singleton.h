//
// Created by kneeclass on 11.12.23.
//
// Source: https://stackoverflow.com/questions/34519073/inherit-singleton

#ifndef ROYAL_TRACER_SINGLETON_H
#define ROYAL_TRACER_SINGLETON_H

#include <type_traits>

template<typename T>
class Singleton
{
protected:
    Singleton() noexcept = default;

    Singleton(const Singleton&) = delete;

    Singleton& operator=(const Singleton&) = delete;

    virtual ~Singleton() = default; // to silence base class Singleton<T> has a
    // non-virtual destructor [-Weffc++]

public:
    static T& get_instance() noexcept(std::is_nothrow_constructible<T>::value)
    {
        // Guaranteed to be destroyed.
        // Instantiated on first use.
        // Thread safe in C++11
        static T instance{};

        return instance;
    }
};

#endif //ROYAL_TRACER_SINGLETON_H
