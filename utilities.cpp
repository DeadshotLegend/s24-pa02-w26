// utilities.cpp  
#include "utilities.h"
#include <fstream>
#include <stdexcept>
#include <cctype>
#include <cmath>     
#include <string>

static std::string trim(std::string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
                          s.back() == ' '  || s.back() == '\t')) {
        s.pop_back();
    }
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) i++;
    return s.substr(i);
}

static int parseRating10(const std::string& ratingStrRaw) {
    std::string ratingStr = trim(ratingStrRaw);

    long double d = 0.0L;
    try {
        d = std::stold(ratingStr);
    } catch (...) {
        throw std::runtime_error("Bad rating value: " + ratingStr);
    }

    long long r10 = llround(d * 10.0L); 

    if (r10 < 0) r10 = 0;
    if (r10 > 100) r10 = 100;

    return static_cast<int>(r10);
}

static Movie parseMovieLine(const std::string& lineRaw) {
    std::string line = lineRaw;
    if (!line.empty() && line.back() == '\r') line.pop_back();

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
        if (!line.empty() && line.back() == '\r') line.pop_back(); // CRLF safe
        prefixes.push_back(line); // keep whitespace inside prefix
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