#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netinet/in.h>
#include <netdb.h>

#include <unistd.h>

int compute(int a, int m, int n)
{
	int r;
	int y = 1;

	while (m > 0)
	{
		r = m % 2;
		// fast exponention
		if (r == 1)
			y = (y*a) % n;
		a = a*a % n;

		m = m / 2;
	}

	return y;
}

int main(int argc, char * argv[]) {
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;

  char buffer[256];

  if (argc < 3) {
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  portno = atoi(argv[2]);

  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno);
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0)
  {
    perror("ERROR opening socket");
    exit(EXIT_FAILURE);
  }

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("ERROR connecting");
    exit(EXIT_FAILURE);
  }
  // scp -i ~/.ssh/key DiffieHellman.c aturner2@172.26.37.44:dh.c
  int g = 15, b = 0, p = 97;

	// get username
	printf("Username: ");
	fgets(buffer, 255, stdin);

	// send username
	n = write(sockfd, buffer, strlen(buffer));
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(EXIT_FAILURE);
	}

	// get b
  printf("Enter secret: ");
  scanf("%d", &b);
	printf("Secret b = %d\n", b);

	// calculate g^b(mod p)
  int gb = compute(g, b, p);
	// convert to string
  sprintf(buffer, "%d\n", gb);
	printf("Current buffer: %d\n", gb);
	// send to server
  n = write(sockfd, buffer, strlen(buffer));
  if (n < 0) {
    perror("ERROR writing to socket");
    exit(EXIT_FAILURE);
  }

	// receive response
  n = read(sockfd, buffer, 255);
  if (n < 0) {
    perror("ERROR reading from socket");
    exit(EXIT_FAILURE);
  }

	printf("Received: %s", buffer);

	// // convert response to int
	int gamodp = 0;
	for (int i=0; buffer[i]!='\n'; i++) {
	    gamodp *= 10;
	    gamodp += buffer[i] - '0';
	}
	// less reliable (seems to result in non valid integer response?)
	// buffer[n] = 0;
	// int gamodp = atoi(buffer);

  printf("Converted to: %d\n", gamodp);

  int s = compute(gamodp, b, p);
	printf("A^b(mod p) = %d\n", s);
  sprintf(buffer, "%d\n", s);
	printf("Current buffer: %s", buffer);

  n = write(sockfd, buffer, strlen(buffer));
  if (n < 0) {
    perror("ERROR writing to socket");
    exit(EXIT_FAILURE);
  }

	n = read(sockfd, buffer, 255);
  if (n < 0) {
    perror("ERROR reading from socket");
    exit(EXIT_FAILURE);
  }
  printf("Received: %s\n", buffer);



  return 0;
	close(sockfd);
}
