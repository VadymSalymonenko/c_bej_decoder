#define TESTING


#include "gtest/gtest.h"
#include "../bej_decoder.h"

/**
 * @brief Converts a string of hexadecimal digits into an array of bytes.
 * This function iterates through pairs of characters in the input string,
 * converting each pair from a hexadecimal digit to a byte value using sscanf, 
 * and storing the resulting bytes in a dynamically allocated array.
 *
 * @param hex A pointer to a null-terminated string containing hexadecimal digits.
 * It is assumed that the string length is even and the string contains only valid 
 * hexadecimal digits (0-9, A-F, a-f).
 *
 * @return A pointer to the newly allocated array of bytes. The length of the array 
 * is half the length of the input string. The caller is responsible for freeing 
 * this memory when it is no longer needed.
 */
unsigned char* hex_to_bytes(const char* hex)
{
    size_t len = strlen(hex);
    unsigned char* result = (unsigned char*)malloc(len/2);
    for (size_t i = 0; i < len; i += 2){
        sscanf(hex+i, "%2hhx", &result[i/2]);
    }
    return result;
}

/**
 * @test parse_SFLV_init_test_returns_correct_format
 * @brief This test case checks the function 'parse_sflv_init' for the correct 
 * decoding of format, count, sequence, and dictionary type fields.
 * 
 * @details The input data is a hexadecimal string representing an array of bytes. 
 * The 'hex_to_bytes' function is used to convert this string into an array of bytes.
 * The function 'parse_sflv_init' is then expected to parse this byte array into a 
 * 'bej_node' structure. The test checks the 'format', 'count', 'sequence', and 
 * 'dictionary_type' fields of the returned structure, comparing them to expected values.
 * If they match, the test passes.
 */
TEST(parse_SFLV_init_test, returns_correct_format)
{
    const char* data_hex = "00f0f0f1000000010000010a0101010830010140";
    unsigned char* data = hex_to_bytes(data_hex);
    size_t data_len = strlen(data_hex)/2;

    struct bej_node* result = parse_sflv_init(data, data_len);
    
    EXPECT_EQ(result->format, 0x10);
    EXPECT_EQ(result->count, 1);  
    EXPECT_EQ(result->sequence, 255);  
    EXPECT_EQ(result->dictionary_type, 255);  

    free(result);
    free(data);
}

/**
 * @test parse_SFLV_integer_node_returns_correct_format
 * @brief This test case checks the function 'parse_sflv_init' for the correct 
 * decoding of format, count, sequence, and dictionary type fields.
 * 
 * @details The input data is a hexadecimal string representing an array of bytes. 
 * The function 'parse_sflv_init' is expected to parse this byte array into a 
 * 'bej_node' structure. The test checks the 'format', 'count', 'sequence', and 
 * 'dictionary_type' fields of the second element of the returned structure, 
 * comparing them to expected values. If they match, the test passes.
 */
TEST(parse_SFLV_integer_node, returns_correct_format)
{
    const char* data_hex = "00f0f0f1000000010000011001020108300103000001010a30010140";
    unsigned char* data = hex_to_bytes(data_hex);
    size_t data_len = strlen(data_hex)/2;

    struct bej_node* result = parse_sflv_init(data, data_len);

    if (result->count >= 2) {
        struct bej_node* second_element = (struct bej_node*)result->value[1];
        EXPECT_EQ(second_element->format, 0x30);
        EXPECT_EQ(second_element->count, 0);  
        EXPECT_EQ(second_element->sequence, 5);  
        EXPECT_EQ(second_element->dictionary_type, 0); 
    }
 

    free(result);
    free(data);
}

/**
 * @test append_string_test_multiple_appends
 * @brief This test case checks the 'append_string' function for multiple string appends.
 * 
 * @details An initial dynamic string is created with 'create_dynamic_string' and 
 * then appended with multiple strings using the 'append_string' function.
 * After each append operation, the test checks if the string has been correctly 
 * appended and its length has been updated correctly. The test also checks if the 
 * overall string is as expected after each append operation. If the appended string 
 * and its length match the expected string and length, the test passes.
 */
TEST(append_string_test, multiple_appends)
{
    struct dynamic_string* str = create_dynamic_string();
    const char* append_str1 = "Hello";
    const char* append_str2 = ", World";
    const char* append_str3 = "!";
    const char* append_str4 = " test test?";
    
    append_string(str, append_str1);
    EXPECT_EQ(str->length, strlen(append_str1));
    EXPECT_STREQ(str->data, append_str1);
    
    append_string(str, append_str2);
    const char* expected_str1 = "Hello, World";
    EXPECT_EQ(str->length, strlen(expected_str1));
    EXPECT_STREQ(str->data, expected_str1);
    
    append_string(str, append_str3);
    const char* expected_str2 = "Hello, World!";
    EXPECT_EQ(str->length, strlen(expected_str2));
    EXPECT_STREQ(str->data, expected_str2);
    
    append_string(str, append_str4);
    const char* expected_str3 = "Hello, World! test test?";
    EXPECT_EQ(str->length, strlen(expected_str3));
    EXPECT_STREQ(str->data, expected_str3);
    
    free(str->data);
	free(str);
}

/**
 * @test read_str_test_simple_string
 * @brief This test case checks the function 'read_str_const_ptr' for a simple string.
 * 
 * @details The input data is the string "Hello" encoded as an array of unsigned char. 
 * The expected output is the original string "Hello". The function is expected to 
 * correctly decode the input data and return the original string. The returned string 
 * is compared to the expected output using the 'ASSERT_STREQ' macro. If they match, 
 * the test passes.
 */

TEST(read_str_test, simple_string)
{
    unsigned char data[] = { 'H', 'e', 'l', 'l', 'o' };
    const char* expected_value = "Hello";
    char* result = read_str_const_ptr(data, 5);
    
    ASSERT_STREQ(expected_value, result);  

    free(result);  
}

/**
 * @test read_str_test_empty_string
 * @brief This test case checks the function 'read_str_const_ptr' for an empty string.
 * 
 * @details The input data is an empty string, encoded as a one-element array of 
 * unsigned char containing a null character. The expected output is an empty string. 
 * The function is expected to correctly decode the input data and return an empty string.
 * The returned string is compared to the expected output using the 'ASSERT_STREQ' macro.
 * If they match, the test passes.
 */
TEST(read_str_test, empty_string)
{
    unsigned char data[] = { '\0' };
    const char* expected_value = "";
    char* result = read_str_const_ptr(data, 1);
    
    ASSERT_STREQ(expected_value, result);

    free(result);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
