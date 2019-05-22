#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_WORD_LEN 40

int main(int argc, char** argv){
    FILE* read_fp = fopen(argv[1], "r");
    FILE* write_fp = fopen(argv[2], "a");
    int upper = atoi(argv[3]);
    int lower = atoi(argv[4]);

    char word[MAX_WORD_LEN+1];

    while (fscanf(read_fp, "%s", word) == 1){
        if ((strlen(word) <= upper) && (strlen(word) >= lower)){
            fprintf(write_fp, "%s\n", word);
        }
    }

    fclose(read_fp);
    fclose(write_fp);

    return 0;
}
