#include "service_reference.h"

celix_status_t wst_create(void); 
celix_status_t wst_destroy(void); 
celix_status_t wst_service_added(void * handle, service_reference_pt ref, void *service);
celix_status_t wst_service_removed(void * handle, service_reference_pt reference, void * service);
