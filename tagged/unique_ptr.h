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

    template <uint16_t Acc, typename X, typename Y, typename... Rest> static constexpr uint16_t index_of_type() {
        if constexpr (std::is_same<X, Y>::value) {
            return Acc;
        } else {
            static_assert(sizeof...(Rest) > 0, "Type not present in tagged pointer");
            return index_of_type<1 + Acc, X, Rest...>();
        }
    }

    template <uint16_t Ix, typename T, typename... Rest> static void delete_helper(uint16_t tag, void *data) {
        if (tag == Ix) {
            delete reinterpret_cast<T *>(data);
        } else {
            delete_helper<Ix + 1, Rest...>(tag, data);
        }
    }

    template <uint16_t Ix> static void delete_helper(uint16_t tag, void *data) {
        return;
    }

    void cleanup() {
        if (this->ptr == nullptr) {
            return;
        }

        auto tag = this->tag();
        auto ptr = this->get();
        this->ptr = nullptr;

        delete_helper<1, Types...>(tag, ptr);
    }

protected:
    // Access the tag for this pointer. This function is protected by default, so that you can choose whether or not to
    // have it part of your api when you subclass `UniquePtr`.
    uint16_t tag() const {
        return this->ptr.tag();
    }

public:
    // For taking over tagged data
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
        std::swap(this->ptr, other.ptr);

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
        return index_of_type<1, T, Types...>();
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
};

} // namespace tagged

#endif
