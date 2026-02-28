#include "movies.h"
#include <algorithm>

void sortByName(std::vector<Movie>& movies) {
    std::sort(movies.begin(), movies.end(),
              [](const Movie& a, const Movie& b) {
                  return a.name < b.name;
              });
}

bool betterForPrefixOutput(const Movie* a, const Movie* b) {
    if (a->rating10 != b->rating10) return a->rating10 > b->rating10;
    return a->name < b->name;
}