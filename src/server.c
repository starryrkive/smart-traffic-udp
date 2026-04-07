#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "common.h"

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>

struct TrafficData latest_data[MAX_INTERSECTIONS];

char* get_signal_text(int status) {
    if (status == RED) return "RED";
    else if (status == YELLOW) return "YELLOW";
    else return "GREEN";
}

void print_traffic_status(int vehicles) {
    if (vehicles > 80) {
        printf("Traffic Status  : HEAVY 🚨\n");
    } else if (vehicles > 40) {
        printf("Traffic Status  : MODERATE ⚠️\n");
    } else {
        printf("Traffic Status  : NORMAL ✅\n");
    }
}

void check_congestion(struct TrafficData data) {
    if (data.vehicle_count > CONGESTION_THRESHOLD &&
        data.signal_status == RED) {

        printf("💡 Suggestion   : Increase GREEN signal duration\n");
    }
}

/* Generate keys only if not present */
void generate_rsa_keypair_if_needed() {
    FILE *check = fopen("private.pem", "rb");
    if (check) {
        fclose(check);
        return;
    }

    RSA *rsa = RSA_new();
    BIGNUM *e = BN_new();
    BN_set_word(e, RSA_F4);

    RSA_generate_key_ex(rsa, 2048, e, NULL);

    FILE *fp = fopen("private.pem", "wb");
    PEM_write_RSAPrivateKey(fp, rsa, NULL, NULL, 0, NULL, NULL);
    fclose(fp);

    fp = fopen("public.pem", "wb");
    PEM_write_RSA_PUBKEY(fp, rsa);
    fclose(fp);

    RSA_free(rsa);
    BN_free(e);
}

int main() {
    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    struct SecurePacket packet;
    struct TrafficData data;

    char client_ip[INET_ADDRSTRLEN];
    int packet_count = 0;

    /* Generate keys once */
    generate_rsa_keypair_if_needed();

    /* Load private key */
    FILE *fp = fopen("private.pem", "rb");
    if (!fp) {
        perror("Private key not found");
        exit(1);
    }

    RSA *rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
    fclose(fp);

    if (!rsa) {
        printf("Failed to load private key\n");
        exit(1);
    }

    /* Create socket */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    printf("🚦 UDP Traffic Server started on port %d...\n\n", PORT);

    while (1) {

        /* 1️⃣ Receive encrypted packet */
        recvfrom(sock, &packet, sizeof(packet), 0,
                 (struct sockaddr*)&client_addr, &addr_len);

        /* 2️⃣ Decrypt AES key using RSA */
        unsigned char aes_key[32];

        int decrypted_len = RSA_private_decrypt(
            256,
            packet.encrypted_key,
            aes_key,
            rsa,
            RSA_PKCS1_OAEP_PADDING
        );

        if (decrypted_len == -1) {
            printf("RSA decryption failed\n");
            continue;
        }

        /* 3️⃣ Decrypt actual data using AES */
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

        int out_len;
        int plaintext_len;

        EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, packet.iv);

        EVP_DecryptUpdate(ctx,
            (unsigned char*)&data, &out_len,
            packet.ciphertext, packet.ciphertext_len);

        plaintext_len = out_len;

        EVP_DecryptFinal_ex(ctx,
            ((unsigned char*)&data) + out_len, &out_len);

        plaintext_len += out_len;

        EVP_CIPHER_CTX_free(ctx);

        /* 4️⃣ Process data normally */

        inet_ntop(AF_INET, &client_addr.sin_addr,
                  client_ip, INET_ADDRSTRLEN);

        latest_data[data.intersection_id] = data;

        printf("-------- Packet #%d --------\n", ++packet_count);
        printf("Source IP       : %s\n", client_ip);
        printf("Intersection ID : %d\n", data.intersection_id);
        printf("Vehicle Count   : %d\n", data.vehicle_count);
        printf("Signal Status   : %s\n",
               get_signal_text(data.signal_status));

        print_traffic_status(data.vehicle_count);
        check_congestion(data);

        printf("\n");
    }

    close(sock);
    RSA_free(rsa);
    return 0;
}
