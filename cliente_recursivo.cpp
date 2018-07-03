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
			fp = fopen("text.html", "a+");
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
				fp = fopen("text.html", "a+");
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
			fp = fopen("text.html", "a+");
			num++;
			return;
		}
    }
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
        fprintf(stderr," Erro no formato do endereco do servidor! Nao pode obter a pagina\n");
        //exit (1);
        return;
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
	/** TODO: corrigir a extração dos links, pois possue links quebrados
	**/
    char dados[5000], ABC;
    string line, url, hostname;
    FILE *fp1;
    int i, n, j, flag;
    fp1 = fopen("text.html","r");

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
                        if(compara(gquiz,str5) != 1) gquiz.push(str5);
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
                    if(compara(gquiz,str8) != 1) gquiz.push(str8);
                    //cout << str8;cout << '\n';getchar();
                }
            }
        }
    }
    fclose(fp1);
    showq(gquiz);
}
void recursivo(queue <string> gq)
{
    queue <string> g = gq;
    string path, file;
    while (!g.empty())
    {
        string str = g.front();
	    size_t found = str.find_last_of("/");
	    if (found!=std::string::npos)
        {
        	path = str.substr(0,found);
        	file = str.substr(found+1);
        }
        char nome[5000];
        strcpy(nome, str.c_str());
        chdir("cd ~/Trab-TR2");
        directory(nome);
        get_page (nome);
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