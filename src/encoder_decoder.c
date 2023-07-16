#include "bej_decoder.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Represents the command line arguments.
 *
 * The arguments structure holds all command line arguments which can be provided by the user.
 */
struct arguments {
    bool verbose;                   ///< Verbose flag for extra output.
    bool silent;                    ///< Silent flag for minimal output.
    char *operation;                ///< Operation mode, such as "encode".
    char *schema_dictionary;        ///< Path to the schema dictionary file.
    char *annotation_dictionary;    ///< Path to the annotation dictionary file.
    char *json_file;                ///< Path to the JSON file.
    char *bej_output_file;          ///< Path to the BEJ output file.
    char *pdr_map_file_encode;      ///< Path to the PDR map file for encoding.
    char *bej_encoded_file;         ///< Path to the BEJ encoded file.
    char *pdr_map_file_decode;      ///< Path to the PDR map file for decoding.
};

/**
 * @brief Parses the command line arguments.
 *
 * This function goes through the given command line arguments and extracts them into
 * a structured form for easier use. If the provided arguments are invalid, the function
 * will print an error message and exit.
 *
 * @return The parsed arguments.
 */
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


/**
 * @brief Reads a file into a buffer.
 *
 * This function reads a file and places its content into a buffer. It handles file
 * opening, size determination, memory allocation for the buffer, and file reading. 
 * If there is any error during these operations, it will print an error message and exit.
 *
 * @param file_path The path to the file.
 * @param buffer The pointer to the buffer to fill.
 * @return The size of the file.
 */
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

/**
 * @brief Appends a suffix to the filename.
 *
 * This function appends a given suffix to the filename by replacing the ".bin" extension.
 * If the filename does not have a ".bin" extension, it returns NULL.
 *
 * @param filename The original filename.
 * @param suffix The suffix to be appended.
 * @return The new filename with the appended suffix. NULL if the filename does not have a ".bin" extension.
 */
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

/**
 * @brief Writes the content of a dynamic string to a file.
 *
 * This function writes the content of a dynamic string to a file with the given filename.
 * If the file cannot be opened, an error message will be printed.
 *
 * @param str The dynamic string to be written to a file.
 * @param filename The name of the file.
 */
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

/**
 * @brief Main function.
 *
 * This function is the entry point of the program. 
 * It contains the main algorithm of the program: parses the command line arguments,
 * reads binary files, performs BEJ decoding, and writes the decoded message to a file.
 *
 * @param argc The argument count.
 * @param argv The argument array.
 * @return The exit status of the program.
 */
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
