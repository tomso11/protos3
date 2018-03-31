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


enum request_state request_parser_feed (struct request_parser* p, const uint8_t c) {
    enum request_state next;

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