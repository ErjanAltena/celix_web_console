#include "bundle_context.h"
#define WEB_ROOT "web_root"

celix_status_t copyWebResources(bundle_context_pt context, char* resources[], char* webRoot);
celix_status_t removeWebResources(bundle_context_pt context, char* resources[], char *webRoot);
