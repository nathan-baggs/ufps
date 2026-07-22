#include <meta>

#include <gtest/gtest.h>

#include "events/input_map.h"
#include "events/key.h"

TEST(input_map, simple)
{
    auto im = ufps::InputMap{};

    ASSERT_EQ(im.delta_x, 0.0f);
    ASSERT_EQ(im.delta_y, 0.0f);

    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^ufps::Key)))
    {
        ASSERT_FALSE(im[[:e:]]);
        ASSERT_FALSE(im.is_set([:e:]));
    }
}

TEST(input_map, key_down)
{
    auto im = ufps::InputMap{};
    const auto key = ufps::Key::A;

    im.set(ufps::KeyEvent{key, ufps::KeyState::DOWN});

    ASSERT_TRUE(im[key]);
    ASSERT_TRUE(im.is_set(key));

    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^ufps::Key)))
    {
        if ([:e:] != key)
        {
            ASSERT_FALSE(im[[:e:]]);
            ASSERT_FALSE(im.is_set([:e:]));
        }
    }
}

TEST(input_map, key_up)
{
    auto im = ufps::InputMap{};
    const auto key = ufps::Key::A;

    im.set(ufps::KeyEvent{key, ufps::KeyState::DOWN});
    im.set(ufps::KeyEvent{key, ufps::KeyState::UP});

    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^ufps::Key)))
    {
        ASSERT_FALSE(im[[:e:]]);
        ASSERT_FALSE(im.is_set([:e:]));
    }
}

TEST(input_map, key_down_multi)
{
    auto im = ufps::InputMap{};
    const auto keys = std::set{ufps::Key::A, ufps::Key::ESC, ufps::Key::X};

    for (const auto key : keys)
    {
        im.set(ufps::KeyEvent{key, ufps::KeyState::DOWN});

        ASSERT_TRUE(im[key]);
        ASSERT_TRUE(im.is_set(key));
    }

    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^ufps::Key)))
    {
        if (!keys.contains([:e:]))
        {
            ASSERT_FALSE(im[[:e:]]);
            ASSERT_FALSE(im.is_set([:e:]));
        }
    }
}
