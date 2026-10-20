# include "Converter_sha256.h"

/**
 * Converts the provided string into a digested SHA-256 string
 * @param digest Pointer to memory holder of digested string
 * @param string to convert
 * @return 0 on success, -1 on failure
 */
int digestContent (char* digest, char* string, size_t size) {
    if (string == NULL || size < 1) return -1;
    
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, string, size);

    unsigned char hash[32];
    SHA256_Final(hash, &ctx);
    for(int i = 0; i < 32; i++) sprintf(digest + (i * 2), "%02x", hash[i]);
    
    return 0;
}