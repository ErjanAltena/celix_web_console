# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
celix_subproject(WEB_CONSOLE_BUNDLE_SERVICE  "Option to enable building the Web Console Bundle Service" OFF DEPS LAUNCHER SHELL WEB_CONSOLE)
if (WEB_CONSOLE_BUNDLE_SERVICE)

        SET_HEADER(BUNDLE_VERSION "1.0.0")
	SET_HEADER(BUNDLE_SYMBOLICNAME "apache_celix_web_console_bundle_service")
	SET_HEADERS("Bundle-Name: Apache Celix Web Console Bundle Service`") 

        bundle(web_console_bundle_service
    	        SOURCES 
                        private/src/web_console_bundle_activator.c
                        private/src/web_console_bundle_service.c
                        ../web_console_util/private/src/web_resources.c
                FILES	
                        www/bundles.html
	)
	
	install_bundle(web_console_bundle_service)
	
        include_directories("private/include")
        include_directories("../web_console_service/public/include")
        include_directories("../web_console_util/private/include")
        include_directories("${PROJECT_SOURCE_DIR}/utils/public/include")
        include_directories("${PROJECT_SOURCE_DIR}/shell/public/include")
        target_link_libraries(web_console_bundle_service)
endif (WEB_CONSOLE_BUNDLE_SERVICE)
