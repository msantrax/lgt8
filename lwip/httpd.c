/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *         Simon Goldschmidt
 *
 */


#include "lwip/opt.h"

#if LWIP_TCP

#include <string.h>
#include <stdlib.h>

#include "httpd.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "httpd_structs.h"
#include "lwip/tcp.h"
#include "fs.h"

#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/timers.h"
#include "lwip/inet_chksum.h"
#include "lwip/init.h"
#include "netif/etharp.h"

#include "ethernetif.h"
#include "board.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsdata.h"
#include "fsl_device_registers.h"
#include "board.h"
#include "clock_config.h"

#include "antares.h"



/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define EXAMPLE_ENET ENET

/* IP address configuration. */
#define configIP_ADDR0 192
#define configIP_ADDR1 168
#define configIP_ADDR2 1
#define configIP_ADDR3 110

/* Netmask configuration. */
#define configNET_MASK0 255
#define configNET_MASK1 255
#define configNET_MASK2 255
#define configNET_MASK3 0

/* Default gateway address configuration */
#define configGW_ADDR0 192
#define configGW_ADDR1 168
#define configGW_ADDR2 1
#define configGW_ADDR3 100

#define configPHY_ADDRESS 1


#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG LWIP_DBG_OFF
#endif


/* Set this to 1 and add the next line to lwippools.h to use a memp pool
* for allocating struct http_state instead of the heap:
* LWIP_MEMPOOL(HTTPD_STATE, 20, 100, "HTTPD_STATE")
*/
#ifndef HTTPD_USE_MEM_POOL
#define HTTPD_USE_MEM_POOL 0
#endif

/* The server port for HTTPD to use*/
#ifndef HTTPD_SERVER_PORT
#define HTTPD_SERVER_PORT 80
#endif

/*
* Maximum retries before the connection is aborted/closed.
* - number of times pcb->poll is called -> default is 4*500ms = 2s;
* - reset when pcb->sent is called
*/
#ifndef HTTPD_MAX_RETRIES
#define HTTPD_MAX_RETRIES 4
#endif

/* The poll delay is X*500ms*/
#ifndef HTTPD_POLL_INTERVAL
#define HTTPD_POLL_INTERVAL 4
#endif

/*
* Priority for tcp pcbs created by HTTPD (very low by default).
* Lower priorities get killed first when running out of memroy.
*/
#ifndef HTTPD_TCP_PRIO
#define HTTPD_TCP_PRIO TCP_PRIO_MIN
#endif

/* Set this to 1 to enabled timing each file sent */
#ifndef LWIP_HTTPD_TIMING
#define LWIP_HTTPD_TIMING 0
#endif
#ifndef HTTPD_DEBUG_TIMING
#define HTTPD_DEBUG_TIMING LWIP_DBG_OFF
#endif

/* Set this to 1 on platforms where strnstr is not available */
#ifndef LWIP_HTTPD_STRNSTR_PRIVATE
#define LWIP_HTTPD_STRNSTR_PRIVATE 1
#endif

/* Set this to one to show error pages when parsing a request fails instead
 * of simply closing the connection.
 */
#ifndef LWIP_HTTPD_SUPPORT_EXTSTATUS
#define LWIP_HTTPD_SUPPORT_EXTSTATUS 0
#endif

/* Set this to 0 to drop support for HTTP/0.9 clients (to save some bytes) */
#ifndef LWIP_HTTPD_SUPPORT_V09
#define LWIP_HTTPD_SUPPORT_V09 1
#endif

/* Set this to 1 to enable HTTP/1.1 persistent connections.
 * ATTENTION: If the generated file system includes HTTP headers, these must
 * include the "Connection: keep-alive" header (pass argument "-11" to makefsdata).
 */
#ifndef LWIP_HTTPD_SUPPORT_11_KEEPALIVE
#define LWIP_HTTPD_SUPPORT_11_KEEPALIVE 0
#endif

/* Set this to 1 to support HTTP request coming in in multiple packets/pbufs */
#ifndef LWIP_HTTPD_SUPPORT_REQUESTLIST
#define LWIP_HTTPD_SUPPORT_REQUESTLIST 1
#endif

#if LWIP_HTTPD_SUPPORT_REQUESTLIST
/* Number of rx pbufs to enqueue to parse an incoming request (up to the first newline) */
#ifndef LWIP_HTTPD_REQ_QUEUELEN
#define LWIP_HTTPD_REQ_QUEUELEN 5
#endif

/* Number of (TCP payload-) bytes (in pbufs) to enqueue to parse and incoming
 * request (up to the first double-newline)
 */
#ifndef LWIP_HTTPD_REQ_BUFSIZE
#define LWIP_HTTPD_REQ_BUFSIZE LWIP_HTTPD_MAX_REQ_LENGTH
#endif

/* Defines the maximum length of a HTTP request line (up to the first CRLF, */
/* copied from pbuf into this a global buffer when pbuf- or packet-queues */
/* are received - otherwise the input pbuf is used directly) */
#ifndef LWIP_HTTPD_MAX_REQ_LENGTH
#define LWIP_HTTPD_MAX_REQ_LENGTH LWIP_MIN(1023, (LWIP_HTTPD_REQ_QUEUELEN * PBUF_POOL_BUFSIZE))
#endif
#endif /* LWIP_HTTPD_SUPPORT_REQUESTLIST */

/* Maximum length of the filename to send as response to a POST request, */
/* filled in by the application when a POST is finished. */
#ifndef LWIP_HTTPD_POST_MAX_RESPONSE_URI_LEN
#define LWIP_HTTPD_POST_MAX_RESPONSE_URI_LEN 63
#endif

/* Set this to 0 to not send the SSI tag (default is on, so the tag will */
/* be sent in the HTML page */
#ifndef LWIP_HTTPD_SSI_INCLUDE_TAG
#define LWIP_HTTPD_SSI_INCLUDE_TAG 1
#endif

/* Set this to 1 to call tcp_abort when tcp_close fails with memory error. */
/* This can be used to prevent consuming all memory in situations where the */
/* HTTP server has low priority compared to other communication. */
#ifndef LWIP_HTTPD_ABORT_ON_CLOSE_MEM_ERROR
#define LWIP_HTTPD_ABORT_ON_CLOSE_MEM_ERROR 0
#endif

/* Set this to 1 to kill the oldest connection when running out of */
/* memory for 'struct http_state' or 'struct http_ssi_state'. */
/* ATTENTION: This puts all connections on a linked list, so may be kind of slow. */
#ifndef LWIP_HTTPD_KILL_OLD_ON_CONNECTIONS_EXCEEDED
#define LWIP_HTTPD_KILL_OLD_ON_CONNECTIONS_EXCEEDED 0
#endif

/* Minimum length for a valid HTTP/0.9 request: "GET /\r\n" -> 7 bytes */
#define MIN_REQ_LEN 7

#define CRLF "\r\n"
#define HTTP11_CONNECTIONKEEPALIVE "Connection: keep-alive"

#define LWIP_HTTPD_IS_SSI(hs) 0


/* These defines check whether tcp_write has to copy data or not */

/* This was TI's check whether to let TCP copy data or not */
/* #define HTTP_IS_DATA_VOLATILE(hs) ((hs->file < (char *)0x20000000) ? 0 : TCP_WRITE_FLAG_COPY) */
#ifndef HTTP_IS_DATA_VOLATILE
/* Default: don't copy if the data is sent from file-system directly */
#define HTTP_IS_DATA_VOLATILE(hs)                                            \
    (((hs->file != NULL) && (hs->handle != NULL) &&                          \
      (hs->file == (char *)hs->handle->data + hs->handle->len - hs->left)) ? \
         0 :                                                                 \
         TCP_WRITE_FLAG_COPY)
#endif


/* Default: headers are sent from ROM */
#ifndef HTTP_IS_HDR_VOLATILE
#define HTTP_IS_HDR_VOLATILE(hs, ptr) 0
#endif

/* Return values for http_send_*() */
#define HTTP_DATA_TO_SEND_BREAK 2
#define HTTP_DATA_TO_SEND_CONTINUE 1
#define HTTP_NO_DATA_TO_SEND 0

#define HTTP_ALLOC_SSI_STATE() (struct http_ssi_state *) mem_malloc(sizeof(struct http_ssi_state))
#define HTTP_ALLOC_HTTP_STATE() (struct http_state *) mem_malloc(sizeof(struct http_state))

#define BOARD_LED_GPIO BOARD_LED_RED_GPIO
#define BOARD_LED_GPIO_PIN BOARD_LED_RED_GPIO_PIN
#define BOARD_SW_GPIO BOARD_SW3_GPIO
#define BOARD_SW_GPIO_PIN BOARD_SW3_GPIO_PIN
#define BOARD_SW_PORT BOARD_SW3_PORT
#define BOARD_SW_IRQ BOARD_SW3_IRQ
#define BOARD_SW_IRQ_HANDLER BOARD_SW3_IRQ_HANDLER


typedef struct{
    const char *name;
    u8_t shtml;
} default_filename;

const default_filename g_psDefaultFilenames[] = {
    {"/index.shtml", 1}, {"/index.ssi", 1}, {"/index.shtm", 1}, {"/index.html", 0}, {"/index.htm", 0}};

#define NUM_DEFAULT_FILENAMES (sizeof(g_psDefaultFilenames) / sizeof(default_filename))

#if LWIP_HTTPD_SUPPORT_REQUESTLIST
/* HTTP request is copied here from pbufs for simple parsing */
static char httpd_req_buf[LWIP_HTTPD_MAX_REQ_LENGTH + 1];
#endif /* LWIP_HTTPD_SUPPORT_REQUESTLIST */

struct http_state{
    struct fs_file file_handle;
    struct fs_file *handle;
    char *file; /* Pointer to first unsent byte in buf. */
    struct tcp_pcb *pcb;
    struct pbuf *req;
    u32_t left;  /* Number of unsent bytes in buf. */
    u8_t retries;
    char *params[LWIP_HTTPD_MAX_CGI_PARAMETERS];     /* Params extracted from the request URI */
    char *param_vals[LWIP_HTTPD_MAX_CGI_PARAMETERS]; /* Values for each extracted param */
};

enum echo_states{
    ES_NONE = 0,
    ES_ACCEPTED,
    ES_RECEIVED,
    ES_CLOSING
};

struct echo_state{
    u8_t state;
    u8_t retries;
    struct tcp_pcb *pcb;
    /* pbuf (chain) to recycle */
    struct pbuf *p;
};


//static struct tcp_pcb *echo_pcb;


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static err_t http_close_conn(struct tcp_pcb *pcb, struct http_state *hs);
static err_t http_close_or_abort_conn(struct tcp_pcb *pcb, struct http_state *hs, u8_t abort_conn);
static err_t http_find_file(struct http_state *hs, const char *uri, int is_09);
static err_t http_init_file(struct http_state *hs, struct fs_file *file, int is_09, const char *uri, u8_t tag_check);
static err_t http_poll(void *arg, struct tcp_pcb *pcb);


err_t echo_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
err_t echo_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void echo_error(void *arg, err_t err);
err_t echo_poll(void *arg, struct tcp_pcb *tpcb);
err_t echo_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

void echo_send(struct tcp_pcb *tpcb, struct echo_state *es);
void echo_close(struct tcp_pcb *tpcb, struct echo_state *es);


/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_ButtonPress = false;
#if LWIP_HTTPD_CGI
/* CGI handler information */
const tCGI *g_pCGIs;
int g_iNumCGIs;
#endif /* LWIP_HTTPD_CGI */

/*******************************************************************************
 * Code
 ******************************************************************************/

#if LWIP_HTTPD_STRNSTR_PRIVATE
/* Like strstr but does not need 'buffer' to be NULL-terminated */
static char *strnstr(const char *buffer, const char *token, size_t n){
    const char *p;
    int tokenlen = (int)strlen(token);
    if (tokenlen == 0){
        return (char *)buffer;
    }
    for (p = buffer; *p && (p + tokenlen <= buffer + n); p++){
        if ((*p == *token) && (strncmp(p, token, tokenlen) == 0)){
            return (char *)p;
        }
    }
    return NULL;
}
#endif /* LWIP_HTTPD_STRNSTR_PRIVATE */

/* Initialize a struct http_state.*/
static void http_state_init(struct http_state *hs){
    /* Initialize the structure. */
    memset(hs, 0, sizeof(struct http_state));
}

/* Allocate a struct http_state. */
static struct http_state *http_state_alloc(void){

	struct http_state *ret = HTTP_ALLOC_HTTP_STATE();
    if (ret != NULL){
        http_state_init(ret);
    }
    return ret;
}

/* Free a struct http_state. Also frees the file data if dynamic. */
static void http_state_eof(struct http_state *hs){

	if (hs->handle){
        fs_close(hs->handle);
        hs->handle = NULL;
    }
}

/* Free a struct http_state. Also frees the file data if dynamic. */
static void http_state_free(struct http_state *hs){

	if (hs != NULL){
        http_state_eof(hs);
        mem_free(hs);
    }
}

/*!
 * @brief Call tcp_write() in a loop trying smaller and smaller length
 *
 * @param pcb tcp_pcb to send
 * @param ptr Data to send
 * @param length Length of data to send (in/out: on return, contains the
 *        amount of data sent)
 * @param apiflags directly passed to tcp_write
 * @return the return value of tcp_write
 */
static err_t http_write(struct tcp_pcb *pcb, const void *ptr, u16_t *length, u8_t apiflags){

	u16_t len;
    err_t err;
    LWIP_ASSERT("length != NULL", length != NULL);
    len = *length;
    if (len == 0){
        return ERR_OK;
    }

    do{
        LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("Trying go send %d bytes\r\n", len));
        err = tcp_write(pcb, ptr, len, apiflags);
        if (err == ERR_MEM){
            if ((tcp_sndbuf(pcb) == 0) || (tcp_sndqueuelen(pcb) >= TCP_SND_QUEUELEN)){
                /* no need to try smaller sizes */
                len = 1;
            }
            else{
                len /= 2;
            }
            LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("Send failed, trying less (%d bytes)\r\n", len));
        }
    } while ((err == ERR_MEM) && (len > 1));

    if (err == ERR_OK){
        LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("Sent %d bytes\r\n", len));
    }
    else{
        LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("Send failed with err %d (\"%s\")\r\n", err, lwip_strerr(err)));
    }

    *length = len;
    return err;
}

/*!
 * @brief The connection shall be actively closed (using RST to close from fault states).
 * Reset the sent- and recv-callbacks.
 *
 * @param pcb the tcp pcb to reset callbacks
 * @param hs connection state to free
 */
static err_t http_close_or_abort_conn(struct tcp_pcb *pcb, struct http_state *hs, u8_t abort_conn){

	err_t err;
    LWIP_DEBUGF(HTTPD_DEBUG, ("Closing connection %p\r\n", (void *)pcb));

    tcp_arg(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_err(pcb, NULL);
    tcp_poll(pcb, NULL, 0);
    tcp_sent(pcb, NULL);

    if (hs != NULL){
        http_state_free(hs);
    }

    if (abort_conn){
        tcp_abort(pcb);
        return ERR_OK;
    }

    err = tcp_close(pcb);
    if (err != ERR_OK){
        LWIP_DEBUGF(HTTPD_DEBUG, ("Error %d closing %p\r\n", err, (void *)pcb));
        /* error closing, try again later in poll */
        tcp_poll(pcb, http_poll, HTTPD_POLL_INTERVAL);
    }
    return err;
}

/*!
 * @brief The connection shall be actively closed.
 * Reset the sent- and recv-callbacks.
 *
 * @param pcb the tcp pcb to reset callbacks
 * @param hs connection state to free
 */
static err_t http_close_conn(struct tcp_pcb *pcb, struct http_state *hs){
    return http_close_or_abort_conn(pcb, hs, 0);
}

/* End of file: either close the connection (Connection: close)
 * or close the file (Connection: keep-alive)
 */
static void http_eof(struct tcp_pcb *pcb, struct http_state *hs){

	http_close_conn(pcb, hs);
}

#if LWIP_HTTPD_CGI
/*!
 * @brief Extract URI parameters from the parameter-part of an URI in the form
 * "test.cgi?x=y" @todo: better explanation!
 * Pointers to the parameters are stored in hs->param_vals.
 *
 * @param hs http connection state
 * @param params pointer to the NULL-terminated parameter string from the URI
 * @return number of parameters extracted
 */
static int extract_uri_parameters(struct http_state *hs, char *params){

	char *pair;
    char *equals;
    int loop;

    /* If we have no parameters at all, return immediately. */
    if (!params || (params[0] == '\0')){
        return (0);
    }

    /* Get a pointer to our first parameter */
    pair = params;

    /*  Parse up to LWIP_HTTPD_MAX_CGI_PARAMETERS from the passed string
     *  and ignore the remainder (if any)
     */
    for (loop = 0; (loop < LWIP_HTTPD_MAX_CGI_PARAMETERS) && pair; loop++){
        /* Save the name of the parameter */
        hs->params[loop] = pair;

        /* Remember the start of this name=value pair */
        equals = pair;

        /* Find the start of the next name=value pair and replace the delimiter
         * with a 0 to terminate the previous pair string.
         */
        pair = strchr(pair, '&');
        if (pair){
            *pair = '\0';
            pair++;
        }
        else{
            /* We didn't find a new parameter so find the end of the URI
             * and replace the space with a '\0'
             */
            pair = strchr(equals, ' ');
            if (pair){
                *pair = '\0';
            }

            /* Revert to NULL so that we exit the loop as expected. */
            pair = NULL;
        }

        /* Now find the '=' in the previous pair, replace it with '\0'
         * and save the parameter value string.
         */
        equals = strchr(equals, '=');
        if (equals){
            *equals = '\0';
            hs->param_vals[loop] = equals + 1;
        }
        else{
            hs->param_vals[loop] = NULL;
        }
    }

    return loop;
}
#endif /* LWIP_HTTPD_CGI */


/*!
 * @brief Sub-function of http_send(): end-of-file (or block) is reached,
 * either close the file or read the next block (if supported).
 *
 * @returns: 0 if the file is finished or no data has been read
 *           1 if the file is not finished and data has been read
 */
static u8_t http_check_eof(struct tcp_pcb *pcb, struct http_state *hs){

    /* Do we have a valid file handle? */
    if (hs->handle == NULL){
        /* No - close the connection. */
        http_eof(pcb, hs);
        return 0;
    }

    if (fs_bytes_left(hs->handle) <= 0){
        /* We reached the end of the file so this request is done. */
        LWIP_DEBUGF(HTTPD_DEBUG, ("End of file.\r\n"));
        http_eof(pcb, hs);
        return 0;
    }

    LWIP_ASSERT("SSI and DYNAMIC_HEADERS turned off but eof not reached", 0);

    return 1;
}

/*!
 * @brief Sub-function of http_send(): This is the normal send-routine for non-ssi files
 *
 * @returns: - 1: data has been written (so call tcp_ouput)
 *           - 0: no data has been written (no need to call tcp_output)
 */
static u8_t http_send_data_nonssi(struct tcp_pcb *pcb, struct http_state *hs){

	err_t err;
    u16_t len;
    u16_t mss;
    u8_t data_to_send = 0;

    /* We are not processing an SHTML file so no tag checking is necessary. */
    /* Just send the data as we received it from the file. */

    /* We cannot send more data than space available in the send buffer. */
    if (tcp_sndbuf(pcb) < hs->left){
        len = tcp_sndbuf(pcb);
    }
    else{
        len = (u16_t)hs->left;
        LWIP_ASSERT("hs->left did not fit into u16_t!", (len == hs->left));
    }
    mss = tcp_mss(pcb);
    if (len > (2 * mss)){
        len = 2 * mss;
    }

    err = http_write(pcb, hs->file, &len, HTTP_IS_DATA_VOLATILE(hs));
    if (err == ERR_OK){
        data_to_send = 1;
        hs->file += len;
        hs->left -= len;
    }

    return data_to_send;
}

/*!
 * @brief Try to send more data on this pcb. *
 * @param pcb the pcb to send data
 * @param hs connection state
 */
static u8_t http_send(struct tcp_pcb *pcb, struct http_state *hs){

	u8_t data_to_send = HTTP_NO_DATA_TO_SEND;

    LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE,
                ("http_send: pcb=%p hs=%p left=%d\r\n", (void *)pcb, (void *)hs, hs != NULL ? (int)hs->left : 0));

    /* If we were passed a NULL state structure pointer, ignore the call. */
    if (hs == NULL){
        return 0;
    }

    /* Have we run out of file data to send? If so, we need to read the next block from the file. */
    if (hs->left == 0){
        if (!http_check_eof(pcb, hs)){
            return 0;
        }
    }
    data_to_send = http_send_data_nonssi(pcb, hs);

    if ((hs->left == 0) && (fs_bytes_left(hs->handle) <= 0)){
        /* We reached the end of the file so this request is done. */
        /* This adds the FIN flag right into the last data segment. */
        LWIP_DEBUGF(HTTPD_DEBUG, ("End of file.\r\n"));
        http_eof(pcb, hs);
        return 0;
    }
    LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("send_data end.\r\n"));
    return data_to_send;
}

#define http_find_error_file(hs, error_nr) ERR_ARG


/*!
 * @brief Get the file struct for a 404 error page.
 * Tries some file names and returns NULL if none found.
 *
 * @param uri pointer that receives the actual file name URI
 * @return file struct for the error page or NULL no matching file was found
 */
static struct fs_file *http_get_404_file(struct http_state *hs, const char **uri){

	err_t err;

    *uri = "/404.html";
    err = fs_open(&hs->file_handle, *uri);
    if (err != ERR_OK){
        /* 404.html doesn't exist. Try 404.htm instead. */
        *uri = "/404.htm";
        err = fs_open(&hs->file_handle, *uri);
        if (err != ERR_OK){
            /* 404.htm doesn't exist either. Try 404.shtml instead. */
            *uri = "/404.shtml";
            err = fs_open(&hs->file_handle, *uri);
            if (err != ERR_OK){
                /* 404.htm doesn't exist either. Indicate to the caller that it should */
                /* send back a default 404 page. */
                *uri = NULL;
                return NULL;
            }
        }
    }

    return &hs->file_handle;
}

/*!
 * When data has been received in the correct state, try to parse it
 * as a HTTP request.
 *
 * @param p the received pbuf
 * @param hs the connection state
 * @param pcb the tcp_pcb which received this packet
 * @return ERR_OK if request was OK and hs has been initialized correctly
 *         ERR_INPROGRESS if request was OK so far but not fully received
 *         another err_t otherwise
 */
static err_t http_parse_request(struct pbuf **inp, struct http_state *hs, struct tcp_pcb *pcb){

	char *data;
    char *crlf;
    u16_t data_len;
    struct pbuf *p = *inp;
    u16_t clen;
    LWIP_UNUSED_ARG(pcb); /* only used for post */
    LWIP_ASSERT("p != NULL", p != NULL);
    LWIP_ASSERT("hs != NULL", hs != NULL);

    if ((hs->handle != NULL) || (hs->file != NULL)){
        LWIP_DEBUGF(HTTPD_DEBUG, ("Received data while sending a file\r\n"));
        /* already sending a file */
        /* @todo: abort? */
        return ERR_USE;
    }

    LWIP_DEBUGF(HTTPD_DEBUG, ("Received %" U16_F " bytes\r\n", p->tot_len));

    /* first check allowed characters in this pbuf? */

    /* enqueue the pbuf */
    if (hs->req == NULL){
        LWIP_DEBUGF(HTTPD_DEBUG, ("First pbuf\r\n"));
        hs->req = p;
    }
    else{
        LWIP_DEBUGF(HTTPD_DEBUG, ("pbuf enqueued\r\n"));
        pbuf_cat(hs->req, p);
    }

    if (hs->req->next != NULL){
        data_len = LWIP_MIN(hs->req->tot_len, LWIP_HTTPD_MAX_REQ_LENGTH);
        pbuf_copy_partial(hs->req, httpd_req_buf, data_len, 0);
        data = httpd_req_buf;
    }
    else{
        data = (char *)p->payload;
        data_len = p->len;
        if (p->len != p->tot_len){
            LWIP_DEBUGF(HTTPD_DEBUG, ("Warning: incomplete header due to chained pbufs\r\n"));
        }
    }

    /* received enough data for minimal request? */
    if (data_len >= MIN_REQ_LEN){
        /* wait for CRLF before parsing anything */
        crlf = strnstr(data, CRLF, data_len);
        if (crlf != NULL){
            int is_09 = 0;
            char *sp1, *sp2;
            u16_t left_len, uri_len;
            LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("CRLF received, parsing request\r\n"));
            /* parse method */
            if (!strncmp(data, "GET ", 4)) {
                sp1 = data + 3;
                /* received GET request */
                LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("Received GET request\"\r\n"));
            }
            else{
                /* null-terminate the METHOD (pbuf is freed anyway wen returning) */
                data[4] = 0;
                /* unsupported method! */
                LWIP_DEBUGF(HTTPD_DEBUG, ("Unsupported request method (not implemented): \"%s\"\r\n", data));
                return http_find_error_file(hs, 501);
            }
            /* if we come here, method is OK, parse URI */
            left_len = data_len - ((sp1 + 1) - data);
            sp2 = strnstr(sp1 + 1, " ", left_len);

            if (sp2 == NULL){
                /* HTTP 0.9: respond with correct protocol version */
                sp2 = strnstr(sp1 + 1, CRLF, left_len);
                is_09 = 1;
            }
            uri_len = sp2 - (sp1 + 1);
            if ((sp2 != 0) && (sp2 > sp1)){
                /* wait for CRLFCRLF (indicating end of HTTP headers) before parsing anything */
                if (strnstr(data, CRLF CRLF, data_len) != NULL){
                    char *uri = sp1 + 1;
                    /* null-terminate the METHOD (pbuf is freed anyway wen returning) */
                    *sp1 = 0;
                    uri[uri_len] = 0;
                    LWIP_DEBUGF(HTTPD_DEBUG, ("Received \"%s\" request for URI: \"%s\"\r\n", data, uri));
                    return http_find_file(hs, uri, is_09);
                }
            }
            else{
                LWIP_DEBUGF(HTTPD_DEBUG, ("invalid URI\r\n"));
            }
        }
    }

    clen = pbuf_clen(hs->req);
    if ((hs->req->tot_len <= LWIP_HTTPD_REQ_BUFSIZE) && (clen <= LWIP_HTTPD_REQ_QUEUELEN)){
        /* request not fully received (too short or CRLF is missing) */
        return ERR_INPROGRESS;
    }
    else{
        LWIP_DEBUGF(HTTPD_DEBUG, ("bad request\r\n"));
        /* could not parse request */
        return http_find_error_file(hs, 400);
    }
}

/*!
 * @brief Try to find the file specified by uri and, if found, initialize hs
 * accordingly.
 *
 * @param hs the connection state
 * @param uri the HTTP header URI
 * @param is_09 1 if the request is HTTP/0.9 (no HTTP headers in response)
 * @return ERR_OK if file was found and hs has been initialized correctly
 *         another err_t otherwise
 */
static err_t http_find_file(struct http_state *hs, const char *uri, int is_09){
    size_t loop;
    struct fs_file *file = NULL;
    char *params;
    err_t err;
    int i;
    int count;

    const u8_t tag_check = 0;

    /* Have we been asked for the default root file? */
    if ((uri[0] == '/') && (uri[1] == 0)){
        /* Try each of the configured default filenames until we find one that exists. */
        for (loop = 0; loop < NUM_DEFAULT_FILENAMES; loop++){
            LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("Looking for %s...\r\n", g_psDefaultFilenames[loop].name));
            err = fs_open(&hs->file_handle, (char *)g_psDefaultFilenames[loop].name);
            uri = (char *)g_psDefaultFilenames[loop].name;
            if (err == ERR_OK){
                file = &hs->file_handle;
                LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("Opened.\r\n"));
                break;
            }
        }
        if (file == NULL){
            /* None of the default filenames exist so send back a 404 page */
            file = http_get_404_file(hs, &uri);
        }
    }
    else{
        /* No - we've been asked for a specific file. */
        /* First, isolate the base URI (without any parameters) */
        params = (char *)strchr(uri, '?');
        if (params != NULL){
            /* URI contains parameters. NULL-terminate the base URI */
            *params = '\0';
            params++;
        }

        /* Does the base URI we have isolated correspond to a CGI handler? */
        if (g_iNumCGIs && g_pCGIs){
            for (i = 0; i < g_iNumCGIs; i++){
                if (strcmp(uri, g_pCGIs[i].pcCGIName) == 0){
                    /* We found a CGI that handles this URI so extract the */
                    /* parameters and call the handler. */
                    count = extract_uri_parameters(hs, params);
                    uri = g_pCGIs[i].pfnCGIHandler(i, count, hs->params, hs->param_vals);
                    break;
                }
            }
        }

        LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("Opening %s\r\n", uri));

        err = fs_open(&hs->file_handle, uri);
        if (err == ERR_OK){
            file = &hs->file_handle;
        }
        else{
            file = http_get_404_file(hs, &uri);
        }
    }
    return http_init_file(hs, file, is_09, uri, tag_check);
}

/*!
 * @brief Initialize a http connection with a file to send (if found).
 * Called by http_find_file and http_find_error_file.
 *
 * @param hs http connection state
 * @param file file structure to send (or NULL if not found)
 * @param is_09 1 if the request is HTTP/0.9 (no HTTP headers in response)
 * @param uri the HTTP header URI
 * @param tag_check enable SSI tag checking
 * @return ERR_OK if file was found and hs has been initialized correctly
 *         another err_t otherwise
 */
static err_t http_init_file(struct http_state *hs, struct fs_file *file, int is_09, const char *uri, u8_t tag_check){

	if (file != NULL){
		/* file opened, initialise struct http_state */
        LWIP_UNUSED_ARG(tag_check);
        hs->handle = file;
        hs->file = (char *)file->data;
        LWIP_ASSERT("File length must be positive!", (file->len >= 0));
        hs->left = file->len;
        hs->retries = 0;
        LWIP_ASSERT("HTTP headers not included in file system", hs->handle->http_header_included);

        if (hs->handle->http_header_included && is_09){
            /* HTTP/0.9 responses are sent without HTTP header, search for the end of the header. */
            char *file_start = strnstr(hs->file, CRLF CRLF, hs->left);
            if (file_start != NULL){
                size_t diff = file_start + 4 - hs->file;
                hs->file += diff;
                hs->left -= (u32_t)diff;
            }
        }
    }
    else{
        hs->handle = NULL;
        hs->file = NULL;
        hs->left = 0;
        hs->retries = 0;
    }
    LWIP_UNUSED_ARG(uri);
    return ERR_OK;
}

/*
 * The pcb had an error and is already deallocated.
 * The argument might still be valid (if != NULL).
 */
static void http_err(void *arg, err_t err){
    struct http_state *hs = (struct http_state *)arg;
    LWIP_UNUSED_ARG(err);

    LWIP_DEBUGF(HTTPD_DEBUG, ("http_err: %s", lwip_strerr(err)));

    if (hs != NULL)
    {
        http_state_free(hs);
    }
}

/*
 * Data has been sent and acknowledged by the remote host.
 * This means that more data can be sent.
 */
static err_t http_sent(void *arg, struct tcp_pcb *pcb, u16_t len){
    struct http_state *hs = (struct http_state *)arg;

    LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("http_sent %p\r\n", (void *)pcb));

    LWIP_UNUSED_ARG(len);

    if (hs == NULL){
        return ERR_OK;
    }

    hs->retries = 0;

    http_send(pcb, hs);

    return ERR_OK;
}

/*
 * The poll function is called every 2nd second.
 * If there has been no data sent (which resets the retries) in 8 seconds, close.
 * If the last portion of a file has not been sent in 2 seconds, close.
 *
 * This could be increased, but we don't want to waste resources for bad connections.
 */
static err_t http_poll(void *arg, struct tcp_pcb *pcb){

	struct http_state *hs = (struct http_state *)arg;
    LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE,
                ("http_poll: pcb=%p hs=%p pcb_state=%s\r\n", (void *)pcb, (void *)hs, tcp_debug_state_str(pcb->state)));

    if (hs == NULL){
        err_t closed;
        /* arg is null, close. */
        LWIP_DEBUGF(HTTPD_DEBUG, ("http_poll: arg is NULL, close\r\n"));
        closed = http_close_conn(pcb, NULL);
        LWIP_UNUSED_ARG(closed);
        return ERR_OK;
    }
    else{
        hs->retries++;
        if (hs->retries == HTTPD_MAX_RETRIES){
            LWIP_DEBUGF(HTTPD_DEBUG, ("http_poll: too many retries, close\r\n"));
            http_close_conn(pcb, hs);
            return ERR_OK;
        }

        /* If this connection has a file open, try to send some more data. If */
        /* it has not yet received a GET request, don't do this since it will */
        /* cause the connection to close immediately. */
        if (hs && (hs->handle)){
            LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("http_poll: try to send more data\r\n"));
            if (http_send(pcb, hs)){
                /* If we wrote anything to be sent, go ahead and send it now. */
                LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("tcp_output\r\n"));
                tcp_output(pcb);
            }
        }
    }

    return ERR_OK;
}

/*
 * Data has been received on this pcb.
 * For HTTP 1.0, this should normally only happen once (if the request fits in one packet).
 */
static err_t http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err){

	err_t parsed = ERR_ABRT;
    struct http_state *hs = (struct http_state *)arg;
    LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE,
                ("http_recv: pcb=%p pbuf=%p err=%s\r\n", (void *)pcb, (void *)p, lwip_strerr(err)));

    if ((err != ERR_OK) || (p == NULL) || (hs == NULL)){
        /* error or closed by other side? */
        if (p != NULL){
            /* Inform TCP that we have taken the data. */
            tcp_recved(pcb, p->tot_len);
            pbuf_free(p);
        }
        if (hs == NULL){
            /* this should not happen, only to be robust */
            LWIP_DEBUGF(HTTPD_DEBUG, ("Error, http_recv: hs is NULL, close\r\n"));
        }
        http_close_conn(pcb, hs);
        return ERR_OK;
    }

	/* Inform TCP that we have taken the data. */
	tcp_recved(pcb, p->tot_len);

    {
        if (hs->handle == NULL){
            parsed = http_parse_request(&p, hs, pcb);
            LWIP_ASSERT("http_parse_request: unexpected return value",
                        parsed == ERR_OK || parsed == ERR_INPROGRESS || parsed == ERR_ARG || parsed == ERR_USE);
        }
        else{
            LWIP_DEBUGF(HTTPD_DEBUG, ("http_recv: already sending data\r\n"));
        }

        if (parsed != ERR_INPROGRESS){
            /* request fully parsed or error*/
            if (hs->req != NULL){
                pbuf_free(hs->req);
                hs->req = NULL;
            }
        }

        if (parsed == ERR_OK){
            {
                LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("http_recv: data %p len %" S32_F "\r\n", hs->file, hs->left));
                http_send(pcb, hs);
            }
        }
        else if (parsed == ERR_ARG){
            /* @todo: close on ERR_USE?*/
            http_close_conn(pcb, hs);
        }
    }
    return ERR_OK;
}

/*
 * A new incoming connection has been accepted.
 */
static err_t http_accept(void *arg, struct tcp_pcb *pcb, err_t err){

	struct http_state *hs;
    struct tcp_pcb_listen *lpcb = (struct tcp_pcb_listen *)arg;
    LWIP_UNUSED_ARG(err);
    LWIP_DEBUGF(HTTPD_DEBUG, ("http_accept %p / %p\r\n", (void *)pcb, arg));

    /* Decrease the listen backlog counter*/
    tcp_accepted(lpcb);
    /* Set priority*/
    tcp_setprio(pcb, HTTPD_TCP_PRIO);

    /* Allocate memory for the structure that holds the state of the
     * connection - initialized by that function.
     */
    hs = http_state_alloc();
    if (hs == NULL){
        LWIP_DEBUGF(HTTPD_DEBUG, ("http_accept: Out of memory, RST\r\n"));
        return ERR_MEM;
    }
    hs->pcb = pcb;

    /* Tell TCP that this is the structure we wish to be passed for our callbacks.*/
    tcp_arg(pcb, hs);

    /* Set up the various callback functions*/
    tcp_recv(pcb, http_recv);
    tcp_err(pcb, http_err);
    tcp_poll(pcb, http_poll, HTTPD_POLL_INTERVAL);
    tcp_sent(pcb, http_sent);

    return ERR_OK;
}

/*
 * Initialize the httpd with the specified local address.
 */
static void httpd_init_addr(ip_addr_t *local_addr){

	struct tcp_pcb *httppcb;
	struct tcp_pcb *servpcb;

    err_t err;

    httppcb = tcp_new();
    LWIP_ASSERT("httpd_init: tcp_new failed", httppcb != NULL);
    tcp_setprio(httppcb, HTTPD_TCP_PRIO);
    /* set SOF_REUSEADDR here to explicitly bind httpd to multiple interfaces*/
    err = tcp_bind(httppcb, local_addr, HTTPD_SERVER_PORT);
    LWIP_ASSERT("httpd_init: tcp_bind failed", err == ERR_OK);

    httppcb = tcp_listen(httppcb);
    LWIP_ASSERT("httpd_init: tcp_listen failed", httppcb != NULL);
    /*initialize callback arg and accept callback*/
    tcp_arg(httppcb, httppcb);
    tcp_accept(httppcb, http_accept);


    // Set Soquet =======================================================
    servpcb = tcp_new();
    if (servpcb != NULL){
		err_t err;
		err = tcp_bind(servpcb, local_addr, 10887);
		if (err == ERR_OK){
			servpcb = tcp_listen(servpcb);
			tcp_accept(servpcb, echo_accept);
		}
		else{
			/* abort? output diagnostic? */
		}
	}
	else{

	}

}

/*
 * Initialize the httpd: set up a listening PCB and bind it to the defined port
 */
void httpd_init(void){

    LWIP_DEBUGF(HTTPD_DEBUG, ("httpd_init\r\n"));
    httpd_init_addr(IP_ADDR_ANY);
}


/*!
 * @brief Set an array of CGI filenames/handler functions
 *
 * @param cgis an array of CGI filenames/handler functions
 * @param num_handlers number of elements in the 'cgis' array
 */
void http_set_cgi_handlers(const tCGI *cgis, int num_handlers){
    LWIP_ASSERT("no cgis given", cgis != NULL);
    LWIP_ASSERT("invalid number of handlers", num_handlers > 0);

    g_pCGIs = cgis;
    g_iNumCGIs = num_handlers;
}


// TCP echo ================================================================================


err_t echo_accept(void *arg, struct tcp_pcb *newpcb, err_t err){

	err_t ret_err;
    struct echo_state *es;

    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);

    /* Unless this pcb should have NORMAL priority, set its priority now. */
    /* When running out of pcbs, low priority pcbs can be aborted to create */
    /* new pcbs of higher priority. */
    tcp_setprio(newpcb, TCP_PRIO_MIN);

    logMessage(INFO, "Someone is trying to talk\r\n");

    es = (struct echo_state *)mem_malloc(sizeof(struct echo_state));
    if (es != NULL){
        es->state = ES_ACCEPTED;
        es->pcb = newpcb;
        es->retries = 0;
        es->p = NULL;
        /* pass newly allocated es to our callbacks */
        tcp_arg(newpcb, es);
        tcp_recv(newpcb, echo_recv);
        tcp_err(newpcb, echo_error);
        tcp_poll(newpcb, echo_poll, 0);
        ret_err = ERR_OK;
    }
    else{
        ret_err = ERR_MEM;
    }
    return ret_err;
}

err_t echo_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err){

	struct echo_state *es;
    err_t ret_err;


    LWIP_ASSERT("arg != NULL", arg != NULL);

    es = (struct echo_state *)arg;

    if (p == NULL){
        /* remote host closed connection */
    	logMessage(INFO, "Virna undocked - socket is free and listeneing now \r\n");
        es->state = ES_CLOSING;
        if (es->p == NULL){
            /* we're done sending, close it */
            echo_close(tpcb, es);
        }
        else{
            /* we're not done yet */
            tcp_sent(tpcb, echo_sent);
            echo_send(tpcb, es);
        }
        ret_err = ERR_OK;
    }
    else if (err != ERR_OK){
        /* cleanup, for unkown reason */
    	logMessage(INFO, "---- Recv -- unknown error received -> cleanup\r\n");
        if (p != NULL){
            es->p = NULL;
            pbuf_free(p);
        }
        ret_err = err;
    }
    else if (es->state == ES_ACCEPTED){
    	logMessage(INFO, "Virna docked on default port (TCP unencripted)\r\n");
        /* first data chunk in p->payload */
        es->state = ES_RECEIVED;
        /* store reference to incoming pbuf (chain) */
        es->p = p;
        /* install send completion notifier */
        tcp_sent(tpcb, echo_sent);
        echo_send(tpcb, es);
        ret_err = ERR_OK;
    }
    else if (es->state == ES_RECEIVED){
        /* read some more data */
        if (es->p == NULL){
        	//logMessage(INFO, "---- Recv ES_RECEIVED and done -> send back \r\n");
            es->p = p;
            tcp_sent(tpcb, echo_sent);
            void * acpm = es->p->payload;
            ethergate (acpm);
            echo_send(tpcb, es);
        }
        else{
            struct pbuf *ptr;
            /* chain pbufs to the end of what we recv'ed previously */
            logMessage(INFO, "---- Recv ES_RECEIVED and not done -> pbuf_chain \r\n");
            ptr = es->p;
            pbuf_chain(ptr, p);
        }

        ret_err = ERR_OK;
    }
    else if (es->state == ES_CLOSING){
    	/* odd case, remote side closing twice, trash data */
    	logMessage(INFO, "---- Recv ES_CLOSING -- Remote side closing twice\r\n");
    	tcp_recved(tpcb, p->tot_len);
        es->p = NULL;
        pbuf_free(p);
        ret_err = ERR_OK;
    }
    else{
    	/* unkown es->state, trash data */
    	logMessage(INFO, "---- Recv unknown state -- trashing data\r\n");
    	tcp_recved(tpcb, p->tot_len);
        es->p = NULL;
        pbuf_free(p);
        ret_err = ERR_OK;
    }
    return ret_err;
}


void echo_error(void *arg, err_t err){

	struct echo_state *es;

	logMessage(INFO, "Echo error....\r\n");

    LWIP_UNUSED_ARG(err);

    es = (struct echo_state *)arg;
    if (es != NULL){
        mem_free(es);
    }
}


err_t echo_poll(void *arg, struct tcp_pcb *tpcb){
    err_t ret_err;
    struct echo_state *es;

    logMessage(INFO, "Echo pool....\r\n");

    es = (struct echo_state *)arg;
    if (es != NULL){
        if (es->p != NULL){
            /* there is a remaining pbuf (chain) */
            tcp_sent(tpcb, echo_sent);
            echo_send(tpcb, es);
        }
        else{
            /* no remaining pbuf (chain) */
            if (es->state == ES_CLOSING){
                echo_close(tpcb, es);
            }
        }
        ret_err = ERR_OK;
    }
    else{
        /* nothing to be done */
        tcp_abort(tpcb);
        ret_err = ERR_ABRT;
    }
    return ret_err;
}

err_t echo_sent(void *arg, struct tcp_pcb *tpcb, u16_t len){

	struct echo_state *es;
    LWIP_UNUSED_ARG(len);

    //logMessage(INFO, "Echo sent....\r\n");

    es = (struct echo_state *)arg;
    es->retries = 0;

    if (es->p != NULL){
        /* still got pbufs to send */
        tcp_sent(tpcb, echo_sent);
        echo_send(tpcb, es);
    }
    else{
        /* no more pbufs to send */
        if (es->state == ES_CLOSING){
            echo_close(tpcb, es);
        }
    }
    return ERR_OK;
}


void echo_send(struct tcp_pcb *tpcb, struct echo_state *es){

	struct pbuf *ptr;
    err_t wr_err = ERR_OK;

    //logMessage(INFO, "Echo send....\r\n");

    while ((wr_err == ERR_OK) && (es->p != NULL) && (es->p->len <= tcp_sndbuf(tpcb))){
        ptr = es->p;

        /* enqueue data for transmission */
        wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
        if (wr_err == ERR_OK){
            u16_t plen;
            u8_t freed;

            plen = ptr->len;
            /* continue with next pbuf in chain (if any) */
            es->p = ptr->next;
            if (es->p != NULL){
                /* new reference! */
                pbuf_ref(es->p);
            }
            /* chop first pbuf from chain */
            do{
                /* try hard to free pbuf */
                freed = pbuf_free(ptr);
            } while (freed == 0);
            /* we can read more data now */
            tcp_recved(tpcb, plen);
        }
        else if (wr_err == ERR_MEM){
            /* we are low on memory, try later / harder, defer to poll */
            es->p = ptr;
        }
        else{
            /* other problem ?? */
        }
    }
}

void echo_close(struct tcp_pcb *tpcb, struct echo_state *es){

	logMessage(INFO, "Echo close....\r\n");

	tcp_arg(tpcb, NULL);
    tcp_sent(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_err(tpcb, NULL);
    tcp_poll(tpcb, NULL, 0);

    if (es != NULL){
        mem_free(es);
    }
    tcp_close(tpcb);
}


char *button_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){

	if (true == g_ButtonPress){
        g_ButtonPress = false;
        return "/btnprss.cgi";
    }
    else{
        return "/btnunprss.cgi";
    }
}

/*!
 * @brief Toggle RED RED LED.
 *
 */

char *toggle_redled(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
	//LED_RED_TOGGLE();
	GPIO_TogglePinsOutput(BOARD_LED_GPIO, 1U << BOARD_LED_GPIO_PIN);
    return "/toggleredled.cgi";
}

/*!
 * @brief Interrupt service for button press is detected.
 *
 */
void BOARD_SW_IRQ_HANDLER(void){
    /* Clear external interrupt flag. */
    GPIO_ClearPinsInterruptFlags(BOARD_SW_GPIO, 1U << BOARD_SW_GPIO_PIN);
    /* Change state of button. */
    g_ButtonPress = true;
}

/*!
 * @brief Initialize Button and LED.
 *
 */
void gpio_init(){
    /* Define the init structure for the input switch pin */
    gpio_pin_config_t sw_config = {
        kGPIO_DigitalInput, 0,
    };

    /* Define the init structure for the output LED pin */
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput, 0,
    };

    /* Init input switch GPIO. */
    PORT_SetPinInterruptConfig(BOARD_SW_PORT, BOARD_SW_GPIO_PIN, kPORT_InterruptFallingEdge);
    EnableIRQ(BOARD_SW_IRQ);
    GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_config);

    /* Init output LED GPIO. */
    GPIO_PinInit(BOARD_LED_GPIO, BOARD_LED_GPIO_PIN, &led_config);
}



// Was before @ main entry point (should be persistent ?)
struct netif fsl_netif0;

tCGI cgis[2] = {
		{"/btndata.cgi", (tCGIHandler)button_handler,},
		{"/toggleredled.cgi", (tCGIHandler)toggle_redled,}
};

/*!
 * @brief Main function
 */
void initEthernet(void){


    ip_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;

    lwip_init();
    IP4_ADDR(&fsl_netif0_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&fsl_netif0_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&fsl_netif0_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);
    netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, NULL, ethernetif_init, ethernet_input);
    netif_set_default(&fsl_netif0);
    netif_set_up(&fsl_netif0);
    http_set_cgi_handlers(cgis, 2);
    httpd_init();


    gpio_init();


    logMessage(INFO, "Antares HTTP Server is up @ %u.%u.%u.%u\r\n",
    										((u8_t *)&fsl_netif0_ipaddr)[0],
											((u8_t *)&fsl_netif0_ipaddr)[1],
											((u8_t *)&fsl_netif0_ipaddr)[2],
											((u8_t *)&fsl_netif0_ipaddr)[3]);


//    LWIP_PLATFORM_DIAG(("\r\n************************************************"));
//    LWIP_PLATFORM_DIAG((" LGT8 -- HTTP Server"));
//    LWIP_PLATFORM_DIAG(("************************************************"));
//    LWIP_PLATFORM_DIAG((" IPv4 Address     : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_ipaddr)[0],
//                        ((u8_t *)&fsl_netif0_ipaddr)[1], ((u8_t *)&fsl_netif0_ipaddr)[2],
//                        ((u8_t *)&fsl_netif0_ipaddr)[3]));
//    LWIP_PLATFORM_DIAG((" IPv4 Subnet mask : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_netmask)[0],
//                        ((u8_t *)&fsl_netif0_netmask)[1], ((u8_t *)&fsl_netif0_netmask)[2],
//                        ((u8_t *)&fsl_netif0_netmask)[3]));
//    LWIP_PLATFORM_DIAG((" IPv4 Gateway     : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_gw)[0], ((u8_t *)&fsl_netif0_gw)[1],
//                        ((u8_t *)&fsl_netif0_gw)[2], ((u8_t *)&fsl_netif0_gw)[3]));
//    LWIP_PLATFORM_DIAG(("************************************************"));

//    while (1){
//        sys_check_timeouts();
//    }

}

void serviceEthernet(void){

	 sys_check_timeouts();
}


#endif /* LWIP_TCP */
