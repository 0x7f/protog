#include <gtest/gtest.h>

#include "messages.pb.h"
#include "nestedmessage_parser.pb.h"

namespace protog {
namespace test {

TEST(nested_message, should_allow_omitting_optional_sub_message) {
    const auto json = R"*({})*";
    const auto msg = nestedmessage_parser_easy(json);
    ASSERT_FALSE(msg.has_id());
    ASSERT_FALSE(msg.has_my_inner());
    ASSERT_EQ(msg.my_list_size(), 0);
}

TEST(nested_message, should_parse_empty_inner_message) {
    const auto json = R"*({ "my_inner": {} })*";
    const auto msg = nestedmessage_parser_easy(json);
    ASSERT_TRUE(msg.has_my_inner());
}

TEST(nested_message, should_parse_empty_inner_message_with_id_before) {
    const auto json = R"*({ "id": "foo", "my_inner": {} })*";
    const auto msg = nestedmessage_parser_easy(json);
    ASSERT_TRUE(msg.has_id());
    ASSERT_TRUE(msg.has_my_inner());
}

TEST(nested_message, should_parse_empty_inner_message_with_id_after) {
    const auto json = R"*({ "my_inner": {}, "id": "foo" })*";
    const auto msg = nestedmessage_parser_easy(json);
    ASSERT_TRUE(msg.has_id());
    ASSERT_TRUE(msg.has_my_inner());
}

TEST(nested_message, should_parse_fields_in_inner_message) {
    const auto json = R"*({ "my_inner": { "a": "foo", "b": [42, 23] } })*";
    const auto msg = nestedmessage_parser_easy(json);
    ASSERT_TRUE(msg.has_my_inner());
    const auto& inner = msg.my_inner();
    ASSERT_EQ(inner.a(), "foo");
    ASSERT_EQ(inner.b_size(), 2);
    ASSERT_EQ(inner.b(0), 42);
    ASSERT_EQ(inner.b(1), 23);
}

} // namespace test
} // namespace protog
