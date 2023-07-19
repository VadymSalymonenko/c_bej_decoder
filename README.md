# c_bej_decoder
Aplication that implements decoding bej to json.

Features

1. Decoding: This project primarily focuses on decoding BEJ files into JSON format. It's capable of translating the compact BEJ format into a human-readable JSON file, supporting all data types including 'integer', 'string', 'array', and others. This comprehensive decoding solution covers all possible JSON data types, making it a robust tool for BEJ decoding.

2. Error Handling: The decoder is designed to handle different error situations gracefully and provide useful error messages that aid in debugging and troubleshooting.

3. Testing: The project includes several unit tests for various functions, which ensure the reliability and correctness of the decoder.

Getting Started

The project uses CMake as a build system. After cloning the repo, navigate to the project directory and build the project using the following commands:

mkdir build
cd build
cmake ..
make
After successful compilation, the program can be run using the command "make run".
