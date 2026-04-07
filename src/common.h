#ifndef COMMON_H
#define COMMON_H

#define PORT 8080
#define MAX_INTERSECTIONS 100
#define CONGESTION_THRESHOLD 70
#define AES_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

struct SecurePacket {
    unsigned char encrypted_key[256]; // RSA encrypted AES key
    unsigned char iv[AES_BLOCK_SIZE];
    unsigned char ciphertext[512];
    int ciphertext_len;
};

// Signal states
#define RED 0
#define YELLOW 1
#define GREEN 2

struct TrafficData {
    int intersection_id;
    int vehicle_count;
    int signal_status;
};

#endif
