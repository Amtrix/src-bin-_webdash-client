/**
 * Tutorial on pipes: http://www.rozmichelle.com/pipes-forks-dups/
 **/

// Local includes
#include "global.hpp"

// WebDash includes
#include <webdash-config.hpp>
#include <webdash-core.hpp>
#include "../common/websocket.h"

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
    return {"register-current", "list", "list-config", "create-built-init"};
}

int main(int argc, char **argv) {
    cout << "WebDash: Client (root: " << WebDashCore::Get().GetMyWorldRootDirectory() << ")" << endl;

    bool was_handled = false;
    const fs::path configPath = fs::current_path() / "webdash.config.json";
    std::optional<string> o_path_arg = nullopt, o_cmd_arg = nullopt;

    if (std::filesystem::exists(configPath)) {
        o_path_arg = configPath;
    }

    if (argc >= 2) {
        const string strArg = string(argv[1]);

        if (strArg == "register-current" && o_path_arg.has_value()) {
            WebDashRegister(o_path_arg.value());
            return 0;
        }

        if (strArg == "list" && o_path_arg.has_value()) {
            WebDashList(o_path_arg.value());
            return 0;
        }

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

        if (strArg.find("webdash.config.json") != string::npos) {
            cout << "Config file provided." << endl;

            string patharg = "", cmdarg = "";
            bool path_switch = false;
            for (int i = ((int) strArg.size()) - 1; i >= 0; --i) {
                if (strArg[i] == ':' && !path_switch) {
                    path_switch = true;
                    continue;
                }

                if (path_switch) {
                    patharg += strArg[i];
                } else {
                    cmdarg += strArg[i];
                }
            }
            std::reverse(patharg.begin(), patharg.end());
            std::reverse(cmdarg.begin(), cmdarg.end());

            o_path_arg = patharg;
            o_cmd_arg = cmdarg;
            
            cout << patharg << " : " << cmdarg << endl;
        }

        if (o_path_arg.has_value()) {
            WebDashConfig wdConfig(o_path_arg.value());
            auto ret = wdConfig.Run(o_cmd_arg.value_or(argv[1]));

            if (!ret.empty()) was_handled = true;
        } else {
            cout << "No config file specified NOR found in current directory." << endl;
        }
    }

    if (!was_handled) {
        cout << "Please select one of the following:" << endl;

        vector<string> cmds = WebDashList(o_path_arg.value());
    
        for (auto cmd : cmds)
            cout << " " << cmd << endl;

        cout << "  ----  " << endl;
        
        cmds = ListClientCmds();

        for (auto cmd : cmds)
            cout << " " << cmd << endl;
    }

    return 0;
}