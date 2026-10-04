# include "Converter_sha256.h"

/**
 * Converts the provided string into a digested SHA-256 string
 * @param Pointer to memory in caller, will hold digested string
 * @param string char* to convert
 * @return Result of the procedure (0=ok, 1=error)
 */
int digestContent (char* digest, char* string) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    SHA256_Update(&ctx, string, strlen(string));

    unsigned char hash[32];
    SHA256_Final(hash, &ctx);

    for(int i = 0; i < 32; i++) sprintf(digest + (i * 2), "%02x", hash[i]);
    
    return 0;
}