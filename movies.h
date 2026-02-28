// movies.h  (NEW)

#ifndef MOVIES_H
#define MOVIES_H

#include <string>
#include <vector>

struct Movie {
    std::string name;
    int rating10; 
};

void sortByName(std::vector<Movie>& movies);

// (Optional) You can keep this declared/defined if you want, but it's no longer needed by main.cpp
bool betterForPrefixOutput(const Movie* a, const Movie* b);

#endif