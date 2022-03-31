#ifndef TAGGED_TAGGING_H
#define TAGGED_TAGGING_H

#include <cstdint>

namespace tagged {

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

};

} // namespace tagged

#endif
