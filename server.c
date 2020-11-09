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

void send_ack(int sock, int good) {
	if (good) {
		write(sock, "ok", sizeof("ok"));
	} else {
		write(sock, "err", sizeof("err"));
	}
}

int main(int argc, char const *argv[]) {
	int listen_sock, connd_sock;
	struct sockaddr_in address;

	// Creating socket file descriptor
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) { // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// This is to lose the pesky "Address already in use" error message
	int opt = 1;
	if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { // SOL_SOCKET is the socket layer itself
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	memset(&address, '0', sizeof(address)); // to make sure the struct is empty. Essentially sets sin_zero as 0
	address.sin_family = AF_INET;			// Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc.
	address.sin_addr.s_addr = INADDR_ANY;	// Accept connections from any IP address - listens from all interfaces.
	address.sin_port = htons(PORT);			// Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

	// Forcefully attaching socket to the port 8080
	if (bind(listen_sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// Port bind is done. You want to wait for incoming connections and handle them in some way.
	// The process is two step: first you listen(), then you accept()
	if (listen(listen_sock, 3) < 0) { // 3 is the maximum size of queue - connections you haven't accepted
		perror("listen");
		exit(EXIT_FAILURE);
	}
	for (;;) {
		// returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
		if ((connd_sock = accept(listen_sock, NULL, NULL)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		uint32_t nof = 0;
		read(connd_sock, &nof, sizeof(nof));
		nof = ntohl(nof);
		if (nof > 1e9) { // server wont send that many files in one go
			// it is just implementation choice can be removed
			send_ack(connd_sock, 0);
			close(connd_sock);
			continue;
		} else
			send_ack(connd_sock, 1);

		printf("Got request for %d files.\n", nof);

		for (int i = 0; i < nof; i++) {
			int n = read(connd_sock, buf, BUF_SIZE);
			printf("Got request : %s\n", buf);
			int fd = open(buf, O_RDONLY);
			if (fd < 0) {
				send_ack(connd_sock, 0);
			} else {
				send_ack(connd_sock, 1);
				uint64_t file_size = lseek(fd, 0, SEEK_END);
				lseek(fd, 0, SEEK_SET);
				printf("sending %ldB...\n", file_size);
				file_size = htonll(file_size);
				write(connd_sock, &file_size, sizeof(file_size));
				while ((n = read(fd, buf, BUF_SIZE)) > 0) {
					write(connd_sock, buf, n);
				}
				close(fd);
				printf("done.\n");
			}
		}
		close(connd_sock);
	}
	close(listen_sock);
	return 0;
}
