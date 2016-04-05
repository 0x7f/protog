#include <gtest/gtest.h>

#include "messages.pb.h"
#include "simplemessage_parser.pb.h"

namespace protog {
namespace test {

TEST(simple_message, should_throw_on_invalid_json_input) {
    EXPECT_THROW(simplemessage_parser_easy(""), std::runtime_error);
    EXPECT_THROW(simplemessage_parser_easy("{"), std::runtime_error);
    EXPECT_THROW(simplemessage_parser_easy("}"), std::runtime_error);
    EXPECT_THROW(simplemessage_parser_easy("."), std::runtime_error);
    // TODO: EXPECT_THROW(simplemessage_parser_easy("a"), std::runtime_error);
    // TODO: EXPECT_THROW(simplemessage_parser_easy("1"), std::runtime_error);
    // TODO: EXPECT_THROW(simplemessage_parser_easy("id"), std::runtime_error);
}

TEST(simple_message, should_reject_empty_message_because_of_missing_fields) {
    const auto msg = simplemessage_parser_easy("{}");
    ASSERT_FALSE(msg.has_id());
    ASSERT_FALSE(msg.has_my_int32());
    ASSERT_FALSE(msg.has_my_double());
}

TEST(simple_message, should_parse_optional_pod_fields) {
    const auto json = R"*({ "id": "foo", "my_int32": 42, "my_double": 42.23 })*";
    const auto msg = simplemessage_parser_easy(json);
    ASSERT_TRUE(msg.has_id());
    ASSERT_EQ("foo", msg.id());
    ASSERT_TRUE(msg.has_my_int32());
    ASSERT_EQ(42, msg.my_int32());
    ASSERT_TRUE(msg.has_my_double());
    ASSERT_EQ(42.23, msg.my_double());
}

TEST(simple_message, should_allow_any_order) {
    const auto json = R"*({ "my_int32": 42, "id": "foo" })*";
    const auto msg = simplemessage_parser_easy(json);
    ASSERT_TRUE(msg.has_id());
    ASSERT_EQ("foo", msg.id());
    ASSERT_TRUE(msg.has_my_int32());
    ASSERT_EQ(42, msg.my_int32());
    ASSERT_FALSE(msg.has_my_double());
}

// TODO: test every single type conversion!

TEST(simple_message, should_allow_int_as_double) {
    const auto json = R"*({ "my_double": 42 })*";
    const auto msg = simplemessage_parser_easy(json);
    ASSERT_TRUE(msg.has_my_double());
    ASSERT_EQ(42.0, msg.my_double());
}

} // namespace test
} // namespace protog
