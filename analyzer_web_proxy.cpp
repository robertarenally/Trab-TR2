#include "analyzer_web_proxy.hpp"

#define DEFAULT_NHDRS 8
#define MAX_REQ_LEN 65535
#define MIN_REQ_LEN 4

static const char *root_abs_path = "/";

/* declaração de funcoes privadas*/
int PedidoAnalisado_printRequestLine(struct PedidoAnalisado *pr, char * buf, size_t buflen, size_t *tmp);
size_t PedidoAnalisado_requestLineLen(struct PedidoAnalisado *pr);

/*
 * debug() imprime informações de depuração se DEBUG for definido como 1
 *
 * formato dos parâmetros: o mesmo do printf
 *
 */
void debug(const char * format, ...) {
     va_list args;
     if (DEBUG) {
	  va_start(args, format);
	  vfprintf(stderr, format, args);
	  va_end(args);
     }
}

/* Seta um cabeçalho com o par cheave-valor*/
int CabecalhoDoPedido_set(struct PedidoAnalisado *pr, const char * key, const char * value)
{
     struct CabecalhoDoPedido *ph;
     CabecalhoDoPedido_remove (pr, key);

     if (pr->size_headers <= pr->headersused+1) {
	  pr->size_headers = pr->size_headers * 2;
	  pr->headers = (struct CabecalhoDoPedido *)realloc(pr->headers, pr->size_headers * sizeof(struct CabecalhoDoPedido));
	  if (!pr->headers)
	       return -1;
     }

     ph = pr->headers + pr->headersused;
     pr->headersused += 1;
     
     ph->key = (char *)malloc(strlen(key)+1);
     memcpy(ph->key, key, strlen(key));
     ph->key[strlen(key)] = '\0';

     ph->value = (char *)malloc(strlen(value)+1);
     memcpy(ph->value, value, strlen(value));
     ph->value[strlen(value)] = '\0';

     ph->sizeKey = strlen(key)+1;
     ph->sizeValue = strlen(value)+1;
     return 0;
}

/* pega o CabecalhoDoPedido com uma chave especifica ou NULL*/
struct CabecalhoDoPedido* CabecalhoDoPedido_get(struct PedidoAnalisado *pr, const char * key)
{
     size_t i = 0;
     struct CabecalhoDoPedido * tmp;
     while(pr->headersused > i)
     {
	  tmp = pr->headers + i;
	  if(tmp->key && key && strcmp(tmp->key, key) == 0)
	  {
	       return tmp;
	  }
	  i++;
     }
     return NULL;
}

/* remove uma chave especifica de um CabecalhoDoPedido */
int CabecalhoDoPedido_remove(struct PedidoAnalisado *p, const char *key)
{
     struct CabecalhoDoPedido *tmp;
     tmp = CabecalhoDoPedido_get(p, key);
     if(tmp == NULL)
	  return -1;

     free(tmp->key);
     free(tmp->value);
     tmp->key = NULL;
     return 0;
}
void CabecalhoDoPedido_create(struct PedidoAnalisado *p)
{
     p->headers = (struct CabecalhoDoPedido *)malloc(sizeof(struct CabecalhoDoPedido)*DEFAULT_NHDRS);
     p->size_headers = DEFAULT_NHDRS;
     p->headersused = 0;
} 


size_t CabecalhoDoPedido_sizeLine(struct CabecalhoDoPedido * ph)
{
     if(ph->key != NULL)
     {
	  return strlen(ph->key)+strlen(ph->value)+4;
     }
     return 0; 
}

size_t CabecalhoDoPedido_size(struct PedidoAnalisado *p) 
{
     if (!p || !p->buf)
	  return 0;

     size_t i = 0;
     int len = 0;
     while(p->headersused > i)
     {
	  len += CabecalhoDoPedido_sizeLine(p->headers + i);
	  i++;
     }
     len += 2;
     return len;
}

int CabecalhoDoPedido_printHeaders(struct PedidoAnalisado * pr, char * buf, size_t len)
{
     char * current = buf;
     struct CabecalhoDoPedido * ph;
     size_t i = 0;

     if(len < CabecalhoDoPedido_size(pr))
     {
	  debug("buffer para mostrar o cabecalho do pedido e muito pequeno\n");
	  return -1;
     }
  
     while(pr->headersused > i)
     {
	  ph = pr->headers+i;
	  if (ph->key) {
	       memcpy(current, ph->key, strlen(ph->key));
	       memcpy(current+strlen(ph->key), ": ", 2);
	       memcpy(current+strlen(ph->key) +2 , ph->value, strlen(ph->value));
	       memcpy(current+strlen(ph->key) +2+strlen(ph->value) , "\r\n", 2);
	       current += strlen(ph->key)+strlen(ph->value)+4;
	  }
	  i++;
     }
     memcpy(current, "\r\n",2);
     return 0;
}


void CabecalhoDoPedido_destroyOne(struct CabecalhoDoPedido * ph)
{
     if(ph->key != NULL)
     {
	  free(ph->key);
	  ph->key = NULL;
	  free(ph->value);
	  ph->value = NULL;
	  ph->sizeKey = 0;
	  ph->sizeValue = 0;
     }
}

void CabecalhoDoPedido_destroy(struct PedidoAnalisado * pr)
{
     size_t i = 0;
     while(pr->headersused > i)
     {
	  CabecalhoDoPedido_destroyOne(pr->headers + i);
	  i++;
     }
     pr->headersused = 0;

     free(pr->headers);
     pr->size_headers = 0;
}


int Analise_CabecalhoDoPedido(struct PedidoAnalisado * pr, char * line)
{
     char * key;
     char * value;
     char * index1;
     char * index2;

     index1 = index(line, ':');
     if(index1 == NULL)
     {
	  debug("Dois pontos nao encontrado\n");
	  return -1;
     }
     key = (char *)malloc((index1-line+1)*sizeof(char));
     memcpy(key, line, index1-line);
     key[index1-line]='\0';

     index1 += 2;
     index2 = strstr(index1, "\r\n");
     value = (char *) malloc((index2-index1+1)*sizeof(char));
     memcpy(value, index1, (index2-index1));
     value[index2-index1] = '\0';

     CabecalhoDoPedido_set(pr, key, value);
     free(key);
     free(value);
     return 0;
}
void PedidoAnalisado_destroy(struct PedidoAnalisado *p)
{
     if(p->buf != NULL)
     {
	  free(p->buf);
     }
     if (p->path != NULL) {
	  free(p->path);
     }
     if(p->size_headers > 0)
     {
	  CabecalhoDoPedido_destroy(p);
     }
     free(p);
}

struct PedidoAnalisado* PedidoAnalisado_create()
{
     struct PedidoAnalisado *pr;
     pr = (struct PedidoAnalisado *)malloc(sizeof(struct PedidoAnalisado));
     if (pr != NULL)
     {
	  CabecalhoDoPedido_create(pr);
	  pr->buf = NULL;
	  pr->method = NULL;
	  pr->protocol = NULL;
	  pr->host = NULL;
	  pr->path = NULL;
	  pr->version = NULL;
	  pr->buf = NULL;
	  pr->buflen = 0;
     }
     return pr;
}

/* 
   Recupera o buffer inteiro de um objeto do tipo  PedidoAnalisado. 
*/
int recuperaPedidoHTTP(struct PedidoAnalisado *p, char *buffer, size_t size_buffer)
{
     if (!p || !p->buf)
	  return -1;

     size_t tmp;
     if (PedidoAnalisado_printRequestLine(p, buffer, size_buffer, &tmp) < 0)
	  return -1;
     if (CabecalhoDoPedido_printHeaders(p, buffer+tmp, size_buffer-tmp) < 0)
	  return -1;
     return 0;
}

/* 
   Recria os cabeçalhos HTTP a partir de um objeto do tipo pedido analisado(onde é armazenado o os dados depois do parser)
   buffer deve ser alocado previamente
*/
int recupera_cabecalho_PedidoHTTP(struct PedidoAnalisado *p, char *buffer,  size_t size_buffer)
{
     if (!p || !p->buf)
	  return -1;

     if (CabecalhoDoPedido_printHeaders(p, buffer, size_buffer) < 0)
	  return -1;
     return 0;
}


/* Retorna o tamanho da estrutura PedidoAnalisado */
size_t PedidoAnalisado_sizeTotal(struct PedidoAnalisado *p)
{
     if (!p || !p->buf)
	  return 0;
     return PedidoAnalisado_requestLineLen(p)+CabecalhoDoPedido_size(p);
}


/* 
   Realiza o parser da requisicao http
 
   Parametros:
   struct PedidoAnalisado *p: ponteiro do tipo PedidoAnalisadp
   buffer: buffer que contem a requisicao http. O buffer precissa terminar com \r\n\r\n
   size_buffer: tamanho do buffer
   
   Retorna:
   -1: failha
   0: sucesso
*/
int Analise_do_pedido(struct PedidoAnalisado *p, const char *buffer, int size_buffer)
{
     char *full_addr;
     char *saveptr;
     char *index;
     char *currentHeader;

     if (p->buf != NULL) {
	  debug("pobjeto de análise já atribuído a uma solicitação\n");
	  return -1;
     }
   
     if (size_buffer < MIN_REQ_LEN || size_buffer > MAX_REQ_LEN) {
	  debug("tamanho do buffer e invalido %d", size_buffer);
	  return -1;
     }
   
     /* Create NUL terminated tmp buffer */
     char *tmp_buf = (char *)malloc(size_buffer + 1); /* including NUL */
     memcpy(tmp_buf, buffer, size_buffer);
     tmp_buf[size_buffer] = '\0';
   
     index = strstr(tmp_buf, "\r\n\r\n");
     if (index == NULL) {
	  debug("linha de solicitacao invalida, sem fim de cabecalho\n");
	  free(tmp_buf);
	  return -1;
     }
   
     /* Copy request line into parse->buf */
     index = strstr(tmp_buf, "\r\n");
     if (p->buf == NULL) {
	  p->buf = (char *) malloc((index-tmp_buf)+1);
	  p->buflen = (index-tmp_buf)+1;
     }
     memcpy(p->buf, tmp_buf, index-tmp_buf);
     p->buf[index-tmp_buf] = '\0';

     /* Parse request line */
     p->method = strtok_r(p->buf, " ", &saveptr);
     if (p->method == NULL) {
	  debug( "linha de solicitacao invalida, sem espaco em branco\n");
	  free(tmp_buf);
	  free(p->buf);
	  p->buf = NULL;
	  return -1;
     }

     full_addr = strtok_r(NULL, " ", &saveptr);

     if (full_addr == NULL) {
	  debug( "linha de solicitacao invalida, nao possui o endereco completo\n");
	  free(tmp_buf);
	  free(p->buf);
	  p->buf = NULL;
	  return -1;
     }

     p->version = full_addr + strlen(full_addr) + 1;

     if (p->version == NULL) {
	  debug( "linha de solicitacao invalida, falta a versao do http\n");
	  free(tmp_buf);
	  free(p->buf);
	  p->buf = NULL;
	  return -1;
     }
     if (strncmp (p->version, "HTTP/", 5)) {
	  debug( "linha de solicitacao invalida, versao nao suportada %s\n", p->version);
	  free(tmp_buf);
	  free(p->buf);
	  p->buf = NULL;
	  return -1;
     }
     p->protocol = strtok_r(full_addr, "://", &saveptr);
     if (p->protocol == NULL) {
	  debug( "linha de solicitacao invalida, falta a porta\n");
	  free(tmp_buf);
	  free(p->buf);
	  p->buf = NULL;
	  return -1;
     }
     
     const char *rem = full_addr + strlen(p->protocol) + strlen("://");
     size_t abs_uri_len = strlen(rem);

     p->host = strtok_r(NULL, "/", &saveptr);
     if (p->host == NULL) {
	  debug( "linha de solicitacao invalida, nao possui host:\n");
	  free(tmp_buf);
	  free(p->buf);
	  p->buf = NULL;
	  return -1;
     }
     
     if (strlen(p->host) == abs_uri_len) {
	  debug("linha de solicitação inválida, sem caminho absoluto\n");
	  free(tmp_buf);
	  free(p->buf);
	  p->buf = NULL;
	  return -1;
     }
     p->path = strtok_r(NULL, " ", &saveptr);
     if (p->path == NULL) {          // replace empty abs_path with "/"
	  int rlen = strlen(root_abs_path);
	  p->path = (char *)malloc(rlen + 1);
	  strncpy(p->path, root_abs_path, rlen + 1);
     } else if (strncmp(p->path, root_abs_path, strlen(root_abs_path)) == 0) {
	  debug("inha de solicitação inválida, o caminho não pode começar "" com dois caracteres de barra\n");
	  free(tmp_buf);
	  free(p->buf);
	  p->buf = NULL;
	  p->path = NULL;
	  return -1;
     } else {
	  // copia p->path, prefixo com uma barra
	  char *tmp_path = p->path;
	  int rlen = strlen(root_abs_path);
	  int plen = strlen(p->path);
	  p->path = (char *)malloc(rlen + plen + 1);
	  strncpy(p->path, root_abs_path, rlen);
	  strncpy(p->path + rlen, tmp_path, plen + 1);
     }

     p->host = strtok_r(p->host, ":", &saveptr);
     p->port = strtok_r(NULL, "/", &saveptr);

     if (p->host == NULL) {
	  debug( "linha de solicitacao invalida, nao possui host:\n");
	  free(tmp_buf);
	  free(p->buf);
	  free(p->path);
	  p->buf = NULL;
	  p->path = NULL;
	  return -1;
     }

     if (p->port != NULL) {
	  int port = strtol (p->port, (char **)NULL, 10);
	  if (port == 0 && errno == EINVAL) {
	       debug("linha de solicitacao invalida, porta incorreta: %s\n", p->port);
	       free(tmp_buf);
	       free(p->buf);
	       free(p->path);
	       p->buf = NULL;
	       p->path = NULL;
	       return -1;
	  }
     }

   
     /* Parse headers */
     int ret = 0;
     currentHeader = strstr(tmp_buf, "\r\n")+2;
     while (currentHeader[0] != '\0' && 
	    !(currentHeader[0] == '\r' && currentHeader[1] == '\n')) {

	  if (Analise_CabecalhoDoPedido(p, currentHeader)) {
	       ret = -1;
	       break;
	  }

	  currentHeader = strstr(currentHeader, "\r\n");
	  if (currentHeader == NULL || strlen (currentHeader) < 2)
	       break;

	  currentHeader += 2;
     }
     free(tmp_buf);
     return ret;
}
size_t PedidoAnalisado_requestLineLen(struct PedidoAnalisado *pr)
{
     if (!pr || !pr->buf)
	  return 0;

     size_t len =  
	  strlen(pr->method) + 1 + strlen(pr->protocol) + 3 + 
	  strlen(pr->host) + 1 + strlen(pr->version) + 2;
     if(pr->port != NULL)
     {
	  len += strlen(pr->port)+1;
     }
     /*caminho é pelo menos uma barra */
     len += strlen(pr->path);
     return len;
}

int PedidoAnalisado_printRequestLine(struct PedidoAnalisado *pr, char * buf, size_t buflen, size_t *tmp)
{
     char * current = buf;

     if(buflen < PedidoAnalisado_requestLineLen(pr))
     {
	  debug("memoria insuficiente para a primeira linha\n");
	  return -1; 
     }
     memcpy(current, pr->method, strlen(pr->method));
     current += strlen(pr->method);
     current[0]  = ' ';
     current += 1;

     memcpy(current, pr->protocol, strlen(pr->protocol));
     current += strlen(pr->protocol);
     memcpy(current, "://", 3);
     current += 3;
     memcpy(current, pr->host, strlen(pr->host));
     current += strlen(pr->host);
     if(pr->port != NULL)
     {
	  current[0] = ':';
	  current += 1;
	  memcpy(current, pr->port, strlen(pr->port));
	  current += strlen(pr->port);
     }
     /*caminho é pelo menos uma barra */
     memcpy(current, pr->path, strlen(pr->path));
     current += strlen(pr->path);

     current[0] = ' ';
     current += 1;

     memcpy(current, pr->version, strlen(pr->version));
     current += strlen(pr->version);
     memcpy(current, "\r\n", 2);
     current +=2;
     *tmp = current-buf;
     return 0;
}