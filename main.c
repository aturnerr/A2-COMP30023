#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "sha256.h"

// BYTE *sha256(const char *string) {
//   SHA256_CTX ctx;
//   sha256_init(&ctx);
//   sha256_update(&ctx, string, strlen(string));
//   BYTE result[32];
//   sha256_final(&ctx, result);
//   return *result;
// }

char *sha256_byteToHexString(BYTE data[]);

int main(int argc, char * argv[]) {

  SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, (const BYTE *)argv[1], strlen(argv[1]));
  BYTE result[32];
  sha256_final(&ctx, result);


  // FILE *fp, *output;
  // fp = fopen("pwd6sha256", "rb");
  // output = fopen("hashes.txt", "w");
  // int read = 0, length = 0;
  // unsigned char buffer[33];
  // unsigned char c;

  // for (int i=0; i < 640; i += 32) {
  //   read = fread(buffer, 1, 32, fp);
  //   buffer[32]='\0';
  //   char *hexS = sha256_byteToHexString(buffer);
  //   printf("%s\n", hexS);
  //   printf("%s\n", buffer);
  //   fprintf(output, "%s\n", hexS);
  // }
  return 1;
}
