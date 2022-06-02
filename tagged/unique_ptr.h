#ifndef CORE_TAGGED_PTR_H
#define CORE_TAGGED_PTR_H

#include <cstdint>
#include <type_traits>
#include <utility>

#include "tagged/tagging.h"

namespace tagged {

// A tagged unique_ptr style pointer to one of a collection of types.
template <typename ChildPtr, typename... Types> class UniquePtr {
    TaggedPtr ptr;

    static constexpr detail::TagHelpers<Types...> helpers;

    void cleanup() {
        if (this->ptr == nullptr) {
            return;
        }

        auto tag = this->tag();
        auto ptr = this->get();
        this->ptr = nullptr;

        helpers.destroy(tag, ptr);
    }

protected:
    // Access the tag for this pointer. This function is protected by default, so that you can choose whether or not to
    // have it part of your api when you subclass `UniquePtr`.
    uint16_t tag() const {
        return this->ptr.tag();
    }

public:
    // Initialization of tagged data from the make function
    explicit constexpr UniquePtr(TaggedPtr ptr) : ptr{ptr} {}

    constexpr UniquePtr() : ptr{0} {}
    constexpr UniquePtr(std::nullptr_t) : ptr{0} {}

    ~UniquePtr() {
        this->cleanup();
    }

    UniquePtr(const UniquePtr &other) = delete;
    UniquePtr &operator=(const UniquePtr &other) = delete;

    UniquePtr(UniquePtr &&other) : ptr{other.ptr} {
        other.ptr = nullptr;
    }

    UniquePtr &operator=(UniquePtr &&other) {
        if (*this == other) {
            return *this;
        }

        this->cleanup();

        // cleanup will zero out our data
        swap(*this, other);

        return *this;
    }

    bool operator==(std::nullptr_t) const {
        return this->ptr == nullptr;
    }

    bool operator!=(std::nullptr_t) const {
        return this->ptr != nullptr;
    }

    operator bool() const {
        return this->ptr != nullptr;
    }

    bool operator==(const UniquePtr &other) const {
        return this->ptr == other.ptr;
    }

    bool operator!=(const UniquePtr &other) const {
        return this->ptr != other.ptr;
    }

    template <typename T> static constexpr uint16_t tag_of() {
        return helpers.template index_of_type<1, T, Types...>();
    }

    void *get() const {
        return this->ptr.get();
    }

    template <typename T, typename... Args> static ChildPtr make(Args &&...args) {
        return ChildPtr{TaggedPtr(tag_of<T>(), new T(std::forward<Args>(args)...))};
    }

    template <typename T> bool isa() const {
        return tag() == tag_of<T>();
    }

    template <typename T> T &cast() & {
        return *reinterpret_cast<T *>(this->get());
    }

    template <typename T> const T &cast() const & {
        return *reinterpret_cast<T *>(this->get());
    }

    template <typename T> T *dyn_cast() & {
        if (this->isa<T>()) {
            return reinterpret_cast<T *>(this->get());
        } else {
            return nullptr;
        }
    }

    template <typename T> const T *dyn_cast() const & {
        if (this->isa<T>()) {
            return reinterpret_cast<T *>(this->get());
        } else {
            return nullptr;
        }
    }

    friend void swap(UniquePtr &a, UniquePtr &b) {
        swap(a.ptr, b.ptr);
    }
};

} // namespace tagged

#endif
