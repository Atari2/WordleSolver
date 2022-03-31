#define DEBUG_PRINT 0
#include "Solver.h"
#include <charconv>
#include <chrono>
#include <iostream>

#include <mutex>
#include <thread>

using namespace std::string_view_literals;
using namespace std::chrono;

constexpr auto help_message = R"(
There are 3 ways to invoke this program:
    - If you invoke it without arguments it'll just try to solve all wordles from day 0 to current day.
    - If you invoke with 1 argument there are 2 options:
        1 - A single number (e.g. ".\WordleSolver.exe 200"), this will make it attempt the Wordle of that day.
        2 - Two numbers divided by a hyphen (e.g. ".\WordleSolver.exe 1-20") this will make it attempt the wordles of days [begin, end)
You can additionally add a -p before the number (still space separated, e.g. .\WordleSolver.exe -p 1-2309) to enable parallelization.
A number can be additional added after -p (without space) to signal how many threads to spawn. 
If no number is specified std::thread::max_concurrency() is used.
It has no effect on single-solution program calls. Enabling parallelization won't print each result but just the final stats.
)"sv;

struct GuessData {
    size_t total_guesses = 0;
    size_t guessed = 0;
    size_t max_guesses = 0;
    size_t min_guesses = std::numeric_limits<size_t>::max();
};

struct RAIIPerfTimer {
    const high_resolution_clock::time_point start;
    RAIIPerfTimer() : start(high_resolution_clock::now()) {}
    ~RAIIPerfTimer() { std::cout << "Elapsed " << (high_resolution_clock::now() - start) << '\n'; }
};

bool solve_guess(Board& b, Solver& s, GuessData& data, bool print_intermediate) {
    while (!b.solved() && b.guesses() < b.max_guesses()) {
        b.guess(s.next_guess(b));
    }
    if (print_intermediate) b.print(s);
    size_t n_guesses = b.guesses();
    bool solved = b.solved();
    data.total_guesses += n_guesses;
    if (solved) {
        data.guessed++;
        if (print_intermediate)
            std::cout << "Solved in " << b.guesses() << " guesse(s), the word was " << b.solution() << "\n\n";
    } else {
        if (print_intermediate) std::cout << "Didn't solve it, the word was " << b.solution() << "\n\n";
    }
    data.max_guesses = std::max(data.max_guesses, n_guesses);
    data.min_guesses = std::min(data.min_guesses, n_guesses);
    return solved;
}

bool solve_guess(Board& b, Solver& s) {
    while (!b.solved() && b.guesses() < b.max_guesses()) {
        b.guess(s.next_guess(b));
    }
    b.print(s);
    size_t n_guesses = b.guesses();
    bool solved = b.solved();
    if (solved) {
        std::cout << "Solved in " << b.guesses() << " guesse(s), the word was " << b.solution() << "\n\n ";
    } else {
        std::cout << "Didn't solve it, the word was " << b.solution() << "\n\n";
    }
    return solved;
}

GuessData solve_loop(const std::span<std::string_view>& solutions, const std::span<WordView>& dict, size_t start,
                     size_t end, bool parallel = false) {
    GuessData data{};
    for (size_t i = start; i < end; i++) {
        Board b{solutions, i};
        Solver s{dict};
        solve_guess(b, s, data, !parallel);
    }
    return data;
}

GuessData solve_loop_parallel(const std::span<std::string_view>& solutions, const std::span<WordView>& dict,
                              size_t start, size_t end, size_t n_threads) {
    size_t n_problems_per_thread = (end - start) / n_threads;
    size_t rest = (end - start) % n_threads;
    std::vector<std::jthread> threads{};
    std::mutex m{};
    GuessData global_data{};
    threads.reserve(n_threads);
    for (size_t i = 0; i < n_threads; i++) {
        size_t t_start = start + (i * n_problems_per_thread);
        size_t t_end = end - ((n_threads - i - 1) * n_problems_per_thread) - (rest * (i != n_threads - 1));
        threads.emplace_back([&solutions, &dict, &m, &global_data, t_start, t_end]() {
            auto data = solve_loop(solutions, dict, t_start, t_end, true);
            {
                std::scoped_lock lock{m};
                global_data.total_guesses += data.total_guesses;
                global_data.guessed += data.guessed;
                global_data.max_guesses = std::max(global_data.max_guesses, data.max_guesses);
                global_data.min_guesses = std::min(global_data.min_guesses, data.min_guesses);
            }
        });
    }
    // .clear() calls the dtor of the vector elements one-by-one
    // since jthread join on destruction this is equal to calling .join on each thread
    threads.clear();
    return global_data;
}

void print_result(const GuessData& data, size_t sample_size) {
    std::cout << "Correctly guessed " << data.guessed << " out of " << sample_size << '\n';
    std::cout << "Max guesses " << data.max_guesses << ", Min guesses " << data.min_guesses << '\n';
    std::cout << "Average guesses: " << (static_cast<double>(data.total_guesses) / static_cast<double>(sample_size))
              << '\n';
}

int invalid_argument(std::string_view arg) {
    std::cout << "Invalid program arguments " << arg << "\n "sv;
    std::cout << help_message << '\n';
    return EXIT_FAILURE;
}

int main(int argc, char** argv) {
    const auto& solutions = get_solutions();
    const auto& dict = get_dictionary();
    if (argc < 2) {
        constexpr auto first_day = sys_days{2021y / June / 19};
        auto sol_idx_point = time_point{system_clock::now()} - time_point{first_day};
        size_t sol_idx = duration_cast<days>(sol_idx_point).count() + 1ull;
        RAIIPerfTimer timer{};
        auto data = solve_loop(solutions, dict, 0, sol_idx);
        print_result(data, sol_idx);
    } else {
        std::string_view arg{argv[argc - 1]};
        bool parallel = false;
        size_t n_threads = std::thread::hardware_concurrency();
        if (argc == 3) {
            std::string_view parallel_arg{argv[argc - 2]};
            constexpr auto arg_cmp = "-p"sv;
            constexpr size_t arg_size = arg_cmp.size();
            if (parallel_arg.starts_with(arg_cmp)) {
                parallel = true;
                if (parallel_arg.size() > arg_size) {
                    auto resp = std::from_chars(parallel_arg.data() + arg_size,
                                                parallel_arg.data() + parallel_arg.size(), n_threads);
                    if (resp.ec != std::errc{}) { return invalid_argument(parallel_arg); }
                }
            } else {
                return invalid_argument(parallel_arg);
            }
        }
        if (arg == "help"sv) {
            std::cout << help_message << '\n';
            return EXIT_SUCCESS;
        }
        if (size_t idx = arg.find('-'); idx != std::string_view::npos) {
            size_t start_idx = 0;
            size_t end_idx = 0;
            std::string_view start = arg.substr(0, idx);
            std::string_view end = arg.substr(idx + 1);
            auto res1 = std::from_chars(start.data(), start.data() + start.size(), start_idx);
            auto res2 = std::from_chars(end.data(), end.data() + end.size(), end_idx);
            if (res1.ec != std::errc{} || res2.ec != std::errc{}) { return invalid_argument(arg); }
            if (start_idx > end_idx) std::swap(start_idx, end_idx);
            RAIIPerfTimer timer{};
            if (!parallel) {
                auto data = solve_loop(solutions, dict, start_idx, end_idx);
                print_result(data, end_idx - start_idx);
            } else {
                auto data = solve_loop_parallel(solutions, dict, start_idx, end_idx, n_threads);
                print_result(data, end_idx - start_idx);
            }
        } else {
            idx = 0;
            auto res1 = std::from_chars(arg.data(), arg.data() + arg.size(), idx);
            if (res1.ec != std::errc{}) { return invalid_argument(arg); }
            Board b{solutions, idx};
            Solver s{dict};
            solve_guess(b, s);
        }
    }
    return EXIT_SUCCESS;
}
