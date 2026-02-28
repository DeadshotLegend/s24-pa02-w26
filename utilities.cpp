#include "utilities.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <cctype>

static std::string rtrim(std::string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
                          s.back() == ' '  || s.back() == '\t')) {
        s.pop_back();
    }
    return s;
}

static std::string ltrim(std::string s) {
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) i++;
    return s.substr(i);
}

static std::string trim(std::string s) {
    return ltrim(rtrim(std::move(s)));
}

static int ratingOutToRating10(const std::string& out) {
    // out is guaranteed to be like "7.7" or "10.0" from ostringstream formatting
    int whole = 0;
    int frac = 0;
    size_t i = 0;

    while (i < out.size() && std::isdigit(static_cast<unsigned char>(out[i]))) {
        whole = whole * 10 + (out[i] - '0');
        i++;
    }
    if (i < out.size() && out[i] == '.') {
        i++;
        if (i < out.size() && std::isdigit(static_cast<unsigned char>(out[i]))) {
            frac = out[i] - '0';
        }
    }
    int r10 = whole * 10 + frac;
    if (r10 < 0) r10 = 0;
    if (r10 > 100) r10 = 100;
    return r10;
}

static Movie parseMovieLine(const std::string& lineRaw) {
    std::string line = rtrim(lineRaw);

    std::size_t commaPos = line.rfind(',');
    if (commaPos == std::string::npos) {
        throw std::runtime_error("Bad CSV line (no comma): " + line);
    }

    Movie m;
    m.name = trim(line.substr(0, commaPos));

    std::string ratingStr = trim(line.substr(commaPos + 1));

    // Parse rating as long double, then format EXACTLY like reference output:
    // fixed << setprecision(1)
    long double d;
    try {
        d = std::stold(ratingStr);
    } catch (...) {
        throw std::runtime_error("Bad rating value: " + ratingStr);
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << static_cast<long double>(d);
    m.rating_out = oss.str();          // exact printable rating
    m.rating10 = ratingOutToRating10(m.rating_out); // numeric key consistent with printed value

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
        line = rtrim(line);          // remove CRLF
        prefixes.push_back(line);    // keep whitespace inside prefix intact
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