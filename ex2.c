#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "org_tree.h"

#define FP_LEN 9


static void print_success(int mask, char *op, char* fingerprint, char* First_Name, char* Second_Name)
{
    printf("Successful Decrypt! The Mask used was mask_%d of type (%s) and The fingerprint was %.*s belonging to %s %s\n",
                       mask, op, FP_LEN, fingerprint, First_Name, Second_Name);
}

static void print_unsuccess()
{
    printf("Unsuccesful decrypt, Looks like he got away\n");
}

/* Convert an 8-bit binary string line ("01010101") to a byte */
static unsigned char bin_to_byte(const char *bin_str)
{
    /* strtol with base 2 will parse the 0/1s and ignore the trailing '\n' */
    return (unsigned char)strtol(bin_str, NULL, 2);
}

/* Check if applying mask+op to plain_fp gives cipher_bytes */
static int check_match(const char *plain_fp,
                       const unsigned char *cipher_bytes,
                       int mask,
                       int op_type)   /* 0 = XOR, 1 = AND */
{
    for (int i = 0; i < FP_LEN; i++) {
        unsigned char plain_val = (unsigned char)plain_fp[i];
        unsigned char calc_val;

        if (op_type == 0) {
            /* XOR */
            calc_val = (unsigned char)(plain_val ^ (unsigned char)mask);
        } else {
            /* AND */
            calc_val = (unsigned char)(plain_val & (unsigned char)mask);
        }

        if (calc_val != cipher_bytes[i]) {
            return 0;
        }
    }
    return 1;
}

/* Search all nodes (Boss, Hands, Supports) for a matching fingerprint */
static Node *search_all(const Org *org,
                        const unsigned char *cipher_bytes,
                        int mask,
                        int op_type)
{
    if (!org) return NULL;

#define TEST_NODE(nodeptr) \
    do { \
        if ((nodeptr) && check_match((nodeptr)->fingerprint, cipher_bytes, mask, op_type)) { \
            return (Node *)(nodeptr); \
        } \
    } while (0)

    /* Boss */
    TEST_NODE(org->boss);

    /* Left Hand + its supports */
    TEST_NODE(org->left_hand);
    if (org->left_hand) {
        Node *curr = org->left_hand->supports_head;
        while (curr) {
            TEST_NODE(curr);
            curr = curr->next;
        }
    }

    /* Right Hand + its supports */
    TEST_NODE(org->right_hand);
    if (org->right_hand) {
        Node *curr = org->right_hand->supports_head;
        while (curr) {
            TEST_NODE(curr);
            curr = curr->next;
        }
    }

#undef TEST_NODE
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        printf("Usage: %s <clean_file.txt> <cipher_bits.txt> <mask_start_s>\n", argv[0]);
        return 0;
    }

    const char *clean_path  = argv[1];
    const char *cipher_path = argv[2];

    /* Build organization from clean file */
    Org org = build_org_from_clean_file(clean_path);

    /* Open cipher bits file */
    FILE *f_cipher = fopen(cipher_path, "r");
    if (!f_cipher) {
        printf("Error opening file: %s\n", cipher_path);
        free_org(&org);
        return 0;
    }

    unsigned char cipher_bytes[FP_LEN];
    char line_buf[64];
    int lines_read = 0;

    /* Read exactly 9 lines of 8-bit binary */
    while (lines_read < FP_LEN && fgets(line_buf, sizeof(line_buf), f_cipher)) {
        cipher_bytes[lines_read++] = bin_to_byte(line_buf);
    }
    fclose(f_cipher);

    if (lines_read < FP_LEN) {
        /* Not enough data – just exit gracefully */
        free_org(&org);
        return 0;
    }

    int s = atoi(argv[3]);
    int found = 0;

    /* Try all masks in [s, s+10] with XOR then AND */
    for (int mask = s; mask <= s + 10 && !found; mask++) {
        Node *res;

        /* Try XOR */
        res = search_all(&org, cipher_bytes, mask, 0);
        if (res) {
            print_success(mask, "XOR", res->fingerprint, res->first, res->second);
            found = 1;
            break;
        }

        /* Try AND */
        res = search_all(&org, cipher_bytes, mask, 1);
        if (res) {
            print_success(mask, "AND", res->fingerprint, res->first, res->second);
            found = 1;
            break;
        }
    }

    if (!found) {
        print_unsuccess();
    }

    free_org(&org);
    return 0;
}
