#ifndef MOVIES_H
#define MOVIES_H

#include <string>
#include <vector>

struct Movie {
    std::string name;       
    std::string rating_out; 
    int rating10;           
};

void sortByName(std::vector<Movie>& movies);
bool betterForPrefixOutput(const Movie* a, const Movie* b);

#endif