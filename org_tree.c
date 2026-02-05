#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "org_tree.h"

/* Helper: print one node in the required format */
static void print_node_format(Node* n) {
    if (!n) return;
    printf("First Name: %s\n", n->first);
    printf("Second Name: %s\n", n->second);
    printf("Fingerprint: %s\n", n->fingerprint);
    printf("Position: %s\n\n", n->position);
}

/* Helper: allocate and initialize a node */
static Node* create_node(const char* f, const char* s, const char* fp, const char* pos) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) {
        return NULL;
    }

    /* Copy strings safely */
    strncpy(n->first, f, MAX_FIELD - 1);
    n->first[MAX_FIELD - 1] = '\0';

    strncpy(n->second, s, MAX_FIELD - 1);
    n->second[MAX_FIELD - 1] = '\0';

    strncpy(n->fingerprint, fp, MAX_FIELD - 1);
    n->fingerprint[MAX_FIELD - 1] = '\0';

    strncpy(n->position, pos, MAX_POS - 1);
    n->position[MAX_POS - 1] = '\0';

    /* Initialize pointers */
    n->left = NULL;
    n->right = NULL;
    n->supports_head = NULL;
    n->next = NULL;

    return n;
}

/* Helper: extract value after a label in a single line */
static void extract_value(char* dest, size_t dest_size, const char* line, const char* label) {
    const char* p = strstr(line, label);
    if (!p) {
        if (dest_size > 0) dest[0] = '\0';
        return;
    }
    p += strlen(label);

    /* Skip leading spaces after label */
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }

    /* Copy up to end of line, trimming trailing whitespace/newline */
    size_t len = strlen(p);
    while (len > 0 &&
           (p[len - 1] == '\n' || p[len - 1] == '\r' ||
            isspace((unsigned char)p[len - 1]))) {
        len--;
    }

    if (len >= dest_size) {
        len = dest_size - 1;
    }

    memcpy(dest, p, len);
    dest[len] = '\0';
}

/* Helper: append a node to a simple singly-linked list (for supports) */
static void append_to_list(Node** head, Node* new_node) {
    if (!new_node) return;
    new_node->next = NULL;

    if (!*head) {
        *head = new_node;
        return;
    }

    Node* curr = *head;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = new_node;
}

Org build_org_from_clean_file(const char *path) {
    Org org;
    org.boss = NULL;
    org.left_hand = NULL;
    org.right_hand = NULL;

    FILE* f = fopen(path, "r");
    if (!f) {
        printf("Error opening file: %s\n", path);
        return org;
    }

    char line[256];
    char f_name[MAX_FIELD];
    char s_name[MAX_FIELD];
    char fp[MAX_FIELD];
    char pos[MAX_POS];

    while (fgets(line, sizeof(line), f)) {
        /* Skip blank lines between records */
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') {
            continue;
        }

        /* 1) First Name line */
        extract_value(f_name, sizeof(f_name), line, "First Name:");

        /* 2) Second Name line */
        if (!fgets(line, sizeof(line), f)) break;
        extract_value(s_name, sizeof(s_name), line, "Second Name:");

        /* 3) Fingerprint line */
        if (!fgets(line, sizeof(line), f)) break;
        extract_value(fp, sizeof(fp), line, "Fingerprint:");

        /* 4) Position line */
        if (!fgets(line, sizeof(line), f)) break;
        extract_value(pos, sizeof(pos), line, "Position:");

        Node* new_node = create_node(f_name, s_name, fp, pos);
        if (!new_node) {
            /* Allocation failure, stop building */
            break;
        }

        /* Attach according to position */
        if (strcmp(pos, "Boss") == 0) {
            org.boss = new_node;
        } else if (strcmp(pos, "Left Hand") == 0) {
            org.left_hand = new_node;
            if (org.boss) {
                org.boss->left = new_node;
            }
        } else if (strcmp(pos, "Right Hand") == 0) {
            org.right_hand = new_node;
            if (org.boss) {
                org.boss->right = new_node;
            }
        } else if (strcmp(pos, "Support_Left") == 0) {
            if (org.left_hand) {
                append_to_list(&org.left_hand->supports_head, new_node);
            }
        } else if (strcmp(pos, "Support_Right") == 0) {
            if (org.right_hand) {
                append_to_list(&org.right_hand->supports_head, new_node);
            }
        } else {
            /* Unknown position; ignoring */
        }
    }

    fclose(f);
    return org;
}

void print_tree_order(const Org *org) {
    if (!org || !org->boss) {
        return;
    }

    /* 1. Boss */
    print_node_format(org->boss);

    /* 2. Left Hand + its supports */
    if (org->left_hand) {
        print_node_format(org->left_hand);
        Node* curr = org->left_hand->supports_head;
        while (curr) {
            print_node_format(curr);
            curr = curr->next;
        }
    }

    /* 3. Right Hand + its supports */
    if (org->right_hand) {
        print_node_format(org->right_hand);
        Node* curr = org->right_hand->supports_head;
        while (curr) {
            print_node_format(curr);
            curr = curr->next;
        }
    }
}

/* Helper: free a supports linked list */
static void free_list(Node* head) {
    while (head) {
        Node* tmp = head;
        head = head->next;
        free(tmp);
    }
}

void free_org(Org *org) {
    if (!org) return;

    if (org->left_hand) {
        free_list(org->left_hand->supports_head);
        free(org->left_hand);
        org->left_hand = NULL;
    }
    if (org->right_hand) {
        free_list(org->right_hand->supports_head);
        free(org->right_hand);
        org->right_hand = NULL;
    }
    if (org->boss) {
        free(org->boss);
        org->boss = NULL;
    }
}
