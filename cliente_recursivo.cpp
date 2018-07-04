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
int num = 0, flag = 0;
char *raiz, flagStr[5000];

void recursivo(queue <string> gq);

void pause (float delay1) {

   if (delay1<0.001) return; // pode ser ajustado e/ou evita-se valores negativos.

   float inst1=0, inst2=0;

   inst1 = (float)clock()/(float)CLOCKS_PER_SEC;

   while (inst2-inst1<delay1) inst2 = (float)clock()/(float)CLOCKS_PER_SEC;

   return;

}

void directory(char *host)
{
    struct stat st = {0};
    string path, file, name;
    char caminho[5000], *temporary;
    name = host;
    if(num == 0)
    {
    	size_t found1 = name.find_last_of("/");
		if (found1!=std::string::npos)
		{	
		    path = name.substr(0,found1);
		    file = name.substr(found1+1);
		    strcpy(caminho, path.c_str());
		    temporary = strtok(caminho, "/");
		    while (temporary != NULL)
			{
				if ((stat(temporary, &st) == -1) && (temporary != NULL))
				{
				    mkdir(temporary, 0700);
				    flag++;
				}
				chdir(temporary);
				temporary = strtok(NULL, "/");
			}
		    fp = fopen(file.c_str(), "ab");
		    num++;
		    return;
		}else{
			temporary = strtok(host, "/");
			while (temporary != NULL)
			{
				if ((stat(temporary, &st) == -1) && (temporary != NULL))
				{
					mkdir(temporary, 0700);
					flag++;
				}
				chdir(temporary);
				temporary = strtok(NULL, "/");
			}
			fp = fopen("index.html", "a+");
			num++;
			return;
		}
    }else{
		string str1 = raiz;
		size_t p = name.find(raiz);
		if(p!=std::string::npos)
		{
			while(flag > 1)
			{
				chdir("..");
				flag--;
			}
			size_t found1 = name.find_first_of("/");
			size_t found2 = name.find_last_of("/");
			if (found1!=std::string::npos && found2!=std::string::npos)
			{	
			    path = name.substr(found1+1,found2);
			    file = name.substr(found2+1);
			    strcpy(caminho, path.c_str());
			    temporary = strtok(caminho, "/");
			    while (temporary != NULL)
				{
					if ((stat(temporary, &st) == -1) && (temporary != NULL))
					{
					    mkdir(temporary, 0700);
					    flag++;
					}
					chdir(temporary);
					temporary = strtok(NULL, "/");
				}
			    fp = fopen(file.c_str(), "ab");
			    num++;
			    return;
			}else{
				while(flag > 1)
				{
					chdir("..");
					flag--;
				}
				temporary = strtok(host, "/");
				while (temporary != NULL)
				{
					if ((stat(temporary, &st) == -1) && (temporary != NULL))
					{
						mkdir(temporary, 0700);
						flag++;
					}
					chdir(temporary);
					temporary = strtok(NULL, "/");
				}
				fp = fopen("index.html", "a+");
				num++;
				return;
			}
		}else{
			while(flag > 1)
			{
				chdir("..");
				flag--;
			}
			temporary = strtok(host, "/");
			while (temporary != NULL)
			{
				if ((stat(temporary, &st) == -1) && (temporary != NULL))
				{
					mkdir(temporary, 0700);
					flag++;
				}
				chdir(temporary);
				temporary = strtok(NULL, "/");
			}
			fp = fopen("index.html", "a+");
			num++;
			return;
		}
    }
}
/* Get the web page and print it to standard output. */
int get_page (char *host)
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
    if (getaddrinfo(host, "80", &host_info, &host_info_list) != 0) {
        fprintf(stderr,"Nao pode obter a pagina ");
        printf("%s\n", host);
        //exit (1);
        return 1;
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
    return 0;
}

void showq(queue <string> gq)
{
    queue <string> g = gq;
    while (!g.empty())
    {
    	cout << g.front() << '\n';
        g.pop();
        pause(1);
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
    FILE *fp1;
    char dados[5000];
    fp1 = fopen("index.html","r");
    queue <string> auxiliar;
    if(fp1 == NULL)
    {
        printf("Falha ao abrir o arquivo\n");
        cout << host;
        cout << '\n';
        exit(1);
    }else{
        string domain = raiz;
        size_t pos = domain.find("www.");
        domain = domain.substr(pos+4);
        while(fgets(dados,5000, fp1) != NULL) //le o arquivo de entrada
        {
            string line = dados;
            size_t pos = line.find("href=");
            if (pos!=std::string::npos)
            {
                string hostname;
                string str1 = line.substr(pos+5);
                pos = str1.find(" ");
                string url = str1.substr(0,pos);
                size_t pos1 = url.find_first_of("http://");
                size_t pos2 = url.find_last_of('"');
                if (pos1!=std::string::npos && pos2!=std::string::npos)
                {
                    url = url.substr(pos1, pos2-1);
                    //cout << "url >> "; cout << url << '\n';
                    pos = url.find("http://");
                    if (pos!=std::string::npos)
                    {
                        hostname = url.substr(pos+7);
                        pos = hostname.find_last_of('/');
                        if(pos!=std::string::npos) hostname = hostname.substr(0,pos);
                        //cout << "host >> "; cout << hostname << '\n';
                        pos = hostname.find(domain);
                        if (pos!=std::string::npos && hostname.compare(host) != 0 && compara(gquiz,hostname) != 1)
                        {
                            gquiz.push(hostname);
                            auxiliar.push(hostname);
                            //cout << "host1 >> "; cout << hostname << '\n';
                        }
                    }else{
                        pos = url.find("https://");
                        if (pos==std::string::npos)
                        {
                            hostname = raiz + url;
                            pos = hostname.find_last_of('/');
                            if(pos!=std::string::npos) hostname = hostname.substr(0,pos);
                            if (hostname.compare(host) != 0 && compara(gquiz,hostname) != 1)
                            {
                                gquiz.push(hostname);
                                auxiliar.push(hostname);
                                //cout << "host2 >> "; cout << hostname << '\n';
                            }
                        }else{
                            hostname = url.substr(pos+8);
                            pos = hostname.find_last_of('/');
                            if(pos!=std::string::npos) hostname = hostname.substr(0,pos);
                            //cout << "host >> "; cout << hostname << '\n';
                            pos = hostname.find(domain);
                            if (pos!=std::string::npos && hostname.compare(host) != 0 && compara(gquiz,hostname) != 1)
                            {
                                gquiz.push(hostname);
                                auxiliar.push(hostname);
                                //cout << "host3 >> "; cout << hostname << '\n';
                            }
                        }
                    }
                }
                pos = str1.find("<img src=");
                if (pos!=std::string::npos)
                {
                    string img = str1.substr(pos+9);
                    pos = img.find(" ");
                    if (pos!=std::string::npos) img = img.substr(0,pos);
                    size_t pos1 = img.find_first_of('"');
                    size_t pos2 = img.find_last_of('"');
                    if (pos1!=std::string::npos && pos2!=std::string::npos)
                    {
                        string Img = img.substr(pos1+2, pos2+1);
                        //cout << "imagem >> ";cout << Img;cout << '\n';getchar();
                        pos = Img.find('"');
                        if (pos!=std::string::npos) Img = Img.substr(0,pos);
                        //cout << "imagem >> ";cout << Img;cout << '\n';getchar();
                        string link = hostname + "/" + Img;
                        if(compara(gquiz,link) != 1) {
                            gquiz.push(link);
                            auxiliar.push(hostname);
                        }
                        //cout << "link da imagem >> ";cout << link;cout << '\n';getchar();
                    }
                }
                //cout<< "resto da linha >> ";cout << str1;cout << '\n';getchar();
            }
            pos = line.find("<li class=");
            size_t pos5 = line.find("<ul class=");
            if (pos!=std::string::npos || pos5!=std::string::npos)
            {
                size_t pos = line.find("href=");
                if (pos!=std::string::npos)
                {
                    string hostname;
                    string str1 = line.substr(pos+5);
                    pos = str1.find(" ");
                    string url = str1.substr(0,pos);
                    size_t pos1 = url.find_first_of("http://");
                    size_t pos2 = url.find_last_of('"');
                    if (pos1!=std::string::npos && pos2!=std::string::npos)
                    {
                        url = url.substr(pos1, pos2-1);
                        //cout << "url >> "; cout << url << '\n';
                        pos = url.find("http://");
                        if (pos!=std::string::npos)
                        {
                            hostname = url.substr(pos+7);
                            pos = hostname.find_last_of('/');
                            if(pos!=std::string::npos) hostname = hostname.substr(0,pos);
                            //cout << "host >> "; cout << hostname << '\n';
                            pos = hostname.find(domain);
                            if (pos!=std::string::npos && hostname.compare(host) != 0 && compara(gquiz,hostname) != 1)
                            {
                                gquiz.push(hostname);
                                auxiliar.push(hostname);
                                //cout << "host1 >> "; cout << hostname << '\n';
                            }
                        }else{
                            pos = url.find("https://");
                            if (pos==std::string::npos)
                            {
                                hostname = raiz + url;
                                pos = hostname.find_last_of('/');
                                if(pos!=std::string::npos) hostname = hostname.substr(0,pos);
                                pos = hostname.find(domain);
                                if (pos!=std::string::npos && hostname.compare(host) != 0 && compara(gquiz,hostname) != 1)
                                {
                                    gquiz.push(hostname);
                                    auxiliar.push(hostname);
                                    //cout << "host2 >> "; cout << hostname << '\n';
                                }
                            }else{
                                hostname = url.substr(pos+8);
                                pos = hostname.find_last_of('/');
                                if(pos!=std::string::npos) hostname = hostname.substr(0,pos);
                                //cout << "host >> "; cout << hostname << '\n';
                                pos = hostname.find(domain);
                                if (pos!=std::string::npos && hostname.compare(host) != 0 && compara(gquiz,hostname) != 1)
                                {
                                    gquiz.push(hostname);
                                    auxiliar.push(hostname);
                                    //cout << "host3 >> "; cout << hostname << '\n';
                                }
                            }
                        }
                    }
                    pos = str1.find("<img src=");
                    if (pos!=std::string::npos)
                    {
                        string img = str1.substr(pos+9);
                        pos = img.find(" ");
                        if (pos!=std::string::npos) img = img.substr(0,pos);
                        size_t pos1 = img.find_first_of('"');
                        size_t pos2 = img.find_last_of('"');
                        if (pos1!=std::string::npos && pos2!=std::string::npos)
                        {
                            string Img = img.substr(pos1+2, pos2+1);
                            //cout << "imagem >> ";cout << Img;cout << '\n';getchar();
                            pos = Img.find('"');
                            if (pos!=std::string::npos) Img = Img.substr(0,pos);
                            //cout << "imagem >> ";cout << Img;cout << '\n';getchar();
                            string link = hostname + "/" + Img;
                            if(compara(gquiz,link) != 1) {
                                gquiz.push(link);
                                auxiliar.push(hostname);
                            }
                            //cout << "link da imagem >> ";cout << link;cout << '\n';getchar();
                        }
                    }
                    //cout<< "resto da linha >> ";cout << str1;cout << '\n';getchar();
                }
            }
        }
    }
    fclose(fp1);
    //showq(auxiliar);
    recursivo(auxiliar);
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
	    if(get_page (nome) == 0)
	    {
	       	cout << "\t\t\t";cout << "Download " << nome << '\n';
	        extrai_urls(nome);  	
	    }
	    g.pop();
	    pause(1);
	}   
}
int main (int argc, char *argv[])
{
    /* Get one of the web pages here. */
    char * host = argv[1];
    raiz = argv[1];
    directory(host);
    get_page (host);
    extrai_urls(host);
    recursivo(gquiz);
    destroi(gquiz);
    return 0;
}