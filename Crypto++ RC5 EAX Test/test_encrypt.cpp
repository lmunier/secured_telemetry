/* g++ -g3 -ggdb -O0 -DDEBUG -I/usr/include/cryptopp test_encrypt.cpp -o test_encrypt.x
 * -lcryptopp -lpthread -std=c++11
 */
/* g++ -g -O2 -DNDEBUG -I/usr/include/cryptopp test_encrypt.cpp -o test_encrypt.x
 * -lcryptopp -lpthread -std=c++11
 */

#define ITERATION 10

#define NBR_ROUNDS 20
#define BLOCK_SIZE 64
#define KEY_LENGTH 128

//#define GENERATE_KEY
//#define VERBOSE

#include <ctime>
#include <chrono>
#include <fstream>

#include "osrng.h"
using CryptoPP::AutoSeededRandomPool;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <cstdlib>
using std::exit;

#include "cryptlib.h"
using CryptoPP::Exception;

#include "hex.h"
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

#include "filters.h"
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::AuthenticatedEncryptionFilter;
using CryptoPP::AuthenticatedDecryptionFilter;

#include "rc5.h"
using CryptoPP::RC5;

#include "eax.h"
using CryptoPP::EAX;

#include "secblock.h"
using CryptoPP::SecByteBlock;

#include "algparam.h"
#include "seckey.h"


int main(int argc, char* argv[])
{
	// Record start time
	auto start_init = std::chrono::high_resolution_clock::now();

	// Initialize encryption key
	SecByteBlock key(KEY_LENGTH);

	// Initialize a random generator
        AutoSeededRandomPool prng;

	string output;

#ifdef GENERATE_KEY // If the key has to be generated
        prng.GenerateBlock(key, key.size());

        std::ofstream file("test.txt", std::ios::out | std::ios::binary);
        StringSource(key, key.size(),  true, new HexEncoder(new StringSink(output)));

        file << output;
        file.close();
#else
	cout << "Key is not generated but recover from a text file" << endl;
        string str_text;
        std::ifstream text_input;
        text_input.open("test.txt");

        if (!text_input) {
                cerr << "Unable to open file test.txt";
                exit(1);   // call system to stop
        }

        if(text_input.is_open()){
                getline(text_input, str_text);
                StringSource(str_text, true, new HexDecoder(new StringSink(output)));
        }
#endif

	// Record encryption time
	auto start_encr = std::chrono::high_resolution_clock::now();

	for(int i=0; i<ITERATION; i++){
		SecByteBlock iv(BLOCK_SIZE);
		prng.GenerateBlock(iv, iv.size());
		CryptoPP::AlgorithmParameters parameters = CryptoPP::MakeParameters("Rounds",NBR_ROUNDS)
			("IV", CryptoPP::ConstByteArrayParameter(iv, iv.size(), false));

#ifdef VERBOSE
		cout << "key length: " << KEY_LENGTH << endl;
		cout << "key length (min): " << RC5::MIN_KEYLENGTH << endl;
		cout << "key length (max): " << RC5::MAX_KEYLENGTH << endl;
		cout << "block size: " << BLOCK_SIZE << endl;
#endif
		string plain = "Test simulated hardware";
		string cipher, encoded, recovered;


		// Clear the cipher and recovered text
		cipher.clear();
		recovered.clear();

		// Pretty print key
		encoded.clear();
		StringSource(key, key.size(), true,
			new HexEncoder(
				new StringSink(encoded)
			) // HexEncoder
		); // StringSource

#ifdef VERBOSE
		cout << "key: " << encoded << endl;
#endif
		// Pretty print iv
		encoded.clear();
		StringSource(iv, iv.size(), true,
			new HexEncoder(
				new StringSink(encoded)
			) // HexEncoder
		); // StringSource

#ifdef VERBOSE
		cout << "iv: " << encoded << endl;
#endif

		try
		{
#ifdef VERBOSE
			cout << "plain text: " << plain << endl;
			cout << "plain text size: " << plain.size() << endl;
#endif
			EAX< RC5 >::Encryption e;
			e.SetKey(key, key.size(), parameters);
			//e.SetKeyWithIV(key, key.size(), iv, iv.size());

			StringSource(plain, true, 
				new AuthenticatedEncryptionFilter(e,
					new StringSink(cipher)
				) // StreamTransformationFilter      
			); // StringSource
		}
		catch(const CryptoPP::Exception& e)
		{
			cerr << e.what() << endl;
			exit(1);
		}

		// Pretty print
		encoded.clear();
		StringSource(cipher, true,
			new HexEncoder(
				new StringSink(encoded)
			) // HexEncoder
		); // StringSource

#ifdef VERBOSE
		cout << "cipher text: " << encoded << endl;
		cout << "cipher text size: " << encoded.size() << endl;
#endif

		try
		{
			EAX< RC5 >::Decryption d;
			d.SetKey(key, key.size(), parameters);

			// The StreamTransformationFilter removes
			//  padding as required.
			StringSource s(cipher, true, 
				new AuthenticatedDecryptionFilter(d,
					new StringSink(recovered)
				) // StreamTransformationFilter
			); // StringSource

#ifdef VERBOSE
			cout << "recovered text: " << recovered << endl;
#endif
		}
		catch(const CryptoPP::Exception& e)
		{
			cerr << e.what() << endl;
			exit(1);
		}
	}

	// Record end time
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed_init = finish - start_init;
	std::chrono::duration<double> elapsed_encr = finish - start_encr;

	cout << "Elapsed with init time: " << elapsed_init.count() << " s" << endl;
	cout << "Elapsed encryption/decryption time: " << elapsed_encr.count() << " s" << endl;

	return 0;
}

