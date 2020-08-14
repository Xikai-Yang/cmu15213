#include <stdio.h>
#include "csapp.h"
#include "cache.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);

int parse_http_version(char *version)
{
    
    
    char version_old[MAXLINE] = "HTTP/1.0";
    char version_new[MAXLINE] = "HTTP/1.1";
    if (strcasecmp(version, version_old) == 0) {
        strcpy(version, version_old);
        return 0;    
    } else if (strcasecmp(version, version_new) == 0) {
        strcpy(version, version_old);
        return 0;
    } else if (strcasecmp(version, "HTTP/1.1\r") == 0){
        strcpy(version, version_old);
        return 0;
    } else if (strcasecmp(version, "HTTP/1.0\r") == 0) {
        strcpy(version, version_old);
        return 0;
    } else {
        return -1;
    }
}

void parse_uri(char *uri, char *host, char *port, char *filename)
{

    char *start = strstr(uri, "//");
    if (start != NULL) {
        start += 2;
    } else {
        start = uri;
    }

    char *port_start = strstr(start, ":");
    char *port_end = NULL;
    if (port_start == NULL) {
        port_end = strstr(start, "/");
        strcpy(port, "80");
        strncpy(host, start, (port_end - start));
    } else {
        port_end = strstr(port_start, "/");
        int digits = port_end - port_start - 1;
        strncpy(port, port_start + 1, digits);
        strncpy(host, start, (port_start - start));
    }

    strcpy(filename, port_end);

}

void parse_request(int fd, char *host, char *port, char *request)
{
    //int is_static;
    //struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char proxy_request[MAXLINE];
    // char host[MAXLINE], port[MAXLINE];
    char filename[MAXLINE]; 
    // char cgiargs[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))  //line:netp:doit:readrequest
        return;
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }   
    if (parse_http_version(version) < 0) {
        
        clienterror(fd, version, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }
    parse_uri(uri, host, port, filename);

    strcpy(proxy_request, "GET ");
    strcat(proxy_request, filename);
    strcat(proxy_request, " ");
    strcat(proxy_request, version);
    strcat(proxy_request, "\r\n");
    strcpy(request, proxy_request);

    
    int anyHost = 0;
    while (strcmp(buf,"\r\n")) {
        Rio_readlineb(&rio, buf, MAXLINE);
        if (strstr(buf, "Host") != NULL) {
            anyHost = 1;
        }
        strcat(request, buf);
    }
    char *temp_end = strstr(request, "\r\n\r\n");
    *(temp_end + 2) = '\0';
    *(temp_end + 3) = '\0';

    if (anyHost == 0) {
        char host_buffer[MAXLINE];
        strcpy(host_buffer, "Host: ");
        strcat(host_buffer, host);
        strcat(host_buffer, "\r\n");
        strcat(request, host_buffer);
    }

    strcat(request, user_agent_hdr);
    strcat(request, "Connection: close\r\n");
    strcat(request, "Proxy-Connection: close\r\n");
    strcat(request, "\r\n");
    //printf("host : %s\nport : %s\n%s", host, port, request);
}


void *thread(void *vargp);


int main(int argc, char **argv)
{
    
    int listenfd;
    //int connfd;
    //int proxy_clientfd;
    char hostname[MAXLINE], port[MAXLINE];
    //char request[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    listenfd = Open_listenfd(argv[1]);  // have to deal with errors!
    init();
    while (1) {
        pthread_t tid;
        clientlen = sizeof(clientaddr);
        int *connfdp = Malloc(sizeof(int));
	    *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        Pthread_create(&tid, NULL, thread, (void *)connfdp);
    }

    return 0;
}

void *thread(void *vargp)
{
    pthread_detach(pthread_self());
    int connfd = *(int *)(vargp);
    Free((int *)vargp);
    int proxy_clientfd = 0;
    char hostname[MAXLINE], port[MAXLINE];
    char request[MAXLINE];

    parse_request(connfd, hostname, port, request);
    // check if cached
    char cache_buffer[MAX_OBJECT_SIZE];
    printf("READER BEGINS!\n");
    int cache_length = reader(request, cache_buffer);
    printf("READER ENDS!\n");
    if (cache_length > 0) {
        Rio_writen(connfd, cache_buffer, cache_length);
        return NULL;
    }
    

    proxy_clientfd = Open_clientfd(hostname, port);
        
    int transfer_length = strstr(request, "\r\n\r\n") - request + 4;
    // printf("length is %d\n", transfer_length);
    Rio_writen(proxy_clientfd, request, transfer_length);
    char read_buffer[MAXLINE];
    rio_t read_rio;
    Rio_readinitb(&read_rio, proxy_clientfd);
    int content_length;
    char content[MAX_OBJECT_SIZE];
    int n;
    while ((n = Rio_readlineb(&read_rio, read_buffer, MAXLINE)) > 0) {
        content_length += n;
        if (content_length < MAX_OBJECT_SIZE) {
            strcat(content, read_buffer);
        }
        Rio_writen(connfd, read_buffer, n);
    }
    if (content_length < MAX_OBJECT_SIZE) {
        // write cache
        writer(request, content, content_length);
    }
    Close(connfd);
    Close(proxy_clientfd);
    return NULL;

}


/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
/* $end clienterror */