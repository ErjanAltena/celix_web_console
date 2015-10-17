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
#include "web_console_service.h"
#include "bundle_activator.h"
#include "web_console_services_service.h"

struct web_console_activator {
        web_console_service_pt webConsole;
	service_registration_pt reg;
};


celix_status_t bundleActivator_create(bundle_context_pt context, void **userData)
{
        celix_status_t result = CELIX_SUCCESS;
        struct web_console_activator *wca = calloc(sizeof(struct web_console_activator),1);

        if(wca == NULL) {
                *userData = NULL;
                result = CELIX_START_ERROR;
                        
        } else {
                *userData = wca;
        }
                
        return result;
}

celix_status_t bundleActivator_start(void * userData, bundle_context_pt context)
{
        celix_status_t result = CELIX_SUCCESS;
        struct web_console_activator *wca = (struct web_console_activator*) userData;

        wca->webConsole = calloc(sizeof(struct web_console_service),1);
        if(wca->webConsole) {
                wca->webConsole->webConsole = webConsoleServicesCreate(context);
                wca->webConsole->getServiceName = webConsoleServicesServiceName;
                wca->webConsole->getWebEntry = webConsoleServicesServiceWebEntry;
                wca->webConsole->copyRecourcesToWebDirectory = webConsoleServicesServiceCopyResources;
                wca->webConsole->removeRecourcesFromWebDirectory = webConsoleServicesServiceRemoveResources;
                wca->webConsole->getJsonData = webConsoleServicesServiceGetJsonData;
                wca->webConsole->getMainWebPage = webConsoleServicesServiceGetMainWebPage;
                bundleContext_registerService(context, WEB_CONSOLE_SERVICE, wca->webConsole, NULL, &wca->reg);
        } else {
                wca->webConsole = NULL;
                result = CELIX_START_ERROR;
        }
        return result;
}

celix_status_t bundleActivator_stop(void * userData, bundle_context_pt context) 
{
        celix_status_t result = CELIX_SUCCESS;
        struct web_console_activator *wca = (struct web_console_activator*) userData;

	serviceRegistration_unregister(wca->reg);
        if(wca->webConsole) {
                free(wca->webConsole);
                wca->webConsole = NULL;
        } else {
                result = CELIX_START_ERROR;
        }
        return result;
}

celix_status_t bundleActivator_destroy(void * userData, bundle_context_pt context) 
{
        free(userData);       
        return CELIX_SUCCESS;
}

