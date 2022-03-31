#include "doctest/doctest.h"
#include <cstdint>
#include <string>
#include <unordered_map>

#include "tagged/shared_ptr.h"

using namespace std::literals::string_view_literals;

class DeleteTracker {
public:
    bool &deleted;

    DeleteTracker(bool &deleted) : deleted{deleted} {
        deleted = false;
    }

    ~DeleteTracker() {
        deleted = true;
    }
};

class DeleteTrackingPtr : public tagged::SharedPtr<DeleteTrackingPtr, DeleteTracker> {
public:
    using SharedPtr::SharedPtr;
    using SharedPtr::tag;
};

TEST_CASE("Ref counting") {
    bool deleted;
    {
        auto tracker = DeleteTrackingPtr::make<DeleteTracker>(deleted);

        CHECK_EQ(1, tracker.tag());

        { auto other = tracker; }
        CHECK_EQ(false, deleted);

        auto other = std::move(tracker);
        CHECK_EQ(false, deleted);
        CHECK_EQ(nullptr, tracker.get());
    }
    CHECK_EQ(true, deleted);
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
