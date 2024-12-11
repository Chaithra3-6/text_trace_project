#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_TEXT_SIZE 100000
#define HASH_SIZE 10007   // Larger prime number
#define MAX_WORD_LENGTH 100
#define MAX_STOPWORDS 50

// Stopwords to ignore during comparison
const char* STOPWORDS[MAX_STOPWORDS] = {
    "the", "a", "an", "and", "or", "but", "in", "on", "at", "to",
    "for", "of", "with", "by", "from", "up", "about", "into", "over", "after"
};

// Enhanced token structure with word storage
typedef struct Token {
    char word[MAX_WORD_LENGTH];
    unsigned long hash_value;
    struct Token* next;
} Token;

// Advanced hash function (djb2)
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
        if (strcasecmp(word, STOPWORDS[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Preprocess and sanitize text
void preprocess_text(char* text) {
    int write_index = 0;
    bool last_was_space = false;

    for (int read_index = 0; text[read_index] != '\0'; read_index++) {
        // Convert to lowercase
        char c = tolower(text[read_index]);

        // Remove non-alphanumeric characters except spaces
        if (isalnum(c) || c == ' ') {
            // Avoid multiple consecutive spaces
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

    // Null-terminate and trim trailing spaces
    text[write_index] = '\0';
    while (write_index > 0 && text[write_index-1] == ' ') {
        text[--write_index] = '\0';
    }
}

// Add tokens to hash table with advanced processing
void add_tokens_to_hash_table(char* text, Token* hash_table[]) {
    char* token = strtok(text, " ");
    while (token != NULL) {
        // Skip stopwords and very short words
        if (strlen(token) > 1 && !is_stopword(token)) {
            unsigned long hash_value = hash_function(token);

            // Create new token
            Token* new_token = (Token*)malloc(sizeof(Token));
            strncpy(new_token->word, token, MAX_WORD_LENGTH - 1);
            new_token->word[MAX_WORD_LENGTH - 1] = '\0';
            new_token->hash_value = hash_value;

            // Insert into hash table
            new_token->next = hash_table[hash_value];
            hash_table[hash_value] = new_token;
        }
        token = strtok(NULL, " ");
    }
}

// Advanced similarity calculation with Jaccard-like coefficient
float calculate_similarity(Token* hash_table1[], Token* hash_table2[]) {
    int common_tokens = 0;
    int unique_tokens = 0;
    bool token_found[HASH_SIZE] = {false};

    // Iterate over the first hash table
    for (int i = 0; i < HASH_SIZE; i++) {
        Token* token1 = hash_table1[i];
        while (token1 != NULL) {
            unsigned long hash_value = token1->hash_value;

            // Check if token exists in second hash table
            Token* token2 = hash_table2[hash_value];
            bool found_match = false;
            while (token2 != NULL) {
                if (token2->hash_value == hash_value &&
                    strcmp(token1->word, token2->word) == 0) {
                    found_match = true;
                    break;
                }
                token2 = token2->next;
            }

            // Track unique and common tokens
            if (!token_found[hash_value]) {
                unique_tokens++;
                token_found[hash_value] = true;
            }

            if (found_match) {
                common_tokens++;
            }

            token1 = token1->next;
        }
    }

    // Jaccard-like similarity coefficient
    if (unique_tokens == 0) return 0.0f;
    //return ((float)common_tokens / unique_tokens) * 100;
    float similarity = ((float)common_tokens / unique_tokens) * 100;
    if (similarity > 100.0f) similarity = 100.0f;
    return similarity;
}

// Free hash table memory
void free_hash_table(Token* hash_table[]) {
    for (int i = 0; i < HASH_SIZE; i++) {
        Token* token = hash_table[i];
        while (token != NULL) {
            Token* temp = token;
            token = token->next;
            free(temp);
        }
        hash_table[i] = NULL;
    }
}

int main() {
    char text1[MAX_TEXT_SIZE], text2[MAX_TEXT_SIZE];

    printf("Enter the first text (max 100000 characters):\n");
    if (fgets(text1, MAX_TEXT_SIZE, stdin) == NULL) {
        fprintf(stderr, "Error reading first text\n");
        return 1;
    }
    text1[strcspn(text1, "\n")] = 0;  // Remove newline

    printf("Enter the second text (max 100000 characters):\n");
    if (fgets(text2, MAX_TEXT_SIZE, stdin) == NULL) {
        fprintf(stderr, "Error reading second text\n");
        return 1;
    }
    text2[strcspn(text2, "\n")] = 0;  // Remove newline

    // Preprocess texts
    preprocess_text(text1);
    preprocess_text(text2);

    // Initialize hash tables
    Token* hash_table1[HASH_SIZE] = {0};
    Token* hash_table2[HASH_SIZE] = {0};

    // Add tokens to hash tables
    add_tokens_to_hash_table(text1, hash_table1);
    add_tokens_to_hash_table(text2, hash_table2);

    // Calculate similarity
    float similarity = calculate_similarity(hash_table1, hash_table2);
    printf("Plagiarism similarity: %.2f%%\n", similarity);

    // Clean up memory
    free_hash_table(hash_table1);
    free_hash_table(hash_table2);

    return 0;
}
