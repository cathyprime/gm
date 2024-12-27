#include <queue>
#include <array>
#include <vector>
#include <string>
#include <cassert>
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
    fs::path exe_name;
    string compile_flags;
    string compiler;
    string mode;

    env(fs::path out_dir, string flags, string exe, string mode)
        : build_artifacts(out_dir / mode)
        , exe_name(fs::path("out/") / exe)
        , compile_flags(flags)
        , compiler(maker::utils::get_compiler())
        , mode(mode)
    {}
};

void gen_dir_rules(env &e)
{
    maker::Rule rule(e.build_artifacts.string());
    rule.deps = { e.build_artifacts.string() };
    rule.cmd = "mkdir -p " + e.build_artifacts.string();
    mk += rule;
}

fs::path gen_ofile_and_rule(env &e, const fs::directory_entry &entry, vector<string> deps)
{
    string o_file = (e.build_artifacts / entry.path().filename().replace_extension("o")).string();
    maker::Rule rule(o_file);
    deps.push_back((e.build_artifacts).string());

    rule.deps = { deps };
    rule.cmd = std::string(maker::Cmd_Builder(e.compiler) + "-c -o"s + o_file + e.compile_flags + entry.path().string())
    ;
    mk += rule;
    return entry.path().filename().replace_extension("o");
}

void make_main(env &e, vector<fs::path> &deps)
{
    string o_files;
    vector<string> deps_str;
    deps_str.resize(deps.size());

    transform(deps.begin(), deps.end(), deps_str.begin(), [&](auto &it) {
        auto o = e.build_artifacts / it;
        o_files += ' ' + o.string();
        return o;
    });

    deps_str.push_back("out/");

    auto exe_name = (e.exe_name).string();
    maker::Rule rule(std::move(exe_name), std::move(deps_str));
    mk += rule
        .with_cmd(maker::Cmd{std::string(maker::Cmd_Builder(e.compiler) + e.compile_flags + "-o"s + exe_name + o_files)}),
        maker::Rule(e.mode, std::vector{ exe_name }).with_phony();
}

maker::Rule create_dependency_rule(const fs::path &file, const fs::path &out_dir, const std::string &compile_flags)
{
    namespace fs = std::filesystem;

    assert(fs::is_directory(out_dir));
    assert(fs::exists(file));

    fs::path d_file = file;
    d_file.replace_extension("d");

    fs::path output = out_dir / d_file;
    maker::Rule rule = maker::Rule(output.string());
    rule.cmd = std::string(maker::Cmd_Builder(maker::utils::get_compiler()) + "-MMD -MF"s + output.string() + compile_flags + file.string());
    rule.deps = { file };
    return rule;
}

std::vector<std::string> get_dependencies_from_d(const fs::path &file)
{
    namespace fs = std::filesystem;
    assert(fs::exists(file));

    std::vector<std::string> results;
    std::fstream d_file(file);
}

int main(int argc, char **argv)
{
    const std::string std_version = "-std=c++23 ";
    const std::string compiler = maker::utils::get_compiler();

    GO_REBUILD_YOURSELF(argc, argv);

    env debug_env{
        "build/", std_version + "-g -O0 -Werror -Wall -Wextra -Wpedantic",
        "main", "debug"
    };

    env release_env{
        "build/", std_version + "-O3 -s -Wall -Wextra -fno-rtti -fno-exceptions",
        "gm", "release"
    };

    std::array rules{ &debug_env, &release_env };

    mk += maker::Rule("out/"s, { "out/" }).with_cmd(maker::Cmd("mkdir -p out/"s));

    for_each(rules.begin(), rules.end(), [](env *e) {
        gen_dir_rules(*e);
    });

    vector<fs::path> o_files;

    for (const auto &entry: fs::directory_iterator(".")) {
        if (!entry.is_regular_file()) continue;
        if (entry.path() == "./maker.cc") continue;
        if (entry.path().extension() != ".cc") continue;

        vector<string> deps = maker::utils::get_includes_from_file(entry.path());
        (void)gen_ofile_and_rule(debug_env, entry, deps);
        o_files.push_back(gen_ofile_and_rule(release_env, entry, deps));
    }

    for_each(rules.begin(), rules.end(), [&](env *e) {
        make_main(*e, o_files);
    });

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
