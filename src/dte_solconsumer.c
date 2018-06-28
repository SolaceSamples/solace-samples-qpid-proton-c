/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 * dte_solconsumer
 *
 * This Sample demonstrates how to create a durable subscription
 * and consume messages using an AMQP receiver link with the 
 * solace amqp address prefix 'dsub://'. 
 */

#include <proton/connection.h>
#include <proton/condition.h>
#include <proton/delivery.h>
#include <proton/link.h>
#include <proton/message.h>
#include <proton/proactor.h>
#include <proton/session.h>
#include <proton/transport.h>
#include <proton/sasl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

typedef struct app_data_t {
  const char *host, *port;
  const char *username, *password;
  const char *subscription_name;
  const char *amqp_address;
  const char *amqp_address_prefix;
  const char *container_id;
  int message_count;

  pn_proactor_t *proactor;
  int received;
  bool finished;
  pn_rwbytes_t msgin;       /* Partially received message */
} app_data_t;

static const int BATCH = 1000; /* Batch size for unlimited receive */

static int exit_code = 0;

extern int optind;
extern char* optarg;
extern int optopt;
extern int opterr;

#define str_free(strptr) free((void *)strptr)

static void check_condition(pn_event_t *e, pn_condition_t *cond) {
  if (pn_condition_is_set(cond)) {
    fprintf(stderr, "%s: %s: %s\n", pn_event_type_name(pn_event_type(e)),
            pn_condition_get_name(cond), pn_condition_get_description(cond));
    pn_connection_close(pn_event_connection(e));
    exit_code = 1;
  }
}

static void decode_message(pn_rwbytes_t data) {
  pn_message_t *m = pn_message();
  int err = pn_message_decode(m, data.start, data.size);
  if (!err) {
    /* Print the decoded message */
    pn_string_t *s = pn_string(NULL);
    pn_inspect(pn_message_body(m), s);
    printf("%s\n", pn_string_get(s));
    pn_free(s);
    pn_message_free(m);
    free(data.start);
  } else {
    fprintf(stderr, "decode_message: %s\n", pn_code(err));
    exit_code = 1;
  }
}

/* Return true to continue, false to exit */
static bool handle(app_data_t* app, pn_event_t* event) {
  switch (pn_event_type(event)) {

   case PN_CONNECTION_INIT: {
     pn_connection_t* c = pn_event_connection(event);
     /* Set authenticate credentials if present */
     if (app->username) {
        pn_connection_set_user(c, app->username);
        pn_connection_set_password(c, app->password);
     }
     pn_connection_set_container(c, app->container_id);
     pn_connection_open(c);
   } break;
   
   case PN_CONNECTION_REMOTE_OPEN: {
     pn_connection_t* c = pn_event_connection(event);
     pn_session_t* s = pn_session(c);
     pn_session_open(s);
     {
     char amqp_address[PN_MAX_ADDR];
     /*
      * To Create a durable subscription create an AMQP Receiver link
      * with the following:
      * 1) set a uniquely identifiable link name
      * 2) set an AMQP topic terminus source address using the address 
      *    prefix 'dsub://'
      *
      * Where the link name is the subscription name.
      * Where the AMQP terminus address specifies the topic and that the 
      * subscription is durable.
      *
      * */
     /* the subscription name is the name of the link */
     pn_link_t* l = pn_receiver(s, app->subscription_name);
     if(amqp_destination_address(amqp_address, PN_MAX_ADDR,
                              app->amqp_address, strlen(app->amqp_address),
                              app->amqp_address_prefix, strlen(app->amqp_address_prefix)) < 0) {
        fprintf(stderr, "failed to format amqp terminus address\n");
        exit_code=1;
        return false;
     }
     printf("Setting amqp link terminus address to: '%s'\n", amqp_address);
     /* set the topic on the subscription and durability */
     pn_terminus_set_address(pn_link_source(l), amqp_address);
     pn_link_open(l);
     /* cannot receive without granting credit: */
     pn_link_flow(l, app->message_count ? app->message_count : BATCH);
     }
   } break;

   case PN_DELIVERY: {
     /* A message has been received */
     pn_delivery_t *d = pn_event_delivery(event);
     if (pn_delivery_readable(d)) {
       pn_link_t *l = pn_delivery_link(d);
       size_t size = pn_delivery_pending(d);
       pn_rwbytes_t* m = &app->msgin; /* Append data to incoming message buffer */
       int recv;
       size_t oldsize = m->size;
       m->size += size;
       m->start = (char*)realloc(m->start, m->size);
       recv = pn_link_recv(l, m->start + oldsize, m->size);
       if (recv == PN_ABORTED) {
         fprintf(stderr, "Message aborted\n");
         m->size = 0;           /* Forget the data we accumulated */
         pn_delivery_settle(d); /* Free the delivery so we can receive the next message */
         pn_link_flow(l, 1);    /* Replace credit for aborted message */
       } else if (recv < 0 && recv != PN_EOS) {        /* Unexpected error */
         pn_condition_format(pn_link_condition(l), "broker", "PN_DELIVERY error: %s", pn_code(recv));
         pn_link_close(l);               /* Unexpected error, close the link */
       } else if (!pn_delivery_partial(d)) { /* Message is complete */
         decode_message(*m);
         *m = pn_rwbytes_null;  /* Reset the buffer for the next message*/
         /* Accept the delivery */
         pn_delivery_update(d, PN_ACCEPTED);
         pn_delivery_settle(d);  /* settle and free d */
         if (app->message_count == 0) {
           /* receive forever - see if more credit is needed */
           if (pn_link_credit(l) < BATCH/2) {
             /* Grant enough credit to bring it up to BATCH: */
             pn_link_flow(l, BATCH - pn_link_credit(l));
           }
         } else if (++app->received >= app->message_count) {
           pn_session_t *ssn = pn_link_session(l);
           printf("%d messages received\n", app->received);
           pn_link_close(l);
           pn_session_close(ssn);
           pn_connection_close(pn_session_connection(ssn));
         }
       }
     }
     break;
   }

   case PN_TRANSPORT_CLOSED:
    check_condition(event, pn_transport_condition(pn_event_transport(event)));
    break;

   case PN_CONNECTION_REMOTE_CLOSE:
    check_condition(event, pn_connection_remote_condition(pn_event_connection(event)));
    pn_connection_close(pn_event_connection(event));
    break;

   case PN_SESSION_REMOTE_CLOSE:
    check_condition(event, pn_session_remote_condition(pn_event_session(event)));
    pn_connection_close(pn_event_connection(event));
    break;

   case PN_LINK_REMOTE_CLOSE:
   case PN_LINK_REMOTE_DETACH:
    check_condition(event, pn_link_remote_condition(pn_event_link(event)));
    pn_connection_close(pn_event_connection(event));
    break;

   case PN_PROACTOR_INACTIVE:
    return false;
    break;

   default:
    break;
  }
    return true;
}

void run(app_data_t *app) {
  /* Loop and handle events */
  do {
    pn_event_batch_t *events = pn_proactor_wait(app->proactor);
    pn_event_t *e;
    for (e = pn_event_batch_next(events); e; e = pn_event_batch_next(events)) {
      if (!handle(app, e) || exit_code != 0) {
        return;
      }
    }
    pn_proactor_done(app->proactor, events);
  } while(true);
}

void usage() {
    printf("Usage: dte_solconsumer [options] \n");
    printf("[Options]:\n");
    printf("\t-a      The host address [localhost]\n");
    printf("\t-p      The host port [5672]\n");
    printf("\t-c      # of messages to consume [10]\n");
    printf("\t-t      Target topic address [my_topic]\n");
    printf("\t-n      Subscription name [my_sub]\n");
    printf("\t-i      Container name [dte_sol_consumer]\n");
    printf("\t-u      Client authentication username []\n");
    printf("\t-P      Client authentication password []\n");
    printf("\t-h      Displays this message\n");
    exit(0);

}

#define DEFAULT_AMQP_DURABLE_TOPIC_ENDPOINT_PREFIX "dsub://"

#define AMQP_DURABLE_TOPIC_ENDPOINT_PREFIX DEFAULT_AMQP_DURABLE_TOPIC_ENDPOINT_PREFIX

void parse_args(int argc, char **argv, app_data_t *app) {
    char c;
    char con_id[PN_MAX_ADDR];
    if (container_id(con_id, PN_MAX_ADDR, argv[0], sizeof(argv[0])) < 0){
        fprintf(stderr, "Unable to format container id from source: %s", argv[0]);
        exit(1);
    }
    /* initialize default values*/
    app->container_id = strdup(con_id); /* default to using argv[0] */
    app->host = "localhost";
    app->port = "amqp";
    app->subscription_name = "my_sub"; 
    app->amqp_address = "my_topic";
    app->message_count = 10;
    app->username = NULL;
    app->password = NULL;

    /*
     * The 'dsub://' is the address prefix for durable subscriptions for the
     * Solace PubSub+ Message Broker.
     */
    app->amqp_address_prefix = AMQP_DURABLE_TOPIC_ENDPOINT_PREFIX;

    /* command line options */
    opterr = 0;
    while((c = getopt(argc, argv, "i:a:c:t:p:u:P:n:h")) != -1) {
        switch(c) {
        case 'h': usage(); break;
        case 'c':
            app->message_count = atoi(optarg);
            if (app->message_count < 0) usage();
            break;
        case 'a': app->host = optarg; break;
        case 'i':
            if (container_id(con_id, PN_MAX_ADDR, optarg, sizeof(optarg)) < 0) {
                fprintf(stderr, "Unable to format container id from source: %s", optarg);
                exit(1);
            }
            str_free(app->container_id);
            app->container_id = strdup(con_id);
            break;
        case 't': app->amqp_address = optarg; break;
        case 'n': app->subscription_name = optarg; break;
        case 'p': app->port = optarg; break;
        case 'u': app->username = optarg; break;
        case 'P': app->password = optarg; break;
        default: usage(); break;
        }
    }

}

int main(int argc, char **argv) {
    struct app_data_t app = {0};
    char addr[PN_MAX_ADDR];

    parse_args(argc, argv, &app);

    /* Create the proactor and connect */
    app.proactor = pn_proactor();
    pn_proactor_addr(addr, sizeof(addr), app.host, app.port);
    fprintf(stdout, "Connecting to host: %s\n", addr);

    /* Initialize Sasl transport */
    pn_transport_t *pnt = pn_transport();
    pn_sasl_set_allow_insecure_mechs(pn_sasl(pnt), true);

    pn_proactor_connect2(app.proactor, NULL, pnt, addr);
    fprintf(stdout, "waiting to receive %d messages from amqp address: %s\n", app.message_count, app.amqp_address);
    run(&app);
    pn_proactor_free(app.proactor);
    str_free(app.container_id);
    return exit_code;
}
