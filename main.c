#include <stdio.h>
#include <string.h>
#include "sha256.h"

// BYTE *sha256(const char *string) {
//   SHA256_CTX ctx;
//   sha256_init(&ctx);
//   sha256_update(&ctx, string, strlen(string));
//   BYTE result[32];
//   sha256_final(&ctx, result);
//   return *result;
// }

int main(int argc, char * argv[]) {
  printf("hello\n");

  SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, argv[1], strlen(argv[1]));
  BYTE result[32];
  sha256_final(&ctx, result);
  printf("%s", result);
}
