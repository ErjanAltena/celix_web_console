/**
 * Copyright  2015 Erjan Altena
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

#include "bundle_context.h"
#include "bundle_activator.h"
#include "utils.h"
#include "array_list.h"

#include "web_resources.h"
#include "civetweb.h"
#include "array_list.h"
#include "service_tracker.h" 
#include "service_tracker_customizer.h" 

#include "web_console_service.h"
#include "web_service_tracker.h"
#include "web_console.h"

char* resources[] = {"index.html",
                     "celix.png",
                      NULL};

typedef struct shellWebActivator * shell_web_activator_pt;

static int event_handler(struct mg_connection *conn, void * data); 

celix_status_t bundleActivator_create(bundle_context_pt context, void **userData) {
        celix_status_t status = CELIX_SUCCESS;

        shell_web_activator_pt activator = (shell_web_activator_pt) calloc(1, sizeof(*activator));

        if (activator) {
                (*userData) = activator;
        }
        else {
                status = CELIX_ENOMEM;
        }

        return status;
}

celix_status_t bundleActivator_destroy(void * userData, bundle_context_pt context) {
        shell_web_activator_pt activator = (shell_web_activator_pt) userData;

        free(activator);

        return CELIX_SUCCESS;
}

celix_status_t bundleActivator_start(void * userData, bundle_context_pt context) {
        celix_status_t status = CELIX_SUCCESS;
        service_tracker_customizer_pt cust = NULL;
        service_tracker_pt tracker = NULL;

        shell_web_activator_pt act = (shell_web_activator_pt) userData;
        mkdir(WEB_ROOT, 0777); 
        if (copyWebResources(context, resources, WEB_ROOT) != CELIX_SUCCESS) {
                printf("Error copyWebResources\n");
        }
        struct mg_callbacks callbacks;
        const char * options[] = { "document_root", WEB_ROOT,
                                   "listening_ports", "8080", 0, 
                                 };

        memset(&callbacks, 0, sizeof(callbacks));
        struct mg_context *ctx;
        ctx = mg_start(&callbacks, NULL, options);
        mg_set_request_handler(ctx, "/", event_handler, act);
        mg_set_request_handler(ctx, "/plugins", plugin_event_handler, act);
        act->webContext = ctx;
        arrayList_create(&(act->trackedServices));
        serviceTrackerCustomizer_create(act, NULL, wst_service_added, NULL, wst_service_removed, &cust);
        serviceTracker_create(context, WEB_CONSOLE_SERVICE, cust, &tracker);
        act->tracker = tracker;
        serviceTracker_open(tracker);
        act->context = context;
        act->running = true;

        return status;
}

celix_status_t bundleActivator_stop(void * userData, bundle_context_pt context) {
        celix_status_t status = CELIX_SUCCESS;
        shell_web_activator_pt activator = (shell_web_activator_pt) userData;

        activator->running = false;
        activator->context = NULL;

        mg_set_request_handler(activator->webContext, "/plugins", NULL, activator);
        mg_stop(activator->webContext);
        serviceTracker_close(activator->tracker);
        serviceTracker_destroy(activator->tracker);
        removeWebResources(context, resources, WEB_ROOT);

        return status;
}

char * psCommand_stateString(bundle_state_e state) {
        switch (state) {
                case OSGI_FRAMEWORK_BUNDLE_ACTIVE:
                        return "Active      ";
                case OSGI_FRAMEWORK_BUNDLE_INSTALLED:
                        return "Installed   ";
                case OSGI_FRAMEWORK_BUNDLE_RESOLVED:
                        return "Resolved    ";
                case OSGI_FRAMEWORK_BUNDLE_STARTING:
                        return "Starting    ";
                case OSGI_FRAMEWORK_BUNDLE_STOPPING:
                        return "Stopping    ";
                default:
                        return "Unknown     ";
        }
}

static int event_handler(struct mg_connection *conn, void * data) 
{
        // return 0 means that the contents of the webroot will be returned
        return 0;
}

int plugin_event_handler(struct mg_connection *conn, void *data)
{
        char jsonbuffer[4096];
        int offset = 0;
        int i = 0;
        shell_web_activator_pt  act = (shell_web_activator_pt) data;
        if(act->running) {
                int size = arrayList_size(act->trackedServices);
                offset += sprintf(&(jsonbuffer[offset]), "{\"plugins\":[");
                for(i = 0; i < size; i++) {
                        web_console_service_pt webService = (web_console_service_pt) arrayList_get(act->trackedServices, i);
                        char * title = webService->getServiceName(webService->webConsole);
                        char * page  = webService->getMainWebPage(webService->webConsole);
                        offset += sprintf(&(jsonbuffer[offset]), "{\"title\":\"%s\", \"mainpage\":\"%s\"}", title, page);
                        if(i != (size - 1)) {
                                offset += sprintf(&(jsonbuffer[offset]), ",");
                        }
                }
                offset += sprintf(&(jsonbuffer[offset]), "]}");
                mg_printf(conn,
                          "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/plain\r\n"
                          "Content-Length: %d\r\n"        // Always set Content-Length
                          "\r\n"
                          "%s",
                          offset, jsonbuffer);
        }
        return 1;
}

int cgi_event_handler(struct mg_connection *conn, void * data) 
{
    char *content;
    int i;
    int result = 0;
    shell_web_activator_pt  act = (shell_web_activator_pt) data;
    if(act->running) { 
        // find the service which is requested
        int size = arrayList_size(act->trackedServices);
        for(i = 0; i < size; i++) {
                web_console_service_pt webService = (web_console_service_pt) arrayList_get(act->trackedServices, i);
                char * entry = webService->getWebEntry(webService->webConsole);

                const struct mg_request_info *reqInfo = mg_get_request_info(conn);
                if (!strcmp(entry, reqInfo->uri )) {
                        // Prepare the message we're going to send
                        content = webService->getJsonData(webService->webConsole);
            
                        // Send HTTP reply to the client
                        mg_printf(conn,
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: %d\r\n"        // Always set Content-Length
                                "\r\n"
                                "%s",
                                (int)strlen(content), content);
                                result = 1; //request is handled, no further actions needed. 
                                break;
                }  
        } 
    }
 
    return result;
}


