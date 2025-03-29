#pragma once
#include <cstddef>
#include <functional>

namespace Helios {
template <typename T> class SharedPtr {
  public:
    template <typename... Args> static SharedPtr<T> create(Args... args) {
        SharedPtr<T> ptr;
        ptr.m_data = new SharedPtrData;
        ptr.m_data->ref_counter = 1;
        ptr.m_data->data = new T(args...);
        return ptr;
    };

    SharedPtr() : m_data(nullptr) {}
    SharedPtr(std::nullptr_t) : m_data(nullptr) {};

    ~SharedPtr() { destroy(); }

    SharedPtr(const SharedPtr& other) {
        m_data = other.m_data;
        if (m_data) {
            ++m_data->ref_counter;
        }
    }

    SharedPtr(SharedPtr&& other)  noexcept {
        m_data = other.m_data;
        other.m_data = nullptr;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            destroy();
            m_data = other.m_data;
            if (m_data) {
                ++m_data->ref_counter;
            }
        }
        return *this;
    }

    SharedPtr& operator=(std::nullptr_t) {
        destroy();
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other)  noexcept {
        if (this != &other) {
            destroy();
            m_data = other.m_data;
            other.m_data = nullptr;
        }
        return *this;
    }

    operator bool() { return m_data != nullptr; }
    operator bool() const { return m_data != nullptr; }

    bool operator==(const SharedPtr& other) const {
        return m_data == other.m_data;
    }
    bool operator==(std::nullptr_t) const { return m_data == nullptr; }
    T* operator->() { return m_data ? m_data->data : nullptr; }
    T* operator->() const { return m_data ? m_data->data : nullptr; }
    T& operator*() { return *m_data->data; }

    T* get() { return m_data ? m_data->data : nullptr; }
    const T* get() const { return m_data ? m_data->data : nullptr; }

    std::size_t hash() const { return std::hash<void*>{}(m_data); }
    std::size_t hash() { return std::hash<void*>{}(m_data); }

    void reset() {
        if (m_data) {
            m_data->ref_counter = 1;
            destroy();
        }
    }

  private:
    void destroy() {
        if (m_data) {
            --m_data->ref_counter;
            if (m_data->ref_counter == 0) {
                delete m_data->data;
                delete m_data;
            }
            m_data = nullptr;
        }
    }

  private:
    struct SharedPtrData {
        uint32_t ref_counter;
        T* data;
    };

    SharedPtrData* m_data = nullptr;
};
} // namespace Helios

template <typename T> struct std::hash<Helios::SharedPtr<T>> {
    std::size_t operator()(const Helios::SharedPtr<T>& k) const {
        return k.hash();
    };
    std::size_t operator()(const Helios::SharedPtr<T>& k) { return k.hash(); };
};
