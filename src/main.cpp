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

const string _WEBDASH_INTERNAL_CMD_PREFIX = "_internal_:";

const string _WEBDASH_TERMINAL_INIT_FILE_WARNING = "# Warning: This is an automatically generated file by the app-persistent/bin/webdash client. Auto generated. Don't modify.";

const std::string _WEBDASH_TERMINAL_INIT_FILE_APPENDIX = R"(# Footer

if pgrep -x "webdash-server" > /dev/null
then
    echo 'WebDash: Server (<running>)'
else
    echo 'WebDash: Server (<stopped>)'
fi

)";

namespace NATIVE_COMMANDS {
    string REGISTER = "register";
    string LIST = "list";
    string LIST_CONFIG = "list-config";
    string _INT_CREATE_BUILD_INIT = _WEBDASH_INTERNAL_CMD_PREFIX + "create-build-init";
    string _INT_CREATE_PROJECT_CLONER = _WEBDASH_INTERNAL_CMD_PREFIX + "create-project-cloner";
};

vector<string> ListClientCmds() {
    return { NATIVE_COMMANDS::REGISTER,
             NATIVE_COMMANDS::LIST,
             NATIVE_COMMANDS::LIST_CONFIG,
             NATIVE_COMMANDS::_INT_CREATE_BUILD_INIT,
             NATIVE_COMMANDS::_INT_CREATE_PROJECT_CLONER };
}

bool IsInternalCommand(string cmd) {
    if (cmd.length() < _WEBDASH_INTERNAL_CMD_PREFIX.length()) return false;

    return cmd.substr(0, _WEBDASH_INTERNAL_CMD_PREFIX.length()) == _WEBDASH_INTERNAL_CMD_PREFIX;
}

int main(int argc, char **argv) {
    cout << "WebDash: Client (root: " << WebDashCore::Get().GetMyWorldRootDirectory() << ")" << endl;

    // Commands config-independent.
    if (argc >= 2) {
        const string strArg = string(argv[1]);

        if (strArg == NATIVE_COMMANDS::LIST_CONFIG) {
            WebDashConfigList();
            return 0;
        }

        if (strArg == NATIVE_COMMANDS::_INT_CREATE_BUILD_INIT) {
            auto padd = WebDashCore::Get().GetPathAdditions();
            auto envs = WebDashCore::Get().GetEnvAdditions();

            WebDashCore::Get().WriteToMyStorage("webdash.terminal.init.sh", [&](WriterType writer) {
                writer(WebDash::StoreWriteType::Clear, "");

                writer(WebDash::StoreWriteType::Append, _WEBDASH_TERMINAL_INIT_FILE_WARNING + "\n");
                
                string out = "PATH=$PATH";
                for (auto e : padd) out = out + ":" + e;
                writer(WebDash::StoreWriteType::Append, "export " + out + "\n");

                for (auto &[key, val] : envs) {
                    writer(WebDash::StoreWriteType::Append, "export " + key + "=" + val + "\n");
                }

                writer(WebDash::StoreWriteType::Append, _WEBDASH_TERMINAL_INIT_FILE_APPENDIX);

                writer(WebDash::StoreWriteType::End, "");
            });
            return 0;
        }

        if (strArg == NATIVE_COMMANDS::_INT_CREATE_PROJECT_CLONER) {
            auto entries = WebDashCore::Get().GetExternalProjects();
            WebDashCore::Get().WriteToMyStorage("initialize-projects.sh", [&](WriterType writer) {
                writer(WebDash::StoreWriteType::Clear, "");

                for (auto entry : entries) {
                    writer(WebDash::StoreWriteType::Append, "rm -r " + entry.destination + "\n");
                    writer(WebDash::StoreWriteType::Append, "git clone " + entry.source + " " + entry.destination + "\n");
                    writer(WebDash::StoreWriteType::Append, "webdash " + entry.destination + "/webdash.config.json" + entry.webdash_task + "\n");

                    if (entry.do_register) {
                        writer(WebDash::StoreWriteType::Append, "webdash register " + entry.destination + "/webdash.config.json\n");
                    }
                }

                writer(WebDash::StoreWriteType::End, "");
            });
            return 0;
        }

        if (strArg == NATIVE_COMMANDS::REGISTER && argc >= 3) {
            auto wconfig = WebDashConfig(argv[2]);
            
            if (wconfig.IsInitialized()) {
                WebDashRegister(wconfig.GetPath());
                return 0;
            }
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
        }
        
        if (cmd.length() > 0) {
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

        for (auto cmd : cmds) {
            if (IsInternalCommand(cmd) == false) {
                cout << " " << cmd << endl;
            }
        }
    }

    return 0;
}