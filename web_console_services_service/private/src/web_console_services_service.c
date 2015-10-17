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
#include "web_console_services_service.h"

struct web_console {
        bundle_context_pt bundleContext;        
        char *webroot;
};

char* resources[] = {
        "services.html",
        NULL
};

web_console_pt webConsoleServicesCreate(bundle_context_pt context)
{
        web_console_pt wc = calloc(sizeof(struct web_console), 1); 
        if(wc) {
                wc->webroot = NULL;
                wc->bundleContext = context;
        }

        return wc;             
}

char *webConsoleServicesServiceName(web_console_pt wc)
{
        return "Services viewer";
} 

char *webConsoleServicesServiceWebEntry(web_console_pt wc) 
{
        return "/services";
}

celix_status_t webConsoleServicesServiceCopyResources(web_console_pt wc, char *webRoot) 
{
        return copyWebResources(wc->bundleContext, resources, webRoot);
}

celix_status_t webConsoleServicesServiceRemoveResources(web_console_pt wc, char *webRoot)
{
        return removeWebResources(wc->bundleContext, resources, webRoot);
}

char *webConsoleServicesServiceGetJsonData(web_console_pt wc) 
{
        static char jsonBuffer[4096];
        int idx = 0;
        array_list_pt servRefs = NULL;

        bundleContext_getServiceReferences(wc->bundleContext, NULL, NULL, &servRefs);
        int len= arrayList_size(servRefs);
        int x;
        
        idx += sprintf(&(jsonBuffer[idx]), "{\"Services\":[");
        for (x=0; x < len; x++) {
                char *servName = NULL;
                service_reference_pt servRef = arrayList_get(servRefs, x);
                
                serviceReference_getProperty(servRef, "objectClass", &servName);
                idx += sprintf(&(jsonBuffer[idx]), "{\"servicename\":\"%s\", \"properties\": [", servName);
                char **keys;
                unsigned int size;
                serviceReference_getPropertyKeys(servRef, &keys, &size);
                int k=0;
                for(k = 0; k < size; k++) {
                        //if(strcmp(keys[k], "objectClass")) { // objectclass already used for servicename, not needed
                                char *val = NULL;
                                serviceReference_getProperty(servRef, keys[k], &val);
                                idx += sprintf(&(jsonBuffer[idx]), "{\"key\":\"%s\", \"value\":\"%s\" } ", keys[k], val);
                        //}
                        if(k != (size -1)) {
                                idx += sprintf(&(jsonBuffer[idx]), ",");
                        }
                }
                idx += sprintf(&(jsonBuffer[idx]), "]}");
                if(x != (len -1)) {
                        idx += sprintf(&(jsonBuffer[idx]), ",");
                }
                idx += sprintf(&(jsonBuffer[idx]), "\n");
        } 
        idx += sprintf(&(jsonBuffer[idx]), "]}");
        return jsonBuffer;

}

char * webConsoleServicesServiceGetMainWebPage(web_console_pt wc) 
{
        return "services.html";
}
