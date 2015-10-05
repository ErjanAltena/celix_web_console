#include "celix_errno.h"

web_console_pt webConsoleBundleCreate(bundle_context_pt context);
char *webConsoleBundleServiceName(web_console_pt wc);
char *webConsoleBundleServiceWebEntry(web_console_pt wc); 
celix_status_t webConsoleBundleServiceCopyResources(web_console_pt wc, char *webRoot); 
celix_status_t webConsoleBundleServiceRemoveResources(web_console_pt wc, char *webRoot);
char *webConsoleBundleServiceGetJsonData(web_console_pt wc);
