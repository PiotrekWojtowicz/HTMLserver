/*
 *
 * It's a one-header basic HTML webserver
 * capable of serving static and dynamic
 * content, although no serious security.
 *
 * Using the GET method to serve content.
 *
 */
#ifndef WEBHTMLSERVER_WEBSERVER_H
#define WEBHTMLSERVER_WEBSERVER_H
//Include csapp.h as a framework
#include "csapp.h"

void my_handler(int a){
    printf("Closing the server deamon\n");
    _exit(EXIT_FAILURE);
}

void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];
    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
        }
    //This basically ignores the request headers as we don't use them
}

void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) { /* Static content */
        strcpy(cgiargs, "");
        strcpy(filename, "./text/html/");
        strcat(filename, uri);

        if (uri[strlen(uri)-1] == '/')
            strcat(filename, "home.html");
        return 1;
    }
    else { /* Dynamic content */
        ptr = index(uri, '?');

        if (ptr) {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");

        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}


void clienterror(int fd, char *cause, char *errnum,
char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

void serve_static(int fd, char *filename, int filesize){
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXLINE];

    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) { /* child */
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO); /* Redirect stdout to client */
        Execve(filename, emptylist, environ); /* Run CGI program */
    }
    Wait(NULL); /* Parent waits for and reaps child */
}

void Procced(int fd){
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];

    rio_t rio;

    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);
    if(strcasecmp(method, "GET")){
        clienterror(fd, method, "501", "NOT IMPLEMENTED", "Server does not implement this method");
        return;
    }
    //read_requesthdrs(&rio);

    is_static = parse_uri(uri, filename, cgiargs);
    if(stat(filename, &sbuf) < 0){
        clienterror(fd, method, "404", "NOT FOUND", "Could not find the file");
        return;
    }

    if(is_static){
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
            clienterror(fd, method, "403", "FORBIDDEN", "Could not read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }
    else{
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
            clienterror(fd, method, "403", "FORBIDDEN", "Could not read the file");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }

}

#endif //WEBHTMLSERVER_WEBSERVER_H
