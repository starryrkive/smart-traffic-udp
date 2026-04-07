#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "common.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>


int main() {
    int sock;
    struct sockaddr_in server_addr;
    struct TrafficData data;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    char server_ip[20];
    printf("Enter Server IP (127.0.0.1 for same system): ");
    scanf("%s", server_ip);

    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    int id;
    printf("Enter Intersection ID: ");
    scanf("%d", &id);

    srand(time(NULL) + id);
FILE *fp = fopen("public.pem", "rb");
if (!fp) {
    perror("Public key not found");
    exit(1);
}

RSA *rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
fclose(fp);

if (!rsa) {
    printf("Failed to load public key\n");
    exit(1);
}
    while (1) {
        data.intersection_id = id;
        data.vehicle_count = rand() % 120;
        data.signal_status = rand() % 3;
unsigned char aes_key[32];
unsigned char iv[16];

RAND_bytes(aes_key, sizeof(aes_key));
RAND_bytes(iv, sizeof(iv));

EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
unsigned char encrypted_key[256];

int encrypted_key_len = RSA_public_encrypt(
    32,                // AES key size
    aes_key,
    encrypted_key,
    rsa,
    RSA_PKCS1_OAEP_PADDING
);

if (encrypted_key_len == -1) {
    printf("RSA encryption failed\n");
    exit(1);
}
unsigned char ciphertext[512];
int len, ciphertext_len;

EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv);

EVP_EncryptUpdate(ctx, ciphertext, &len,
    (unsigned char*)&data, sizeof(data));

ciphertext_len = len;

EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
ciphertext_len += len;

EVP_CIPHER_CTX_free(ctx);


struct SecurePacket packet;

memcpy(packet.encrypted_key, encrypted_key, 256);
memcpy(packet.iv, iv, AES_BLOCK_SIZE);
memcpy(packet.ciphertext, ciphertext, ciphertext_len);
packet.ciphertext_len = ciphertext_len;


        sendto(sock, &packet, sizeof(packet), 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr));

        printf("Sent -> ID:%d | Vehicles:%d | Signal:%d\n",
               data.intersection_id,
               data.vehicle_count,
               data.signal_status);

        usleep(500000); // 0.5 sec
    }

    close(sock);
    return 0;
}
