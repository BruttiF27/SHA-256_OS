#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

#include "Utils.h"

int main (int argc, char *argv[]) {

    struct MessageHeader m;

    m.mtype = 1;

    printf("I am the server - %d\n", m.mtype);



    if (argc < 2) {
        printf("Usage: %s <string with no spaces>\n", argv[0]);
        exit(1);
    }

    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    SHA256_Update(&ctx, argv[1], strlen(argv[1]));

    unsigned char hash[32];
    SHA256_Final(hash, &ctx);

    char char_hash[65];
    for(int i = 0; i < 32; i++) sprintf(char_hash + (i * 2), "%02x", hash[i]);

    printf("%s\n", char_hash);

    return 0;
}