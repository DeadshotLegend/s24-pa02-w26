#include "movies.h"
#include <algorithm>

void sortByName(std::vector<Movie>& movies) {
    std::sort(movies.begin(), movies.end(),
              [](const Movie& a, const Movie& b) {
                  return a.name < b.name;
              });
}

bool betterForPrefixOutput(const Movie* a, const Movie* b) {
    if (a->rating != b->rating) return a->rating > b->rating;
    return a->name < b->name;
}