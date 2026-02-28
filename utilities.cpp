#include "utilities.h"
#include <fstream>
#include <stdexcept>

static Movie parseMovieLine(const std::string& line) {
    std::size_t commaPos = line.rfind(',');
    if (commaPos == std::string::npos) {
        throw std::runtime_error("Bad CSV line (no comma): " + line);
    }

    Movie m;
    m.name = line.substr(0, commaPos);
    std::string ratingStr = line.substr(commaPos + 1);
    m.rating = std::stod(ratingStr);
    return m;
}

std::vector<Movie> readMoviesCSV(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Could not open movie file: " + filename);

    std::vector<Movie> movies;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        movies.push_back(parseMovieLine(line));
    }
    return movies;
}

std::vector<std::string> readPrefixes(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Could not open prefix file: " + filename);

    std::vector<std::string> prefixes;
    std::string line;
    while (std::getline(in, line)) {
        prefixes.push_back(line); // keep whitespace as part of prefix
    }
    return prefixes;
}

bool startsWith(const std::string& s, const std::string& prefix) {
    if (prefix.size() > s.size()) return false;
    for (std::size_t i = 0; i < prefix.size(); i++) {
        if (s[i] != prefix[i]) return false;
    }
    return true;
}