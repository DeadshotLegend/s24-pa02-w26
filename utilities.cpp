// utilities.cpp 
#include "utilities.h"
#include <fstream>
#include <stdexcept>
#include <cctype>

static int parseRating10(std::string s) {
    // Handle Windows line endings / trailing whitespace
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) {
        s.pop_back();
    }

    // Skip leading whitespace
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;

    int whole = 0;
    while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) {
        whole = whole * 10 + (s[i] - '0');
        i++;
    }

    int frac = 0;
    if (i < s.size() && s[i] == '.') {
        i++;
        if (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) {
            frac = s[i] - '0'; // only one decimal place needed
        }
    }

    return whole * 10 + frac;
}

static Movie parseMovieLine(const std::string& line) {
    std::size_t commaPos = line.rfind(',');
    if (commaPos == std::string::npos) {
        throw std::runtime_error("Bad CSV line (no comma): " + line);
    }

    Movie m;
    m.name = line.substr(0, commaPos);

    std::string ratingStr = line.substr(commaPos + 1);
    m.rating10 = parseRating10(ratingStr);

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
        if (!line.empty() && line.back() == '\r') line.pop_back(); // handle CRLF
        prefixes.push_back(line);
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