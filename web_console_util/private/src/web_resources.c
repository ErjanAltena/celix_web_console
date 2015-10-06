#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bundle_context.h"
#include "web_resources.h"

#define BUF_SIZE 1024

static int copyFile(char* src, char*dst);

celix_status_t copyWebResources(bundle_context_pt context, char* resources[], char* webRoot)
{
        celix_status_t status;
        bundle_archive_pt archive = NULL;
        bundle_pt bundle = NULL;
        bundle_revision_pt revision = NULL;
        char *revLoc;
        int i=0;

        status = bundleContext_getBundle(context, &bundle); 
        if(status != CELIX_SUCCESS) {
                return status;
        }
        status = bundle_getArchive(bundle, &archive);
        if(status != CELIX_SUCCESS) {
                return status;
        }
        status = bundleArchive_getCurrentRevision(archive, &revision);
        if(status != CELIX_SUCCESS) {
                return status;
        }
        status = bundleRevision_getRoot(revision, &revLoc);
        if(status != CELIX_SUCCESS) {
                return status;
        }


        while (resources[i] != NULL) {
                char source_loc[BUF_SIZE];
                char dest_loc[BUF_SIZE];
                snprintf(source_loc, BUF_SIZE, "%s/%s", revLoc, resources[i]);  
                snprintf(dest_loc, BUF_SIZE, "%s/%s", webRoot, resources[i]);
                if(copyFile(source_loc, dest_loc)) {
                        return CELIX_FILE_IO_EXCEPTION; 
                }
                i++;
        }                 
        return CELIX_SUCCESS;   
        
}


celix_status_t removeWebResources(bundle_context_pt context, char* resources[], char *webRoot)
{
        int i = 0;
        char buffer[BUF_SIZE];
        while(resources[i] != NULL) {
                snprintf(buffer, BUF_SIZE, "%s/%s", webRoot, resources[i]);
                unlink(buffer);       
                i++;
        }
        return CELIX_SUCCESS;   
        
}

static int copyFile(char* src, char*dst)
{
        char buffer[4096];
        int dsrc = open(src, O_RDONLY);
        if(dsrc < 0) {
                return -1;
        }
        int ddst = open(dst, O_WRONLY | O_CREAT, 0777);
        if(ddst < 0) {
                return -1;
        }

        int nrbytes;
        while((nrbytes = read(dsrc, buffer, 4096)) > 0) {
                if(write(ddst, buffer, nrbytes) < 0) {
                        return -1;
                }
        }
        close(dsrc);
        close(ddst);

        return 0; 
}
