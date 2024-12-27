#include <array>
#include <vector>
#include <string>
#include <future>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <system_error>
#include <unordered_map>
#include <unordered_set>

#ifndef MAKER_FLAGS
#define MAKER_FLAGS "-std=c++17 -Wfatal-errors -Oz -fno-rtti -fno-exceptions -Wall -Wextra -march=native -s -Werror -Wpedantic"
#endif

#define INF(mess) std::cerr << "[INFO]: " << mess << '\n'
#define WRN(mess) std::cerr << "[WARN]: " << mess << '\n'
#define ERR(mess) std::cerr << "[ERROR]: " << mess << '\n'
#define CMD(mess) std::cerr << "[CMD]: " << mess << '\n'
#define shift(argc, argv) ((argc)--, *(argv)++)

namespace maker {

using cmd_t = std::function<int(void)>;

struct Cmd {
    cmd_t func;
    std::string description;
};

template<typename T, typename U>
using isU = std::enable_if<std::is_same<std::decay_t<T>, U>::value>;

template<typename T>
using isString = isU<T, std::string>;

template<typename T>
using isCmd = isU<T, Cmd>;

template<typename S, typename = isString<S>>
Cmd from_string(S &&str)
{
    Cmd cmd;
    cmd.func = [=]() {
        return std::system(str.c_str());
    };
    cmd.description = std::forward<S>(str);
    return cmd;
}

Cmd from_string(const char *str)
{
    return from_string(std::string(str));
}

struct Rule {
    std::optional<Cmd> cmd;
    std::vector<std::string> deps;
    std::string target;
    bool phony;

    Rule()
        : cmd()
        , deps()
        , target()
        , phony(false)
    {}
    template<typename S, typename = isString<S>>
    Rule(S &&t)
        : cmd()
        , deps()
        , target(std::forward<S>(t))
        , phony(false)
    {}

    template<typename F, typename = isCmd<F>>
    Rule &with_cmd(F &&c)
    {
        cmd = std::make_optional(std::forward<F>(c));
        return *this;
    }

    Rule &with_phony()
    {
        phony = true;
        return *this;
    }

    bool should_rebuild()
    {
        if (!std::filesystem::exists(target))
            return true;
        if (std::filesystem::is_directory(target))
            return false;

        auto target_mod_time = std::filesystem::last_write_time(target);
        for (const auto &dep: deps) {
            if (!std::filesystem::exists(dep))
                return true;

            if (std::filesystem::is_directory(dep))
                continue;

            auto dependency_mod_time = std::filesystem::last_write_time(dep);
            if (dependency_mod_time > target_mod_time)
                return true;
        }
        return false;
    }

    template<typename T, typename = isString<T>>
    Rule(T &&t, std::vector<std::string>&& v)
        : cmd()
        , deps(std::move(v))
        , target(std::forward<T>(t))
        , phony(false)
    {}

    template<typename T, typename = isString<T>>
    Rule(T &&t, std::vector<std::string> &vec)
        : cmd()
        , deps(vec)
        , target(std::forward<T>(t))
        , phony(false)
    {}
};

namespace {

struct Tree_Node {
    Rule *rule;
    std::vector<size_t> deps;
    bool visited;

    Tree_Node()
        : rule(nullptr)
        , deps()
        , visited(false)
    {}

    Tree_Node(Rule* rule)
        : rule(rule)
        , deps()
        , visited(false)
    {}
};

struct Tree {
    std::vector<Tree_Node> nodes;

    Tree_Node &operator[](size_t idx)
    {
        return nodes[idx];
    }
    size_t add_node()
    {
        nodes.emplace_back();
        return nodes.size() - 1;
    }
    size_t add_node(Rule *rule)
    {
        nodes.emplace_back(rule);
        return nodes.size() - 1;
    }
};

struct Job {
    std::vector<Cmd> todo;

    Job() = default;
    Job(Cmd &s)
        : todo()
    {
        todo.push_back(s);
    }

    size_t size()
    {
        return todo.size();
    }

    template<typename F, typename = isCmd<F>>
    Job &operator+=(F &&cmd)
    {
        todo.push_back(std::forward<F>(cmd));
        return *this;
    }

    operator bool()
    {
        return !todo.empty();
    }

    bool operator()(size_t &idx, size_t total, int &ecode)
    {
        std::vector<std::future<int>> futures;

        for (auto &it: todo) {
            std::string cmd;
            cmd += "[" + std::to_string(++idx);
            cmd += "/" + std::to_string(total);
            cmd += "]: " + it.description + '\n';
            std::cout << cmd;

            futures.push_back(std::async(std::launch::async, it.func));
        }

        for (auto &it: futures) {
            int result = it.get();
            if (result != 0) {
                ecode = result;
                return false;
            }
        }

        return true;
    }
};

}

struct Maker {
    std::unordered_map<std::string, Rule> rules;

    void build_tree(Tree &tree, size_t node_idx)
    {
        Tree_Node *node = &tree.nodes[node_idx];
        if (tree.nodes[node_idx].visited) return;
        node->visited = true;

        for (const auto &dep: node->rule->deps) {
            if (rules.find(dep) == rules.end()) continue;

            Rule* rule = &rules[dep];

            size_t new_node_idx = tree.add_node();
            Tree_Node &new_node = tree[new_node_idx];
            tree.nodes[node_idx].deps.push_back(new_node_idx);
            new_node.rule = rule;
            if (!(tree.nodes[node_idx].rule->target == dep))
                build_tree(tree, tree.nodes.size() - 1);
        }
    }

    bool recursive_rebuild(Tree &tree, size_t current_idx, std::vector<Job> &jobs)
    {
        Job job;
        bool any_true = false;
        static std::unordered_set<std::string> seen_commands;
        Tree_Node &current_node = tree[current_idx];

        for (const auto &dep: current_node.deps) {
            if (recursive_rebuild(tree, dep, jobs)) {
                any_true = true;
                if (!tree[dep].rule->cmd)
                    continue;
                if (seen_commands.find(tree[dep].rule->cmd->description) == seen_commands.end()) {
                    seen_commands.insert(tree[dep].rule->cmd->description);
                    if (tree[dep].rule->cmd)
                    job += *(tree[dep].rule->cmd);
                }
            }
        }

        if (job) jobs.push_back(job);
        return any_true || current_node.rule->phony || current_node.rule->should_rebuild();
    }

    void clean()
    {
        bool run_once = false;
        for (const auto &it: rules) {
            if (!it.second.phony) {
                if (!std::filesystem::exists(it.second.target))
                    continue;
                if (std::filesystem::is_directory(it.second.target))
                    continue;
                run_once = true;
                std::string cmd = "rm -r ";
                cmd += it.first;
                CMD(cmd);
                std::system(cmd.c_str());
            }
        }
        if (!run_once) {
            INF("Already spotless clean...");
        }
    }

  public:
    Maker()
        : rules()
    {}

    template<typename R, typename = isU<R, Rule>>
    Maker &operator+=(R &&rule)
    {
        rules[rule.target] = std::forward<R>(rule);
        return *this;
    }

    template<typename R, typename = isU<R, Rule>>
    Maker &operator,(R &&rule)
    {
        rules[rule.target] = std::forward<R>(rule);
        return *this;
    }

    void operator()(const std::string &target)
    {
        if (target == "clean") {
            clean();
            return;
        }

        Tree tree;
        if (rules.find(target) == rules.end()) {
            ERR("rule " << target << " not found");
            return;
        }
        Rule *rule = &rules[target];
        (void)tree.add_node(rule);
        build_tree(tree, 0);
        std::vector<Job> jobs;

        if (recursive_rebuild(tree, 0, jobs)) {
            if (tree.nodes[0].rule->cmd)
                jobs.push_back(Job(*tree.nodes[0].rule->cmd));
        }

        size_t total = 0;
        for (auto &it: jobs) {
            total += it.size();
        }

        if (total == 0) {
            INF("nothing to be done for '" << target << "'");
            return;
        }

        int stage = 0;
        size_t count = 0;
        int ecode;
        for (auto &it: jobs) {
            stage++;
            if(!it(count, total, ecode)) {
                ERR("Compilation failed at stage: " << stage << " Aborting!");
                std::exit(ecode);
            }
        }
    }
};

namespace utils {

inline std::vector<std::string> get_includes_from_file(std::filesystem::path filename)
{
    std::vector<std::string> results;
    std::ifstream file(filename);

    std::string line;
    while (getline(file, line)) {
        if (line.rfind("#include \"", 0) == 0) {
            std::size_t start = line.find('"') + 1;
            std::size_t end = line.find('"', start);
            if (start != std::string::npos && end != std::string::npos) {
                results.push_back(line.substr(start, end - start));
            }
        }
    }
    return results;
}

std::string get_compiler()
{
    char *compiler = std::getenv("CXX");
    return compiler == nullptr ? "c++" : compiler;
}

}

} // maker

namespace maker { namespace __internal {

inline void go_rebuild_yourself(int *argc, char ***argv,
                                const std::string &compiler,
                                const std::filesystem::path &execpath,
                                const std::string &filename)
{
    if (execpath.extension() == ".old") {
        return;
    }
#ifndef _MAKER_OPTIMIZED
    INF("First run detected, optimizing...");
#else
        auto exec_time = std::filesystem::last_write_time(execpath);
        auto file_time = std::filesystem::last_write_time(filename);
        if (file_time > exec_time) {
            INF("New recipe detected, rebuilding...");
        auto oldexec = std::filesystem::path(execpath).replace_extension("old");
        std::error_code ecode;
        if (std::filesystem::exists(oldexec)) {
            std::filesystem::remove(oldexec, ecode);
            if (ecode) {
                ERR("Could not remove the old version: " << ecode.message());
                std::exit(ecode.value());
            }
        }
        INF("Renaming " << execpath.string() << " -> " << oldexec.string());
        std::filesystem::rename(execpath, oldexec, ecode);
        if (ecode) {
            ERR("Could not rename file: " << ecode.message());
            std::exit(ecode.value());
        }
#endif
        std::string cmd;
        cmd += compiler + " -o ";
        cmd += "'" + execpath.string() + "' ";
        cmd += filename + ' ';
        cmd += std::string(MAKER_FLAGS) + ' ';
        cmd += "-D_MAKER_OPTIMIZED";
        int result = std::system(cmd.c_str());
        if (result == 0) {
            INF("Compiled successfully!");
            std::string cmd = execpath.string();
            while (*argc > 0)
                cmd += ' ' + std::string(shift(*argc, *argv));
            std::exit(std::system(cmd.c_str()));
        } else {
            ERR("Compilation failed, fix errors and try again!");
            std::exit(result);
        }
#ifdef _MAKER_OPTIMIZED
    }
#endif
}

}} // namespace maker::__internal

#define GO_REBUILD_YOURSELF(argc, argv)                                                     \
    do {                                                                                    \
        std::string compiler = maker::utils::get_compiler();                                \
        std::string filename = __FILE__;                                                    \
        std::filesystem::path execpath = shift(argc, argv);                                 \
        maker::__internal::go_rebuild_yourself(&argc, &argv, compiler, execpath, filename); \
    } while(0);
