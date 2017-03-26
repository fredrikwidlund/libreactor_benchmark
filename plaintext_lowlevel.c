#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/queue.h>

#include <dynamic.h>
#include <reactor.h>

#include "setup.h"

static void http_event(void *state, int type, void *data)
{
  reactor_http *http = state;

  switch (type)
    {
    case REACTOR_HTTP_EVENT_HANGUP:
    case REACTOR_HTTP_EVENT_ERROR:
      reactor_http_close(http);
      break;
    case REACTOR_HTTP_EVENT_CLOSE:
      free(http);
      break;
    case REACTOR_HTTP_EVENT_REQUEST:
      reactor_http_write_response(http, (reactor_http_response[]){{1, 200, "OK",
              1, (reactor_http_header[]){{"Content-Type", "text/plain"}}, "Hello, World!", 13}});
      break;
    }
}

static void connection_event(void *state, int type, void *data)
{
  reactor_tcp *tcp = state;
  reactor_http *http;

  switch (type)
    {
    case REACTOR_TCP_EVENT_ERROR:
      reactor_tcp_close(tcp);
      break;
    case REACTOR_TCP_EVENT_ACCEPT:
      http = malloc(sizeof *http);
      reactor_http_open(http, http_event, http, *(int *) data, REACTOR_HTTP_FLAG_SERVER);
      break;
      }
}

int main()
{
  reactor_tcp tcp;

  setup(2, 0);
  reactor_core_construct();
  reactor_tcp_open(&tcp, connection_event, &tcp, "127.0.0.1", "80", REACTOR_TCP_FLAG_SERVER);
  reactor_core_run();
  reactor_core_destruct();
}
