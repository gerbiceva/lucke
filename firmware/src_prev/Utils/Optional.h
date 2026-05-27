#pragma once
#include <utility> // for std::move

namespace Utils 
{
template <typename T>
class Optional {
    bool m_hasValue;
    alignas(T) unsigned char m_storage[sizeof(T)];

    T* ptr() { return reinterpret_cast<T*>(m_storage); }
    const T* ptr() const { return reinterpret_cast<const T*>(m_storage); }

public:
    Optional() noexcept : m_hasValue(false) {}

    Optional(const T& value) 
        : m_hasValue(true) 
    {
        new (m_storage) T(value);
    }

    Optional(T&& value) 
        : m_hasValue(true) 
    {
        new (m_storage) T(std::move(value));
    }

    Optional(const Optional& other) : m_hasValue(other.m_hasValue) 
    {
        if (other.m_hasValue)
        {
            new (m_storage) T(*other.ptr());
        }
    }

    Optional(Optional&& other) noexcept : m_hasValue(other.m_hasValue) 
    {
        if (other.m_hasValue)
        {
            new (m_storage) T(std::move(*other.ptr()));
        }
    }

    ~Optional() 
    { 
        reset(); 
    }

    Optional& operator=(const Optional& other) 
    {
        if (this != &other) 
        {
            reset();
            if (other.m_hasValue) 
            {
                new (m_storage) T(*other.ptr());
                m_hasValue = true;
            }
        }

        return *this;
    }

    Optional& operator=(Optional&& other) noexcept 
    {
        if (this != &other) 
        {
            reset();
            if (other.m_hasValue) 
            {
                new (m_storage) T(std::move(*other.ptr()));
                m_hasValue = true;
            }
        }

        return *this;
    }

    bool has_value() const noexcept { return m_hasValue; }
    explicit operator bool() const noexcept { return m_hasValue; }

    T& value() 
    {
        if (!m_hasValue)
        {
            throw "Bad Optional access";
        }

        return *ptr();
    }

    const T& value() const 
    {
        if (!m_hasValue)
        {
            throw "Bad Optional access";
        }

        return *ptr();
    }

    T value_or(const T& defaultValue) const 
    {
        return m_hasValue ? *ptr() : defaultValue;
    }

    void reset() noexcept 
    {
        if (m_hasValue) 
        {
            ptr()->~T();
            m_hasValue = false;
        }
    }

    template <typename... Args>
    void emplace(Args&&... args) 
    {
        reset();
        new (m_storage) T(std::forward<Args>(args)...);
        m_hasValue = true;
    }
};
}
