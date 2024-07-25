#include <stdio.h>
#include <openssl/evp.h>
#include <string.h>

void printHex(const unsigned char *buffer, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        printf("%02x", buffer[i]);
    printf("\n");
}

void handleErrors(void)
{
    fprintf(stderr, "Error occurred.\n");
    exit(EXIT_FAILURE);
}

void calculateHash(const char* message)
{
    // OpenSSL_add_all_digests();
    const EVP_MD *md = EVP_get_digestbyname("md5");

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
    {
        handleErrors();
    }
    if (1 != EVP_DigestInit_ex(mdctx, md, NULL))
    {
        handleErrors();
    }
    if (1 != EVP_DigestUpdate(mdctx, message, strlen(message)))
    {
        handleErrors();
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;

    if (1 != EVP_DigestFinal(mdctx, hash, &hashLen)) //EVP_DigestFinal_ex(); #TODO
    {
        handleErrors();
    }
    printHex(hash, hashLen);
    EVP_MD_CTX_free(mdctx);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* filename = argv[1];
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Unable to open file");
        return EXIT_FAILURE;
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory to hold the file contents
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        perror("Unable to allocate memory");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Read the file contents into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return EXIT_FAILURE;
    }

    // Null-terminate the buffer to ensure it is a valid C string
    buffer[fileSize] = '\0';

    // Close the file as we no longer need it
    fclose(file);

    // Calculate and print the hash
    calculateHash(buffer);

    // Free the allocated memory
    free(buffer);

    return EXIT_SUCCESS;
}