#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <stdarg.h>
#include <iostream>
#include <queue>
#include <string>
#include <sstream>
using namespace std;

/* "BSIZE" is the size of the buffer we use to read from the socket. */

#define BSIZE 0x1000
FILE *fp;
char buf[BSIZE+10];
queue <string> gquiz;

void directory(char *host)
{
    struct stat st = {0};
    string name = host;
    size_t found = name.find_last_of("/");
    if (found!=std::string::npos)
    {
        string path = name.substr(0,found);
        istringstream iss(path);
        string token;
        int i = 0;
        while (getline(iss, token, '/'))
        {
            mkdir(token.c_str(), 0700);
            chdir(token.c_str());
            i++;
        }
        if(i == 0){
            mkdir(path.c_str(), 0700);
            chdir(path.c_str());
        }
    }
    char *temporary = strtok(host, "/");
    char *extension;
    int flag = 0;
    while (temporary != NULL)
    {
        if ((stat(temporary, &st) == -1) && (temporary != NULL))
        {
            mkdir(temporary, 0700);
            flag = 1;
        }
        chdir(temporary);
        extension = temporary;
        temporary = strtok(NULL, "/");
    }
    string file = name.substr(found+1);
    if(name.size() > found) fp = fopen(file.c_str(), "ab");
    else fp = fopen("text.html", "a+");
}

/* Get the web page and print it to standard output. */

void get_page (char *host)
{
    char buffer[65535];
    int bytesRecv;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    int idSocket;
    //Obtem informações de endereço para o soquete de fluxo na porta de entrada
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, "http", &host_info, &host_info_list) != 0) {
        fprintf(stderr," Erro no formato do endereco do servidor! O programa foi encerrado\n");
        exit (1);
    }
    //cria um socket
    if ((idSocket = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol)) < 0) 
    {
        fprintf(stderr," Erro ao criar socket para o servidor! O programa foi encerrado\n");
        exit (1);
    }
    //faz a conecção
    if (connect(idSocket, host_info_list->ai_addr, host_info_list->ai_addrlen) < 0)
    {
        fprintf(stderr," Erro ao tentar conectar o servidor! O programa foi encerrado\n");
        exit (1);
    }
    freeaddrinfo(host_info_list);

    /* "format" is the format of the HTTP request we send to the web
       server. */
    char format[1000];
    memset(format,0,sizeof (format));
    strcpy(format,"GET / HTTP/1.1\r\nHost:");
    strcat(format,host);
    strcat(format,"\r\nConnection: close\r\n\r\n");
    /* Send the request. */
    int bytesSend;
    bytesSend = send (idSocket, format, strlen (format), 0);
    /* Recv data */
    memset(buffer,0,sizeof (buffer));  
    while ((bytesRecv = recv(idSocket, buffer, sizeof (buffer), 0)) > 0) {
        fprintf(fp,"%s", buffer);         
    } 
    fclose(fp);
}

void showq(queue <string> gq)
{
    queue <string> g = gq;
    while (!g.empty())
    {
        cout << '\n' << g.front();
        g.pop();
    }
    cout << '\n';
}

int compara(queue <string> gq, string name)
{
    queue <string> g = gq;
    while (!g.empty())
    {
        string str = g.front();
        if(str.compare(name) != 0) g.pop();
        else return 1; //retorna 1 se for igual
    }
    return 0;
}

void destroi(queue <string> gq)
{
    while (!gq.empty())
    {
        gq.pop();
    }
}

void extrai_urls(char *host)
{
    char arqname[5000], dados[5000], ABC;
    string buffer, url, hostname;
    FILE *fp1;
    int i, n, j, flag;
    strcpy(arqname,"/home/sala/Trab-TR2/");
    strcat(arqname,host);
    strcat(arqname,"/text.html");
    fp1 = fopen(arqname,"r");

    if(fp1 == NULL)
    {
        printf("Falha ao abrir o arquivo\n");
        exit(1);
    }else{
        while(fgets(dados,5000, fp1) != NULL) //le o arquivo de entrada
        {
            buffer = dados;
            size_t pos = buffer.find("href=");
            if (pos!=std::string::npos)
            {
                string str1 = buffer.substr(pos+5);
                pos = str1.find(" ");
                url = str1.substr(0,pos);
                size_t pos1 = url.find_first_of('"');
                size_t pos2 = url.find_last_of('"');
                if (pos1!=std::string::npos && pos2!=std::string::npos)
                {
                    string str2 = url.substr(pos1, pos2);
                    pos = str2.find("http://");
                    if (pos!=std::string::npos)
                    {
                        hostname = str2.substr(pos+7);
                        pos2 = hostname.find_last_of('/');
                        if(pos2!=std::string::npos) 
                            hostname = hostname.substr(0,pos2);
                        if (hostname.compare(host) != 0 && compara(gquiz,hostname) != 1)
                            gquiz.push(hostname);
                        //cout << hostname;cout << '\n';
                        /**
							Na função extrai urls, TO DO:
							verificar para cada link inserido na lista se ele pertence ao dominio do site host
						**/
                    }
                }
                pos1 = url.find_first_of('>');
                if (pos1!=std::string::npos)
                {
                    string str3 = url.substr(0, pos1);
                    pos1 = str3.find_first_of('"');
                    pos2 = str3.find_last_of('"');
                    if (pos1!=std::string::npos && pos2!=std::string::npos)
                    {
                        string str4 = str3.substr(pos1+1, pos2);
                        string str5 = host + str4;
                        if(compara(gquiz,str5) != 1)
                            gquiz.push(str5);
                        //cout << host;cout << str4;cout << '\n';getchar();
                    }
                }
                //cout << url;cout << '\n';getchar();
            }
            //cout << buffer;cout << '\n';getchar();
            pos = 0;
            pos = buffer.find("<img src=");
            if (pos!=std::string::npos)
            {
                string str6 = buffer.substr(pos+8);
                pos = str6.find(" ");
                str6 = str6.substr(0,pos);
                size_t pos1 = str6.find_first_of('"');
                size_t pos2 = str6.find_last_of('"');
                if (pos1!=std::string::npos && pos2!=std::string::npos)
                {
                    string str7 = str6.substr(pos1+1, pos2+1);
                    pos = str7.find('"');
                    str7 = str7.substr(0,pos);
                    string str8 = host + str7;
                    if(compara(gquiz,str8) != 1) gquiz.push(str8);
                    //cout << host;cout << str7;cout << '\n';getchar();
                }
            }
        }
    }
    fclose(fp1);
    showq(gquiz);
}

int main (int argc, char *argv[])
{
    /* Get one of the web pages here. */
    char * host = argv[1];
    directory(host);
    get_page (host);
    extrai_urls(host);
    destroi(gquiz);
    return 0;
}
void recursivo(queue <string> gq)
{
    queue <string> g = gq;
    while (!g.empty())
    {
        string str = g.front();
        char nome[5000];
        strcpy(nome, str.c_str());
        directory(nome);
        get_page (nome);
	    size_t found = str.find_last_of("/");
	    string path = str.substr(0,found);
	    string file = str.substr(found+1);
	    if(str.size() > found) 
	    {
	    	cout << "download ";
	    	cout << str;
	    	cout << '\n';
	    }else{
	    	extrai_urls(nome);
	    	cout << "download ";
	    	cout << str;
	    	cout << "/text.html";
	    	cout << '\n';
	    }
        g.pop();
    }
}