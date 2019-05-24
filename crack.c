/*
 * Dictionary mangler and bruteforce hash cracker for Assignment 2 of COMP30023
 * Written by Adam Turner, May 2019
 *
 * sha256_byteToHexString function:
 * https://github.com/RemyNoulin/sha256
 *
 * strcmp_unsigned function:
 * https://stackoverflow.com/questions/1356741/
*/

#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "sha256.h"
#include "ctype.h"

// constants
#define ALPHA_COUNT 26
#define ASCII 97
#define BRUTEFORCE_NUM_START 100000
#define BRUTEFORCE_NUM_END 999999

#define MAX_WORD_LEN 6
#define MAX_SUBSTITUTIONS 10
#define MAX_POTFILE_LINE_LENGTH 10
#define MAX_PASSWORD_LINE_LENGTH 10000
#define MAX_FOUND_SIZE 10000

#define DICTIONARY "list.txt"
#define HASH_FILE "pwd6sha256"
#define POTFILE "potfile"

static char const * const PWD_FORMAT = "%s %d\n";

/*
 * Generate the SHA256 sum of a given string and return result array
*/
BYTE *sha256(const char *string) {
  SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, (const BYTE *)string, strlen(string));
  BYTE *result = (BYTE *) malloc(sizeof(BYTE) * SHA256_BLOCK_SIZE);
  sha256_final(&ctx, result);
  return result;
}

/*
 * Convert unsigned char (BYTE) array to hexidecimal
*/
void sha256_byteToHexString(BYTE data[], char output[]) {
	char *hexC = "0123456789abcdef";
	char *hexS = malloc(SHA256_BLOCK_SIZE*2+1);
	for(BYTE i; i<SHA256_BLOCK_SIZE; i++) {
		hexS[i*2]   = hexC[data[i]>>4];
		hexS[i*2+1] = hexC[data[i]&0xF];
	}
	hexS[SHA256_BLOCK_SIZE*2] = 0;
  strcpy(output, hexS);
}

/*
 * Hash and compare a supplied wordlist against a list of SHA256 hashes
*/
void compare_lists(char wordlist[], char hashlist[]) {
  // open files
  FILE *word_file = fopen(wordlist, "r");
  if (word_file == NULL) { perror("INVALID PASSWORD LIST"); return; }
  FILE *hash_file = fopen(hashlist, "r");
  if (hash_file == NULL) { perror("INVALID HASH LIST"); return; }
  // get size of hash file
  fseek(hash_file, 0L, SEEK_END);
  int hash_file_size = ftell(hash_file);
  fseek(hash_file, 0L, SEEK_SET);
  // calculate number of hashes
  int n_hashes = hash_file_size / SHA256_BLOCK_SIZE;
  // initialise arrays
  char hashes[n_hashes][SHA256_BLOCK_SIZE*2+1];
  unsigned char buffer[SHA256_BLOCK_SIZE+1];
  char hex_buffer[SHA256_BLOCK_SIZE*2+1];
  char line [MAX_PASSWORD_LINE_LENGTH];

  // convert hashes to hex and store in array
  for (int i = 0; i < hash_file_size; i+=SHA256_BLOCK_SIZE) {
    fread(buffer, 1, SHA256_BLOCK_SIZE, hash_file);
    buffer[SHA256_BLOCK_SIZE]='\0';
    sha256_byteToHexString(buffer, hex_buffer);
    strcpy(hashes[i/SHA256_BLOCK_SIZE], hex_buffer);
  }

  // hash, convert to hex then compare every word against the hashes
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
  }
  // close files
  fclose(word_file);
  fclose(hash_file);
}

/*
 * Reimplementation of strcmp for unsigned char (BYTE)
*/
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

/*
 * Hash and compare a word against a list of SHA256 hashes
*/
int check_hash(char word[MAX_WORD_LEN+1], char *found_passwords) {
  FILE *fp = fopen(HASH_FILE, "r");
  unsigned char buffer[SHA256_BLOCK_SIZE+1];
  char pwd_buffer[MAX_POTFILE_LINE_LENGTH];
  // check if password is already found
  char *already_found = strstr(found_passwords, word);
  // hash the word
  BYTE *result = sha256(word);

  // get size of hash file
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (!already_found) {
    // compare against hashes in file
    for (int i = 0; i < fsize; i += SHA256_BLOCK_SIZE) {
      fread(buffer, 1, SHA256_BLOCK_SIZE, fp);
      if (strcmp_unsigned(buffer, result) == 0) {
          printf("%s %d\n", word, i/SHA256_BLOCK_SIZE + 1);
          sprintf(pwd_buffer, PWD_FORMAT, word, i/SHA256_BLOCK_SIZE + 1);
          strcat(found_passwords, pwd_buffer);
      }
    }
  }
  // free and close
  free(result);
  fclose(fp);
  return 1;
}

/*
 * Basic character mutation, check the original word and uppercase
*/
void mutate_basic(char word[MAX_WORD_LEN+1], char found_passwords[], int *num_generated, int max_passwords, int compare_hash) {
  int i = 0;
  char word_buffer[MAX_WORD_LEN+1];

  strcpy(word_buffer, word);
  // check regular word
  if ((*num_generated < max_passwords) && (compare_hash == 0)) {
    printf("%s\n", word_buffer);
    (*num_generated)++;
  } else if (compare_hash == 1) {
    check_hash(word_buffer, found_passwords);
  }
  // convert to uppercase
  for (i = 0; i < strlen(word); i++) {
    word_buffer[i] = toupper(word_buffer[i]);
  }
  // check the word again
  if ((*num_generated < max_passwords) && (compare_hash == 0)) {
    printf("%s\n", word_buffer);
    (*num_generated)++;
  } else if (compare_hash == 1) {
    check_hash(word_buffer, found_passwords);
  }
}

/*
 * Mutate characters in a word with a list of substitutions
*/
void mutate_characters(char word[MAX_WORD_LEN+1], char found_passwords[], char charset[26][10], int *num_generated, int max_passwords, int compare_hash, int depth) {
  int i = 0, j = 0;
  char word_buffer[MAX_WORD_LEN+1];

  // terminal test
  if (depth > 0) {
    for (i = 0; i < strlen(word); i++) {
      // make sure character is alphabetic
      if (isalpha(word[i])) {
        // if substitution exists
        if (charset[tolower(word[i]) - ASCII] != 0) {
          for (j = 0; j < strlen(charset[tolower(word[i]) - ASCII]); j++) {
            strcpy(word_buffer, word);
            // replace the character
            word_buffer[i] = charset[tolower(word[i]) - ASCII][j];
            // output or compare hash
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

/*
 * Mutate lower case characters in a word
*/
void mutate_case(char word[MAX_WORD_LEN+1], char found_passwords[], int *num_generated, int max_passwords, int compare_hash, int depth) {
  int i = 0;
  char word_buffer[MAX_WORD_LEN+1];

  // terminal test
  if (depth > 0) {
    for (i = 0; i < strlen(word); i++) {
      strcpy(word_buffer, word);
      // if lowercase, convert to upper
      if (islower(word_buffer[i])) {
        word_buffer[i] = toupper(word_buffer[i]);
        // output or compare hash
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

/*
 * Append characters to the end of smaller words
*/
void append_characters(char word[MAX_WORD_LEN+1], char found_passwords[], int *num_generated, long long int max_passwords, int compare_hash, int num_to_append) {
  int i = 0;
  char word_buffer[MAX_WORD_LEN+1];
  // list of characters to append
  char charset[] = "1234567890!* ";

  if (num_to_append > 0) {
    for (i = 0; i < strlen(charset); i++) {
      strcpy(word_buffer, word);
      size_t len = strlen(word_buffer);
      // increase size of buffer to make space for char
      word_buffer[len++] = charset[i];
      word_buffer[len] = '\0';
      // output or compare hash
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

/*
 * Generate numbers for a set length
*/
void generate_numbers(char found_passwords[], int *num_generated, long long int max_passwords, int compare_hash, int length) {
  int count = BRUTEFORCE_NUM_START;
  char num[MAX_WORD_LEN];
  while ((count < BRUTEFORCE_NUM_END+1) ) {
    // output or compare hash
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

/*
 * Last-resort nested bruteforce for all 6 character combinations
*/
void bruteforce_char(char found_passwords[], int *num_generated, long long int max_passwords, int compare_hash) {
  char brute[7] = "aaaaaa\0";
  // iterate over ascii values of char range
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

/*
 * Generate n (max_passwords) guesses of passwords, hash if required
*/
void generate_guesses(long long int max_passwords, int compare_hash) {
  // open the password list to read
  FILE *read_fp = fopen(DICTIONARY, "r");
  // potfile to store found passwords
  FILE *potfile = fopen(POTFILE, "w");

  char charset[ALPHA_COUNT][MAX_SUBSTITUTIONS] = {{ 0 }};
  char word[MAX_WORD_LEN+1];
  int num_generated = 0;

  // get size of potfile
  fseek(potfile, 0, SEEK_END);
  long fsize = ftell(potfile);
  fseek(potfile, 0, SEEK_SET);

  char found_passwords[MAX_FOUND_SIZE];
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

  // for every word, check basic, mutate basic and case
  while ((fscanf(read_fp, "%s", word) == 1) && (num_generated < max_passwords)) {
    mutate_basic(word, found_passwords, &num_generated, max_passwords, compare_hash);
    if (strlen(word) == MAX_WORD_LEN) {
      mutate_characters(word, found_passwords, charset, &num_generated, max_passwords, compare_hash, 1);
      mutate_case(word, found_passwords, &num_generated, max_passwords, compare_hash, 1);
    // use below when password list has words less than 6 chars
    // } else if (strlen(word) == 5) {
    //   append_characters(word, found_passwords, &num_generated, max_passwords, compare_hash, 1);
    // } else if (strlen(word) == 4) {
    //   append_characters(word, found_passwords, &num_generated, max_passwords, compare_hash, 2);
    }
  }
  // run final bruteforce
  generate_numbers(found_passwords, &num_generated, max_passwords, compare_hash, 6);
  bruteforce_char(found_passwords, &num_generated, max_passwords, compare_hash);

  // save potfile and close
  fprintf(potfile, "%s", found_passwords);
  fclose(potfile);
  fclose(read_fp);
}

/*
 * Main controller for behaviour depending on argc as required
*/
int main(int argc, char * argv[]) {
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
