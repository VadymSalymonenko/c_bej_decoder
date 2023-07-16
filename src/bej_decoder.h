#ifndef BEJ_DECODER_H
#define BEJ_DECODER_H

#include <stddef.h>
#include <string.h>

#define INITIAL_CAPACITY 20
#define CAPACITY_INCREASE_STEP 50

struct bej_node {
	unsigned char dictionary_type;
	unsigned char sequence;
    unsigned char format;
    unsigned int length;
	unsigned int count;
    void** value;
};

enum data_types {
	//BOOLEAN = 0x30,
	INTEGER = 0x30,
	ENUM = 0x40,
	STRING = 0x50,
	ARRAY = 0x10
};

enum bej_node_to_str_types {
	WITH_KEY,
	WITHOUT_KEY
};

struct dynamic_string {
    char* data;
    size_t length;
    size_t capacity;
};

struct dynamic_string* decode_bej(unsigned char *data, size_t data_len, 
								unsigned char *schema_dictionary, size_t schema_dictionary_len, 
								const unsigned char *annotation_dictionary, size_t annotation_dictionary_len);
	
#endif
