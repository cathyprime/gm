#include <vector>
#include <string>
#include <cstddef>
#include <numeric>
#include <fstream>
#include <filesystem>
#include <unordered_set>

enum class Mode {
    Debug,
    Release,
    Clean,
};

#include "maker.hh"

#define build_dir "build/"
#define out_dir "out/"
#define compiler "g++"

#define debug_dir "debug/"
#define release_dir "release/"

#define debug_exe "main"
#define release_exe "gm"

namespace fs = std::filesystem;
using namespace std;

Mode from_string(const string &str)
{
    if (str == "release") return Mode::Release;
    if (str == "clean") return Mode::Clean;
    return Mode::Debug;
}

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

int main(int argc, char **argv)
{
    GO_REBUILD_YOURSELF(compiler, argc, argv);

    maker::Maker mk;

    string arg;
    if (argc >= 1)
         arg = shift(argc, argv);

    Mode mode = from_string(arg);

    string debug_artifacts = build_dir debug_dir;
    string debug_exe_name = out_dir debug_exe;

    string release_artifacts = build_dir release_dir;
    string release_exe_name = out_dir release_exe;

    mk += maker::Rule(out_dir, { out_dir }).with_cmd(maker::from_string("mkdir -p " out_dir)),
          maker::Rule(debug_artifacts, { debug_artifacts }).with_cmd(maker::from_string("mkdir -p " + debug_artifacts)),
          maker::Rule(release_artifacts, { release_artifacts }).with_cmd(maker::from_string("mkdir -p " + release_artifacts));

    vector<fs::directory_entry> source_files;

    for (const auto &entry: fs::directory_iterator(".")) {
        if (!entry.is_regular_file()) continue;
        if (entry.path() == "./maker.cc") continue;
        if (entry.path().extension() != ".cc") continue;

        source_files.push_back(entry);
    }

    vector<string> debug_main_deps;
    vector<string> release_main_deps;

    auto on_source = [&](const auto &entry, const string &artifacts, vector<string> &main_deps) {
        string o_file = artifacts + entry.path().filename().replace_extension(".o").string();
        main_deps.push_back(o_file);
        maker::Rule rule(o_file);

        auto deps = get_included_headers_from_file(entry.path());
        deps.push_back(artifacts);
        deps.push_back(entry.path().string());
        rule.deps = move(deps);
        rule.cmd = maker::from_string(string(compiler) + " -c -o " + o_file + " " + entry.path().string());
        mk += rule;
    };

    for (const auto &entry: source_files) {
        on_source(entry, debug_artifacts, debug_main_deps);
        on_source(entry, release_artifacts, release_main_deps);
    }

    string debug_o_files_str = std::accumulate(
        debug_main_deps.begin() + 1, debug_main_deps.end(), debug_main_deps[0],
        [](const string &lhs, const string &rhs) {
            return lhs + " " + rhs;
        }
    );

    string release_o_files_str = std::accumulate(
        release_main_deps.begin() + 1, release_main_deps.end(), release_main_deps[0],
        [](const string &lhs, const string &rhs) {
            return lhs + " " + rhs;
        }
    );

    debug_main_deps.push_back(out_dir);
    release_main_deps.push_back(out_dir);

    mk += maker::Rule(debug_exe_name, debug_main_deps)
        .with_cmd(maker::from_string(string(compiler) + " -o " + debug_exe_name + " " + debug_o_files_str)),
        maker::Rule("debug", { debug_exe_name }).with_phony();

    mk += maker::Rule(release_exe_name, release_main_deps)
        .with_cmd(maker::from_string(string(compiler) + " -o " + release_exe_name + " " + release_o_files_str)),
        maker::Rule("release", { release_exe_name }).with_phony();

    switch (mode) {
    case Mode::Debug:
        mk("debug");
        break;
    case Mode::Release:
        mk("release");
        break;
    case Mode::Clean:
        mk("clean");
        break;
    }

}
