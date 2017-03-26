#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/queue.h>

#include <dynamic.h>
#include <reactor.h>

#include "setup.h"

typedef struct reactor_http_data reactor_http_data;
struct reactor_http_data
{
  void   *base;
  size_t  size;
};

typedef struct client client;
struct client
{
  reactor_http http;
  vector       data;
};

static void push(vector *data, void *base, size_t size)
{
  vector_push_back(data, (reactor_http_data[]){{.base = base, .size = size}});
}

static void pushs(vector *data, char *string)
{
  push(data, string, strlen(string));
}

static void respond(client *client, char *body, char *length)
{
  vector *data = &client->data;

  pushs(data, "HTTP/1.1 200 OK\r\n");
  pushs(data, "Content-Type: text/plain\r\n");
  pushs(data, "Content-Length: ");
  pushs(data, length);
  pushs(data, "\r\n\r\n");
  pushs(data, body);
}

static void flush(client *client)
{
  reactor_http_data *data;
  size_t i, n;

  i = 0;
  if (!buffer_size(&client->http.stream.output))
    {
      n = writev(client->http.stream.fd, vector_data(&client->data), vector_size(&client->data));
      while (n && i < vector_size(&client->data))
        {
          data = vector_at(&client->data, i);
          if (n < data->size)
            {
              data->base = (char *) data->base + n;
              data->size -= n;
              break;
            }
          n -= data->size;
          i ++;
        }
    }
  while (i < vector_size(&client->data))
    {
      data = vector_at(&client->data, i);
      buffer_insert(&client->http.stream.output, buffer_size(&client->http.stream.output), data->base, data->size);
      i ++;
    }

  vector_erase_range(&client->data, 0, vector_size(&client->data));
}

static void http_event(void *state, int type, void *data)
{
  client *client = state;

  switch (type)
    {
    case REACTOR_HTTP_EVENT_HANGUP:
    case REACTOR_HTTP_EVENT_ERROR:
      reactor_http_close(&client->http);
      break;
    case REACTOR_HTTP_EVENT_CLOSE:
      free(client);
      break;
    case REACTOR_HTTP_EVENT_REQUEST:
      respond(client, "Hello, World!", "13");
      flush(client);
      break;
    }
}

static void connection_event(void *state, int type, void *data)
{
  reactor_tcp *tcp = state;
  client *client;

  switch (type)
    {
    case REACTOR_TCP_EVENT_ERROR:
      reactor_tcp_close(tcp);
      break;
    case REACTOR_TCP_EVENT_ACCEPT:
      client = malloc(sizeof *client);
      vector_construct(&client->data, sizeof(reactor_http_data));
      reactor_http_open(&client->http, http_event, client, *(int *) data, REACTOR_HTTP_FLAG_SERVER);
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
