#pragma once

// WebDash includes
#include <webdash-config.hpp>
#include <webdash-core.hpp>

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

inline pair<string, string> SplitOnLastColon(string str) {
    string A = "", B = "";
    bool path_switch = false;
    for (int i = ((int)str.size()) - 1; i >= 0; --i) {
        if (str[i] == ':' && !path_switch) {
            path_switch = true;
            continue;
        }

        if (path_switch) {
            A += str[i];
        } else {
            B += str[i];
        }
    }

    reverse(A.begin(), A.end());
    reverse(B.begin(), B.end());

    // if only B found, then make only A found instead.
    if (!path_switch) {
        swap(A, B);
    }

    return make_pair(A, B);
}

// Returns (config, cmd) if possible.
inline std::optional<pair<WebDashConfig, string>> TryGetConfig(int argc, char **argv) {
    std::optional<pair<WebDashConfig, string>> ret = nullopt;
    std::optional<string> o_path_arg = nullopt, o_cmd_arg = nullopt;

    vector<pair<string, std::optional<string>>> paths;

    // Paths checked according to priority. More explicit asks come first.
    if (argc >= 2) {
        const string strarg = string(argv[1]);
        auto parts = SplitOnLastColon(strarg);
        paths.push_back(make_pair(parts.first, parts.second));
        paths.push_back(make_pair(parts.first + "/webdash.config.json", parts.second));
    }
    paths.push_back(make_pair(fs::current_path() / "webdash.config.json", nullopt));

    for (auto path : paths) {
        if (o_path_arg.has_value() == false && std::filesystem::exists(path.first)) {
            auto tconfig = WebDashConfig(path.first);

            // We could have tested a directory with fs::exists. Make sure it is an actual config file.
            if (!tconfig.IsInitialized())
                continue;

            o_path_arg = path.first;

            if (path.second.has_value()) { // a valid cmd was determined or "" if the config path was explicitly given.
                const string tcmd = path.second.value();
                if (tcmd.length() > 0) o_cmd_arg = tcmd;
                else if (argc >= 3) o_cmd_arg = argv[2]; // config was explicitly given and ate on argument, check for one extra to get the command.
            } else if (argc >= 2) {
                o_cmd_arg = argv[1];
            }
        }
    }

    // No sense to provide any kind of command if a corresponding config wasn't found.
    if (o_path_arg.has_value())
        ret = std::make_optional(make_pair(WebDashConfig(o_path_arg.value()), o_cmd_arg.value_or("")));

    return ret;
}