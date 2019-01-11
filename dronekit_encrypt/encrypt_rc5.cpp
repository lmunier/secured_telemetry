#include "encrypt_rc5.h"

/*
 * Generate key of the encryption algorithm then store them into a file
 */
void generate_key(){
    string str_key;
    SecByteBlock key(RC5::DEFAULT_KEYLENGTH);
    AutoSeededRandomPool prng;
    prng.GenerateBlock(key, key.size());

    std::ofstream file("test.txt", std::ios::out);

    if(file.is_open()){
        StringSource(key, key.size(), true,
            new HexEncoder(
                new StringSink(str_key)
            ) // HexEncoder
        ); // StringSource

        file << str_key;
        file.close();
    }
}

/*
 * Read file where the key is stored
 */
string read_key(){
    string str_key, output;
    std::ifstream text_input;
    text_input.open("test.txt");

    if (!text_input) {
        cerr << "Unable to open file test.txt";
        exit(1);   // call system to stop
    }

    if(text_input.is_open()){
        getline(text_input, str_key);

        StringSource(str_key, true, 
            new HexDecoder(
                new StringSink(output)
            ) // HexDecoder
        ); // StringSource
    }

    cout << output << endl;

    return output;
}