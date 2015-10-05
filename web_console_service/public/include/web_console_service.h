#include "celix_errno.h"
#define WEB_CONSOLE_SERVICE "Web_Console_Service"

typedef struct web_console_service *web_console_service_pt;
typedef struct web_console *web_console_pt;

struct web_console_service {
        web_console_pt webConsole;
        char *(*getServiceName)(web_console_pt wc); 
        char *(*getWebEntry)(web_console_pt wc); 
        char *(*getJsonData)(web_console_pt wc);
        celix_status_t (*copyRecourcesToWebDirectory)(web_console_pt handle, char * wwwRoot);
        celix_status_t (*removeRecourcesFromWebDirectory)(web_console_pt handle, char * wwwRoot);
};
