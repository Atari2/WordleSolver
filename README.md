# WordleSolver

Self-explicatory name, this a very simple and naive wordle solver made in C++.
It doesn't use fancy maths or complex algorithms to find solutions, just a bit of logic.
Everything in this was self-made, I didn't use any external source for information theories, algorithms to find the best solutions and stuff like that. 
I wanted this to be as simple as it can be while still solving a nice amount of the puzzles.

In fact, this solver is not perfect (as in, it doesn't solve every single wordle), the current stats are:
```
Correctly guessed 2152 out of 2309
Max guesses 6, Min guesses 1
Average guesses: 4.67259
```
with about a 93.2% success rate.

It is MSVC only and requires C++23 (will require C++20 when std::ranges are available on /std:c++20 instead of /std:c++latest).