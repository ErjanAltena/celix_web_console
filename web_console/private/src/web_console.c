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
 *  \author    	<a href="mailto:dev@celix.apache.org">Apache Celix Project Team</a>
 *  \copyright	Apache License, Version 2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#include "bundle_context.h"
#include "bundle_activator.h"
#include "utils.h"
#include "civetweb.h"
#include "array_list.h"

struct web_console_service {
        char *name;
        char *webservice;
};
 
struct shellWebActivator {
	bundle_context_pt context;
	service_reference_pt reference;
	service_listener_pt listener;
	bool running;
	celix_thread_t runnable;
};

typedef struct shellWebActivator * shell_web_activator_pt;

static void *shellWeb_runnable(void *data);

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

char *get_service_name(service_event_pt event)
{
        service_registration_pt reg;
        web_console_service *service;
        serviceReference_getServiceRegistration(event->reference, &reg);
        bundleContext_getService(context , event->reference, (void**)&service);
        return strdup(service->name);
}

//celix_status_t (*serviceChanged)(void * listener, service_event_pt event);
celix_status_t serviceChanged(void *listener_handle, service_event_pt event)  //shell_pt shell = (shell_pt) listener->handle;
{
        service_listener_pt listener = (service_listener_pt) listener_handle;
        array_list_pt registeredServices = (array_list_pt)listener->handle; 

	if (event->type == OSGI_FRAMEWORK_SERVICE_EVENT_REGISTERED) {
                char* service_name= get_service_name(event);
		//shell_addCommand(shell, event->reference);
	} else if (event->type == OSGI_FRAMEWORK_SERVICE_EVENT_UNREGISTERING) {
		//shell_removeCommand(shell, event->reference);
	}
        return CELIX_SUCCESS;

}

celix_status_t bundleActivator_start(void * userData, bundle_context_pt context) {
	celix_status_t status = CELIX_SUCCESS;

	shell_web_activator_pt act = (shell_web_activator_pt) userData;

	act->listener = (service_listener_pt) calloc(1, sizeof(act->listener));

        arrayList_create((array_list_pt*)&(act->listener->handle));

        act->listener->serviceChanged = serviceChanged; 
        bundleContext_addServiceListener(context, act->listener, "(objectClass=webConsoleService)");

	act->context = context;
	act->running = true;

	if (status == CELIX_SUCCESS) {
		celixThread_create(&act->runnable, NULL, shellWeb_runnable, act);
	}

	return status;
}

celix_status_t bundleActivator_stop(void * userData, bundle_context_pt context) {
	celix_status_t status;
	shell_web_activator_pt activator = (shell_web_activator_pt) userData;
	status = bundleContext_removeServiceListener(context, activator->listener);

	if (status == CELIX_SUCCESS) {
		free(activator->listener);

		activator->running = false;
		activator->listener = NULL;
		activator->context = NULL;
		activator->running = false;

		celixThread_join(activator->runnable, NULL);
	}

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
                        if(first) {
                                first = 0;
                        } else {
                                cnt += sprintf(&(line[cnt]), ",\n");
                        }
			cnt += sprintf(&(line[cnt]), "{\"id\":\"%d\", \"state\":\"%s\", \"name\":\"%s\", \"loc\":\"%s\"}", (int)id, stateString, name, loc);
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

static int cgi_event_handler(struct mg_connection *conn, void * data) 
{
    char *content;
    shell_web_activator_pt  act = (shell_web_activator_pt) data;
    
    // Prepare the message we're going to send
    content=getJsonProcessInfo(act);
    
    // Send HTTP reply to the client
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %d\r\n"        // Always set Content-Length
              "\r\n"
              "%s",
              strlen(content), content);

    // Returning non-zero tells civetweb that our function has replied to
    // the client, and civetweb should not send client any more data.
    return 1;
}

static void* shellWeb_runnable(void *data) {
        struct mg_callbacks callbacks;
        struct mg_context *ctx;
	shell_web_activator_pt act = (shell_web_activator_pt) data;
        const char * options[] = { "document_root", "www",
                                   "listening_ports", "8080", 0, 
                                 };

        memset(&callbacks, 0, sizeof(callbacks));
        ctx = mg_start(&callbacks, NULL, options);
        mg_set_request_handler(ctx, "/cgi", cgi_event_handler, data);
        mg_set_request_handler(ctx, "/", event_handler, data);

        while(act->running) {
                sleep(1);
        }

        return NULL;
}


