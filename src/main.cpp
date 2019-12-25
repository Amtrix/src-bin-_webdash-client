/**
 * Tutorial on pipes: http://www.rozmichelle.com/pipes-forks-dups/
 **/

// Local includes
#include "global.hpp"

// WebDash includes
#include <webdash-config.hpp>
#include <webdash-core.hpp>
#include "../common/websocket.h"
#include "../common/utils.hpp"

// Standard includes
#include <cstdio>
#include <unistd.h> 
#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <sys/wait.h>

namespace fs = std::filesystem;

using namespace std;
using json = nlohmann::json;

const string _WEBDASH_PROJECT_NAME_ = "webdash-client";

vector<string> ListClientCmds() {
    return {"register", "list", "list-config", "create-built-init"};
}

int main(int argc, char **argv) {
    cout << "WebDash: Client (root: " << WebDashCore::Get().GetMyWorldRootDirectory() << ")" << endl;

    // Commands config-independent.
    if (argc >= 2) {
        const string strArg = string(argv[1]);

        if (strArg == "list-config") {
            WebDashConfigList();
            return 0;
        }

        if (strArg == "create-built-init") {
            auto padd = WebDashCore::Get().GetPathAdditions();
            auto envs = WebDashCore::Get().GetEnvAdditions();

            WebDashCore::Get().WriteToMyStorage("init.sh", [&](WriterType writer) {
                writer(WebDash::StoreWriteType::Clear, "");
                
                string out = "PATH=$PATH";
                for (auto e : padd) out = out + ":" + e;
                writer(WebDash::StoreWriteType::Append, "export " + out + "\n");

                for (auto &[key, val] : envs) {
                    writer(WebDash::StoreWriteType::Append, "export " + key + "=" + val + "\n");
                }
                writer(WebDash::StoreWriteType::End, "");
            });
            return 0;
        }

        if (strArg == "create-project-puller") {
            auto entries = WebDashCore::Get().GetExternalProjects();
            WebDashCore::Get().WriteToMyStorage("init-projects.sh", [&](WriterType writer) {
                writer(WebDash::StoreWriteType::Clear, "");

                for (auto entry : entries) {
                    writer(WebDash::StoreWriteType::Append, "git clone " + entry.source + " " + entry.destination + "\n");
                    writer(WebDash::StoreWriteType::Append, "webdash " + entry.destination + "/webdash.config.json" + entry.webdash_task + "\n");
                }

                writer(WebDash::StoreWriteType::End, "");
            });
            return 0;
        }
    }

    // No resolution? Try config specific.
    bool was_handled = false;
    auto configAndCmd = TryGetConfig(argc, argv);
    
    // Commands specific to a config file.
    if (configAndCmd.has_value()) {
        WebDashConfig config = configAndCmd.value().first;
        const string cmd = configAndCmd.value().second;

        if (cmd == "register") {
            WebDashRegister(config.GetPath());
            return 0;
        } else if (cmd == "list") {
            // just dont do anything. It's coming anyway.
        } else if (cmd.length() > 0) {
            auto ret = config.Run(configAndCmd.value().second);
            if (!ret.empty()) was_handled = true;
        }
    } else {
        cout << "No config provided NOR found in current directory." << endl;
    }
    

    // Still not handled? Error.
    if (!was_handled) {
        cout << "Please select one of the following:" << endl;

        vector<string> cmds;

        if (configAndCmd.has_value()) {
            cmds = WebDashList(configAndCmd.value().first.GetPath());
            for (auto cmd : cmds)
                cout << " " << cmd << endl;

            cout << "  ----  " << endl;
        }

        cmds = ListClientCmds();

        for (auto cmd : cmds)
            cout << " " << cmd << endl;
    }

    return 0;
}