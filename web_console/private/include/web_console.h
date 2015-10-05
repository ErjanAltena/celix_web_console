#include "service_tracker.h" 
struct shellWebActivator {
        bundle_context_pt context;
        service_reference_pt reference;
        bool running;
        celix_thread_t runnable;
        service_tracker_pt tracker;
        struct mg_context *webContext;
        array_list_pt trackedServices;
};


int cgi_event_handler(struct mg_connection *conn, void * data);
