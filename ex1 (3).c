#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BUFFER 100000

typedef struct EmployeeNode {
    char first[100];
    char second[100];
    char finger[50];
    char pos[50];
    struct EmployeeNode *next;
} EmployeeNode;

EmployeeNode *head = NULL;
EmployeeNode *tail = NULL;

const char *CORRUPTION = "#?!@&$";

int is_corruption(char c) {
    return strchr(CORRUPTION, c) != NULL;
}

void trim(char *str) {
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    char *end = start + strlen(start);
    if (end != start) {
        end--;
        while (end > start && isspace((unsigned char)*end)) {
            end--;
        }
        end[1] = '\0';
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int is_duplicate(const char *fp) {
    EmployeeNode *curr = head;
    while (curr) {
        if (strcmp(curr->finger, fp) == 0) {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

void add_node(const char *f, const char *s, const char *fp, const char *p) {
    EmployeeNode *node = (EmployeeNode *)malloc(sizeof(EmployeeNode));
    if (!node) {
        return;
    }

    strncpy(node->first, f, sizeof(node->first) - 1);
    node->first[sizeof(node->first) - 1] = '\0';

    strncpy(node->second, s, sizeof(node->second) - 1);
    node->second[sizeof(node->second) - 1] = '\0';

    strncpy(node->finger, fp, sizeof(node->finger) - 1);
    node->finger[sizeof(node->finger) - 1] = '\0';

    strncpy(node->pos, p, sizeof(node->pos) - 1);
    node->pos[sizeof(node->pos) - 1] = '\0';

    node->next = NULL;

    if (!head) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
}

void write_group(FILE *f, const char *role) {
    EmployeeNode *curr = head;
    while (curr) {
        if (strcmp(curr->pos, role) == 0) {
            fprintf(f, "First Name: %s\n", curr->first);
            fprintf(f, "Second Name: %s\n", curr->second);
            fprintf(f, "Fingerprint: %s\n", curr->finger);
            fprintf(f, "Position: %s\n\n", curr->pos);
        }
        curr = curr->next;
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_corrupted.txt> <output_clean.txt>\n", argv[0]);
        return 0;
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        printf("Error opening file: %s\n", argv[1]);
        return 0;
    }

    char *clean_data = (char *)malloc(MAX_BUFFER);
    if (!clean_data) {
        fclose(fin);
        return 0;
    }

    int c;
    int idx = 0;
    while ((c = fgetc(fin)) != EOF) {
        char ch = (char)c;
        /* Remove corruption chars and newlines from the stream */
        if (is_corruption(ch) || ch == '\n' || ch == '\r') {
            continue;
        }
        if (idx < MAX_BUFFER - 1) {
            clean_data[idx++] = ch;
        }
    }
    clean_data[idx] = '\0';
    fclose(fin);

    const char *L_F  = "First Name:";
    const char *L_S  = "Second Name:";
    const char *L_FP = "Fingerprint:";
    const char *L_P  = "Position:";

    char *cursor = clean_data;
    while ((cursor = strstr(cursor, L_F)) != NULL) {
        char *p_sec = strstr(cursor, L_S);
        char *p_fin = strstr(cursor, L_FP);
        char *p_pos = strstr(cursor, L_P);

        if (!p_sec || !p_fin || !p_pos) {
            break;
        }

        char tmp_f[100];
        char tmp_s[100];
        char tmp_fp[50];
        char tmp_p[50];
        size_t len;

        /* First Name */
        len = (size_t)(p_sec - (cursor + strlen(L_F)));
        if (len >= sizeof(tmp_f)) len = sizeof(tmp_f) - 1;
        strncpy(tmp_f, cursor + strlen(L_F), len);
        tmp_f[len] = '\0';
        trim(tmp_f);

        /* Second Name */
        len = (size_t)(p_fin - (p_sec + strlen(L_S)));
        if (len >= sizeof(tmp_s)) len = sizeof(tmp_s) - 1;
        strncpy(tmp_s, p_sec + strlen(L_S), len);
        tmp_s[len] = '\0';
        trim(tmp_s);

        /* Fingerprint */
        len = (size_t)(p_pos - (p_fin + strlen(L_FP)));
        if (len >= sizeof(tmp_fp)) len = sizeof(tmp_fp) - 1;
        strncpy(tmp_fp, p_fin + strlen(L_FP), len);
        tmp_fp[len] = '\0';
        trim(tmp_fp);

        /* Position */
        char *pos_val_start = p_pos + strlen(L_P);
        char *next_entry = strstr(pos_val_start, L_F);
        if (next_entry) {
            len = (size_t)(next_entry - pos_val_start);
        } else {
            len = strlen(pos_val_start);
        }
        if (len >= sizeof(tmp_p)) len = sizeof(tmp_p) - 1;
        strncpy(tmp_p, pos_val_start, len);
        tmp_p[len] = '\0';
        trim(tmp_p);

        if (!is_duplicate(tmp_fp)) {
            add_node(tmp_f, tmp_s, tmp_fp, tmp_p);
        }

        if (next_entry) {
            cursor = next_entry;
        } else {
            break;
        }
    }

    free(clean_data);

    FILE *fout = fopen(argv[2], "w");
    if (!fout) {
        printf("Error opening file: %s\n", argv[2]);
        EmployeeNode *curr = head;
        while (curr) {
            EmployeeNode *t = curr;
            curr = curr->next;
            free(t);
        }
        return 0;
    }

    /* Order required by the assignment */
    write_group(fout, "Boss");
    write_group(fout, "Right Hand");
    write_group(fout, "Left Hand");
    write_group(fout, "Support_Right");
    write_group(fout, "Support_Left");

    fclose(fout);

    EmployeeNode *curr = head;
    while (curr) {
        EmployeeNode *t = curr;
        curr = curr->next;
        free(t);
    }

    return 0;
}
