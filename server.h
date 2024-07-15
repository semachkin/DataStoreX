#pragma once

#define Cr '\r'
#define Lf '\n'

#define Assert(e, pf) (e?SUCCESS:pf())
#define ErrnoAssert(e) Assert(e, Panic)
#define WSAAssert(e) Assert(e, WSAPanic)

#define LOGLVL_INFO_STR "INFO"
#define LOGLVL_WARN_STR "WARN"
#define LOGLVL_FAIL_STR "FAIL"
#define LOGLVL_NULL_STR "NULL"

#define MAX_LISTENERS 16
#define MAX_CLIENTS 64

#define MAX_DATING_TIME 2

#define MAX_STORAGE_KEY_SIZE 0xA00000 // 10 MiB

#define DEFAULT_PORT 1922
#define DEFAULT_LOGFILE "console.log"
#define DEFAULT_LOGLVL "INFO"

#define STORAGES_DIR "storages"

#define BUFFER_LEN 1024

#define RECV_FLAGS 0
#define SEND_FLAGS MSG_DONTROUTE

#define CONFIG_INI cast(LPCSTR, "./config.ini")
#define CONFIG_INI_APP "Server"

#define ReadINI(cnfgp) \
	GetINIInt((cnfgp)->port, "Port", DEFAULT_PORT); \
	GetINIStr((cnfgp)->logfile, "LogFile", DEFAULT_LOGFILE); \
	GetINIStr((cnfgp)->loglvl, "LogLevel", DEFAULT_LOGLVL); \
	(cnfgp)->loglvlint = LogLvlInt((cnfgp)->loglvl);

#define GetINIInt(v, k, d) (v = GetPrivateProfileIntA(CONFIG_INI_APP, k, d, CONFIG_INI))
#define GetINIStr(p, k, d) { \
		int res = GetPrivateProfileStringA(CONFIG_INI_APP, k, d, p, sizeof(p)-1, CONFIG_INI); \
		ErrnoAssert((res != sizeof(p)-2) && (res != sizeof(p)-3) && (errno != ENOENT)); \
	}

#define SUCCESS_OK "200 OK"
#define SUCCESS_CREATED "201 Created"
#define SUCCESS_NO_CONTENT "204 No Content"
#define ERR_BAD_REQUEST "400 Bad Request"
#define ERR_UNAUTHORIZED "401 Unauthorized"
#define ERR_FORBIDDEN "403 Forbidden"
#define ERR_NOT_FOUND "404 Not Found"
#define ERR_INTERNAL_SERVER_ERR "500 Internal Server Error"
#define ERR_NOT_IMPLEMENTED "501 Not Implemented"
#define ERR_BAD_GATEWAY "502 Bad Gateway"
#define ERR_SERVICE_UNAVAILABLE "503 Service unavailable"
#define ERR_TIMEOUT "504 Gateway Timeout"
#define ERR_INSUFICIENT "507 Insufficient Storage"

#define ResponseStatus(s) ("HTTP/1.1 "s"\r\n\r\n")
#define ResponseFormat(pt, s, h, b) (sprintf(pt, "HTTP/1.1 %s\r\n%s\r\n\r\n%s", s, h, b))

#define TBUFF(t, l) struct {uint32_t len; t buff[l];}

#define TBUFFAdd(b, o) { \
		b->buff[b->len] = o; \
		b->len++; \
	}
#define TBUFFDel(b, i, t) { \
		for (size_t j = i; j < b->len; j++) { \
			b->buff[j] = b->buff[j+1]; \
		} \
		b->len--; \
	}

typedef unsigned char ENUMT;

typedef struct SRVCONFIG_S {
	WORD port;
	WORD loglvlint;
	char loglvl[MAX_PATH];
	char logfile[MAX_PATH];
} SRVCONFIG;

typedef struct CLIENT_S {
	char *token;
	char *storage;
	time_t lastrqst;
} CLIENT;

typedef TBUFF(CLIENT, MAX_CLIENTS) CLIENTSTB;

#pragma pack(push, 2)
typedef struct RQSTHEADERS_S {
	STRVAL useragent;
	STRVAL body;
	STRVAL token;
	union {
		STRVAL key;
		STRVAL storage;
	} d;
	ENUMT method;
	ENUMT action;
} RQSTHEADERS;
#pragma pack(pop)

enum LOGLVL {
	LOGLVL_NULL,
	LOGLVL_FAIL,
	LOGLVL_WARN,
	LOGLVL_INFO
};
enum STATUS {
	SUCCESS,
	SEND_ERROR,
	CLIENT_CLOSE,
	SEND_BODY,
	FILE_TOO_BIG
};
enum HTTPMETHOD {
	METHOD_UNKNOWN,
	METHOD_POST,
	METHOD_GET,
	METHOD_HEAD
};
enum SRCACTION {
	ACT_UNKNOWN,
	ACT_HANDSHAKE,
	ACT_PUT,
	ACT_DELETE,
	ACT_GET
};
enum DATATYPE {
	DT_NULL,
	DT_STRING,
	DT_NUMBER
};

void WSAPanic();
void Panic();

byte LogLvlInt(char *lvlstr);
void FilterOutdatedClients(CLIENTSTB *clients);
ENUMT SendResponse(SOCKET sock, char *response);
RQSTHEADERS ParseRequest(char *buffer);
ENUMT CheckRequestMeta(RQSTHEADERS headers, SOCKET sock);
ENUMT DoAction(RQSTHEADERS headers, CLIENTSTB *clients, SOCKET sock);
ENUMT EditStorage(RQSTHEADERS *headers, char *keydir);

static SRVCONFIG config;
