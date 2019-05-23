#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 40

int main(int argc, char** argv){
    FILE* read_fp = fopen(argv[1], "r");
    FILE* write_fp = fopen(argv[2], "a");
    int upper = atoi(argv[3]);
    int lower = atoi(argv[4]);
    int has_alpha = 0;

    char word[MAX_WORD_LEN+1];

    while (fscanf(read_fp, "%s", word) == 1){
      for (int i=0; i < strlen(word); i++) {
        if (isalpha(word[i])) {
          if (islower(word[i])){
            has_alpha = 1;
          }
        }
      }
      if (has_alpha == 1) {
        fprintf(write_fp, "%s\n", word);
      }
      has_alpha = 0;
    }

    fclose(read_fp);
    fclose(write_fp);

    return 0;
}
