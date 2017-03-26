#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <dynamic.h>
#include <reactor.h>

#include "setup.h"

void hello(void *state, int type, void *data)
{
  reactor_http_server_context *context = data;

  (void) state;
  (void) type;
  reactor_http_server_respond_text(context->session, "Hello, World!");
}

int main()
{
  reactor_http_server server;

  setup(2, 0);
  reactor_core_construct();
  reactor_http_server_open(&server, NULL, &server, "127.0.0.1", "80");
  reactor_http_server_route(&server, hello, NULL, "GET", "/");
  reactor_core_run();
  reactor_core_destruct();
}
