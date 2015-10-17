# celix_web_console
A web framework which shows information from a running Celix instantiation in your browser

## Description
The web_console is a framework for Celix which provides a web-server (Civet web is used). This webserver searches for
web-console-services. Each found service provides a web-page and an entry-point to provide data (typical JSON). For
each service a tab is created and when the tab is clicked the provided web-page will be shown in an HTML iframe.

### web_console
This is the framework which search for services as plugins. 

### web_console_bundle_service
This is a service which shows the bundles and their status

### web_console_service_service
This is a service which shows all the registered services with threir porperties

## ToDo
New web-services shall be added:
- Log-information 
- Dependency-manager .



