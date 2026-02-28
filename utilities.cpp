// utilities.cpp — FINAL, robust for PA02 large dataset
#include "utilities.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <vector>
#include <algorithm>

/* ------------------ helpers ------------------ */

static inline bool is_ws(unsigned char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static std::string trim(std::string s) {
    while (!s.empty() && is_ws((unsigned char)s.front())) s.erase(s.begin());
    while (!s.empty() && is_ws((unsigned char)s.back()))  s.pop_back();
    return s;
}

// Remove ALL remaining double-quote characters from title
static void removeAllDoubleQuotes(std::string& s) {
    s.erase(std::remove(s.begin(), s.end(), '"'), s.end());
}

// Unquote CSV field and unescape "" → "
static std::string unquoteCSVField(std::string s) {
    s = trim(std::move(s));

    // Strip outer quotes if present
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        s = s.substr(1, s.size() - 2);
    }

    // Unescape doubled quotes
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '"' && i + 1 < s.size() && s[i + 1] == '"') {
            out.push_back('"');
            i++;
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

// Convert formatted rating string like "7.7" → 77
static int ratingOutToRating10(const std::string& out) {
    int whole = 0, frac = 0;
    size_t i = 0;

    while (i < out.size() && std::isdigit((unsigned char)out[i])) {
        whole = whole * 10 + (out[i] - '0');
        i++;
    }
    if (i < out.size() && out[i] == '.') {
        i++;
        if (i < out.size() && std::isdigit((unsigned char)out[i])) {
            frac = out[i] - '0';
        }
    }

    return whole * 10 + frac;
}

// Find LAST comma that is NOT inside quotes
static size_t find_last_comma_outside_quotes(const std::string& line) {
    bool inQuotes = false;
    size_t last = std::string::npos;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                i++; // skip escaped quote
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            last = i;
        }
    }
    return last;
}

/* ------------------ parsing ------------------ */

static Movie parseMovieLine(const std::string& rawLine) {
    std::string line = rawLine;
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
        line.pop_back();
    }

    size_t commaPos = find_last_comma_outside_quotes(line);
    if (commaPos == std::string::npos) {
        throw std::runtime_error("Bad CSV line: " + line);
    }

    std::string nameField   = line.substr(0, commaPos);
    std::string ratingField = line.substr(commaPos + 1);

    Movie m;
    m.name = unquoteCSVField(nameField);
    removeAllDoubleQuotes(m.name);     // 🔥 FINAL FIX
    m.name = trim(m.name);

    std::string ratingStr = trim(ratingField);

    long double rating;
    try {
        rating = std::stold(ratingStr);
    } catch (...) {
        throw std::runtime_error("Bad rating value: " + ratingStr);
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << rating;
    m.rating_out = oss.str();
    m.rating10   = ratingOutToRating10(m.rating_out);

    return m;
}

/* ------------------ API ------------------ */

std::vector<Movie> readMoviesCSV(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Could not open movie file: " + filename);

    std::vector<Movie> movies;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            movies.push_back(parseMovieLine(line));
        }
    }
    return movies;
}

std::vector<std::string> readPrefixes(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Could not open prefix file: " + filename);

    std::vector<std::string> prefixes;
    std::string line;
    while (std::getline(in, line)) {
        prefixes.push_back(trim(line));
    }
    return prefixes;
}

bool startsWith(const std::string& s, const std::string& prefix) {
    if (prefix.size() > s.size()) return false;
    for (size_t i = 0; i < prefix.size(); i++) {
        if (s[i] != prefix[i]) return false;
    }
    return true;
}