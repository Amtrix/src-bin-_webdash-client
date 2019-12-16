/**
 * Tutorial on pipes: http://www.rozmichelle.com/pipes-forks-dups/
 **/

// Local includes
#include "global.hpp"

// WebDash includes
#include <webdash-config.hpp>
#include <webdash-core.hpp>
#include "../../common/websocket.h"

// Standard includes
#include <cstdio>
#include <unistd.h> 
#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <sys/wait.h>

namespace fs = std::filesystem;

using namespace std;
using json = nlohmann::json;

const string _WEBDASH_PROJECT_NAME_ = "webdash-client";

vector<string> ListClientCmds() {
    return {"register-current", "list", "list-config"};
}

int main(int argc, char **argv) {
    cout << "WebDash: Client (root: " << WebDashCore::Get().GetMyWorldRootDirectory() << ")" << endl;

    bool was_handled = false;
    const fs::path configPath = fs::current_path() / "webdash.config.json";

    if (std::filesystem::exists(configPath) == false) {
        cout << "No config file exists." <<endl;
        return 0;
    }

    if (argc >= 2) {
        const string strArg = string(argv[1]);
        const string strPath = configPath.u8string();

        if (strArg == "register-current") {
            WebDashRegister(configPath);
            return 0;
        }

        if (strArg == "list") {
            WebDashList(configPath);
            return 0;
        }

        if (strArg == "list-config") {
            WebDashConfigList();
            return 0;
        }

        WebDashConfig wdConfig(configPath);
        auto ret = wdConfig.Run(argv[1]);

        if (!ret.empty()) was_handled = true;
    }

    if (!was_handled) {
        cout << "Please select one of the following:" << endl;

        vector<string> cmds = WebDashList(configPath);
    
        for (auto cmd : cmds)
            cout << " " << cmd << endl;

        cout << "  ----  " << endl;
        
        cmds = ListClientCmds();

        for (auto cmd : cmds)
            cout << " " << cmd << endl;
    }

    return 0;
}