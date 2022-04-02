#ifndef TAGGED_SHARED_PTR_H
#define TAGGED_SHARED_PTR_H

#include <atomic>

#include "tagged/tagging.h"

namespace tagged {

// A tagged shared_ptr style pointer to one of a collection of types.
template <typename ChildPtr, typename... Types> class SharedPtr {

    std::atomic<int> *refs;
    TaggedPtr ptr;

    static constexpr detail::TagHelpers<Types...> helpers;

    void release() {
        if (this->refs == nullptr) {
            return;
        }

        auto count = this->refs->fetch_sub(1);
        if (count > 1) {
            return;
        }

        auto tag = this->ptr.tag();
        auto ptr = this->ptr.get();

        helpers.destroy(tag, ptr);
        delete this->refs;
    }

protected:
    // Access the tag for this pointer. This function is protected by default, so that you can choose whether or not to
    // have it part of your api when you subclass `UniquePtr`.
    uint16_t tag() const {
        return this->ptr.tag();
    }

public:
    // Initialization of tagged data from the make function
    explicit SharedPtr(TaggedPtr ptr) : refs{new std::atomic<int>(1)}, ptr{ptr} {}

    explicit SharedPtr() : refs{nullptr}, ptr{nullptr} {}
    explicit SharedPtr(std::nullptr_t) : SharedPtr{} {}

    SharedPtr(const SharedPtr &other) {
        if (*this == other || other.refs == nullptr) {
            return;
        }

        other.refs->fetch_add(1);
        this->refs = other.refs;
        this->ptr = other.ptr;
    }

    SharedPtr(SharedPtr &&other) {
        if (*this == other || other.refs == nullptr) {
            return;
        }

        this->refs = nullptr;
        this->ptr = nullptr;

        std::swap(other.refs, this->refs);
        std::swap(other.ptr, this->ptr);
    }

    ~SharedPtr() {
        this->release();
    }

    SharedPtr &operator=(const SharedPtr &other) {
        if (*this == other) {
            return *this;
        }

        this->release();
        if (other.refs == nullptr) {
            this->refs = nullptr;
            this->ptr = nullptr;
            return *this;
        }

        other.refs->fetch_add(1);
        this->refs = other.refs;
        this->ptr = other.ptr;

        return *this;
    }

    SharedPtr &operator=(SharedPtr &&other) {
        if (*this == other) {
            return *this;
        }

        this->release();
        this->refs = nullptr;
        this->ptr = nullptr;

        std::swap(other.refs, this->refs);
        std::swap(other.ptr, this->ptr);

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

    bool operator==(const SharedPtr &other) const {
        return this->ptr == other.ptr;
    }

    bool operator!=(const SharedPtr &other) const {
        return this->ptr != other.ptr;
    }

    void *get() const {
        return this->ptr.get();
    }

    template <typename T> static constexpr uint16_t tag_of() {
        return helpers.template index_of_type<1, T, Types...>();
    }

    template <typename T, typename... Args> static ChildPtr make(Args &&...args) {
        return ChildPtr(TaggedPtr(tag_of<T>(), new T(std::forward<Args>(args)...)));
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
