#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include <direct.h>

#include "hashlist.h"
#include "server.h"
#include "console.h"

int main(void) {

	ReadINI(&config);
	_mkdir(STORAGES_DIR);

	WSADATA wsadata = {0};
	WSAAssert(WSAStartup(WINSOCK_VERSION, &wsadata) != SOCKET_ERROR);
	printinfo1("SERVER START");

	SOCKADDR_IN serversockaddr = {0};
	serversockaddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serversockaddr.sin_family = AF_INET;
	serversockaddr.sin_port = htons(config.port);

	SOCKET serversock = socket(serversockaddr.sin_family, SOCK_STREAM, IPPROTO_TCP);
	WSAAssert(serversock != INVALID_SOCKET);

	BOOL reusesocketval = TRUE;
	DWORD rcvtimeoutval = 1000;
	TIMEVAL seltimeout = {0, 100};
	int sockaddrlen = sizeof(SOCKADDR_IN);

	WSAAssert(setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, cast(char*, &reusesocketval), sizeof(BOOL)) != SOCKET_ERROR);
	WSAAssert(bind(serversock, cast(const PSOCKADDR, &serversockaddr), sockaddrlen) != SOCKET_ERROR);
	WSAAssert(listen(serversock, SOMAXCONN) != SOCKET_ERROR);

	printinfo3("Server listen in %s:%d", inet_ntoa(serversockaddr.sin_addr), htons(serversockaddr.sin_port));
	
	fd_set socketsset = {0};
	CLIENTSTB activeclients = {0};

	for (;;) {
		fd_set socketsclone = socketsset;
		FD_ZERO(&socketsset);
		FD_SET(serversock, &socketsset);

		int selectres = select(0, &socketsset, NULL, NULL, &seltimeout);
		WSAAssert(selectres != SOCKET_ERROR);

		if (selectres > 0) {
			SOCKADDR_IN clientsockaddr;
			SOCKET clientsock = accept(serversock, cast(const PSOCKADDR, &clientsockaddr), cast(int*, &sockaddrlen));
			WSAAssert((clientsock != INVALID_SOCKET) && (clientsock != SOCKET_ERROR));
			printinfo3("Connected to %s:%d", inet_ntoa(clientsockaddr.sin_addr), htons(clientsockaddr.sin_port));

			WSAAssert(setsockopt(clientsock, SOL_SOCKET, SO_RCVTIMEO, cast(char*, &rcvtimeoutval), sizeof(DWORD)) != SOCKET_ERROR);
			if (socketsset.fd_count < MAX_LISTENERS) {
				FD_SET(clientsock, &socketsset);
			} else {
				printwarn1("Server full.. request rejected\n");
				SendResponse(clientsock, ResponseStatus(ERR_SERVICE_UNAVAILABLE));
			}
		}

		for (size_t i = 1; i < socketsclone.fd_count; i++) {
			FD_SET(socketsclone.fd_array[i], &socketsset);
		}
		
		FilterOutdatedClients(&activeclients);

		#define removesock() \
			FD_CLR(cursocket, &socketsset); closesocket(cursocket)

		for (size_t i = 1; i < socketsset.fd_count; i++) {
			//printf("%d\n", i);
			SOCKET cursocket = socketsset.fd_array[i];
			
			char buffer[BUFFER_LEN+1] = {0};
			size_t bufferlen = recv(cursocket, buffer, BUFFER_LEN, RECV_FLAGS);
			WSAAssert(bufferlen != SOCKET_ERROR);
			if (WSAGetLastError() == WSAETIMEDOUT) {
				removesock();
				continue;
			}

			if (bufferlen == BUFFER_LEN) {
				printwarn2("request size more than %d", BUFFER_LEN);
				SendResponse(cursocket, ResponseStatus(ERR_BAD_REQUEST));
				removesock();
			} else if (bufferlen > 0) {
				RQSTHEADERS headers = ParseRequest(buffer);
				ENUMT checkStatus = CheckRequestMeta(headers, cursocket);
				if (checkStatus == SUCCESS) {
					ENUMT actionStatus = DoAction(headers, &activeclients, cursocket);
					if (actionStatus != SUCCESS) { removesock(); }
				} else
					removesock();
				continue;
			}
			removesock();
		}
		#undef removesock
	}
	return SUCCESS;
}

void FilterOutdatedClients(CLIENTSTB *clients) {
	for (size_t i = 0; i < clients->len; i++) {
		CLIENT client = clients->buff[i];
		time_t curtime = time(0);
		uint16_t timediff = cast(uint16_t, difftime(curtime, client.lastrqst));
		if (timediff > MAX_DATING_TIME) {
			char *token; STRPrint(token, client.token);
			printinfo2("client %s disconnected", token);
			free(token);
			TBUFFDel(clients, i, CLIENT);
			i--;
		}
	}
}

ENUMT DoAction(RQSTHEADERS headers, CLIENTSTB *clients, SOCKET sock) {
	CLIENT *client = NULL;
	for (size_t i = 0; i < clients->len; i++) {
		CLIENT *curclient = clients->buff + i;
		if (strcmp(curclient->token.p, headers.token.p) == 0) {
			client = curclient;
			break;
		}
	}
	switch (headers.action) {
		case ACT_HANDSHAKE: {
			if (client == NULL) {
				CLIENT clientobj = {0};
				clientobj.token = headers.token;
				if (clients->len == MAX_CLIENTS) {
					SendResponse(sock, ResponseStatus(ERR_SERVICE_UNAVAILABLE));
					return SEND_ERROR;
				}
				client = clients->buff + clients->len;
				TBUFFAdd(clients, clientobj);

				char *token; STRPrint(token, headers.token);
				printinfo2("client %s added", token);
				free(token);
			}
			client->storage = headers.d.storage;
			SendResponse(sock, ResponseStatus(SUCCESS_CREATED));
		}
		case ACT_PUT: {

		}
	}
	if (client) {
		client->lastrqst = time(0);
	}
	return SUCCESS;
}

ENUMT CheckRequestMeta(RQSTHEADERS headers, SOCKET sock) {
	#define nasserth(e, err) if (e) { SendResponse(sock, ResponseStatus(err)); return SEND_ERROR; }
	
	nasserth(headers.useragent.p == NULL, ERR_BAD_REQUEST);
	nasserth(headers.action == ACT_UNKNOWN, ERR_BAD_REQUEST);
	nasserth(headers.token.p == NULL, ERR_BAD_REQUEST);
	nasserth(headers.method == METHOD_UNKNOWN, ERR_BAD_REQUEST);

	if (headers.method != METHOD_HEAD) {
		nasserth(headers.d.key.p == NULL, ERR_BAD_REQUEST);
	} else {
		nasserth(headers.d.storage.p == NULL, ERR_BAD_REQUEST);
	}
	nasserth(strncmp(headers.useragent.p, "RobloxStudio", sizeof("RobloxStudio")-1) != 0, ERR_BAD_REQUEST);

	#undef nasserth
	return SUCCESS;
}

RQSTHEADERS ParseRequest(char *buffer) {
	RQSTHEADERS headers = {0};
	#define findo(b,c,o) (strchr(b,c)+o)

	char *start = buffer;
	char *buffend = findo(start, '\0', 0);

	// TODO: get http method
	STRVAL method = stro(start, findo(start, 32, 0)-start);
	#define ismethod(s) (strncmp(method.p, s, method.len) == 0)
	if (ismethod("POST")) {
		headers.method = METHOD_POST;
	} else if (ismethod("HEAD")) {
		headers.method = METHOD_HEAD;
	} else if (ismethod("GET")) {
		headers.method = METHOD_GET;
	} else {
		headers.method = METHOD_UNKNOWN;
	}
	#undef ismethod

	HASHLIST headerstable;
	HashListCreate(&headerstable);

	do {
		start = findo(start, Cr, 2);
		if (*start == Cr) {
			if (headers.method == METHOD_POST) {
				start += 2;
				STRVAL body = stro(start, buffend-start);
				headers.body = body;
			}
			break;
		}
		char *headerend = findo(start, Cr, 0);
		char *valstart = findo(start, ':', 2);

		STRVAL name = stro(start, valstart-start-2);
		STRVAL *valuestr = ARRAlloc(STRVAL, 1);
		valuestr->p = valstart;
		valuestr->len = headerend-valstart;

		TYPEOBJECT value = typeo(valuestr, DT_STRING);
		HashSetVal(&headerstable, name, value);
	} while (1);

	// TODO: get request action
	HASHSTRVAL *actionval; 
	HashIndexing(&headerstable, "Action", actionval);
	if (actionval) {
		STRVAL *actionstr = actionval->obj.data;
		#define isaction(s) (strncmp(actionstr->p, s, actionstr->len) == 0)
		if (isaction("HANDSHAKE")) {
			headers.action = ACT_HANDSHAKE;
		} else if (isaction("PUT")) {
			headers.action = ACT_PUT;
		} else if (isaction("DELETE")) {
			headers.action = ACT_DELETE;
		} else {
			headers.action = ACT_UNKNOWN;
		}
		#undef isaction
	}

	#define gethashval(key) HASHSTRVAL* val; HashIndexing(&headerstable, key, val); if (val)
	#define setstrval(key, skey) { gethashval(key) { headers.skey = *(cast(STRVAL*, val->obj.data)); } }

	setstrval("Token", token);
	setstrval("User-Agent", useragent);
	if (headers.method == METHOD_HEAD) {
		setstrval("Storage", d.storage);
	} else {
		setstrval("Key", d.key);
	}

	#undef gethashval
	#undef setstrval
	HashListClean(&headerstable);

	#undef findo
	return headers;
}

ENUMT SendResponse(SOCKET sock, char *response) {
	size_t responselen = strlen(response);
	int32_t sentlen;

	while ((sentlen = send(sock, response, responselen, SEND_FLAGS)) < responselen) {
        if (sentlen < 1) { 
            closesocket(sock);
            break; 
        }
    }
	WSAAssert(sentlen != SOCKET_ERROR);

	if (sentlen == 0) {
		printwarn1("Client closed connection");
        return CLIENT_CLOSE;
	}
	return SUCCESS;
}

byte LogLvlInt(char *logstr) {
	#define isa(a) (strcmp(logstr, a) == 0)
	if (isa(LOGLVL_INFO_STR)) 
		return LOGLVL_INFO;
	else if (isa(LOGLVL_WARN_STR)) 
		return LOGLVL_WARN;
	else if (isa(LOGLVL_FAIL_STR)) 
		return LOGLVL_FAIL;
	else if (isa(LOGLVL_NULL_STR)) 
		return LOGLVL_NULL;
	return LOGLVL_NULL;
	#undef isa
}

void WSAPanic() {
	printfail2("WSA Error: %d", WSAGetLastError());
	WSACleanup();
	exit(EXIT_FAILURE);
}
void Panic() {
	printfail2("Error: %lu", GetLastError());
	exit(EXIT_FAILURE);
}