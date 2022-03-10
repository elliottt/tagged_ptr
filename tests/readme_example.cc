#include "doctest/doctest.h"

#include "tagged/unique_ptr.h"
#include <iostream>

class A {};
class B {};

class Ptr final : public tagged::UniquePtr<Ptr, A, B> {
public:
    using UniquePtr::UniquePtr;

    // Use the builtin tag function that returns `uint16_t` tags.
    using UniquePtr::tag;
};

template <typename T> void use(T &val) {}

TEST_CASE("readme example") {

    Ptr a = Ptr::make<A>();
    auto b = Ptr::make<B>();

    // manual tag checking
    if (a.tag() == Ptr::tag_of<A>()) {
        std::cout << "the system works!" << std::endl;
    }

    // convenience function for tag checking
    if (a.isa<B>()) {
        std::cout << "well that's a surprise" << std::endl;
    }

    // dynamic cast
    if (A *aptr = a.dyn_cast<A>()) {
        std::cout << "a is still an A" << std::endl;
    }

    if (B *bptr = b.dyn_cast<B>()) {
        std::cout << "b is still a B" << std::endl;
    }

    // regular cast (make sure you know this will succeed!)
    A &aref = a.cast<A>();
    auto &bref = b.cast<B>();

    use(aref);
    use(bref);

    // both a and b behave like std::unique_ptr, and are freed at this point
}
