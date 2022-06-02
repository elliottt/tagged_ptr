#ifndef TAGGED_TAGGING_H
#define TAGGED_TAGGING_H

#include <cstdint>
#include <utility>

namespace tagged {

namespace detail {

template <typename... Types> class TagHelpers final {

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

public:

    static void destroy(uint16_t tag, void *ptr) {
        delete_helper<1, Types...>(tag, ptr);
    }

    template <uint16_t Acc, typename X, typename Y, typename... Rest> static constexpr uint16_t index_of_type() {
        if constexpr (std::is_same<X, Y>::value) {
            return Acc;
        } else {
            static_assert(sizeof...(Rest) > 0, "Type not present in tagged pointer");
            return index_of_type<1 + Acc, X, Rest...>();
        }
    }
};

} // namespace detail

class TaggedPtr final {

    uint64_t data;

    static constexpr int TAG_BITS = 16;
    static constexpr uint64_t TAG_MASK = 0xffff;

    static constexpr uint64_t tagged_data(uint16_t tag, uint64_t data) {
        return static_cast<uint64_t>(tag) | (data << TAG_BITS);
    }

public:
    constexpr TaggedPtr() : data{0} {}
    constexpr TaggedPtr(std::nullptr_t) : TaggedPtr{} {}

    TaggedPtr(uint16_t tag, void *data) : data{tagged_data(tag, reinterpret_cast<uint64_t>(data))} {}

    inline uint16_t tag() const {
        return static_cast<uint16_t>(this->data & TAG_MASK);
    }

    inline void *get() const {
        return reinterpret_cast<void *>(this->data >> TAG_BITS);
    }

    inline bool operator==(std::nullptr_t) const {
        return this->data == 0;
    }

    inline bool operator!=(std::nullptr_t) const {
        return this->data != 0;
    }

    inline bool operator==(const TaggedPtr &other) const {
        return this->data == other.data;
    }

    inline bool operator!=(const TaggedPtr &other) const {
        return this->data != other.data;
    }

    friend void swap(TaggedPtr &a, TaggedPtr &b) {
        std::swap(a.data, b.data);
    }
};

} // namespace tagged

#endif
