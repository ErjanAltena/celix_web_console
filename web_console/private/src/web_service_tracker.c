#include "web_service_tracker.h"
#include "web_console_service.h"
#include "civetweb.h"
#include "web_console.h"

#define WEB_ROOT_DIR "web_root"

celix_status_t wst_create(void) 
{
        printf("wst_create\n");
        return CELIX_SUCCESS;
}

celix_status_t wst_destroy(void)
{
        printf("wst_destroy\n");
        return CELIX_SUCCESS;
}

celix_status_t wst_service_added(void * handle, service_reference_pt ref, void *service)
{
        struct shellWebActivator *act = (struct shellWebActivator*)handle;
        web_console_service_pt wcs = (web_console_service_pt) service;
        printf("wst_added: service %s, Entry: %s\n", wcs->getServiceName(handle), wcs->getWebEntry(handle));
        mg_set_request_handler(act->webContext, wcs->getWebEntry(handle), cgi_event_handler, act);
        wcs->copyRecourcesToWebDirectory(wcs->webConsole, WEB_ROOT_DIR);
        arrayList_add(act->trackedServices, wcs);
        return CELIX_SUCCESS;
}

celix_status_t wst_service_removed(void * handle, service_reference_pt reference, void * service)
{
        web_console_service_pt wcs = (web_console_service_pt) service;
        struct shellWebActivator *act = (struct shellWebActivator*)handle;
        printf("wst_removed: service %s, Entry: %s\n", wcs->getServiceName(handle), wcs->getWebEntry(handle));
        // removes registered callback
        mg_set_request_handler(act->webContext, wcs->getWebEntry(handle), NULL, wcs);
        wcs->removeRecourcesFromWebDirectory(wcs->webConsole, WEB_ROOT_DIR);
        arrayList_removeElement(act->trackedServices, wcs);
        
        return CELIX_SUCCESS;
}


