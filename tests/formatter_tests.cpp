#include <gtest/gtest.h>

#include "utils/formatter.h"

namespace
{

struct ObjWithToStringMember
{
    auto to_string() const -> std::string
    {
        return "ObjWithToStringMember to_string";
    }
};

struct ObjWithToStringFree
{
};

auto to_string(const ObjWithToStringFree &) -> std::string
{
    return "ObjWithToStringFree to_string free";
}

struct ObjWithStringMemberAndFree
{
    auto to_string() const -> std::string
    {
        return "ObjWithStringMemberAndFree to_string member";
    }
};

auto to_string(const ObjWithStringMemberAndFree &) -> std::string
{
    return "ObjWithStringMemberAndFree to_string free";
}

}

TEST(formatter, to_string_member)
{
    ObjWithToStringMember obj;

    std::string result = std::format("{}", obj);

    ASSERT_EQ(result, "ObjWithToStringMember to_string");
}

TEST(formatter, to_string_free)
{
    ObjWithToStringFree obj;

    std::string result = std::format("{}", obj);

    ASSERT_EQ(result, "ObjWithToStringFree to_string free");
}

TEST(formatter, to_string_member_and_free)
{
    ObjWithStringMemberAndFree obj;

    std::string result = std::format("{}", obj);

    ASSERT_EQ(result, "ObjWithStringMemberAndFree to_string member");
}
