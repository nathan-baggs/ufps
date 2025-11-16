#include <gtest/gtest.h>

#include "utils/error.h"

TEST(error, expect_true)
{
    EXPECT_NO_THROW(ufps::expect(true, "This should not throw"));
}

TEST(error, expect_false)
{
    EXPECT_DEATH(ufps::expect(false, "This should terminate"), "");
}

TEST(error, ensure_true)
{
    EXPECT_NO_THROW(ufps::ensure(true, "This should not throw"));
}

TEST(error, ensure_false)
{
    EXPECT_THROW(ufps::ensure(false, "This should throw"), ufps::Exception);
}

TEST(error, ensure_auto_release)
{
    ufps::AutoRelease<int *, nullptr> ptr{new int{}, [](int *p) { delete p; }};

    EXPECT_NO_THROW(ufps::ensure(ptr, "This should not throw"));

    ufps::AutoRelease<int *, nullptr> invalid_ptr{nullptr, nullptr};

    EXPECT_THROW(ufps::ensure(invalid_ptr, "This should throw"), ufps::Exception);
}

TEST(error, ensure_unique_ptr)
{
    std::unique_ptr<int> ptr = std::make_unique<int>(42);

    EXPECT_NO_THROW(ufps::ensure(ptr, "This should not throw"));

    std::unique_ptr<int> invalid_ptr;

    EXPECT_THROW(ufps::ensure(invalid_ptr, "This should throw"), ufps::Exception);
}
