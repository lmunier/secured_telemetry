#include <stdio.h> 
#include <errno.h> 
#include <unistd.h> 
#include <string.h> 
#include <time.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "utils.h"
#include "encrypt_rc5.h"

static const char MY_NAME[] = "Encryptor";

int initialize_semaphore(sem_t **semaphore, char* name){
    char s[1024];
    *semaphore = sem_open(name, 0);
    
    if (*semaphore == SEM_FAILED) {
        *semaphore = NULL;
        sprintf(s, "Getting a handle to the semaphore failed; errno is %d", errno);
        say(MY_NAME, s);

        return -1;
    } else {
        return 0;
    }
}

int initialize_shared_memory(param_struct params, void **sharedMemory, int *file_shm){
    char s[1024];

    // get a handle to the shared memory
    *file_shm = shm_open(params.shared_memory_name, O_RDWR, params.permissions);
    
    if (*file_shm == -1) {
        sprintf(s, "Couldn't get a handle to the shared memory; errno is %d", errno);
        say(MY_NAME, s);

        return -1;
    } else {
        // mmap it.
        *sharedMemory = mmap((void *)0, (size_t)params.size, PROT_READ | PROT_WRITE, MAP_SHARED, *file_shm, 0);
        
        if (*sharedMemory == MAP_FAILED) {
            sprintf(s, "MMapping the shared memory failed; errno is %d", errno);
            say(MY_NAME, s);

            return -1;
        } else {
            return 0;
        }
    }
}

void send_encrypt_message(char *message){
    int block_size = 16;
    AutoSeededRandomPool prng;
    string key = read_key();

    SecByteBlock iv(block_size);
	prng.GenerateBlock(iv, iv.size());

    string encoded, cipher;
    string plain = message;

    try {
		cout << "plain text: " << message << endl;
        cout << "plain text size: " << plain.size() << endl;

        EAX< RC5 >::Encryption e;
		e.SetKeyWithIV((const byte*) key.data(), key.size(), iv.data());

		// The StreamTransformationFilter adds padding
		// as required. GCM and CBC Mode must be padded
		// to the block size of the cipher.
		StringSource(message, true, 
			new AuthenticatedEncryptionFilter(e,
				new StringSink(cipher)
			) // StreamTransformationFilter
		); // StringSource
	} catch(const CryptoPP::Exception& e) {
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

    cout << "cipher text: " << encoded << endl;
    cout << "cipher text size: " << encoded.size() << endl;
}

int main() {
    sem_t *the_semaphore = NULL;
    int rc;
    int fd;
    char s[1024];
    int i;
    int done;
    void *pSharedMemory = NULL;
    char last_message_i_read[256];
    struct param_struct params;
    
    say(MY_NAME, "Hello, I'm Encryptor !");

    read_params(&params);
    generate_key();

    // Dronekit has already created the semaphore and shared memory. 
    // I just need to get handles to them.
    if (!initialize_semaphore(&the_semaphore, params.semaphore_name)) {
        // get a handle to the shared memory
        if(!initialize_shared_memory(params, &pSharedMemory, &fd)) {
            sprintf(s, "pSharedMemory = %p", pSharedMemory);
            say(MY_NAME, s);

            i = 0;
            done = 0;
            last_message_i_read[0] = '\0';
            while (!done) {
                sprintf(s, "iteration %d", i);
                say(MY_NAME, s);

                // Wait for Dronekit to free up the semaphore.
                rc = acquire_semaphore(MY_NAME, the_semaphore);

                if (rc)
                    done = 1;
                else {
                    while ((!rc) && (!strcmp((char *)pSharedMemory, last_message_i_read))) {
                        // Nothing new; give Dronekit another chance to respond.
                        sprintf(s, "Read %lu characters '%s'", strlen((char *)pSharedMemory), (char *)pSharedMemory);
                        say(MY_NAME, s);

                        rc = release_semaphore(MY_NAME, the_semaphore);

                        if (!rc) {
                            rc = acquire_semaphore(MY_NAME, the_semaphore);
                        }
                    }

                    // I always accept the first message (when i == 0)
                    if ((i == 0) || (strcmp("", (char *)pSharedMemory))) {
                        // All is well
                        i++;
                
                        if (i == params.iterations)
                            done = 1;

                        // Write back and store to confirm reading.
                        strcpy(last_message_i_read, (char *)pSharedMemory);
                        send_encrypt_message(last_message_i_read);
                        strcpy((char *)pSharedMemory, "");

                        sprintf(s, "Reading %lu characters '%s'", strlen(last_message_i_read), last_message_i_read);
                        say(MY_NAME, s);
                    }              
                }

                // Release the semaphore.
                rc = release_semaphore(MY_NAME, the_semaphore);
                if (rc)
                    done = 1;
            }

            // Un-mmap the memory
            rc = munmap(pSharedMemory, (size_t)params.size);
            if (rc) {
                sprintf(s, "Unmapping the memory failed; errno is %d", errno);
                say(MY_NAME, s);
            }
            
            // Close the shared memory segment's file descriptor            
            if (-1 == close(fd)) {
                sprintf(s, "Closing memory's file descriptor failed; errno is %d", errno);
                say(MY_NAME, s);
            }
        }

        rc = sem_close(the_semaphore);
        if (rc) {
            sprintf(s, "Closing the semaphore failed; errno is %d", errno);
            say(MY_NAME, s);
        }
    }

    return 0; 
}
