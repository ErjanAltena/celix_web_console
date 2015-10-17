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
#include "celix_errno.h"
#define WEB_CONSOLE_SERVICE "Web_Console_Service"

typedef struct web_console_service *web_console_service_pt;
typedef struct web_console *web_console_pt;

struct web_console_service {
        web_console_pt webConsole;
        char *(*getServiceName)(web_console_pt wc); 
        char *(*getWebEntry)(web_console_pt wc); 
        char *(*getJsonData)(web_console_pt wc);
        char *(*getMainWebPage)(web_console_pt wc);
        celix_status_t (*copyRecourcesToWebDirectory)(web_console_pt handle, char * wwwRoot);
        celix_status_t (*removeRecourcesFromWebDirectory)(web_console_pt handle, char * wwwRoot);
};
