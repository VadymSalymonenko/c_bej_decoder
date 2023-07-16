all: encoder_decoder run

encoder_decoder: encoder_decoder.c bej_decoder.c
	gcc encoder_decoder.c bej_decoder.c -o encoder_decoder

run: encoder_decoder
	./encoder_decoder --operation decode --schemaDictionary ../DSP8010_2023_1/dictionaries/Memory_v1.bin --annotationDictionary ../DSP8010_2023_1/dictionaries/annotation.bin --bejEncodedFile ../my_test.bin

clean:
	rm encoder_decoder
