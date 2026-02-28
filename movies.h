#ifndef MOVIES_H
#define MOVIES_H

#include <string>
#include <vector>

struct Movie {
    std::string name;        
    int rating10;            
    std::string rating_out;  
};

void sortByName(std::vector<Movie>& movies);

// Optional; not used by new main.cpp, but keep for completeness.
bool betterForPrefixOutput(const Movie* a, const Movie* b);

#endif