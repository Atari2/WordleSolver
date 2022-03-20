#include "Board.h"
#include "Solver.h"
#include "data/DictionaryLoader.h"
#include <charconv>
#include <chrono>
#include <iostream>

using namespace std::string_view_literals;

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

using namespace std::chrono;

int main(int argc, char** argv) {
    std::string_view files[2]{"data/Words.txt"sv, "data/Solutions.txt"sv};
    const auto& solutions = get_solutions(files[1]);
    const auto& dict = get_dictionary(files);
    GuessData data{};
    if (argc != 2) {
        constexpr auto first_day = sys_days{2021y / June / 19};

        auto sol_idx_point = time_point{system_clock::now()} - time_point{first_day};
        size_t sol_idx = floor<days>(sol_idx_point).count() + 1ull;

        for (size_t i = 0; i < sol_idx; i++) {
            Board b{solutions, i};
            Solver s{dict};
            if (!solve_guess(b, s, data)) b.print(s);
        }
        std::cout << "Correctly guessed " << data.guessed << " out of " << sol_idx << '\n';
        std::cout << "Max guesses " << data.max_guesses << ", Min guesses " << data.min_guesses << '\n';
        std::cout << "Average guesses: " << (static_cast<double>(data.total_guesses) / static_cast<double>(sol_idx))
                  << '\n';
    } else {
        size_t idx = 0;
        auto res1 = std::from_chars(argv[1], argv[1] + strlen(argv[1]), idx);
        if (res1.ec != std::errc{}) {
            std::cout << "Invalid program arguments\n";
            return EXIT_FAILURE;
        }
        Board b{solutions, idx};
        Solver s{dict};
        solve_guess(b, s, data);
    }
    return EXIT_SUCCESS;
}
