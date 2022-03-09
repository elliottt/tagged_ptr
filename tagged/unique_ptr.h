#ifndef CORE_TAGGED_PTR_H
#define CORE_TAGGED_PTR_H

#include <cstdint>
#include <type_traits>
#include <utility>

namespace tagged {

// A tagged unique_ptr style pointer to one of a collection of types.
template <typename ChildPtr, typename... Types> class UniquePtr {
    uint64_t data;

    static constexpr uint64_t TAG_BITS = 16;
    static constexpr uint64_t TAG_MASK = 0xffff;

    static constexpr uint64_t tagged_data(uint16_t tag, void *data) {
        return static_cast<uint64_t>(tag) | (reinterpret_cast<uint64_t>(data) << TAG_BITS);
    }

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
        if (this->data == 0) {
            return;
        }

        auto tag = this->tag();
        auto ptr = this->get();
        this->data = 0;

        delete_helper<1, Types...>(tag, ptr);
    }

public:
    // For taking over tagged data
    explicit constexpr UniquePtr(uint64_t data) : data{data} {}

    constexpr UniquePtr() : data{0} {}
    constexpr UniquePtr(std::nullptr_t) : data{0} {}

    ~UniquePtr() {
        this->cleanup();
    }

    UniquePtr(const UniquePtr &other) = delete;
    UniquePtr &operator=(const UniquePtr &other) = delete;

    UniquePtr(UniquePtr &&other) : data{other.data} {
        other.data = 0;
    }

    UniquePtr &operator=(UniquePtr &&other) {
        this->cleanup();

        // cleanup will zero out our data
        std::swap(this->data, other.data);

        return *this;
    }

    bool operator==(std::nullptr_t) const {
        return this->data == 0;
    }

    bool operator!=(std::nullptr_t) const {
        return this->data != 0;
    }

    operator bool() const {
        return this->data != 0;
    }

    bool operator==(const UniquePtr &other) const {
        return this->data == other.data;
    }

    bool operator!=(const UniquePtr &other) const {
        return this->data != other.data;
    }

    template <typename T> static constexpr uint16_t tag_of() {
        return index_of_type<1, T, Types...>();
    }

    uint16_t tag() const {
        return static_cast<uint16_t>(this->data & TAG_MASK);
    }

    void *get() const {
        return reinterpret_cast<void *>(this->data >> TAG_BITS);
    }

    template <typename T, typename... Args> static ChildPtr make(Args &&...args) {
        return ChildPtr{tagged_data(tag_of<T>(), new T(std::forward<Args>(args)...))};
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

} // namespace nsml::util

#endif
