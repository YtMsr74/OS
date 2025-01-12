#include <stdio.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>

volatile sig_atomic_t wasSigHup = 0;
void sigHupHandler(int r)
{
	wasSigHup = 1;
}

int main() {
	struct sigaction sa;
	sigaction(SIGHUP, NULL, &sa);
	sa.sa_handler = sigHupHandler;
	sa.sa_flags |= SA_RESTART;
	sigaction(SIGHUP, &sa, NULL);

	sigset_t blockedMask, origMask;
	sigemptyset(&blockedMask);
	sigaddset(&blockedMask, SIGHUP);
	sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	int client;
	int maxFD = serverSocket;
	fd_set fds;
	bool occupied = false;

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(3000);

	bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	listen(serverSocket, SOMAXCONN);
	printf("Server started\n");

	while (true) {
		FD_ZERO(&fds);
		FD_SET(serverSocket, &fds);
		maxFD = serverSocket;
		FD_SET(client, &fds);
		if (client > maxFD) maxFD = client;

		if (pselect(maxFD + 1, &fds, NULL, NULL, NULL, &origMask) == -1) {
			if (errno == EINTR) {
				if (wasSigHup) {
					wasSigHup = 0;
					printf("Sighup received\n");
				}
				continue;
			}
			else {
				break;
			}
		}

		if (FD_ISSET(serverSocket, &fds)) {
			int newClient = accept(serverSocket, NULL, NULL);
			if (newClient != -1) {
				printf("New connection %d\n", newClient);
				if (occupied) {
					printf("Closing connection %d\n", client);
					close(client);
				}
				occupied = true;
				client = newClient;
			}
		}

		if (FD_ISSET(client, &fds)) {
			char buffer[512];
			ssize_t msgSize = read(client, buffer, sizeof(buffer));
			if (msgSize <= 0) {
				printf("Connection %d closed\n", client);
				close(client);
				occupied = false;
			}
			else printf("Received %zd byte from %d\n", msgSize, client);
		}
	}

	close(serverSocket);
	return(0);
}
