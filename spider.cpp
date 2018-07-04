#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
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

queue <string> gquiz;
char *raiz;

/* Get the web page and print it to standard output. */
int get_mensagem (char *host)
{
    char buffer[65535];
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    int idSocket;
    FILE *fp1;
    //Obtem informações de endereço para o soquete de fluxo na porta de entrada
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, "http", &host_info, &host_info_list) != 0) {
        fprintf(stderr," Nao pode obter a pagina >> ");
        printf("%s\n", host);
        //exit (1);
        return 1;
    }
    //cria um socket
    if ((idSocket = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol)) < 0) 
    {
        fprintf(stderr," Erro ao criar socket para o servidor! O programa foi encerrado\n");
        return 1;
    }
    //faz a conecção
    if (connect(idSocket, host_info_list->ai_addr, host_info_list->ai_addrlen) < 0)
    {
        fprintf(stderr," Erro ao tentar conectar o servidor! O programa foi encerrado\n");
        return 1;
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
    int bytesRecv;
    fp1 = fopen("index.html","w");
    while ((bytesRecv = recv(idSocket, buffer, sizeof (buffer), 0)) > 0) {
        fprintf(fp1,"%s", buffer);         
    } 
    fclose(fp1);
    return 0;
}
void pause (float delay1) {

   if (delay1<0.001) return; // pode ser ajustado e/ou evita-se valores negativos.

   float inst1=0, inst2=0;

   inst1 = (float)clock()/(float)CLOCKS_PER_SEC;

   while (inst2-inst1<delay1) inst2 = (float)clock()/(float)CLOCKS_PER_SEC;

   return;

}
void showq(queue <string> gq)
{
    queue <string> g = gq;
    size_t found1, found2, found3, found4, found5, found6;
    while (!g.empty())
    {
        string aux = g.front();
        found1 = aux.find("secom/favicon.ico");
        found2 = aux.find("bootstrap");
        found3 = aux.find("<img src=");
        found4 = aux.find("drupal");
        found5 = aux.find(":");
        found6 = aux.find(".css");
        if(found1==std::string::npos && found2==std::string::npos && found3==std::string::npos && found4==std::string::npos && found5==std::string::npos && found6==std::string::npos && aux.size() > 17)
        {
            printf("\t\t\t");
            printf("-- %s %s-- ", __DATE__,__TIME__);
            cout << g.front() << '\n';
            pause(1);
        }
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
    /** TODO: corrigir a extração dos links, pois possue links quebrados
    **/
    queue <string> aux;
    char dados[5000], ABC;
    string line, url, hostname;
    FILE *fp1;
    int i, n, j, flag;
    fp1 = fopen("index.html","r");

    if(fp1 == NULL)
    {
        printf("Falha ao abrir o arquivo\n");
        cout << host;
        cout << '\n';
        exit(1);
    }else{
        while(fgets(dados,5000, fp1) != NULL) //le o arquivo de entrada
        {
            line = dados;
            size_t pos = line.find("href=");
            if (pos!=std::string::npos)
            {
                string str1 = line.substr(pos+5);
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
                        {
                            string temp = host;
                            size_t pos3 = temp.find("www.");
                            if(pos3!=std::string::npos)
                            {
                                string domain = temp.substr(pos3+4);
                                //cout << domain;cout << '\n'; getchar();
                                size_t pos4 = hostname.find(domain);
                                if(pos4!=std::string::npos)
                                {
                                    gquiz.push(hostname);
                                    aux.push(hostname);
                                    //cout << hostname;cout << '\n';
                                }
                            }
                        }
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
                        if(compara(gquiz,str5) != 1) {
                            gquiz.push(str5);
                            aux.push(str5);
                        }
                        //cout << str5;cout << '\n';
                    }
                }
                //cout << url;cout << '\n';getchar();
            }
            //cout << line;cout << '\n';getchar();
            pos = 0;
            pos = line.find("<img src=");
            if (pos!=std::string::npos)
            {
                string str6 = line.substr(pos+8);
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
                    if(compara(gquiz,str8) != 1) {
                        gquiz.push(str8);
                        aux.push(str8);
                    }
                    //cout << str8;cout << '\n';getchar();
                }
            }
        }
    }
    fclose(fp1);
    printf("-- %s %s-- ", __DATE__,__TIME__);
    cout << host;cout << " >> ";cout << '\n';
    showq(aux);
}
void recursivo(queue <string> gq)
{
    queue <string> g = gq;
    string path, file;
    while (!g.empty())
    {
        string str = g.front();
        size_t found = str.find_last_of("/");
        char nome[5000];
        strcpy(nome, str.c_str());
        get_mensagem (nome);
        extrai_urls(nome);
        g.pop();
    }
}
int main (int argc, char *argv[])
{
    /* Get one of the web pages here. */
    char * host = argv[1];
    raiz = argv[1];
    get_mensagem(host);
    extrai_urls(host);
    recursivo(gquiz);
    destroi(gquiz);
    return 0;
}