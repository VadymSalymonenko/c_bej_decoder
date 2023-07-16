#include "bej_decoder.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief Advances the position of a pointer.
 *
 * This function advances the position of a pointer by a given number of bytes.
 *
 */
void advance_ptr(unsigned char** data, int bytes_to_advance)
{
    *data += bytes_to_advance;
}

/**
 * @brief Reads an integer from data.
 *
 * This function reads an integer of a given length from data. 
 * The position of the data pointer is not modified.
 *
 * @return The integer value read from the data.
 */
int read_int_const_ptr(unsigned char *data, int length)
{
	int value = 0;
	for (int i = length-1; i >= 0; i--) {
        value <<= 8; 
        value += *(data+i); 
    }
	return value;
}

/**
 * @brief Reads a string from data.
 *
 * This function reads a string of a given length from data. 
 * The position of the data pointer is not modified.
 *
 * @return A pointer to the string read from the data.
 */
char* read_str_const_ptr(unsigned char *data, int length)
{
    char *str = (char*) malloc((length + 1) * sizeof(char));

    if (str == NULL) {
        fprintf(stderr, "Could not allocate memory for str\n");
        exit(1);
    }
    for (int i = 0; i < length; i++) {
        str[i] = *(data+i);
    }
    str[length] = '\0';	
    return str;
}

/**
 * @brief Reads an integer from data.
 *
 * This function reads an integer of a given length from data and 
 * advances the data pointer by the same number of bytes.
 *
 * @param data A double pointer to the data. The double pointer is needed to allow the pointer to shift after the data is read
 * @param length The length of the integer to read.
 * @return The integer value read from the data.
 */
int read_int(unsigned char **data, int length)
{
	int value = 0;
	for (int i = length-1; i >= 0; i--) {
        value <<= 8;
        value += *((*data)+i);
    }
	advance_ptr(data, length);
	return value;
}

/**
 * @brief Reads the length of an integer from data, then reads the integer.
 *
 * This function first reads a byte from data to determine the length of the integer, 
 * then reads the integer of that length from data. 
 * The data pointer is advanced by the length of the integer plus one byte.
 *
 * @param data A double pointer to the data. The double pointer is needed to allow the pointer to shift after the data is read
 * @return The integer value read from the data.
 */
int read_length_and_get_int(unsigned char **data)
{
	unsigned char length = **data;
	advance_ptr(data, 1);
	return read_int(data, length);
}

/**
 * @brief Reads a string from data.
 *
 * This function reads a string of a given length from data and 
 * advances the data pointer by the same number of bytes. 
 * The string will be null-terminated.
 *
 * @param data A double pointer to the data. The position of the data pointer will be modified.
 * @param length The length of the string to read.
 * @return A pointer to the string read from the data.
 */
char* read_str(unsigned char **data, int length)
{
    char *str = (char*) malloc((length + 1) * sizeof(char));

    if (str == NULL) {
        fprintf(stderr, "Could not allocate memory for str\n");
        exit(1);
    }
	
    for (int i = 0; i < length; i++) {
        str[i] = *((*data)+i);
    }
    str[length] = '\0';
	advance_ptr(data, length);
	
    return str;
}

/**
 * @brief Allocates memory for an array of pointers to SFLV structures.
 *
 * This function allocates memory for an array of pointers to SFLV structures 
 * according to the count of elements specified in the root node. 
 * The memory for each structure is also allocated and initialized to zero.
 *
 * @param root A pointer to the root node. The count of the root node is used 
 * to determine the number of elements in the array.
 */
void allocate_sflv_array(struct bej_node* root)
{
	// allocate memory for the array of pointers to structures
    struct bej_node** array = malloc(root->count * sizeof(struct bej_node*));

	// check whether the memory was successfully allocated
    if (array == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }

    // array initialization
    for (unsigned int i = 0; i < root->count; ++i) {
        struct bej_node* new_sflv = malloc(sizeof(struct bej_node));

        if (new_sflv == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            exit(1);
        }

        *new_sflv = (struct bej_node){0};
        array[i] = new_sflv;
    }

    root->value = (void**)array;
}

/**
 * @brief Frees memory of an array of pointers to SFLV structures.
 *
 * This function frees the memory of each SFLV structure and the array of pointers.
 */
void free_sflv_array(struct bej_node* root)
{
    for (unsigned int i = 0; i < root->count; ++i) {
        free(root->value[i]);
    }
    free(root->value);
}

/**
 * @brief Recursively parses data into a SFLV structure.
 *
 * This function will fill all basic fields for bej nodes. 
 * For each type, general parameters are filled, but for primitives,
 * a reference to the value is written in the value field, and for complex types, 
 * a reference to the next node or several nodes is stored in the value field.
 *
 * @param parent A pointer to the parent node.
 * @param data A double pointer to the data. The position of the data pointer will be modified.
 * @param data_len The length of the data.
 * @return A pointer to the last node created.
 */
struct bej_node* parse_sflv_recursion(struct bej_node *parent, unsigned char **data, size_t data_len)
{
	int seq = read_length_and_get_int(data);	

	parent->count = 0;
	parent->dictionary_type = seq%2;
	parent->sequence = seq>>1;
	
	parent->format = **data;
	advance_ptr(data, 1);
	
	parent->length = read_length_and_get_int(data);

	switch (parent->format) {
		case INTEGER: 
		{
			int *int_ptr = malloc(sizeof(int)); 
			*int_ptr = read_int(data,parent->length);
			parent->value = (void**)int_ptr;				
			break;
		}
		case STRING: 
		{
			char *str_ptr = read_str(data,parent->length);
			parent->value = (void**)str_ptr;		
			break;
		}
		case ARRAY: 
		{
	        parent->count = read_length_and_get_int((data));
			allocate_sflv_array(parent);
			for (int i = 0; i < parent->count; i++) {
				parse_sflv_recursion(parent->value[i],data, data_len);
			}
			break;
		}
		case ENUM: 
		{
			parent->count = 1;
			int *int_ptr = malloc(sizeof(int)); 
			*int_ptr = read_int(data,parent->length); // TODO: make a function that simply writes several bytes in a direct sequence without the \0 terminator
			parent->value = (void**)int_ptr;
			break;
		}
	}	
    return NULL;
}

/**
 * @brief Initializes the parsing of data into a SFLV structure.
 *
 * This is an initial function to prepare for recursion. 
 * Function allocates memory for the root node, advances the data pointer by 12 bytes,
 * reads the count of elements, and then calls parse_sflv_recursion for each child node.
 *
 * @param data A pointer to the data. The position of the data pointer will be modified.
 * @param data_len The length of the data.
 * @return A pointer to the root node.
 */
struct bej_node* parse_sflv_init(unsigned char *data, size_t data_len)
{	
	struct bej_node *root = malloc(sizeof(struct bej_node));
	unsigned char **data_ptr = &data;
	
	advance_ptr(data_ptr,12);
	root->count = read_length_and_get_int((data_ptr));

	root->dictionary_type = 255;
	root->sequence = 255;
	root->format = ARRAY;
	root->length = 255;
	
	allocate_sflv_array(root);
	for (int i = 0; i < root->count; i++) {
		parse_sflv_recursion(root->value[i],data_ptr, data_len);
	}
	
	//free(node);
	
    return root;
}

/**
 * @brief Creates a new dynamic string.
 *
 * This function allocates memory for a dynamic string structure 
 *
 * @return A pointer to the created dynamic string structure.
 */
struct dynamic_string* create_dynamic_string(void)
{
    struct dynamic_string* str = (struct dynamic_string*) malloc(sizeof(struct dynamic_string));
    str->length = 0;
    str->capacity = INITIAL_CAPACITY;  // Initial capacity
    str->data = (char*) malloc(sizeof(char) * (str->capacity + 1));
    return str;
}

/**
 * @brief Checks and reallocates memory for the dynamic string.
 *
 * This function checks whether the current capacity of the dynamic string
 * can accommodate the additional length. If not, it reallocates memory
 * to increase the capacity.
 *
 * @param str A pointer to the dynamic string.
 * @param additional_length The additional length that needs to be accommodated.
 */
void check_realloc(struct dynamic_string* str, size_t additional_length)
{
    if (str->length + additional_length >= str->capacity) {
        str->capacity += CAPACITY_INCREASE_STEP; 
        char* new_data = (char*) realloc(str->data, sizeof(char) * (str->capacity + 1));
        if (new_data == NULL) {
            printf("Memory reallocation error!\n");
            exit(1);
        }
        str->data = new_data;
    }
}

/**
 * @brief Appends a string to the dynamic string.
 *
 * This function appends a given string to the end of the dynamic string.
 * Before appending, it checks and reallocates memory if necessary.
 *
 */
void append_string(struct dynamic_string* str, const char* append_str)
{
    size_t append_length = strlen(append_str);
    check_realloc(str, append_length);
    strcpy(str->data + str->length, append_str);
    str->length += append_length;
}

/**
 * @brief Appends a character to the dynamic string.
 *
 * This function appends a given character to the end of the dynamic string.
 *
 */
void append_char(struct dynamic_string* str, char c)
{
    check_realloc(str, 1);
    str->data[str->length] = c;
    str->length++;
    str->data[str->length] = '\0';
}

/**
 * @brief Retrieves the key from the dictionary for the given node.
 *
 * This function reads the sequence number from the node and finds the corresponding key
 * in the schema dictionary using the sequence number.
 *
 * @param node A pointer to the BEJ node.
 * @param schema_dictionary A pointer to the schema dictionary.
 * @return A string representing the key.
 */
char* get_key_from_dictionary (struct bej_node *node, unsigned char *schema_dictionary)
{
	unsigned char node_sequence = node->sequence;
	unsigned char *schema_dictionary_start_point = schema_dictionary;
    char* node_name;
	char* result;
	int entries_count = read_int_const_ptr(schema_dictionary+2, 2); // schema_dictionary offset for EntryCount = 2, EntryCount size = 2 bytes
			
	// TODO: replace cycle with offset search using node_sequence
	schema_dictionary += 13; // offset for first SequenceNumber
	for (int i = 0; i < entries_count; i++) {
		if (node_sequence == read_int_const_ptr(schema_dictionary, 2)) {
			unsigned char name_length = (char)read_int_const_ptr(schema_dictionary+6, 1);
			unsigned int name_offset = read_int_const_ptr(schema_dictionary+7, 2);

			result = read_str_const_ptr(schema_dictionary_start_point + name_offset, name_length);
			break;
		}
		schema_dictionary += 10;
	}
    return result;
}

/**
 * @brief Appends the key of a node to the dynamic string.
 *
 * This function retrieves the key for the given node from the schema dictionary 
 * and appends it to the dynamic string with formating.
 *
 * @param node A pointer to the BEJ node.
 * @param str A pointer to the dynamic string.
 * @param schema_dictionary A pointer to the schema dictionary.
 */
void add_node_key_to_str(struct bej_node *node, struct dynamic_string* str, unsigned char *schema_dictionary)
{
	append_char(str, '"');
	char* node_name = get_key_from_dictionary(node, schema_dictionary);
	append_string(str, node_name);
	append_string(str, "\" : ");
	free(node_name);
}

/**
 * @brief Adds tabs to the dynamic string.
 *
 * This function adds a specified number of tabs to the dynamic string.
 *
 */
void add_tab(struct dynamic_string* str, int count)
{
	for (int i = 0; i < count; i++) 
		append_string(str, "\t");
}

/**
 * @brief Recursively parses a BEJ node and its children to a string.
 *
 * This function recursively parse the BEJ tree and builds a dynamic string.
 * The function distinguishes between different BEJ formats (INTEGER, STRING, ARRAY) and parses them appropriately.
 * For each node, the function appends its key (if present) and value to the dynamic string.
 * In case of ARRAY type, the function recursively processes all child nodes.
 *
 * @param node A pointer to the current BEJ node.
 * @param str A pointer to the dynamic string.
 * @param parse_type The type of parsing required (with or without key).
 * @param recursion_depth The current depth of recursion.
 * @param schema_dictionary A pointer to the schema dictionary.
 */
void parse_bej_node_to_str_recursion(struct bej_node *node, struct dynamic_string* str, char parse_type, char recursion_depth, unsigned char *schema_dictionary)
{
	recursion_depth++;
	if (node->count == 0 ) {
		if (parse_type == WITH_KEY) {		
			add_node_key_to_str(node, str, schema_dictionary);
		}
		switch (node->format) {
			case INTEGER: 
			{
				char temp_str_for_int[21]; // 21 -- max size of 64 bit number 9223372036854775807 (+ \0)
				sprintf(temp_str_for_int, "%d", *(node->value)); 
				append_string(str, temp_str_for_int);
				free(((int*)node->value));
				free(node);
				break;
			}
			case STRING: 
				append_char(str, '"');
				append_string(str, (char*)node->value);
				append_char(str, '"');
				free(((int*)node->value));
				free(node);
				break;
		}

	} else {
		switch (node->format) {
			case ARRAY: 
				if (node->sequence == 255) {
					parse_type = WITH_KEY;
				} else {
					parse_type = WITHOUT_KEY;
					add_node_key_to_str(node, str, schema_dictionary);
					append_string(str, "[\n");
				}
				for (int i = 0; i < node->count; i++) {
					add_tab(str, recursion_depth);
					parse_bej_node_to_str_recursion(node->value[i], str, parse_type,recursion_depth, schema_dictionary);
					if(i < node->count-1)append_string(str, ",\n");
					if(i == node->count-1)append_string(str, "\n");
				}
				if (node->sequence != 255) {
					add_tab(str, recursion_depth-1);
					append_string(str, "]");
				}
				free(node->value);
				break;
		}
	}
}

/**
 * @brief Initiates the parsing of the BEJ tree to a string.
 *
 * This function initializes the parsing of the BEJ tree by starting from the root of the tree.
 * It adds the enclosing braces to the string and calls the recursive function to parse the nodes.
 *
 * @param root A pointer to the root of the BEJ tree.
 * @param str A pointer to the dynamic string.
 * @param schema_dictionary A pointer to the schema dictionary.
 */
void parse_bej_node_to_str(struct bej_node *root, struct dynamic_string* str, unsigned char *schema_dictionary)
{
	append_string(str, "{\n");
	parse_bej_node_to_str_recursion(root, str, WITH_KEY, 0, schema_dictionary);
	append_string(str, "}");
}

/**
 * @brief Decodes BEJ data into a string.
 *
 * This function is the key function in the file, 
 * that decodes the given BEJ data using the provided dictionaries.
 * It first parses the BEJ data into a BEJ tree, and then converts this tree into a string.
 *
 * @param data A pointer to the BEJ data.
 * @param data_len The length of the BEJ data.
 * @param schema_dictionary A pointer to the schema dictionary.
 * @param schema_dictionary_len The length of the schema dictionary.
 * @param annotation_dictionary A pointer to the annotation dictionary.
 * @param annotation_dictionary_len The length of the annotation dictionary.
 * @return A pointer to the dynamic string holding the decoded data.
 */
struct dynamic_string* decode_bej(unsigned char *data, size_t data_len, 
								unsigned char *schema_dictionary, size_t schema_dictionary_len, 
								const unsigned char *annotation_dictionary, size_t annotation_dictionary_len)
{
	struct bej_node *root;
    struct dynamic_string* str = create_dynamic_string();
	
	root = parse_sflv_init(data, data_len);
	parse_bej_node_to_str(root, str, schema_dictionary);
	
    return str;
}
