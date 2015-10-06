#include "celix_errno.h"

web_console_pt webConsoleServicesCreate(bundle_context_pt context);
char *webConsoleServicesServiceName(web_console_pt wc);
char *webConsoleServicesServiceWebEntry(web_console_pt wc); 
celix_status_t webConsoleServicesServiceCopyResources(web_console_pt wc, char *webRoot); 
celix_status_t webConsoleServicesServiceRemoveResources(web_console_pt wc, char *webRoot);
char *webConsoleServicesServiceGetJsonData(web_console_pt wc);
char * webConsoleServicesServiceGetMainWebPage(web_console_pt wc); 
