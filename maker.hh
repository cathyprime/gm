#include <array>
#include <vector>
#include <string>
#include <future>
#include <utility>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <type_traits>
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

struct Rule;

using cmd_t = std::function<int(void)>;
using deps_t = std::function<std::vector<std::string>(void)>;

template<typename T, typename U>
constexpr bool isU = std::is_same<std::decay_t<T>, U>::value;

template<typename T>
constexpr bool isString = isU<T, std::string>;

template<typename T>
constexpr bool isRule = isU<T, maker::Rule>;

template <typename T, typename... Args>
using areAllOfType = std::enable_if_t<(std::is_same<std::decay_t<Args>, T>::value && ...)>;

template <typename... Args>
using areAllStrings = areAllOfType<std::string, Args...>;

template <typename Container>
constexpr bool isContainer = std::is_same_v<std::decay_t<Container>, std::vector<std::string>>;

template <typename IL>
constexpr bool isInitializerList = std::is_same_v<std::decay_t<IL>, std::initializer_list<std::string>>;

struct Cmd {
    cmd_t func;
    std::string description;

    Cmd()
        : func()
        , description()
    {}

    template<typename S, std::enable_if_t<isString<S>, int> = 0>
    Cmd(S &&s)
        : func([str = std::forward<S>(s)]() { return std::system(str.c_str()); })
        , description(s)
    {}
};

template<typename T>
constexpr bool isCmd = isU<T, Cmd>;

struct Deps {
    deps_t func;

    Deps()
        : func()
    {}

    Deps(std::vector<std::string> &vec)
        : func([=]() mutable -> std::vector<std::string> {
            return vec;
        })
    {}

    Deps(std::vector<std::string> &&vec)
        : func([vec = std::move(vec)]() mutable -> std::vector<std::string> {
            return vec;
        })
    {}

    template<typename IL, std::enable_if_t<isInitializerList<IL>, int> = 0>
    Deps& operator=(IL il)
    {
        func = [=]() -> std::vector<std::string> { return std::vector<std::string>(il); };
        return *this;
    }

    Deps& operator=(std::vector<std::string> il)
    {
        func = [=]() -> std::vector<std::string> { return {il}; };
        return *this;
    }

    std::vector<std::string> operator()() {
        return func();
    }
};

class Cmd_Builder {
    std::vector<std::string> m_cmd;
    std::size_t m_idx;

  public:
    Cmd_Builder() = default;

    template <typename... Args, typename = areAllStrings<Args...>>
    Cmd_Builder(Args... args) noexcept
        : m_cmd({ args... })
        , m_idx(0)
    {}
    template<typename... Args, typename = areAllStrings<Args...>>
    Cmd_Builder &push(Args... args) noexcept
    {
        m_cmd.insert(m_cmd.end(), { args... });
        return *this;
    }
    template<typename S, std::enable_if_t<isString<S>, int> = 0>
    Cmd_Builder &push(S &&s) noexcept
    {
        m_cmd.push_back(std::forward<S>(s));
        return *this;
    }
    template<typename S, std::enable_if_t<isString<S>, int> = 0>
    Cmd_Builder &operator+=(S &&s)
    {
        m_cmd.push_back(std::forward<S>(s));
        return *this;
    }
    template<typename S, std::enable_if_t<isString<S>, int> = 0>
    Cmd_Builder &operator+(S &&s)
    {
        m_cmd.push_back(std::forward<S>(s));
        return *this;
    }
    operator std::string()
    {
        return build();
    }
    [[nodiscard]] std::string build() const noexcept
    {
        std::string result = m_cmd[0];

        for (auto it = m_cmd.begin() + 1; it != m_cmd.end(); ++it) {
            result += ' ' + *it;
        }
        return result;
    }
};

struct Rule {
    std::optional<Cmd> cmd;
    Deps deps;
    std::string target;
    bool phony;

    Rule()
        : cmd()
        , deps()
        , target()
        , phony(false)
    {}
    template<typename S, std::enable_if_t<isString<S>, int> = 0>
    Rule(S &&t)
        : cmd()
        , deps()
        , target(std::forward<S>(t))
        , phony(false)
    {}

    template<typename S, std::enable_if_t<isString<S>, int> = 0>
    Rule(S &&t, std::initializer_list<std::string> vec)
        : cmd()
        , deps(vec)
        , target(std::forward<S>(t))
        , phony(false)
    {}

    template<typename S, typename Container, std::enable_if_t<isString<S> || isContainer<Container>, int> = 0>
    Rule(S &&t, Container &&deps)
        : cmd()
        , deps(std::forward<Container>(deps))
        , target(std::forward<S>(t))
        , phony(false)
    {}

    template<typename F, std::enable_if_t<isCmd<F>, int> = 0>
    Rule &with_cmd(F &&c)
    {
        cmd = std::make_optional(std::forward<F>(c));
        return *this;
    }

    template<typename S, std::enable_if_t<isString<S>, int> = 0>
    Rule &with_cmd(S &&s)
    {
        Cmd cmd;
        cmd.func = [=]() {
            return std::system((static_cast<std::string>(s)).c_str());
        };
        cmd.description = std::forward<S>(s);
        this->cmd = cmd;
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
        for (const auto &dep: deps()) {
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

    template<typename F, std::enable_if_t<isCmd<F>, int> = 0>
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

        for (const auto &dep: node->rule->deps()) {
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

    template<typename R, std::enable_if_t<isU<R, Rule>, int> = 0>
    Maker &operator+=(R &&rule)
    {
        rules[rule.target] = std::forward<R>(rule);
        return *this;
    }

    template<typename R, std::enable_if_t<isRule<R>, int> = 0>
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

#define fs std::filesystem
std::unordered_map<fs::path, std::vector<fs::path>>
parse_d(fs::path d_file)
{
    std::fstream file(d_file);
    std::unordered_map<fs::path, std::vector<fs::path>> dict;
    std::string lex;

    char cur_char;
    char next_char;
    std::string current_key;
    while ((cur_char = file.get()) != EOF) {
        if (std::isspace(cur_char)) {
            if (!lex.empty() && lex[lex.size() - 1] == ':') {
                current_key = {lex.data(), lex.size() - 1};
                dict[current_key] = {};
                lex.clear();
                continue;
            }
            if (!lex.empty())
                dict[current_key].push_back(lex);
            lex.clear();
            continue;
        }
        if (cur_char == '\\') {
            if ((next_char = file.peek()) == ' ') {
                lex += file.get();
                while (isspace(file.peek()))
                    (void)file.get();
                continue;
            } else {
                continue;
            }
        }
        lex += cur_char;
    }
    return dict;
}
#undef fs

template<typename S, std::enable_if_t<isString<S>, int> = 0>
inline Cmd from_string(S &&str)
{
    Cmd cmd;
    cmd.func = [=]() {
        return std::system((static_cast<std::string>(str)).c_str());
    };
    cmd.description = std::forward<S>(str);
    return cmd;
}

inline Cmd from_string(const char *str)
{
    return from_string(std::string(str));
}

inline std::vector<std::string> get_includes_from_file(std::filesystem::path filename)
{
    std::vector<std::string> results;
    std::ifstream file(filename);

    std::string line;
    while (std::getline(file, line)) {
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

} // namespace: utils

} // namespace: maker

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
