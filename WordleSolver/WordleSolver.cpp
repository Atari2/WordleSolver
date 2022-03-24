#include "Solver.h"
#include <charconv>
#include <chrono>
#include <iostream>

using namespace std::string_view_literals;
using namespace std::chrono;

struct GuessData {
    size_t total_guesses = 0;
    size_t guessed = 0;
    size_t max_guesses = 0;
    size_t min_guesses = std::numeric_limits<size_t>::max();
};

bool solve_guess(Board& b, Solver& s, GuessData& data) {
    while (!b.solved() && b.guesses() < b.max_guesses()) {
        b.guess(s.next_guess(b));
    }
    b.print(s);
    size_t n_guesses = b.guesses();
    bool solved = b.solved();
    data.total_guesses += n_guesses;
    if (solved) {
        data.guessed++;
        std::cout << "Solved in " << b.guesses() << " guesse(s), the word was " << b.solution() << "\n\n";
    } else {
        std::cout << "Didn't solve it, the word was " << b.solution() << "\n\n";
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
        std::cout << "Solved in " << b.guesses() << " guesse(s), the word was " << b.solution() << "\n\n";
    } else {
        std::cout << "Didn't solve it, the word was " << b.solution() << "\n\n";
    }
    return solved;
}

void solve_loop(const std::span<const std::string_view>& solutions, const std::span<const WordView>& dict, size_t start, size_t end) {
    GuessData data{};
    for (size_t i = start; i < end; i++) {
        Board b{solutions, i};
        Solver s{dict};
        solve_guess(b, s, data);
    }
    std::cout << "Correctly guessed " << data.guessed << " out of " << (end - start) << '\n';
    std::cout << "Max guesses " << data.max_guesses << ", Min guesses " << data.min_guesses << '\n';
    std::cout << "Average guesses: " << (static_cast<double>(data.total_guesses) / static_cast<double>(end - start))
              << '\n';
}

int main(int argc, char** argv) {
    constexpr auto help_message = R"(
There are 3 ways to invoke this program:
    - If you invoke it without arguments it'll just try to solve all wordles from day 0 to current day.
    - If you invoke with 1 argument there are 2 options:
        1 - A single number (e.g. ".\WordleSolver.exe 200"), this will make it attempt the Wordle of that day.
        2 - Two numbers divided by a hyphen (e.g. ".\WordleSolver.exe 1-20") this will make it attempt the wordles of days [begin, end)
)"sv;
    const auto& solutions = get_solutions();
    const auto& dict = get_dictionary();
    if (argc != 2) {
        constexpr auto first_day = sys_days{2021y / June / 19};
        auto sol_idx_point = time_point{system_clock::now()} - time_point{first_day};
        size_t sol_idx = duration_cast<days>(sol_idx_point).count() + 1ull;
        solve_loop(solutions, dict, 0, sol_idx);
    } else {
        std::string_view arg{argv[1]};
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
            if (res1.ec != std::errc{} || res2.ec != std::errc{}) {
                std::cout << "Invalid program arguments\n"sv;
                std::cout << help_message << '\n';
                return EXIT_FAILURE;
            }
            if (start_idx > end_idx) std::swap(start_idx, end_idx);
            solve_loop(solutions, dict, start_idx, end_idx);
        } else {
            idx = 0;
            auto res1 = std::from_chars(arg.data(), arg.data() + arg.size(), idx);
            if (res1.ec != std::errc{}) {
                std::cout << "Invalid program arguments\n"sv;
                std::cout << help_message << '\n';
                return EXIT_FAILURE;
            }
            Board b{solutions, idx};
            Solver s{dict};
            solve_guess(b, s);
        }
    }
    return EXIT_SUCCESS;
}
