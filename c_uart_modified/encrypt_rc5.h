#ifndef ENCRYPT_RC5_H_
#define ENCRYPT_RC5_H_

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

void generate_key();
string read_key();

#endif // ENCRYPT_RC5_H_