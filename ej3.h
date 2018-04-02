/*
* Un HTTP request se forma de la siguiente manera:
* 
* Start-line= METHOD REQUEST-TARGET VERSION
*	METHOD: GET POST PUT HEAD OPTIONS
*	REQUEST-TARGET:
*			- ABSOLUT PATH: con '?' query
*			- COMPLETE URL: ex: http://itba.edu.ar/
*			- AUTHORITY: URL con port example.com:80
*			- ASTERISK: * todo el server
*	VERSION:
*			de la forma HTTP/ver
*
* Headers= case-insensitive, puede ser request, body o gral.
*		se forman como header: value
*
* Body= contiene informacion, generalmente un archivo de texto
*
*/

#define HEADER_DEF_LEN 19

/* Metodos posibles */
enum req_method {
	GET,
	HEAD,
	POST,
	PUT,
	DELETE,
	CONNECT,
	OPTIONS,
	TRACE
};

enum version {
	HTTP10,
	HTTP11
};

/* Estructura del request en si */
struct request {
	/*start line*/
	enum req_method method;
	char * req_tgt;
	enum version ver;

	/* header line*/
	/* para esto estaria bueno tener algo como un Map<String,String> */
	//char ** headers;
	//char ** values;
	char header[20];
	char * host;
	int cont-len;

	/* body */
	char * body;
}

/* procesamiento del request */

enum request_state {
   request_method,
   request_tgt,
   request_version,
   request_header,
   request_value,
   request_body,
   request_ws, // estado de whitespace para tirar error si no cumple ?

   // apartir de aca están done
   request_done,

   // y apartir de aca son considerado con error
   request_error,
   request_error_unsupported_version,

};

struct request_parser {
   struct request *request;
   enum request_state state;
   /* para body */
   /* cuantos bytes tenemos que leer*/
   uint8_t need;
   /* cuantos bytes ya leimos */
   uint8_t read;
};

/* Funciones */

/* PARSEO */

/** inicializa el parser */
void 
request_parser_init (struct request_parser *p);

/** entrega un byte al parser. retorna true si se llego al final  */
enum request_state 
request_parser_feed (struct request_parser *p, const uint8_t c);

/**
 * por cada elemento del buffer llama a `request_parser_feed' hasta que
 * el parseo se encuentra completo o se requieren mas bytes.
 *
 * @param errored parametro de salida. si es diferente de NULL se deja dicho
 *   si el parsing se debió a una condición de error
 */
enum request_state
request_consume(buffer *b, struct request_parser *p, bool *errored);

/**
 * Permite distinguir a quien usa socks_hello_parser_feed si debe seguir
 * enviando caracters o no. 
 *
 * En caso de haber terminado permite tambien saber si se debe a un error
 */
bool 
request_is_done(const enum request_state st, bool *errored);

void
request_close(struct request_parser *p);

/* OUTPUT */

// TODO