#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <vector>
#include "movies.h"

std::vector<Movie> readMoviesCSV(const std::string& filename);
std::vector<std::string> readPrefixes(const std::string& filename);

bool startsWith(const std::string& s, const std::string& prefix);

#endif