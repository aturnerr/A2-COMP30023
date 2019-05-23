#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "sha256.h"
#include "ctype.h"

// hex output command: xxd -p -c 32 output
#define MAX_WORD_LEN 6
#define ASCII 97
#define MAX_NUMBER 999999

static char const * const PWD_FORMAT = "%s %d\n";

void generate_guesses(long long int max_passwords, int compare_hash);

BYTE *sha256(const char *string) {
  SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, (const BYTE *)string, strlen(string));
  BYTE *result = (BYTE *) malloc(sizeof(BYTE) * 32);
  sha256_final(&ctx, result);
  return result;
}

void sha256_byteToHexString(BYTE data[], char output[]) {
	char *hexC = "0123456789abcdef";
	char *hexS = malloc(65);
	for(BYTE i; i<32; i++) {
		hexS[i*2]   = hexC[data[i]>>4];
		hexS[i*2+1] = hexC[data[i]&0xF];
	}
	hexS[64] = 0;
  strcpy(output, hexS);
}

void compare_lists(char wordlist[], char hashlist[]) {
  FILE *word_file = fopen(wordlist, "r");
  if (word_file == NULL) { perror("INVALID PASSWORD LIST"); return; }
  FILE *hash_file = fopen(hashlist, "r");
  if (hash_file == NULL) { perror("INVALID HASH LIST"); return; }
  fseek(hash_file, 0L, SEEK_END);
  int hash_file_size = ftell(hash_file);
  fseek(hash_file, 0L, SEEK_SET);

  int n_hashes = hash_file_size / SHA256_BLOCK_SIZE;
  char hashes[n_hashes][SHA256_BLOCK_SIZE*2+1];
  unsigned char buffer[33];
  char hex_buffer[SHA256_BLOCK_SIZE*2+1];

  for (int i = 0; i < hash_file_size; i+=SHA256_BLOCK_SIZE) {
    fread(buffer, 1, 32, hash_file);
    buffer[32]='\0';
    sha256_byteToHexString(buffer, hex_buffer);
    strcpy(hashes[i/SHA256_BLOCK_SIZE], hex_buffer);
  }

  char line [256];

  while (fscanf(word_file, "%s", line) == 1) {
    SHA256_CTX ctx;
    BYTE result[SHA256_BLOCK_SIZE];
    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE *)line, strlen(line));
    sha256_final(&ctx, result);
    char hex_result[SHA256_BLOCK_SIZE*2+1];
    sha256_byteToHexString(result, hex_result);

    for (int i = 0; i < n_hashes; i++) {
      if (strcmp(hashes[i], hex_result) == 0) {
        printf("%s %d\n", line, i+1);
      }
    }
    // printf("%s\n", hex_result);
  }



  fclose(word_file);
  fclose(hash_file);
}

int main(int argc, char * argv[]) {
  // char *word = argv[1];
  // BYTE *result = sha256(word);
  // printf("%s", result);
  //
  // free(result);
  if (argc == 1) {
    generate_guesses(1, 1);
  }
  if (argc == 2) {
    long long int n = atoll(argv[1]);
    generate_guesses(n, 0);
  }
  if (argc == 3) {
    compare_lists(argv[1], argv[2]);
  }


  return 0;
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

int strcmp_unsigned(BYTE *s1, BYTE *s2) {
  unsigned char * p1 = (unsigned char *)s1;
  unsigned char * p2 = (unsigned char *)s2;

  while ( ( *p1 ) && ( *p1 == *p2 ) )
    {
        ++p1;
        ++p2;
    }
  return ( *p1 - *p2 );
}

int check_hash(char word[MAX_WORD_LEN+1], char *found_passwords) {
  FILE *fp = fopen("pwd6sha256", "r");

  BYTE *result = sha256(word);
  unsigned char buffer[33];
  char pwd_buffer[10];

  char *already_found = strstr(found_passwords, word);

  if (!already_found) {
    for (int i=0; i < 640; i += 32) {
      fread(buffer, 1, 32, fp);
      if (strcmp_unsigned(buffer, result) == 0) {
          printf("%s %d\n", word, i/32 + 1);
          sprintf(pwd_buffer, PWD_FORMAT, word, i/32 + 1);
          strcat(found_passwords, pwd_buffer);
      }
    }
  }

  free(result);
  fclose(fp);

  return 1;
}

void mutate_basic(char word[MAX_WORD_LEN+1], char found_passwords[], int *num_generated, int max_passwords, int compare_hash) {
  int i = 0;
  char word_buffer[MAX_WORD_LEN+1];

  strcpy(word_buffer, word);

  if ((*num_generated < max_passwords) && (compare_hash == 0)) {
    printf("%s\n", word_buffer);
    (*num_generated)++;
  } else if (compare_hash == 1) {
    check_hash(word_buffer, found_passwords);
  }

  for (i = 0; i < strlen(word); i++) {
    word_buffer[i] = toupper(word_buffer[i]);
  }

  if ((*num_generated < max_passwords) && (compare_hash == 0)) {
    printf("%s\n", word_buffer);
    (*num_generated)++;
  } else if (compare_hash == 1) {
    check_hash(word_buffer, found_passwords);
  }
}

void mutate_characters(char word[MAX_WORD_LEN+1], char found_passwords[], char charset[26][10], int *num_generated, int max_passwords, int compare_hash, int depth) {
  int i = 0, j = 0;
  char word_buffer[MAX_WORD_LEN+1];

  if (depth > 0) {
    for (i = 0; i < strlen(word); i++) {
      if (isalpha(word[i])) {
        if (charset[tolower(word[i]) - ASCII] != 0) {
          for (j = 0; j < strlen(charset[tolower(word[i]) - ASCII]); j++) {
            strcpy(word_buffer, word);
            word_buffer[i] = charset[tolower(word[i]) - ASCII][j];
            if ((*num_generated < max_passwords) && compare_hash == 0) {
              printf("%s\n", word_buffer);
              (*num_generated)++;
              mutate_characters(word_buffer, found_passwords, charset, num_generated, max_passwords, compare_hash, depth - 1);
            } else if (compare_hash == 1) {
              check_hash(word_buffer, found_passwords);
              mutate_characters(word_buffer, found_passwords, charset, num_generated, max_passwords, compare_hash, depth - 1);
            }
          }
        }
      }
    }
  }
}

void mutate_case(char word[MAX_WORD_LEN+1], char found_passwords[], int *num_generated, int max_passwords, int compare_hash, int depth) {
  int i = 0;
  char word_buffer[MAX_WORD_LEN+1];

  if (depth > 0) {
    for (i = 0; i < strlen(word); i++) {
      strcpy(word_buffer, word);
      if (islower(word_buffer[i])) {
        word_buffer[i] = toupper(word_buffer[i]);
        if ((*num_generated < max_passwords) && compare_hash == 0) {
          printf("%s\n", word_buffer);
          (*num_generated)++;
          mutate_case(word_buffer, found_passwords, num_generated, max_passwords, compare_hash, depth - 1);
        } else if (compare_hash == 1) {
          check_hash(word_buffer, found_passwords);
          mutate_case(word_buffer, found_passwords, num_generated, max_passwords, compare_hash, depth - 1);
        }
      }
    }
  }
}

void append_characters(char word[MAX_WORD_LEN+1], char found_passwords[], int *num_generated, long long int max_passwords, int compare_hash, int num_to_append) {
  int i = 0;
  char word_buffer[MAX_WORD_LEN+1];
  char charset[] = "1234567890!* ";

  if (num_to_append > 0) {
    for (i = 0; i < strlen(charset); i++) {
      strcpy(word_buffer, word);
      size_t len = strlen(word_buffer);
      word_buffer[len++] = charset[i];
      word_buffer[len] = '\0';
      if ((*num_generated < max_passwords) && compare_hash == 0) {
        printf("%s\n", word_buffer);
        (*num_generated)++;
        append_characters(word_buffer, found_passwords, num_generated, max_passwords, compare_hash, num_to_append - 1);
      } else if (compare_hash == 1) {
        check_hash(word_buffer, found_passwords);
        append_characters(word_buffer, found_passwords, num_generated, max_passwords, compare_hash, num_to_append - 1);
      }
    }
  }
}

void generate_numbers(char found_passwords[], int *num_generated, long long int max_passwords, int compare_hash, int length) {
  int count = 100000;
  char num[6];
  while ((count < MAX_NUMBER+1) ) {
    if ((*num_generated < max_passwords) && compare_hash == 0) {
      printf("%d\n", count);
      count++;
      (*num_generated)++;
    } else if ((*num_generated >= max_passwords) && compare_hash == 0) {
      return;
    } else if (compare_hash == 1) {
      sprintf(num, "%d", count);
      check_hash(num, found_passwords);
    }
  }
}

void bruteforce_char(char found_passwords[], int *num_generated, long long int max_passwords, int compare_hash) {
  char brute[7] = "aaaaaa\0";

  for (brute[0] = ' '; brute[0] <= '~'; brute[0] ++) {
    for (brute[1] = ' '; brute[1] <= '~'; brute[1] ++) {
      for (brute[2] = ' '; brute[2] <= '~'; brute[2] ++) {
        for (brute[3] = ' '; brute[3] <= '~'; brute[3] ++) {
          for (brute[4] = ' '; brute[4] <= '~'; brute[4] ++) {
            for (brute[5] = ' '; brute[5] <= '~'; brute[5] ++) {
              if ((*num_generated < max_passwords) && compare_hash == 0) {
                printf("%s\n", brute);
                (*num_generated)++;
              } else if ((*num_generated >= max_passwords) && compare_hash == 0) {
                return;
              } else if (compare_hash == 1) {
                check_hash(brute, found_passwords);
              }
            }
          }
        }
      }
    }
  }
}

void generate_guesses(long long int max_passwords, int compare_hash) {
  FILE* read_fp = fopen("list.txt", "r");
  FILE *potfile = fopen("potfile", "w");

  char charset[26][10] = {{ 0 }};
  char word[MAX_WORD_LEN+1];
  int num_generated = 0;

  fseek(potfile, 0, SEEK_END);
  long fsize = ftell(potfile);
  fseek(potfile, 0, SEEK_SET);

  char found_passwords[500];
  fread(found_passwords, 1, fsize, potfile);

  // place substitutions in array
  strcpy(charset['a'- ASCII], "@4");
  strcpy(charset['b'- ASCII], "8");
  strcpy(charset['c'- ASCII], "(");
  strcpy(charset['d'- ASCII], ">");
  strcpy(charset['e'- ASCII], "3");
  strcpy(charset['i'- ASCII], "1i|");
  strcpy(charset['l'- ASCII], "1i7");
  strcpy(charset['o'- ASCII], "0");
  strcpy(charset['t'- ASCII], "+7");
  strcpy(charset['s'- ASCII], "$z5");
  strcpy(charset['z'- ASCII], "2");

  while ((fscanf(read_fp, "%s", word) == 1) && (num_generated < max_passwords)) {
    mutate_basic(word, found_passwords, &num_generated, max_passwords, compare_hash);
    if (strlen(word) == 6) {
      mutate_characters(word, found_passwords, charset, &num_generated, max_passwords, compare_hash, 1);
      mutate_case(word, found_passwords, &num_generated, max_passwords, compare_hash, 1);
  //   // } else if (strlen(word) == 5) {
  //   //   append_characters(word, found_passwords, &num_generated, max_passwords, compare_hash, 1);
  //   // } else if (strlen(word) == 4) {
  //   //   append_characters(word, found_passwords, &num_generated, max_passwords, compare_hash, 2);
    }
  }
  // generate_numbers(found_passwords, &num_generated, max_passwords, compare_hash, 6);
  // bruteforce_char(found_passwords, &num_generated, max_passwords, compare_hash);

  // printf("Done generating!\n");
  fprintf(potfile, "%s", found_passwords);
  fclose(potfile);
  fclose(read_fp);
}
