cmake_minimum_required(VERSION 3.10)
project(encoder_decoder_project)


add_executable(encoder_decoder encoder_decoder.c bej_decoder.c)


add_custom_target(run
    COMMAND encoder_decoder --operation decode --schemaDictionary ../../DSP8010_2023_1/dictionaries/Memory_v1.bin --annotationDictionary ../../DSP8010_2023_1/dictionaries/annotation.bin --bejEncodedFile ../../my_test.bin
    DEPENDS encoder_decoder
)

