#include "data/DictionaryLoader.h"
#include "Board.h"
#include "Solver.h"
#include <iostream>
#include <charconv>

using namespace std::string_view_literals;

int main(int argc, char** argv) {
    std::string_view files[2]{"data/Words.txt"sv, "data/Solutions.txt"sv};
    const auto& solutions = get_solutions(files[1]);
    const auto& dict = get_dictionary(files);
    size_t idx = 0;
    size_t sol_idx = 0;
    if (argc != 3) {
        idx = static_starting_word_idx;
        sol_idx = static_solution_idx;
    } else {
        auto res1 = std::from_chars(argv[1], argv[1] + strlen(argv[1]), idx);
        auto res2 = std::from_chars(argv[2], argv[2] + strlen(argv[2]), sol_idx);
        if (res1.ec != std::errc{} || res2.ec != std::errc{}) {
            std::cout << "Invalid program arguments\n";
            return EXIT_FAILURE;
        }
    }
    size_t total_guesses = 0;
    size_t guessed = 0;
    size_t max_guesses = 0;
    size_t min_guesses = 0;
    for (size_t i = 0; i < 271; i++) {
        Board b{solutions, i};
        Solver s{dict, idx};
        while (!b.solved() && b.guesses() < b.max_guesses()) {
            b.guess(s.next_guess(b));
        }
        b.print(s);
        std::cout << "[" << i << "] ";
        size_t n_guesses = b.guesses();
        if (b.solved()) {
            total_guesses += n_guesses;
            guessed++;
            std::cout << "Solved in " << b.guesses() << " guesse(s), the word was " << b.solution() << "\n";
            if (i == 0) {
                max_guesses = n_guesses;
                min_guesses = n_guesses;
            } else {
                if (max_guesses < n_guesses) max_guesses = n_guesses;
                else if (min_guesses > n_guesses) min_guesses = n_guesses;
            }
        } else {
            std::cout << "Didn't solve it, the word was " << b.solution() << '\n';
        }
    }
    std::cout << "Correctly guessed " << guessed << " out of " << static_solution_idx << '\n';
    std::cout << "Max guesses " << max_guesses << ", Min guesses " << min_guesses << '\n';
    std::cout << "Average guesses: " << (static_cast<double>(total_guesses) / static_cast<double>(static_solution_idx))
              << '\n';
    return EXIT_SUCCESS;
}
