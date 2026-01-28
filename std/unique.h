#pragma once

#include <type_traits>    
#include <utility>      
#include <cstddef>  

namespace custom {

template <typename T>
struct default_delete {
    constexpr default_delete() noexcept = default;
    template <typename U,
              typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    constexpr default_delete(const default_delete<U>&) noexcept {}

    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0, "can not delete ncomplete type");
        static_assert(!std::is_void_v<T>, "can not delete void type");
        delete ptr;
    }
};

template <typename T>
struct default_delete<T[]> {
    constexpr default_delete() noexcept = default;

    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0, "can not delete ncomplete type");
        delete[] ptr;
    }
};

template <typename T, typename Deleter = default_delete<T>>
class unique_ptr;

template <typename T, typename Deleter>
class unique_ptr<T, Deleter> {
    static_assert(!std::is_array_v<T>, "array must use unique_ptr<T[]>");

public:
    using element_type = T;
    using deleter_type = Deleter;

    template <typename D, typename = void>
    struct _pointer_type_helper {
        using type = T*; 
    };

    template <typename D>
    struct _pointer_type_helper<D, std::void_t<typename D::pointer>> {
        using type = typename D::pointer;  
    };

    using pointer = typename _pointer_type_helper<Deleter>::type;

private:
    pointer ptr_;     
    Deleter deleter_;  

public:
    constexpr unique_ptr() noexcept
        : ptr_(nullptr), deleter_() {}

    constexpr unique_ptr(std::nullptr_t) noexcept
        : ptr_(nullptr), deleter_() {}

    explicit unique_ptr(pointer p) noexcept
        : ptr_(p), deleter_() {}

    unique_ptr(pointer p, const Deleter& d) noexcept(
        std::is_nothrow_copy_constructible_v<Deleter>)
        : ptr_(p), deleter_(d) {}

    unique_ptr(pointer p, Deleter&& d) noexcept(
        std::is_nothrow_move_constructible_v<Deleter>)
        : ptr_(p), deleter_(std::move(d)) {}

    unique_ptr(unique_ptr&& other) noexcept(
        std::is_nothrow_move_constructible_v<Deleter>)
        : ptr_(other.release()), deleter_(std::forward<Deleter>(other.get_deleter())) {}

    template <typename U, typename E,
              typename = std::enable_if_t<
                  std::is_convertible_v<U*, T*> &&
                  std::is_convertible_v<E, Deleter>>>
    unique_ptr(unique_ptr<U, E>&& other) noexcept(
        std::is_nothrow_constructible_v<Deleter, E>)
        : ptr_(other.release()), deleter_(std::forward<E>(other.get_deleter())) {}

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;


    unique_ptr& operator=(unique_ptr&& other) noexcept(
        std::is_nothrow_move_assignable_v<Deleter> &&
        std::is_nothrow_move_constructible_v<Deleter>) {
        if (this != &other) {
            reset(other.release());
            deleter_ = std::forward<Deleter>(other.get_deleter());
        }
        return *this;
    }

    template <typename U, typename E>
    unique_ptr& operator=(unique_ptr<U, E>&& other) noexcept(
        std::is_nothrow_constructible_v<Deleter, E>) {
        static_assert(std::is_convertible_v<U*, T*>, "pointer type not compatible");
        reset(other.release());
        deleter_ = std::forward<E>(other.get_deleter());
        return *this;
    }


    unique_ptr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    ~unique_ptr() {
        if (ptr_ != nullptr) {
            deleter_(ptr_);  
        }
    }

    typename std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_;
    }

    pointer operator->() const noexcept {
        return ptr_;
    }

    pointer get() const noexcept {
        return ptr_;
    }

    deleter_type& get_deleter() noexcept {
        return deleter_;
    }

    const deleter_type& get_deleter() const noexcept {
        return deleter_;
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    pointer release() noexcept {
        pointer p = ptr_;
        ptr_ = nullptr;
        return p;
    }

    void reset(pointer p = pointer{}) noexcept {
        pointer old = ptr_;
        ptr_ = p;
        if (old != nullptr) {
            deleter_(old);
        }
    }

    void swap(unique_ptr& other) noexcept(
        std::is_nothrow_swappable_v<Deleter>) {
        using std::swap;
        swap(ptr_, other.ptr_);
        swap(deleter_, other.deleter_);
    }
};

template <typename T, typename Deleter>
class unique_ptr<T[], Deleter> {
public:
    using element_type = T;
    using deleter_type = Deleter;

    template <typename D, typename = void>
    struct _pointer_type_helper {
        using type = T*;
    };

    template <typename D>
    struct _pointer_type_helper<D, std::void_t<typename D::pointer>> {
        using type = typename D::pointer;
    };

    using pointer = typename _pointer_type_helper<Deleter>::type;

private:
    pointer ptr_;
    Deleter deleter_;

public:
    constexpr unique_ptr() noexcept
        : ptr_(nullptr), deleter_() {}

    constexpr unique_ptr(std::nullptr_t) noexcept
        : ptr_(nullptr), deleter_() {}

    explicit unique_ptr(pointer p) noexcept
        : ptr_(p), deleter_() {}

    unique_ptr(pointer p, const Deleter& d) noexcept(
        std::is_nothrow_copy_constructible_v<Deleter>)
        : ptr_(p), deleter_(d) {}

    unique_ptr(pointer p, Deleter&& d) noexcept(
        std::is_nothrow_move_constructible_v<Deleter>)
        : ptr_(p), deleter_(std::move(d)) {}

    unique_ptr(unique_ptr&& other) noexcept(
        std::is_nothrow_move_constructible_v<Deleter>)
        : ptr_(other.release()), deleter_(std::forward<Deleter>(other.get_deleter())) {}

    template <typename U, typename E,
              typename = std::enable_if_t<std::is_convertible_v<U(*)[], T(*)[]>>>
    unique_ptr(unique_ptr<U[], E>&& other) noexcept(
        std::is_nothrow_constructible_v<Deleter, E>)
        : ptr_(other.release()), deleter_(std::forward<E>(other.get_deleter())) {}

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    unique_ptr& operator=(unique_ptr&& other) noexcept(
        std::is_nothrow_move_assignable_v<Deleter>) {
        if (this != &other) {
            reset(other.release());
            deleter_ = std::forward<Deleter>(other.get_deleter());
        }
        return *this;
    }

    template <typename U, typename E>
    unique_ptr& operator=(unique_ptr<U[], E>&& other) noexcept(
        std::is_nothrow_constructible_v<Deleter, E>) {
        reset(other.release());
        deleter_ = std::forward<E>(other.get_deleter());
        return *this;
    }

    unique_ptr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    ~unique_ptr() {
        if (ptr_ != nullptr) {
            deleter_(ptr_);
        }
    }

    T& operator[](std::size_t i) const {
        return ptr_[i];
    }

    pointer get() const noexcept {
        return ptr_;
    }

    deleter_type& get_deleter() noexcept {
        return deleter_;
    }

    const deleter_type& get_deleter() const noexcept {
        return deleter_;
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    pointer release() noexcept {
        pointer p = ptr_;
        ptr_ = nullptr;
        return p;
    }

    void reset(pointer p = pointer{}) noexcept {
        pointer old = ptr_;
        ptr_ = p;
        if (old != nullptr) {
            deleter_(old);
        }
    }

    void swap(unique_ptr& other) noexcept(
        std::is_nothrow_swappable_v<Deleter>) {
        using std::swap;
        swap(ptr_, other.ptr_);
        swap(deleter_, other.deleter_);
    }
};

template <typename T, typename Deleter>
void swap(unique_ptr<T, Deleter>& a, unique_ptr<T, Deleter>& b) noexcept(noexcept(a.swap(b))) {
    a.swap(b);
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator==(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) noexcept {
    return x.get() == y.get();
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator!=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) noexcept {
    return !(x == y);
}

template <typename T, typename D>
bool operator==(const unique_ptr<T, D>& x, std::nullptr_t) noexcept {
    return !x;
}

template <typename T, typename D>
bool operator==(std::nullptr_t, const unique_ptr<T, D>& x) noexcept {
    return !x;
}

template <typename T, typename D>
bool operator!=(const unique_ptr<T, D>& x, std::nullptr_t) noexcept {
    return static_cast<bool>(x);
}

template <typename T, typename D>
bool operator!=(std::nullptr_t, const unique_ptr<T, D>& x) noexcept {
    return static_cast<bool>(x);
}

} // namespace custom
