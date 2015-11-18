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

web_console_pt webConsoleBundleCreate(bundle_context_pt context);
char *webConsoleBundleServiceName(web_console_pt wc);
char *webConsoleBundleServiceWebEntry(web_console_pt wc); 
celix_status_t webConsoleBundleServiceCopyResources(web_console_pt wc, char *webRoot); 
celix_status_t webConsoleBundleServiceRemoveResources(web_console_pt wc, char *webRoot);
char *webConsoleBundleServiceGetJsonData(web_console_pt wc, char *query);
char * webConsoleBundleServiceGetMainWebPage(web_console_pt wc); 
