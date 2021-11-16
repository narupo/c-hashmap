#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

enum {
    HASH_MAP__NTABLE = 2,
};

typedef struct HashMapNode {
    char key[1024];
    void *data;
    struct HashMapNode *next;
} HashMapNode;

typedef struct {
    HashMapNode *table[HASH_MAP__NTABLE];
    void (*deleter)(void *);
} HashMap;

void
HashMapNode_Del(HashMapNode *self) {
    if (!self) {
        return;
    }

    free(self);
}

HashMapNode *
HashMapNode_New(
    const char *key,
    void *data,
    HashMapNode *next
) {
    HashMapNode *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    snprintf(self->key, sizeof self->key, "%s", key);
    self->data = data;
    self->next = next;

    return self;
}

HashMapNode *
HashMapNode_FindTail(HashMapNode *self) {
    for (HashMapNode *cur = self; cur; cur = cur->next) {
        if (!cur->next) {
            return cur;
        }
    }
    return self;
}

static inline void
HashMap_SetDeleter(HashMap *self, void (*deleter)(void *)) {
    self->deleter = deleter;
}

void
HashMap_Del(HashMap *self) {
    for (int32_t i = 0; i < HASH_MAP__NTABLE; i += 1) {
        HashMapNode *node = self->table[i];
        if (node) {
            self->deleter(node->data);
            HashMapNode_Del(node);
        }
    }
}

HashMap *
HashMap_New(void) {
    HashMap *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }    

    return self;
}

static long
create_hash(const char *s) {
    long n = 0;
    long weight = 1;

    for (const char *p = s; *p; p += 1) {
        n += weight * (*p);
        weight += 1;
    }

    return n;
}

HashMap *
HashMap_Set(
    HashMap *self,
    const char *key,
    void *data
) {
    long hash = create_hash(key);
    hash %= HASH_MAP__NTABLE;

    HashMapNode *node = self->table[hash];
    if (!node) {
        node = HashMapNode_New(key, data, NULL);
        if (!node) {
            goto error;
        }
        self->table[hash] = node;
        return self;
    }

    for (HashMapNode *cur = node; cur; cur = cur->next) {
        if (strcmp(cur->key, key) == 0) {
            // found node. overwrite data
            if (self->deleter) {
                self->deleter(cur->data);
            }
            cur->data = data;
            break;
        } else if (!cur->next) {
            // found tail
            cur->next = HashMapNode_New(key, data, NULL);
            if (!cur->next) {
                goto error;
            }
            break;
        }
    }

    return self;
error:
    return NULL;
}

void 
HashMap_Dump(const HashMap *self) {
    for (int32_t i = 0; i < HASH_MAP__NTABLE; i += 1) {
        HashMapNode *node = self->table[i];
        if (!node) {
            continue;
        }
        printf("%d: %s: %d\n",
            i,
            node->key,
            * (int32_t *) node->data
            );

        int32_t j = 1;
        for (HashMapNode *cur = node->next; cur; cur = cur->next) {
            for (int32_t k = 0; k < j; k += 1) {
                printf("  ");                
            }
            j += 1;
            printf("%d: %s: %d\n",
                i,
                cur->key,
                * (int32_t *) cur->data
                );
        }
    }
}

int
main(int argc, char *argv[]) {
    HashMap *hashmap = HashMap_New();
    if (!hashmap) {
        return 1;
    }

    for (;;) {
        char line[1024];

        printf("key > ");
        if (!fgets(line, sizeof line, stdin)) {
            break;
        }
        line[strlen(line) - 1] = '\0';
        char key[1024];
        snprintf(key, sizeof key, "%s", line);

        printf("value > ");
        if (!fgets(line, sizeof line, stdin)) {
            break;
        }
        int32_t value = atoi(line);
        int32_t *pvalue = calloc(1, sizeof(int32_t));
        *pvalue = value;

        HashMap_Set(hashmap, key, pvalue);
        HashMap_Dump(hashmap);
    }

    HashMap_Del(hashmap);
    return 0;
}
