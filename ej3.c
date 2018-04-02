/* Ejercicio 3 tpe intro*/
/* sys */ 


/* user */
#include "buffer.h"
#include "ej3.h"

static void
remaining_set(struct request_parser* p, const int n) {
    p->read = 0;
    p->need = n;
}

static int
remaining_is_done(struct request_parser* p) {
    return p->read >= p->need;
}

//////////////////////////////////////////////////////

/* Pensaba hacer una misma funcion minimamente para todo lo que es start line,
	ya que la forma de procesarlo es basicamente una maquina de estado y 
	guardar los caracteres en un array. El problema es que tengo que poder
	diferenciar los estados para saber en que array guardarlo.
	Adicionalmente, me gustaria realizar un chequeo de validez de por ejemplo
	el metodo que se recibe.
*/

/* GENERAL: Deberia contemplar el error de si mandan un enter en el medio
 de la linea? */

enum request_state target(struct request_parser* p, const uint8_t c){
	enum request_state next;
	static int pos = 0; // no se me ocurre otra forma

	switch ( c ){
		case ' ':
			next = p->state + 1;
			break;
		default:
			p->( req_tgt + pos ) = c;
			pos++;
			next = request_tgt;
			break;
	}

	return next;
}

/* Aca podemos utilizar un array fijo para guardar los nombres de los headers ? 
	Observacion: La verdad es que solo nos interesan los headers Host y 
	Content-Length.
	Cualquier otro header no nos cambia nada, por lo tanto podemos hacer 
	que el parser solo le preste atencion a dichos headers.
*/

enum request_state header(struct request_parser* p, const uint8_t c){
	enum request_state next;
	static int pos = 0; // no se me ocurre otra forma

	switch ( c ){
		case ':':
			p->( header + pos ) = '\0';
			/*TODO:chequear si es igual a Host o Content-Length para guardar
			 el valor*/
			next = request_value;
			break;
		default:
			if ( pos < HEADER_DEF_LEN ){
				p->( header + pos ) = c;
				pos++;
			}
			next = request_header;
			break;
	}

	return next;
}

enum request_state value(struct request_parser* p, const uint8_t c, const int type){
	enum request_state next;
	static int pos = 0; // no se me ocurre otra forma

	switch ( c ){
		case '\n':
			if( type == 1 )
				p->( host + pos ) = '\0';
			/*TODO:chequear si es igual a Host o Content-Length para guardar
			 el valor*/
			next = request_header;
			break;
		default:
			//Content-Length
			if( type == 0 )
				p->cont-len = (p->cont-len)*10 + c;
			else{
				//host
				p->( host + pos )= c;
				pos++;
			}
			next = request_value;
			break;
	}

	return next;
}

/* No recuerdo como terminaba el body msg */

enum request_state body(struct request_parser* p, const uint8_t c){
	enum request_state next;
	static int pos = 0; // no se me ocurre otra forma

	switch ( c ){
		case '\n':
			p->( body + pos ) = '\0';
			next = request_done;
			break;
		default:
			p->( body + pos ) = c;
			pos++;
			next = request_body;
			break;
	}

	return next;
}



enum request_state request_parser_feed (struct request_parser* p, const uint8_t c) {
    enum request_state next;
    int pos;

    switch( p->state ){
    	case request_method:
    		next = method(c, p);
    		break;
    	case request_tgt:
    		next = target(c, p);
    		break;
    	case request_version:
    		next = version(c, p);
    		break;
    	case request_header:
    		next = header(c, p);
    		break;
    	case request_value:
    		next = value(c, p);
    		break;
    	case request_body:
    		next = body(c, p);
    		break;
    	case request_ws:
    		/*si no es un ws devolver un error*/
    		next = ws(c, p);
    		break;
    	case request_done:
    	case request_error:
    	case request_error_unsupported_version:
    		next = p->state;
    		break;
    	default:
    		next = request_error;
    		break;
    }
    /*
    switch(p->state) {
        case request_version:
            next = version(c, p);
            break;
        case request_cmd:
            next = cmd(c, p);
            break;
        case request_rsv:
            next = rsv(c, p);
            break;
        case request_atyp:
            next = atyp(c, p);
            break;
        case request_dstaddr_fqdn:
            next = dstaddr_fqdn(c, p);
            break;
        case  request_dstaddr:
            next = dstaddr(c, p);
            break;
        case request_dstport:
            next = dstport(c, p);
            break;
        case request_done:
        case request_error:
        case request_error_unsupported_version:
        case request_error_unsupported_atyp:
            next = p->state;
            break;
        default:
            next = request_error;
            break;
    }
    */

    return p->state = next;
}


enum request_state request_consume(buffer *b, struct request_parser *p, bool *errored) {
    enum request_state st = p->state;

    while(buffer_can_read(b)) {
       const uint8_t c = buffer_read(b);
       st = request_parser_feed(p, c);
       if(request_is_done(st, errored)) {
          break;
       }
    }
    return st;
}