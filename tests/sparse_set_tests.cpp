#include <gtest/gtest.h>

#include "core/sparse_set.h"

TEST(sparse_set, ctor)
{
    auto s = ufps::SparseSet<int>{};
    ASSERT_EQ(s.size(), 0zu);
    ASSERT_TRUE(s.empty());
}

TEST(sparse_set, emplace_get)
{
    auto s = ufps::SparseSet<int>{};
    auto h = s.emplace(2);

    ASSERT_EQ(s.size(), 1zu);
    ASSERT_TRUE(!s.empty());

    auto value_opt = s[h];
    ASSERT_TRUE(!!value_opt);

    auto &value = *value_opt;
    ASSERT_EQ(value, 2);

    ++value;
    ASSERT_EQ(*s[h], 3);
}

TEST(sparse_set, emplace_get_const)
{
    auto s = ufps::SparseSet<int>{};
    auto h = s.emplace(2);

    const auto &s_const = s;

    ASSERT_EQ(s_const.size(), 1zu);
    ASSERT_TRUE(!s_const.empty());

    auto value_opt = s_const[h];
    ASSERT_TRUE(!!value_opt);

    const auto &value = *value_opt;
    ASSERT_EQ(value, 2);
}

TEST(sparse_set, remove_single)
{
    auto s = ufps::SparseSet<int>{};
    auto h = s.emplace(2);

    s.remove(h);

    ASSERT_EQ(s.size(), 0zu);
    ASSERT_TRUE(s.empty());

    ASSERT_TRUE(!s[h]);
}

TEST(sparse_set, multiple_add_single_remove)
{
    auto s = ufps::SparseSet<int>{};
    auto h1 = s.emplace(2);
    auto h2 = s.emplace(20);
    auto h3 = s.emplace(200);

    s.remove(h2);

    ASSERT_EQ(s.size(), 2zu);
    ASSERT_TRUE(!s.empty());

    auto h1_opt = s[h1];
    ASSERT_TRUE(!!h1_opt);
    ASSERT_EQ(*h1_opt, 2);

    auto h2_opt = s[h2];
    ASSERT_TRUE(!h2_opt);

    auto h3_opt = s[h3];
    ASSERT_TRUE(!!h3_opt);
    ASSERT_EQ(*h3_opt, 200);
}

TEST(sparse_set, multiple_add_remove_first)
{
    auto s = ufps::SparseSet<int>{};
    auto h1 = s.emplace(2);
    auto h2 = s.emplace(20);
    auto h3 = s.emplace(200);

    s.remove(h1);

    ASSERT_EQ(s.size(), 2zu);
    ASSERT_TRUE(!s.empty());

    auto h1_opt = s[h1];
    ASSERT_TRUE(!h1_opt);

    auto h2_opt = s[h2];
    ASSERT_TRUE(!!h2_opt);
    ASSERT_EQ(*h2_opt, 20);

    auto h3_opt = s[h3];
    ASSERT_TRUE(!!h3_opt);
    ASSERT_EQ(*h3_opt, 200);
}

TEST(sparse_set, multiple_add_remove_last)
{
    auto s = ufps::SparseSet<int>{};
    auto h1 = s.emplace(2);
    auto h2 = s.emplace(20);
    auto h3 = s.emplace(200);

    s.remove(h3);

    ASSERT_EQ(s.size(), 2zu);
    ASSERT_TRUE(!s.empty());

    auto h1_opt = s[h1];
    ASSERT_TRUE(!!h1_opt);
    ASSERT_EQ(*h1_opt, 2);

    auto h2_opt = s[h2];
    ASSERT_TRUE(!!h2_opt);
    ASSERT_EQ(*h2_opt, 20);

    auto h3_opt = s[h3];
    ASSERT_TRUE(!h3_opt);
}

TEST(sparse_set, double_remove)
{
    auto s = ufps::SparseSet<int>{};
    auto h = s.emplace(2);

    s.remove(h);

    ASSERT_THROW(s.remove(h), ufps::Exception);
}

TEST(sparse_set, complex_add_remove)
{
    auto s = ufps::SparseSet<int>{};
    auto h1 = s.emplace(2);
    auto h2 = s.emplace(20);

    s.remove(h2);

    auto h3 = s.emplace(200);
    auto h4 = s.emplace(2000);

    s.remove(h1);

    auto h5 = s.emplace(20000);
    auto h6 = s.emplace(200000);

    s.remove(h5);
    s.remove(h3);
    s.remove(h6);
    s.remove(h4);

    ASSERT_EQ(s.size(), 0zu);
    ASSERT_TRUE(s.empty());

    ASSERT_TRUE(!s[h1]);
    ASSERT_TRUE(!s[h2]);
    ASSERT_TRUE(!s[h3]);
    ASSERT_TRUE(!s[h4]);
    ASSERT_TRUE(!s[h5]);
    ASSERT_TRUE(!s[h6]);
}

TEST(sparse_set, handles)
{
    auto s = ufps::SparseSet<int>{};
    s.emplace(2);
    s.emplace(20);
    s.emplace(200);

    const auto handles = s.handles();

    ASSERT_EQ(std::ranges::size(handles), 3zu);
    ASSERT_TRUE(!!s[handles[0]]);
    ASSERT_EQ(*s[handles[0]], 2);
    ASSERT_TRUE(!!s[handles[1]]);
    ASSERT_EQ(*s[handles[1]], 20);
    ASSERT_TRUE(!!s[handles[2]]);
    ASSERT_EQ(*s[handles[2]], 200);
}

TEST(sparse_set, handles_with_gap)
{
    auto s = ufps::SparseSet<int>{};
    s.emplace(2);
    const auto h = s.emplace(20);
    s.emplace(200);

    s.remove(h);

    const auto handles = s.handles();

    ASSERT_EQ(std::ranges::size(handles), 2zu);
    ASSERT_TRUE(!!s[handles[0]]);
    ASSERT_EQ(*s[handles[0]], 2);
    ASSERT_TRUE(!!s[handles[1]]);
    ASSERT_EQ(*s[handles[1]], 200);
}

TEST(sparse_set, handles_complex)
{
    auto s = ufps::SparseSet<int>{};
    const auto h1 = s.emplace(2);
    const auto h2 = s.emplace(20);
    s.emplace(200);
    s.emplace(2000);
    s.emplace(20000);

    s.remove(h1);
    s.remove(h2);

    auto handles = std::vector<ufps::SparseSet<int>::handle_type>{};

    for (const auto &h : s.handles())
    {
        handles.push_back(h);
    }

    const auto expected_values = std::vector<int>{200, 2000, 20000};
    *s[handles[0]];
    *s[handles[1]];
    *s[handles[2]];
    const auto values =
        handles | std::views::transform([&s](const auto &h) { return *s[h]; }) | std::ranges::to<std::vector>();

    ASSERT_EQ(values, expected_values);
}

TEST(sparse_set, handles_with_gap_end)
{
    auto s = ufps::SparseSet<int>{};
    s.emplace(2);
    s.emplace(20);
    const auto h = s.emplace(200);

    s.remove(h);

    const auto handles = s.handles();

    ASSERT_EQ(std::ranges::size(handles), 2zu);
    ASSERT_TRUE(!!s[handles[0]]);
    ASSERT_EQ(*s[handles[0]], 2);
    ASSERT_TRUE(!!s[handles[1]]);
    ASSERT_EQ(*s[handles[1]], 20);
    ASSERT_TRUE(!s[h]);
}

TEST(sparse_set, data)
{
    auto s = ufps::SparseSet<int>{};
    s.emplace(2);
    s.emplace(20);
    s.emplace(200);

    const auto data = s.data();

    ASSERT_EQ(std::ranges::size(data), 3zu);
    ASSERT_EQ(data[0], 2);
    ASSERT_EQ(data[1], 20);
    ASSERT_EQ(data[2], 200);
}

TEST(sparse_set, reuse_handle)
{
    auto s = ufps::SparseSet<int>{};
    s.emplace(2);
    const auto h1 = s.emplace(20);
    s.emplace(200);

    s.remove(h1);

    const auto h2 = s.emplace(2000);

    ASSERT_TRUE(!s[h1]);
    ASSERT_TRUE(!!s[h2]);
    ASSERT_EQ(*s[h2], 2000);
}
