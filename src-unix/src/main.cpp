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

int main(int argc, char **argv) {
    cout << "WebDash: Client" << endl;
    cout << "   root: " << WebDashCore::Get().GetMyWorldRootDirectory() << endl;

    if (argc < 2) {
        cout << "Please provide the desired action." << endl;
        return 0;
    }

    const string strArg = string(argv[1]);
    const fs::path configPath = fs::current_path() / "webdash.config.json";
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
    wdConfig.Run(argv[1]);

    return 0;
}