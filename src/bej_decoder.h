#ifndef BEJ_DECODER_H
#define BEJ_DECODER_H

#include <stddef.h>
#include <string.h>

#define INITIAL_CAPACITY 20
#define CAPACITY_INCREASE_STEP 50

/**
 * @brief Represents a single node in the BEJ data.
 *
 * The bej_node structure holds the information required for a single BEJ data node.
 */
struct bej_node {
    unsigned char dictionary_type;    ///< The type of dictionary (schema or annotation).
    unsigned char sequence;           ///< The sequence number of the node.
    unsigned char format;             ///< The format of the data (e.g., INTEGER, ENUM, STRING, ARRAY).
    unsigned int length;              ///< The length of the data value.
    unsigned int count;               ///< The count of elements in case of ARRAY format.
    void** value;                     ///< A pointer to the data value.
};

/**
 * @brief Defines the types of data in the BEJ format.
 *
 * This enum lists the different types of data that can be present in BEJ.
 * Currently, it includes INTEGER, ENUM, STRING, and ARRAY.
 */
enum data_types {
	//BOOLEAN = 0x30,
	INTEGER = 0x30,
	ENUM = 0x40,
	STRING = 0x50,
	ARRAY = 0x10
};

/**
 * @brief Defines the types of parsing for BEJ nodes to strings.
 *
 * This enum lists the two modes of parsing for BEJ nodes into strings.
 * It can either include the key (WITH_KEY) or exclude it (WITHOUT_KEY).
 */
enum bej_node_to_str_types {
	WITH_KEY,
	WITHOUT_KEY
};

/**
 * @brief Represents a dynamic string used for BEJ data parsing.
 *
 * The dynamic_string structure is used to build a string dynamically during the BEJ data parsing.
 */
struct dynamic_string {
    char* data;
    size_t length;
    size_t capacity;
};

struct dynamic_string* decode_bej(unsigned char *data, size_t data_len, 
								unsigned char *schema_dictionary, size_t schema_dictionary_len, 
								const unsigned char *annotation_dictionary, size_t annotation_dictionary_len);
	
#endif
