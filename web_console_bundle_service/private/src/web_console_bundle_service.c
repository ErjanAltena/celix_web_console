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
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bundle.h"
#include "bundle_activator.h"
#include "bundle_archive.h"
#include "bundle_revision.h"

#include "web_resources.h"
#include "web_console_service.h"
#include "web_console_bundle_service.h"

struct web_console {
        bundle_context_pt bundleContext;        
        char *webroot;
};

char* resources[] = {
        "bundles.html",
        "bundles.js",
        NULL
};

web_console_pt webConsoleBundleCreate(bundle_context_pt context)
{
        web_console_pt wc = calloc(sizeof(struct web_console), 1); 
        if(wc) {
                wc->webroot = NULL;
                wc->bundleContext = context;
        }

        return wc;             
}

char *webConsoleBundleServiceName(web_console_pt wc)
{
        return "Bundle viewer";
} 

char *webConsoleBundleServiceWebEntry(web_console_pt wc) 
{
        return "/bundle";
}

celix_status_t webConsoleBundleServiceCopyResources(web_console_pt wc, char *webRoot) 
{
        return copyWebResources(wc->bundleContext, resources, webRoot);
}

celix_status_t webConsoleBundleServiceRemoveResources(web_console_pt wc, char *webRoot)
{
        return removeWebResources(wc->bundleContext, resources, webRoot);
}

char * psCommand_stateString(bundle_state_e state) {
        switch (state) {
                case OSGI_FRAMEWORK_BUNDLE_ACTIVE:
                        return "Active";
                case OSGI_FRAMEWORK_BUNDLE_INSTALLED:
                        return "Installed";
                case OSGI_FRAMEWORK_BUNDLE_RESOLVED:
                        return "Resolved";
                case OSGI_FRAMEWORK_BUNDLE_STARTING:
                        return "Starting";
                case OSGI_FRAMEWORK_BUNDLE_STOPPING:
                        return "Stopping";
                default:
                        return "Unknown";
        }
}

static void start_bundle(bundle_context_pt context, int bundle_id) {
        bundle_pt bundle = NULL;
        celix_status_t status;

        status = bundleContext_getBundleById(context, bundle_id, &bundle);
        if (status == CELIX_SUCCESS) {
                status = bundle_startWithOptions(bundle, 0);
        }
        if(status != CELIX_SUCCESS) {
                printf("start_bundle failed with code %d\n", status);
        }
}

static void stop_bundle(bundle_context_pt context, int bundle_id) {
        bundle_pt bundle = NULL;
        celix_status_t status;

        status = bundleContext_getBundleById(context, bundle_id, &bundle);
        if (status == CELIX_SUCCESS) {
                status = bundle_stopWithOptions(bundle, 0);
        }
        if(status != CELIX_SUCCESS) {
                printf("stop_bundle failed with code %d\n", status);
        }
}

char *webConsoleBundleServiceGetJsonData(web_console_pt wc, char *query) {
        array_list_pt bundles = NULL;
        celix_status_t status = bundleContext_getBundles(wc->bundleContext, &bundles);
        unsigned int size = arrayList_size(bundles);
        static char line[1024];
        static char *archiveRoot;
        int cnt = 0;
        int i;
        bundle_pt bundlesA[size];

        if (query) {
                int bundle_id;
                if (sscanf(query, "command=stop&id=%d", &bundle_id) == 1) {
                        stop_bundle(wc->bundleContext, bundle_id);
                } else if (sscanf(query, "command=start&id=%d", &bundle_id) == 1) {
                        start_bundle(wc->bundleContext, bundle_id);
                } else if (sscanf(query, "command=uninstall&id=%d", &bundle_id) == 1) {
                        bundle_pt bundle;
                        bundleContext_getBundleById(wc->bundleContext, bundle_id, &bundle);
                        bundle_uninstall(bundle);
                        //                } else if(sscanf(query,"command=install&id=%s", command) == 1) {
                        //                        //install bundle
                }
        }
        for (i = 0; i < size; i++) {
                bundlesA[i] = arrayList_get(bundles, i);
        }
        if (status == CELIX_SUCCESS) {
                int j;
                int first = 1;
                for (i = 0; i < size - 1; i++) {
                        for (j = i + 1; j < size; j++) {
                                bundle_pt first = bundlesA[i];
                                bundle_pt second = bundlesA[j];

                                bundle_archive_pt farchive = NULL, sarchive = NULL;
                                long fid, sid;

                                bundle_getArchive(first, &farchive);
                                bundleArchive_getId(farchive, &fid);
                                bundle_getArchive(second, &sarchive);
                                bundleArchive_getId(sarchive, &sid);

                                if (fid > sid) {
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
                        if (first) {
                                first = 0;
                        } else {
                                cnt += sprintf(&(line[cnt]), ",\n");
                        }
                        cnt += sprintf(&(line[cnt]), "{\"id\":\"%d\", \"state\":\"%s\", \"name\":\"%s\", \"loc\":\"%s\", \"archiveroot\":\"%s\" }", (int) id, stateString, name, loc, archiveRoot);
                }
                cnt += sprintf(&(line[cnt]), "\n]}");

                arrayList_destroy(bundles);
                return line;

        }
        return "<h1>ERROR</h1>";

}

char * webConsoleBundleServiceGetMainWebPage(web_console_pt wc) 
{
        return "bundles.html";
}
