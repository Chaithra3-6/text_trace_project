#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_TEXT_SIZE 100000
#define HASH_SIZE 10007
#define MAX_WORD_LENGTH 100
#define MAX_STOPWORDS 50

// Stopwords to ignore during comparison
const char* STOPWORDS[MAX_STOPWORDS] = {
    "the", "a", "an", "and", "or", "but", "in", "on", "at", "to",
    "for", "of", "with", "by", "from", "up", "about", "into", "over", "after"
};

// Token structure
typedef struct Token {
    char word[MAX_WORD_LENGTH];
    struct Token* next;
} Token;

// Hash function (djb2)
unsigned long hash_function(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_SIZE;
}

// Check if a word is a stopword
bool is_stopword(const char* word) {
    for (int i = 0; i < MAX_STOPWORDS; i++) {
        if (strcasecmp(word, STOPWORDS[i]) == 0) return true;
    }
    return false;
}

// Preprocess and sanitize text
void preprocess_text(char* text) {
    int write_index = 0;
    bool last_was_space = false;

    for (int read_index = 0; text[read_index]; read_index++) {
        char c = tolower(text[read_index]);
        if (isalnum(c) || c == ' ') {
            if (c == ' ') {
                if (!last_was_space) {
                    text[write_index++] = c;
                    last_was_space = true;
                }
            } else {
                text[write_index++] = c;
                last_was_space = false;
            }
        }
    }

    text[write_index] = '\0';
    while (write_index > 0 && text[write_index-1] == ' ') {
        text[--write_index] = '\0';
    }
}

// Add tokens to hash table
void add_tokens_to_hash_table(char* text, Token* hash_table[]) {
    char* token = strtok(text, " ");
    while (token != NULL) {
        if (strlen(token) > 1 && !is_stopword(token)) {
            unsigned long hash_value = hash_function(token);
            Token* new_token = (Token*)malloc(sizeof(Token));
            strncpy(new_token->word, token, MAX_WORD_LENGTH - 1);
            new_token->word[MAX_WORD_LENGTH - 1] = '\0';
            new_token->next = hash_table[hash_value];
            hash_table[hash_value] = new_token;
        }
        token = strtok(NULL, " ");
    }
}

// Insert a word into hash table if not present
bool insert_unique(Token* hash_table[], const char* word) {
    unsigned long hash_value = hash_function(word);
    Token* current = hash_table[hash_value];
    while (current) {
        if (strcmp(current->word, word) == 0) return false;
        current = current->next;
    }
    Token* new_token = (Token*)malloc(sizeof(Token));
    strncpy(new_token->word, word, MAX_WORD_LENGTH - 1);
    new_token->word[MAX_WORD_LENGTH - 1] = '\0';
    new_token->next = hash_table[hash_value];
    hash_table[hash_value] = new_token;
    return true;
}

// Free hash table memory
void free_hash_table(Token* hash_table[]) {
    for (int i = 0; i < HASH_SIZE; i++) {
        Token* current = hash_table[i];
        while (current) {
            Token* temp = current;
            current = current->next;
            free(temp);
        }
        hash_table[i] = NULL;
    }
}

// Improved similarity function
float calculate_similarity_improved(Token* hash_table1[], Token* hash_table2[]) {
    int common_tokens = 0;
    int unique_tokens = 0;
    Token* seen[HASH_SIZE] = {0};

    // Add words from first table
    for (int i = 0; i < HASH_SIZE; i++) {
        Token* token1 = hash_table1[i];
        while (token1) {
            if (insert_unique(seen, token1->word)) unique_tokens++;
            token1 = token1->next;
        }
    }

    // Check words in second table
    for (int i = 0; i < HASH_SIZE; i++) {
        Token* token2 = hash_table2[i];
        while (token2) {
            bool is_common = false;
            unsigned long hash_val = hash_function(token2->word);
            Token* current = hash_table1[hash_val];
            while (current) {
                if (strcmp(current->word, token2->word) == 0) {
                    is_common = true;
                    break;
                }
                current = current->next;
            }
            if (is_common) common_tokens++;

            if (insert_unique(seen, token2->word)) unique_tokens++;
            token2 = token2->next;
        }
    }

    free_hash_table(seen);

    if (unique_tokens == 0) return 0.0f;
    float similarity = ((float)common_tokens / unique_tokens) * 100;
    if (similarity > 100.0f) similarity = 100.0f;
    return similarity;
}

int main() {
    char text1[MAX_TEXT_SIZE], text2[MAX_TEXT_SIZE];

    printf("Enter the first text:\n");
    if (!fgets(text1, MAX_TEXT_SIZE, stdin)) return 1;
    text1[strcspn(text1, "\n")] = 0;

    printf("Enter the second text:\n");
    if (!fgets(text2, MAX_TEXT_SIZE, stdin)) return 1;
    text2[strcspn(text2, "\n")] = 0;

    preprocess_text(text1);
    preprocess_text(text2);

    Token* hash_table1[HASH_SIZE] = {0};
    Token* hash_table2[HASH_SIZE] = {0};

    add_tokens_to_hash_table(text1, hash_table1);
    add_tokens_to_hash_table(text2, hash_table2);

    float similarity = calculate_similarity_improved(hash_table1, hash_table2);
    printf("Plagiarism similarity: %.2f%%\n", similarity);

    free_hash_table(hash_table1);
    free_hash_table(hash_table2);

    return 0;
}

