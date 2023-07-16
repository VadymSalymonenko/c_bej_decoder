#include "bej_decoder.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void advance_ptr(unsigned char** data, int bytes_to_advance)
{
    *data += bytes_to_advance;
}

int read_int_const_ptr(unsigned char *data, int length)
{
	int value = 0;
	for (int i = length-1; i >= 0; i--) {
        value <<= 8; 
        value += *(data+i); 
    }
	return value;
}

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

int read_length_and_get_int(unsigned char **data)
{
	unsigned char length = **data;
	advance_ptr(data, 1);
	return read_int(data, length);
}

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

void free_sflv_array(struct bej_node* root)
{
    for (unsigned int i = 0; i < root->count; ++i) {
        free(root->value[i]);
    }
    free(root->value);
}


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
			*int_ptr = read_int(data,parent->length); // TODO: зробить функцію що просто записує декілька байт в прямій послідовності без \0 термінатора
			parent->value = (void**)int_ptr;
			break;
		}
	}
	// TODO: тепер в залежності від типу даних робити різні дії. по різному викликати або не викликати цю функцію. для примітивів просто записувати, для масиву циклом викликати для кожного елементу і тд.
	
    return NULL;
}

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
	
	// TODO: тепер в залежності від типу даних робити різні дії. по різному викликати або не викликати цю функцію. для примітивів просто записувати, для масиву циклом викликати для кожного елементу і тд.
	//free(node);
	
    return root;
}

struct dynamic_string* create_dynamic_string(void)
{
    struct dynamic_string* str = (struct dynamic_string*) malloc(sizeof(struct dynamic_string));
    str->length = 0;
    str->capacity = INITIAL_CAPACITY;  // Initial capacity
    str->data = (char*) malloc(sizeof(char) * (str->capacity + 1));
    return str;
}

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

void append_string(struct dynamic_string* str, const char* append_str)
{
    size_t append_length = strlen(append_str);
    check_realloc(str, append_length);
    strcpy(str->data + str->length, append_str);
    str->length += append_length;
}

void append_char(struct dynamic_string* str, char c)
{
    check_realloc(str, 1);
    str->data[str->length] = c;
    str->length++;
    str->data[str->length] = '\0';
}

char* get_key_from_dictionary (struct bej_node *node, unsigned char *schema_dictionary)
{
	unsigned char node_sequence = node->sequence;
	unsigned char *schema_dictionary_start_point = schema_dictionary;
    char* node_name;
	char* result;
	int entries_count = read_int_const_ptr(schema_dictionary+2, 2); // schema_dictionary offset for EntryCount = 2, EntryCount size = 2 bytes
			
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

void add_node_key_to_str(struct bej_node *node, struct dynamic_string* str, unsigned char *schema_dictionary)
{
	append_char(str, '"');
	char* node_name = get_key_from_dictionary(node, schema_dictionary);
	append_string(str, node_name);
	append_string(str, "\" : ");
	free(node_name);
}

void add_tab(struct dynamic_string* str, int count)
{
	for (int i = 0; i < count; i++) 
		append_string(str, "\t");
}

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

void parse_bej_node_to_str(struct bej_node *root, struct dynamic_string* str, unsigned char *schema_dictionary)
{
	append_string(str, "{\n");
	parse_bej_node_to_str_recursion(root, str, WITH_KEY, 0, schema_dictionary);
	append_string(str, "}");
}

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
