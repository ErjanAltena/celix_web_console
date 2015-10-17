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
#include "web_service_tracker.h"
#include "web_console_service.h"
#include "civetweb.h"
#include "web_console.h"

#define WEB_ROOT_DIR "web_root"

celix_status_t wst_create(void) 
{
        return CELIX_SUCCESS;
}

celix_status_t wst_destroy(void)
{
        return CELIX_SUCCESS;
}

celix_status_t wst_service_added(void * handle, service_reference_pt ref, void *service)
{
        struct shellWebActivator *act = (struct shellWebActivator*)handle;
        web_console_service_pt wcs = (web_console_service_pt) service;
        mg_set_request_handler(act->webContext, wcs->getWebEntry(handle), cgi_event_handler, act);
        wcs->copyRecourcesToWebDirectory(wcs->webConsole, WEB_ROOT_DIR);
        arrayList_add(act->trackedServices, wcs);
        return CELIX_SUCCESS;
}

celix_status_t wst_service_removed(void * handle, service_reference_pt reference, void * service)
{
        web_console_service_pt wcs = (web_console_service_pt) service;
        struct shellWebActivator *act = (struct shellWebActivator*)handle;
        // removes registered callback
        mg_set_request_handler(act->webContext, wcs->getWebEntry(handle), NULL, wcs);
        wcs->removeRecourcesFromWebDirectory(wcs->webConsole, WEB_ROOT_DIR);
        arrayList_removeElement(act->trackedServices, wcs);
        
        return CELIX_SUCCESS;
}


