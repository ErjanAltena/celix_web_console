/**
 *Licensed to the Apache Software Foundation (ASF) under one
 *or more contributor license agreements.  See the NOTICE file
 *distributed with this work for additional information
 *regarding copyright ownership.  The ASF licenses this file
 *to you under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in compliance
 *with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing,
 *software distributed under the License is distributed on an
 *"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 *specific language governing permissions and limitations
 *under the License.
 */
/*
 * shell_web.c
 *
 *  \date       Aug 13, 2010
 *  \author            <a href="mailto:dev@celix.apache.org">Apache Celix Project Team</a>
 *  \copyright        Apache License, Version 2.0
 */
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

static void *shellWeb_runnable(void *data);
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
        printf("activator-start\n");
        celix_status_t status = CELIX_SUCCESS;
        service_tracker_customizer_pt cust = NULL;
        service_tracker_pt tracker = NULL;

        shell_web_activator_pt act = (shell_web_activator_pt) userData;
        printf("Activator web_console-start: %p\n");
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
        printf("Set webcontext\n");
        mg_set_request_handler(ctx, "/", event_handler, act);
        act->webContext = ctx;
        arrayList_create(&(act->trackedServices));
        printf("ArrayList = %p\n", act->trackedServices);
        serviceTrackerCustomizer_create(act, NULL, wst_service_added, NULL, wst_service_removed, &cust);
        serviceTracker_create(context, WEB_CONSOLE_SERVICE, cust, &tracker);
        act->tracker = tracker;
        serviceTracker_open(tracker);
        act->context = context;
        act->running = true;

        if (status == CELIX_SUCCESS) {
                celixThread_create(&act->runnable, NULL, shellWeb_runnable, act);
        }

        return status;
}

celix_status_t bundleActivator_stop(void * userData, bundle_context_pt context) {
        printf("activator-stop\n");
        celix_status_t status = CELIX_SUCCESS;
        shell_web_activator_pt activator = (shell_web_activator_pt) userData;

        activator->running = false;
        activator->context = NULL;

        celixThread_join(activator->runnable, NULL);
        sleep(2);
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

char * getJsonProcessInfo(shell_web_activator_pt act) 
{
        array_list_pt bundles = NULL;
        celix_status_t status = bundleContext_getBundles(act->context, &bundles);
        unsigned int size = arrayList_size(bundles);
        static char line[1024]; 
        static char *archiveRoot; 
        int cnt = 0;
        int i;
        bundle_pt bundlesA[size];
        for (i = 0; i < size; i++) {
                bundlesA[i] = arrayList_get(bundles, i);
        }
        if (status == CELIX_SUCCESS) {
                int j;
                int first = 1;
                for(i=0; i < size - 1; i++) {
                        for(j=i+1; j < size; j++) {
                                bundle_pt first = bundlesA[i];
                                bundle_pt second = bundlesA[j];

                                bundle_archive_pt farchive = NULL, sarchive = NULL;
                                long fid, sid;

                                bundle_getArchive(first, &farchive);
                                bundleArchive_getId(farchive, &fid);
                                bundle_getArchive(second, &sarchive);
                                bundleArchive_getId(sarchive, &sid);

                                if(fid > sid)
                                {
                                         // these three lines swap the elements bundles[i] and bundles[j].
                                         bundle_pt temp = bundlesA[i];
                                         bundlesA[i] = bundlesA[j];
                                         bundlesA[j] = temp;
                                }
                        }
                }
                cnt += sprintf(&(line[cnt]), "{\"Bundles\":[\n");
                for (i = 0; i < size; i++) {
                        bundle_pt bundle = bundlesA[i];
                        bundle_archive_pt archive = NULL;
                        long id;
                        bundle_state_e state;
                        char * stateString = NULL;
                        module_pt module = NULL;
                        char * name = NULL;
                        char * loc = NULL;

                        bundle_getArchive(bundle, &archive);
                        bundleArchive_getId(archive, &id);
                        bundle_getState(bundle, &state);
                        stateString = psCommand_stateString(state);
                        bundle_getCurrentModule(bundle, &module);
                        module_getSymbolicName(module, &name);
                        bundleArchive_getLocation(archive, &loc);
                        bundleArchive_getArchiveRoot(archive, &archiveRoot);
                        if(first) {
                                first = 0;
                        } else {
                                cnt += sprintf(&(line[cnt]), ",\n");
                        }
                        cnt += sprintf(&(line[cnt]), "{\"id\":\"%d\", \"state\":\"%s\", \"name\":\"%s\", \"loc\":\"%s\", \"archiveroot\":\"%s\" }", (int)id, stateString, name, loc, archiveRoot);
                }
                cnt += sprintf(&(line[cnt]), "\n]}");
               
                arrayList_destroy(bundles);
                return line;

        }
        return "<h1>ERROR</h1>";
}


static int event_handler(struct mg_connection *conn, void * data) 
{
        // return 0 means that the contents of the webroot will be returned
        return 0;
}

int cgi_event_handler(struct mg_connection *conn, void * data) 
{
    char *content;
    int i;
    int result = 0;
    shell_web_activator_pt  act = (shell_web_activator_pt) data;
    printf("cgi_event_handler: activator %p\n", act);
    if(act->running) { 
        // find the service which is requested
        int size = arrayList_size(act->trackedServices);
        printf("Tracker has %d services\n", size);
        for(i = 0; i < size; i++) {
                web_console_service_pt webService = (web_console_service_pt) arrayList_get(act->trackedServices, i);
                char * entry = webService->getWebEntry(webService->webConsole);

                const struct mg_request_info *reqInfo = mg_get_request_info(conn);
                printf("cgi-handler: entry=%s, uri=%s\n", entry, reqInfo->uri);
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

static void* shellWeb_runnable(void *data) {
        shell_web_activator_pt act = (shell_web_activator_pt) data;
        //mg_set_request_handler(ctx, "/cgi", cgi_event_handler, data);
        //mg_set_request_handler(ctx, "/", event_handler, data);

        while(act->running) {
                sleep(1);
        }
        //mg_set_request_handler(ctx, "/cgi", NULL, data);
        //mg_set_request_handler(ctx, "/", NULL, data);

        return NULL;
}


