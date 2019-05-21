#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "sha256.h"
#include "ctype.h"

// hex output command: xxd -p -l 32 output
#define MAX_WORD_LEN 6
#define ASCII 97

void generate_guesses(int max_passwords, int compare_hash);

BYTE *sha256(const char *string) {
  SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, (const BYTE *)string, strlen(string));
  BYTE *result = (BYTE *) malloc(sizeof(BYTE) * 32);
  sha256_final(&ctx, result);
  return result;
}

int main(int argc, char * argv[]) {
  // char *word = argv[1];
  // BYTE *result = sha256(word);
  // printf("%s", result);
  //
  // free(result);
  generate_guesses(50, 0);

  return 1;
}

char *check_replace_char_multiple(char * word, int i, char letter, char replacement) {
  if ((word[i] == letter) || (word[i] == toupper(letter))) {
    word[i] = replacement;
    printf("%s\n", word);
  }
  return word;
}

void check_replace_char(char word[MAX_WORD_LEN+1], int i, char letter, char replacement) {
  char word_buffer[MAX_WORD_LEN+1];
  if ((word[i] == letter) || (word[i] == toupper(letter))) {
    strcpy(word_buffer, word);
    word_buffer[i] = replacement;
    printf("%s\n", word_buffer);
  }
}


void generate_guesses(int max_passwords, int compare_hash) {
  FILE* read_fp = fopen("password_list.txt", "r");
  char charset[26][10] = {{ 0 }};
  char word[MAX_WORD_LEN+1];
  char word_buffer[MAX_WORD_LEN+1];
  int i = 0, j = 0, num_generated = 0;

  // place substitutions in array
  strcpy(charset['a'- ASCII], "@4");
  strcpy(charset['b'- ASCII], "8");
  strcpy(charset['c'- ASCII], "(");
  strcpy(charset['e'- ASCII], "3");
  strcpy(charset['i'- ASCII], "1i");
  strcpy(charset['l'- ASCII], "1i");
  strcpy(charset['o'- ASCII], "0");
  strcpy(charset['t'- ASCII], "+7");
  strcpy(charset['s'- ASCII], "$z");
  strcpy(charset['z'- ASCII], "2");

  while (fscanf(read_fp, "%s", word) == 1) {
    printf("%s\n", word);
    num_generated++;

    for (i = 0; i < strlen(word); i++) {
      if (isalpha(word[i])) {
        if (strcmp(charset[word[i] - ASCII], "")) {
          for (j = 0; j < strlen(charset[word[i] - ASCII]); j++) {
            strcpy(word_buffer, word);
            word_buffer[i] = charset[word[i] - ASCII][j];
            printf("%s\n", word_buffer);
            num_generated++;
          }
        }
      }
    }
  }
}
