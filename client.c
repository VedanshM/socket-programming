#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define htonll(x) ((1 == htonl(1)) ? (x) : ((uint64_t)htonl((x)&0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1 == ntohl(1)) ? (x) : ((uint64_t)ntohl((x)&0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

#define PORT 8000
#define BUF_SIZE 4096

char buf[BUF_SIZE + 1];

char all_good(int sock) {
	char c[5] = {0};
	read(sock, c, 5);
	return strcmp(c, "ok") == 0;
}

int main(int argc, char const *argv[]) {
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
												// which is meant to be, and rest is defined below

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Converts an IP address in numbers-and-dots notation into either a
	// struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) // connect to the server address
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	uint32_t nof = htonl(argc - 1);
	printf("Requesing %d files...", argc - 1);
	write(sock, (void *)&nof, sizeof(nof));

	if (!all_good(sock)) {
		printf("server not responding !!\n");
		return 0;
	} else
		printf("ok\n");

	for (int n, i = 1; i < argc; i++) {
		printf("Requesting: %s ...", argv[i]);
		fflush(0);
		write(sock, argv[i], strlen(argv[i]) + 1);

		if (all_good(sock)) {
			printf("ok\n");
			uint64_t bytes_left = 0, file_size;
			read(sock, &file_size, sizeof(file_size));
			file_size = bytes_left = ntohll(file_size);

			int fd = open(argv[i], O_CREAT | O_TRUNC | O_WRONLY, 0644);
			while (bytes_left > 0 && (n = read(sock, buf, min(bytes_left, BUF_SIZE))) > 0) {
				write(fd, buf, n);
				bytes_left -= n;
				printf("\r                                                            \r"
					   "Received: %ld/%ld\n bytes",
					   file_size - bytes_left, file_size);
			}
			close(fd);
			printf("File : %s transfer done.\n", argv[i]);
		} else {
			printf("File not found !!\n");
		}
	}
	close(sock);
	return 0;
}
