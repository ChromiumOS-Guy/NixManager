## Introduction
[NixManager](https://gitlab.com/ChromiumOS-Guy/lomiri-system-settings-nixmanager/-/blob/main) is a plugin for lomiri system settings which is meant to manage the nix package manager (evident in the naming).

## Backend nix-setup bash script handels:
### Universal Strcuture for response:
the respose will always be a JSON-like QString that should be interprated (i.e opened as) a JSON it's structure is constant and looks like this:
```cpp
// success example:
{request_update_channels(const QVariant& requestId);

Q_INVOKABLE QString request_list_channels(const QVariant& requestId);

	"success": true,
	"output": QJsonArray(),
	"full_error": QJsonArray()
}
```
the value types are as follows:
*  "success" is a boolean
*  "output" is a QJsonArray()
*  "full_error": QJsonArray()

wherein every object in the QJsonArray() is a Line of the output.

if function returns success please check full_error but treat it as warnings!

Note: please keep in mind that full_error is stderr + contextual errors if they were needed and output is stdout  

### Currently available functions:

Important note: these function are accessible when invoking NixManager.(function) in QML.

for more/up-to-date details on these function please refer to the brief in nix-setup.h/controller.h header files.

#### Summery/Function Specifications:
```cpp
Q_INVOKABLE QString request_install_nix_home_manager(const QVariant& requestId, const QString& version);

Q_INVOKABLE QString request_uninstall_nix_home_manager(const QVariant& requestId);

Q_INVOKABLE QString request_detect_nix_home_manager(const QVariant& requestId);
```

* **
#### Connection:
all functions will return from async job to here, you want one big generic block of code to process all of the calls, if needed join the **currentRequestId** with the method name in a key value pair when making the request.

```qml
// root.currentRequestId is this line in root: (property string currentRequestId: "")
Connections {
    target: NixManagerPlugin
    
    // This handler fires for *all* completed operations
    onOperation_result: (resultJson, receivedId, operation) => {
        
        // 3. Match the ID to ensure it's the result we are waiting for
        if (receivedId === root.currentRequestId) {
            console.log("Received result for requested ID:", receivedId);
            
            // --- Process the resultJson string here ---
            try {
                const result = JSON.parse(resultJson);
                if (result.full_error != "") {
                    console.error(result.full_error.join(' '));
                }
                // Example processing:
                if (result.success) {
                    console.log(result.output.join(' '));
                } else {
                    console.log(result.output.join(' '));
                }
            } catch(e) {
                console.error("Failed to parse result JSON:", e);
            }
            
            // Reset the ID after processing
            root.currentRequestId = ""; 
        }
    }
}
```
* **
CALL FUNCTIONS:

* request_install_nix_home_manager:
	
	Does what it says on the tin, it installs nix/home-manager you can specify version number e.g "25.05" or leave version empty "" and get unstable/master on nix/home-manager respectively (done via bash)
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_install_nix_home_manager(root.currentRequestId, "25.05"); // will install stable 25.05 for both nix and home-manager
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_install_nix_home_manager(root.currentRequestId, ""); // will install unstable/master branches.
	```

	operation = install_nix_home_manager

* request_uninstall_nix_home_manager:
	
	Does what it says on the tin, it uninstalls nix/home-manager, by removing everything in /nix and removing nix/home-manager profile files. (done via bash)
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_uninstall_nix_home_manager(root.currentRequestId); // will uninstall both nix and home-manager
	```
	operation = uninstall_nix_home_manager

* request_detect_nix_home_manager:
	
	Does what it says on the tin, it detects if a nix/home-manager installation exist, does so by checking if home-manager is callable via bash, and then proceeds to verify the integrity of the installation by validating the profile directories of nix/home-manager.
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_detect_nix_home_manager(root.currentRequestId); // will detect both nix and home-manager installations
	```
	operation = detect_nix_home_manager

* **
## Backend nix-layer API:
### Universal Strcuture for response:
the respose will always be a JSON-like QString that should be interprated (i.e opened as) a JSON it's structure is constant and looks like this:
```cpp
// success example:
{
	"success": true,
	"message": "Successfully did something.",
	"output": [ "LineA", "LineB" ],
	"simple_error": QJsonArray(),
	"full_error": QJsonArray()
}
```
the value types are as follows:
*  "success" is a boolean
*  "message" is a String 
*  "output" is a QJsonArray()
*  "simple_error" is a QJsonArray()
*  "full_error": QJsonArray()

wherein every object in the QJsonArray() is either a Line of the output, or its own value depending on the function.

Note: that success barring runtime errors is based on exit code of nix/home-manager and so if function returns success please check simple_error/full_error but treat them as warnings!

### Currently available functions:

Important note: these function are accessible when invoking NixManager.(function) in QML.

for more/up-to-date details on these function please refer to the brief in nix-wrapper.h header file.

#### Summery/Function Specifications:
```cpp
Q_INVOKABLE QString request_hm_switch(const QVariant& requestId, const bool allow_insecure = false);

Q_INVOKABLE QString request_hm_version(const QVariant& requestId);

Q_INVOKABLE QString request_read_packages(const QVariant& requestId, const QString& packageType = QString::fromStdString("home"));

Q_INVOKABLE QString request_add_packages(const QVariant& requestId, const QString& packagesJsonString, bool allow_insecure = false, const QString& packageType = QString::fromStdString("home"), bool overwrite = false);

Q_INVOKABLE QString request_delete_packages(const QVariant& requestId, const QString& packagesJsonString, const QString& packageType = QString::fromStdString("home"));

Q_INVOKABLE QString request_search_packages(const QVariant& requestId, const QString& quarry, const bool local = false, const QString& base_url = QString::fromStdString("https://search.devbox.sh"), const int timeout = 10);

Q_INVOKABLE QString request_update_channels(const QVariant& requestId);

Q_INVOKABLE QString request_list_channels(const QVariant& requestId);

Q_INVOKABLE QString request_add_channel(const QVariant& requestId, const QString& url, const QString& name);

Q_INVOKABLE QString request_remove_channel(const QVariant& requestId, const QString& name);

Q_INVOKABLE QString request_list_generations(const QVariant& requestId);

Q_INVOKABLE QString request_switch_generation(const QVariant& requestId, const QString& generation_id);

Q_INVOKABLE QString request_delete_generation(const QVariant& requestId, const QString& generation_id);

Q_INVOKABLE QString request_delete_old_generations(const QVariant& requestId);

Q_INVOKABLE QString request_hm_expire_generations(const QVariant& requestId, const QString& timestamp = "-30 days");

Q_INVOKABLE QString request_hm_list_generations(const QVariant& requestId);
```  
IMPORTANT NOTE: all functions that can write to home.nix config file automatically backup/restore home.nix in case of error.

* **
#### Connection:
all functions will return from async job to here, you want one big generic block of code to process all of the calls, if needed join the **currentRequestId** with the method name in a key value pair when making the request.

```qml
// root.currentRequestId is this line in root: (property string currentRequestId: "")
Connections {
    target: NixManagerPlugin
    
    // This handler fires for *all* completed operations
    onOperation_result: (resultJson, receivedId, operation) => {
        
        // 3. Match the ID to ensure it's the result we are waiting for
        if (receivedId === root.currentRequestId) {
            console.log("Received result for requested ID:", receivedId);
            
            // --- Process the resultJson string here ---
            try {
                const result = JSON.parse(resultJson);
                if (result.simple_error != "") {
                    console.error(result.simple_error.join(' '));
                }
                if (result.full_error != "") {
                    console.error(result.full_error.join(' '));
                }
                // Example processing:
                if (result.success) {
                	console.log(result.message);
                    console.log(result.output.join(' '));
                } else {
                	console.log(result.message);
                    console.log(result.output.join(' '));
                }
            } catch(e) {
                console.error("Failed to parse result JSON:", e);
            }
            
            // Reset the ID after processing
            root.currentRequestId = ""; 
        }
    }
}
```
* **
FUNCTIONS:

* hm_switch:

	Applies the current home.nix config by calling home-manager switch (the equivalent of apt upgrade)
	what you get is the log from applying the configuration (i.e success/failure log)
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_hm_switch(root.currentRequestId); // will error if insecure pakcages are present in config
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_hm_switch(root.currentRequestId, true); // will not error and continue applying even if insecure pakcages are present in config
	```
	operation = hm_switch
	
* hm_version:
	
	gets home-manager version information, what you get is the log from getting the version info (i.e success/failure log)
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_hm_version(root.currentRequestId);
	```
	operation = hm_version

* read_packages:

	Reads the packages from home.nix and sorts them into an array, where package are the names/ids of the packages with pkgs. prefix of nix, can be something else depending on the package but is rarly something else. 
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_read_packages(root.currentRequestId); // defaults to packagetype = "home" as in home-manager do not change nothign else is implemented currently.
	```
	operation = read_packages

* add_packages:

	Lets you add or overwrite the packages in home.nix add_packages accepts an array of package names, package names must exist and have no typos so don't take the user input from search bar always use provided name from search function unless the user explictly wants to add a package manually.

	You must provide a prefix to packages such as "pkgs." please do so.
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	packagestoadd = "["pkgs.firefox","pkgs.libreoffice"]"
	NixManagerPlugin.request_add_packages(root.currentRequestId, packagestoadd); 
	```
	operation = add_packages

* delete_packages:

	Deletes packages from home.nix config file if they exist match the packages in provided array, You must provide a prefix to packages such as "pkgs.".
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	packagestodelete = "["pkgs.firefox","pkgs.libreoffice"]"
	NixManagerPlugin.request_delete_packages(root.currentRequestId, packagestodelete);
	```
	operation = delete_packages
	

* search_packages:

	functions consists of quarry (String), local search enable/disable, and base_url for api.  
	
	please keep in mind that timeout and tries carry over to all api calls needed to filter quarry list, quarry list max lenght is 50 results that means you are willing to wait a maximum of 51 * timeout * tries, the 51 is also for the quarry that gets the list that will be filtered, I recommend 10s for a max of 510s which while unresonable is usually much less.
	
	please discourage user from using local search if possible as it is resource intesive and may timeout on slower phones, recommend something on the par of an FP4 preformance wise and be ready to wait a minute or two per search.
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_search_packages(root.currentRequestId, "firefox"); // will search the keyword firefox on the nixhub api with the base_url "https://search.devbox.sh"  
	  
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_search_packages(root.currentRequestId, "firefox", true); // will use local search with "nix-env -qaP (search quarry) 2> /dev/null"
	```
	operation = search_packages


* update_channels:

	runs nix-channel --update, (the equivalent of apt update) what you get is the log from updating the channels (i.e success/failure log)

	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_update_channels(root.currentRequestId);
	```
	operation = update_channels

* list_channels:
	
	runs nix-channel --list, (the equivalent of apt-cache policy) what you get is an array of dictionaries: {"name" : "channel-name", "url" : "channel-url"}
	#### IMPORTANT: you will want to refuse the user the option to delete the channels home-manager and nixpkgs as we need both to be able to install packages on ubuntu touch.
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_list_channels(root.currentRequestId);
	```
	operation = list_channels

* add_channel:

	runs nix-channel --add, needs a url and a name, what you get is the log from adding the channel (i.e success/failure log)
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_add_channel(root.currentRequestId, "https://nixos.org/channels/nixos-25.05", "nixpkgs");
	```
	operation = add_channel

* remove_channel:

	runs nix-channel --remove, needs channel name, what you get is the log from removing the channel (i.e success/failure log)
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_remove_channel(root.currentRequestId, "somechannelname");
	```
	operation = remove_channel

* list_generations:
	runs nix-env --list-generations, what you get is an array of dictionaries: {"id" : "generation-id", "datetime" : "generation-datetime", "is_current" : boolean}
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_list_generations(root.currentRequestId);
	```
	operation = list_generations

* switch_generation:
	
	runs nix-env --switch-generation (generation_id), what you get is the log from switching the generation profile (make sure to use reapply afterwards) (i.e success/failure log)
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_switch_generation(root.currentRequestId, generation_id);
	```
	operation = switch_generation

* delete_generation:

	runs nix-env --delete-generations, needs generation_id, what you get is the log from deleting the generation (i.e success/failure log)
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_delete_generation(root.currentRequestId, generation_id);
	```
	operation = delete_generation

* delete_old_generations:
	the nuke version of delete_generation, deletes all generations besides current.
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_delete_old_generations(root.currentRequestId);
	```
	operation = delete_old_generations

* hm_expire_generations:
	
	runs home-manager expire-generations (timestamp), what you get is the log from expiring the generations (i.e success/failure log)
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_hm_expire_generations(root.currentRequestId); // by default timestamp is set to "-30 days"
	```
	operation = hm_expire_generations

* hm_list_generations:
	
	runs home-manager generations, what you get is an array of dictionaries: {"id" : "generation-id", "datetime" : "generation-datetime", "path" : "true path to home-manager generation store directory"}
	
	```qml
	root.currentRequestId = "VERSION_REQUEST_" + Date.now();
	NixManagerPlugin.request_hm_list_generations(root.currentRequestId);
	```
	operation = hm_list_generations