#ifdef WIN32
    #include <winsock2.h>
    #include <winbase.h>
	#include <ws2tcpip.h>
    #define errno WSAGetLastError()
#else
    #include <sys/socket.h>
	#include <sys/types.h>
	#include <netdb.h>
	#include <errno.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <zlib.h>
#include <assert.h>
#include <string.h>

#include "dds.h"

#define DDSHEADER "FAF0"
#define CHUNK 65536L /* 64K */

int conn = 0;
unsigned int version = 0;

int inf(unsigned char *source, unsigned int srcSize,
        unsigned char **dest, unsigned long *destSize);

SERVERCODE DDSConnectAssert(char* server, char* port, char* user,
                            DDSRESPONSE* resp) {
    /*struct hostent* host;
    struct sockaddr_in addr;*/
	struct addrinfo hints;
	struct addrinfo *result, *rp;
    int s;
    SERVERCODE sc;

#ifdef WIN32
    unsigned long mode = 0;
    WSADATA WsaDat;
#endif
    /* TODO: Handle socket creation errors*/
    if (!conn) {
#ifdef WIN32
    	WSAStartup(MAKEWORD(2,2),&WsaDat);
#endif
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
		hints.ai_socktype = SOCK_STREAM; /* STREAM socket */
        hints.ai_flags = 0;    
        hints.ai_protocol = 0;          /* Any protocol */
        		
		s = getaddrinfo(server, port, &hints, &result);
		if (s) {
			/*ERROR*/
		}
        for (rp = result; rp != NULL; rp = rp->ai_next) {
			conn = socket(rp->ai_family, rp->ai_socktype,
			             rp->ai_protocol);
			if (conn == -1)
				continue;

			if (connect(conn, rp->ai_addr, rp->ai_addrlen) != -1)
				break;                  /* Success */
			resp->body = malloc(45);
            sprintf((char *)resp->body, "Socket error #%d", errno);
            resp->msgLen = strlen((char *)resp->body);
            
#ifdef WIN32
            WSACleanup();
			closesocket(conn);            
#else
			close(conn);
#endif
			conn = 0;
            return -1;
		}

		if (rp == NULL) {               /* No address succeeded */
			/*ERROR*/
		}

		freeaddrinfo(result);
		
		/*host = gethostbyname(server);
        if(!host){
            resp->body = malloc(45);
            sprintf((char *)resp->body, "Cannot resolve hostname %s.\nError #%d",
                    server, errno);
            resp->msgLen = strlen((char *)resp->body);
            closesocket(conn);
            conn = 0;
#ifdef WIN32
            WSACleanup();
#endif
            return -1;
        }
        addr.sin_port = htons(port);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = *((unsigned long*)host->h_addr);
        res = connect(conn, (struct sockaddr*)(&addr), sizeof(addr));
        if (res != 0) {

            resp->body = malloc(45);
            sprintf((char *)resp->body, "Socket error #%d", errno);
            resp->msgLen = strlen((char *)resp->body);
            closesocket(conn);
            conn = 0;
#ifdef WIN32
            WSACleanup();
#endif
            return -1;
        }*/
#ifdef WIN32
        ioctlsocket(conn, FIONBIO, &mode);
#endif
								
        if ((sc = DDSSendMessage(IdHello, user, resp))) {
#ifdef WIN32
            closesocket(conn);
            WSACleanup();
#else
			close(conn);
#endif
            conn = 0;
        }
        return sc;
    } else {
        resp->body = malloc(45);
        strcpy((char *)resp->body, "Socket has already been connected");
        resp->msgLen = strlen((char *)resp->body);
        return -1;
    }
}


SERVERCODE DDSDisconnect(DDSRESPONSE* resp) {
    SERVERCODE sc;
    if (!conn) {
        /*if (resp->body) free(resp->body);*/
        resp->body = malloc(45);
        strcpy((char *)resp->body, "Socket has not been connected");
        resp->msgLen = strlen((char *)resp->body);
        return -1;
    }
    sc = DDSSendMessage(IdGoodbye, NULL, resp);
#ifdef WIN32
	shutdown(conn, SD_BOTH);
    closesocket(conn);
    WSACleanup();
#else
	close(conn);
#endif
    conn = 0;

    return sc;
}

SERVERCODE DDSSendNetworkList(char *nl, DDSRESPONSE* resp){
    FILE* nlfp;
    SERVERCODE sc;
    char* nlContents, *t;
    struct stat s;
    nlfp = fopen(nl, "rb");
    if (!nlfp) {
        resp->body = malloc(45);
        sprintf((char *)resp->body, "Error opening file %s", nl);
        resp->msgLen = strlen((char *)resp->body);
        return -1;
    }

    fstat(fileno(nlfp), &s);
    t = malloc(s.st_size + 1);
    nlContents = malloc(s.st_size + 1 + 64);
#ifdef WIN32
    sprintf(nlContents, "%-64s", !strrchr(nl, '\\') ? nl : &((char *)strrchr(nl, '\\'))[1]);
#else
    sprintf(nlContents, "%-64s", !strrchr(nl, '/') ? nl : &((char *)strrchr(nl, '/'))[1]);
#endif
    fread(t, sizeof(char), s.st_size, nlfp);
    t[s.st_size] = '\0';
    memcpy(nlContents+64, t, s.st_size+1);
    fclose(nlfp);
    sc = DDSSendMessage(IdPutNetlist, nlContents, resp);
    if (nlContents) free(nlContents);
	if (t) free(t);
    return sc;
}

SERVERCODE DDSSendSearchCriteria(char* sc, DDSRESPONSE* resp){
    FILE* scfp;
    SERVERCODE sCode;
    char* scContents, *t;
    struct stat s;
    scfp = fopen(sc, "rb");
    if (!scfp) {
        resp->body = malloc(45);
        sprintf((char *)resp->body, "Error opening file %s", sc);
        resp->msgLen = strlen((char *)resp->body);
        return -1;
    }

    fstat(fileno(scfp), &s);
    t = malloc(s.st_size + 1);
    scContents = malloc(s.st_size + 1 + 50);
    sprintf(scContents, "%50s", " ");
    fread(t, sizeof(char), s.st_size, scfp);
    fread(t, sizeof(char), s.st_size, scfp);
    t[s.st_size] = '\0';
    memcpy(scContents+50, t, s.st_size+1);
    fclose(scfp);

    sCode = DDSSendMessage(IdCriteria, scContents, resp);

    if (scContents) free(scContents);
	if (t) free(t);
    return sCode;
}

SERVERCODE DDSGetDcpMessage(DDSRESPONSE* resp) {
    return DDSSendMessage(IdDcp, NULL, resp);
}

SERVERCODE DDSGetMultipleDcpMessages(DDSRESPONSE *resp) {
    char *buff;
    int size = 0;
    SERVERCODE sc;

    buff = malloc(1);
    while ((sc = DDSSendMessage(IdDcpBlock, NULL, resp)) != DUNTIL) {
        if (sc) {
            return sc;
        }
        buff = realloc(buff, (size += resp->msgLen));
        memcpy(&buff[size - resp->msgLen], resp->body, resp->msgLen);
    }
    resp->body = (unsigned char *) realloc(resp->body, size);
    memcpy(resp->body, buff, size);
    resp->msgLen = size;
    resp->serverCode = DSUCCESS;
	free(buff);
    return resp->serverCode;
}

SERVERCODE DDSGetMultipleDcpMessagesExtended(DDSRESPONSE *resp) {
    unsigned char *buff;
    int size = 0;
    SERVERCODE sc;
    FILE* fp;

    buff = malloc(1);
    while ((sc = DDSSendMessage(IdDcpBlockExt, NULL, resp)) != DUNTIL) {
        if (sc) {
            return sc;
        }
        buff = realloc(buff, (size += resp->msgLen));
        memcpy(&buff[size - resp->msgLen], resp->body, resp->msgLen);
		free(resp->body);
    }
    if (resp->body) free(resp->body);
    /*resp->body = malloc(102400);*/
    /*ret = uncompress(resp->body, &destlen, buff, size);*/
    inf(buff, size, &resp->body, &(resp->msgLen));
    /*resp->body = buff;
    resp->msgLen = size;
    resp->serverCode = DSUCCESS;*/
    if ((fp = fopen("out.xml", "wb"))) {
        fwrite(resp->body, 1, resp->msgLen, fp);
        fflush(fp);
        fclose(fp);
    }
    /*resp->body = malloc(10);
    strcpy(resp->body, "Written");
    resp->msgLen = 7;*/
	free(buff);
    return resp->serverCode;
}

SERVERCODE DDSSendMessage(TYPECODE tc, char* msg, DDSRESPONSE* resp) {
    char* m;
    char *t = malloc(1025);
    char *r = malloc(10101);
    int sz = 0, n;
    if (!conn) {
        /*if (resp->body) free(resp->body);*/
        resp->body = malloc(45);
        strcpy((char *)resp->body, "Socket has not been connected");
        resp->msgLen = strlen((char *)resp->body);
		free(r);
		free(t);
        return -1;
    }

    m = malloc((msg ? strlen(msg) : 0) + strlen(DDSHEADER) + 7);
    sprintf(m, "%s%c%05u%s",
                DDSHEADER,
                tc,
                (msg ? strlen(msg) : 0),
                (msg ? msg : "\0"));

    n = send(conn, m, strlen(m), 0);
    /*while((sz = recv(conn, r, 10100, MSG_PEEK)) == -1 &&
          errno == WSAEWOULDBLOCK);*/

    do {
        sz = recv(conn, r, 10100, MSG_PEEK);
        strncpy(t, &r[5], 5);
        t[5] = '\0';
    } while (sz < atoi(t) && sz != -1);
	if (sz == -1) {
		/*HANDLE ERRORS*/
		resp->body = malloc(45);
        sprintf((char *)resp->body, "Socket error #%d", errno);
        resp->msgLen = strlen((char *)resp->body);
		free(m);
		free(r);
		free(t);
        return -1;
	}
	free(r);
    r = malloc(sz+1);
    if((sz = recv(conn,r,sz,0)) < 0) {
        /*TODO: HANDLE ERRORS*/
		
    }
    strncpy(resp->sync, r, 4);
    resp->typeCode = r[4];
    if (tc == IdDcp && r[10] != '?') {
        strncpy(t, &r[5], 5);
        t[5] = '\0';
        resp->msgLen = atoi(t) - 40;
        resp->body = malloc(resp->msgLen);
        memcpy(resp->body, &r[50], resp->msgLen);
    } else {
        strncpy(t, &r[5], 5);
        t[5] = '\0';
        resp->msgLen = atoi(t);
		if (resp->msgLen) {
	        resp->body = malloc(resp->msgLen);
		} else {
			resp->body = NULL;
		}
        memcpy(resp->body, &r[10], resp->msgLen);
    }
	free(r);
    free(m);
    free(t);



    if (resp->body && (resp->body)[0] == '?') 
	{
        t = malloc(resp->msgLen + 1);
        sscanf((char *)resp->body, "?%u,%u,%n",
               &(resp->serverCode), &(resp->systemCode), &n);
        strncpy(t, (char *)(&(resp->body[n])), resp->msgLen - n);
        t[resp->msgLen - n] = '\0';
        strcpy((char *)resp->body, t);
        resp->msgLen = resp->msgLen - n;
        if (t) free(t);
        /*return resp->serverCode;*/
    } else {
        resp->systemCode = DSUCCESS;
        resp->serverCode = DSUCCESS;

        /*return DSUCCESS;*/
    }
    return resp->serverCode;
}

int inf(unsigned char *source, unsigned int srcSize,
        unsigned char **dest, unsigned long *destSize)
{
    int ret;
    unsigned int have;
    z_stream strm;

    unsigned char out[CHUNK];
    unsigned int sz = 0;
    unsigned char *t;
    unsigned int left = srcSize;

    t = malloc(1);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, 16+MAX_WBITS); /* Decode GZIP, not ZLIB*/
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.next_in = (Bytef *)(&source[srcSize - left]);
        strm.avail_in = left > CHUNK ? ((left -= CHUNK), CHUNK) : srcSize;

        if (strm.avail_in == 0)
            break;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_SYNC_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            t = realloc(t, sz += have);
            memcpy(&t[sz - have], out, have);
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    /*if (*dest) free(*dest);*/
    *dest = t;
    *destSize = sz;
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
