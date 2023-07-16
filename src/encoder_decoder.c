#include "bej_decoder.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

struct arguments {
    bool verbose;
    bool silent;
    char *operation;
    char *schema_dictionary;
    char *annotation_dictionary;
    char *json_file;
    char *bej_output_file;
    char *pdr_map_file_encode;
    char *bej_encoded_file;
    char *pdr_map_file_decode;
};

struct arguments parse_arguments(int argc, char *argv[])
{
    struct arguments args = {false, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

    // If no arguments were provided, print usage information and exit
    if (argc == 1) {
        fprintf(stderr, "No arguments were provided. Usage: ...\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0) {
            args.verbose = true;
        } else if (strcmp(argv[i], "--silent") == 0) {
            args.silent = true;
        } else if (strcmp(argv[i], "--operation") == 0) {
            args.operation = argv[++i];

            if (strcmp(args.operation, "encode") == 0) {
                fprintf(stderr, "This program cannot encode files.\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "--schemaDictionary") == 0) {
            args.schema_dictionary = argv[++i];
        } else if (strcmp(argv[i], "--annotationDictionary") == 0) {
            args.annotation_dictionary = argv[++i];
        } else if (strcmp(argv[i], "--jsonFile") == 0) {
            args.json_file = argv[++i];
        } else if (strcmp(argv[i], "--bejOutputFile") == 0) {
            args.bej_output_file = argv[++i];
        } else if (strcmp(argv[i], "--pdrMapFileEncode") == 0) {
            args.pdr_map_file_encode = argv[++i];
        } else if (strcmp(argv[i], "--bejEncodedFile") == 0) {
            args.bej_encoded_file = argv[++i];
        } else if (strcmp(argv[i], "--pdrMapFileDecode") == 0) {
            args.pdr_map_file_decode = argv[++i];
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            exit(1);
        }
    }

    // If both verbose and silent were set, override silent
    if (args.verbose && args.silent) {
        fprintf(stderr, "Both verbose and silent were set. Unsetting silent.\n");
        args.silent = false;
    }

    return args;
}


size_t read_file(const char *file_path, unsigned char **buffer)
{
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", file_path);
        exit(1);
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    // Allocate memory for the buffer
    *buffer = malloc(file_size);
    if (*buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for reading file: %s\n", file_path);
        exit(1);
    }

    // Read the file into the buffer
    size_t read_size = fread(*buffer, 1, file_size, file);
    if (read_size != file_size) {
        fprintf(stderr, "Failed to read file: %s\n", file_path);
        exit(1);
    }

    fclose(file);
    return file_size;
}

char* append_to_filename(const char* filename, const char* suffix)
{
    size_t filename_len = strlen(filename);

    if (filename_len < 4 || strcmp(filename + filename_len - 4, ".bin") != 0) {
        return NULL;
    }

    size_t suffix_len = strlen(suffix);

    char* new_filename = (char*) malloc(filename_len - 3 + suffix_len + 1); // "-3" to remove ".bin" and add "_decoded.json"
    if (new_filename == NULL) {
        return NULL;
    }

    strncpy(new_filename, filename, filename_len - 4);
    strcpy(new_filename + filename_len - 4, suffix);

    return new_filename;
}

void write_to_file(struct dynamic_string* str, const char* filename)
{
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("File cannot be opened!%s\n", filename);
        return;
    }
    fwrite(str->data, sizeof(char), str->length, file);
    fclose(file);
}

int main(int argc, char *argv[])
{
    struct arguments args = parse_arguments(argc, argv);

    // Read binary files
    unsigned char *schema_dictionary;
    size_t schema_dictionary_len = read_file(args.schema_dictionary, &schema_dictionary);

    unsigned char *annotation_dictionary;
    size_t annotation_dictionary_len = read_file(args.annotation_dictionary, &annotation_dictionary);

    unsigned char *bej_encoded;
    size_t bej_encoded_len;
    if (args.bej_encoded_file) {
        bej_encoded_len = read_file(args.bej_encoded_file, &bej_encoded);
    } else {
        printf("BEJ encoded file was not provided.\n");
        exit(1);
    }
	struct dynamic_string* result;
	result = decode_bej(bej_encoded, bej_encoded_len, schema_dictionary, schema_dictionary_len, annotation_dictionary, annotation_dictionary_len);
    
	printf("Decoded message: \n%s", result->data);
	
	char* new_filename = append_to_filename(args.bej_encoded_file, "_decoded.json");
    if (new_filename != NULL) {
        write_to_file(result, new_filename);
        free(new_filename);
    }
	free(result->data);
	free(result);

    // Free memory
    free(schema_dictionary);
    free(annotation_dictionary);
    free(bej_encoded);
}
