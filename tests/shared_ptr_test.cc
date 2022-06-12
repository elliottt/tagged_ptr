#include "doctest/doctest.h"
#include <cstdint>
#include <type_traits>

#include "tagged/shared_ptr.h"

class Empty final {};

class DeleteTracker final {
public:
    bool &deleted;

    DeleteTracker(bool &deleted) : deleted{deleted} {
        deleted = false;
    }

    ~DeleteTracker() {
        deleted = true;
    }
};

class DeleteTrackingPtr : public tagged::SharedPtr<DeleteTrackingPtr, Empty, DeleteTracker> {
public:
    using SharedPtr::SharedPtr;
    using SharedPtr::tag;
};

static_assert(std::is_swappable<DeleteTrackingPtr>::value);

TEST_CASE("Ref counting") {
    bool deleted;
    {
        auto tracker = DeleteTrackingPtr::make<DeleteTracker>(deleted);

        CHECK_EQ(2, tracker.tag());

        { auto other = tracker; }
        CHECK_EQ(false, deleted);

        auto other = std::move(tracker);
        CHECK_EQ(false, deleted);
        CHECK_EQ(nullptr, tracker.get());
    }
    CHECK_EQ(true, deleted);

    {
        auto tracker = DeleteTrackingPtr::make<DeleteTracker>(deleted);
        CHECK_EQ(false, tracker.cast<DeleteTracker>().deleted);

        tracker = DeleteTrackingPtr::make<Empty>();
        CHECK_EQ(true, deleted);
        CHECK_EQ(nullptr, tracker.dyn_cast<DeleteTracker>());
    }
}

TEST_CASE("Cleanup") {
    bool first_deleted;
    bool second_deleted;
    {
        auto tracker = DeleteTrackingPtr::make<DeleteTracker>(first_deleted);
        tracker = DeleteTrackingPtr::make<DeleteTracker>(second_deleted);
        CHECK_EQ(true, first_deleted);
    }
    CHECK_EQ(true, second_deleted);
}
