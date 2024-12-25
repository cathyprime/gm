#include <queue>
#include <vector>
#include <string>
#include <cstddef>
#include <numeric>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <unordered_set>

#include "maker.hh"

namespace fs = std::filesystem;
using namespace std;

maker::Maker mk;

struct env {
    fs::path build_artifacts;
    fs::path output_dir;
    string compile_flags;
    string exe_name;
    string compiler;
    string mode;
};

vector<string> get_included_headers_from_file(fs::path filename)
{
    vector<string> results;
    ifstream file(filename);

    string line;
    while (getline(file, line)) {
        if (line.rfind("#include \"", 0) == 0) {
            size_t start = line.find('"') + 1;
            size_t end = line.find('"', start);
            if (start != string::npos && end != string::npos) {
                results.push_back(line.substr(start, end - start));
            }
        }
    }
    return results;
}

void dir_rules(env &e)
{
    fs::path artifacts = e.build_artifacts / e.mode;
    string out_dir = e.output_dir.string();
    mk += maker::Rule(out_dir, { out_dir }).with_cmd(maker::from_string("mkdir -p " + out_dir)),
          maker::Rule(artifacts, { artifacts }).with_cmd(maker::from_string("mkdir -p " + artifacts.string()));
}

fs::path gen_ofile_with_rule(env &e, const fs::directory_entry &entry, vector<string> deps)
{
    string o_file = (e.build_artifacts / e.mode / entry.path().filename().replace_extension("o")).string();
    maker::Rule rule(o_file);
    deps.push_back((e.build_artifacts / e.mode).string());

    rule.deps = deps;
    rule.cmd = maker::from_string(e.compiler + " -c -o " + o_file + " " + e.compile_flags + entry.path().string());
    mk += rule;
    return entry.path().filename().replace_extension("o");
}

void make_main(env &e, vector<fs::path> &deps)
{
    string o_files;
    vector<string> deps_str;
    deps_str.resize(deps.size());

    transform(deps.begin(), deps.end(), deps_str.begin(), [&](auto &it) {
        auto o = e.build_artifacts / e.mode / it;
        o_files += ' ' + o.string();
        return o;
    });

    deps_str.push_back(e.output_dir);

    auto exe_name = (e.output_dir / e.exe_name).string();
    mk += maker::Rule(exe_name, deps_str)
        .with_cmd(maker::from_string(e.compiler + " -o " + exe_name + " " + o_files)),
        maker::Rule(e.mode, { exe_name }).with_phony();
}

int main(int argc, char **argv)
{
    std::string compiler = "g++";

    GO_REBUILD_YOURSELF(compiler, argc, argv);

    env debug_env;
    debug_env.build_artifacts = "build/";
    debug_env.output_dir = "out/";
    debug_env.compile_flags = "-g -O0 -Werror -Wall -Wextra -Wpedantic ";
    debug_env.exe_name = "main";
    debug_env.compiler = compiler;
    debug_env.mode = "debug";

    env release_env {debug_env};
    release_env.compile_flags = "-O3 -s -Wall -Wextra -fno-rtti -fno-exceptions ";
    release_env.exe_name = "gm";
    release_env.mode = "release";

    dir_rules(debug_env);
    dir_rules(release_env);

    vector<fs::path> o_files;

    for (const auto &entry: fs::directory_iterator(".")) {
        if (!entry.is_regular_file()) continue;
        if (entry.path() == "./maker.cc") continue;
        if (entry.path().extension() != ".cc") continue;

        vector<string> deps = get_included_headers_from_file(entry.path());
        (void)gen_ofile_with_rule(debug_env, entry, deps);
        o_files.push_back(gen_ofile_with_rule(release_env, entry, deps));
    }

    make_main(debug_env, o_files);
    make_main(release_env, o_files);

    queue<string> args;
    while (argc >= 1)
        args.push(shift(argc, argv));

    if (args.empty()) {
        mk("debug");
    } else {
        while (!args.empty()) {
            string arg = args.front();
            if (arg == "release") {
                mk("release");
            } else if (arg == "clean") {
                mk("clean");
            } else {
                mk("debug");
            }
            args.pop();
        }
    }
}
