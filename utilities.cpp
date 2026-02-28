#include "utilities.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>

/* ---------- helpers ---------- */

static inline bool is_ws(unsigned char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static std::string trim(std::string s) {
    while (!s.empty() && is_ws((unsigned char)s.front())) s.erase(s.begin());
    while (!s.empty() && is_ws((unsigned char)s.back()))  s.pop_back();
    return s;
}

static void removeAllDoubleQuotes(std::string& s) {
    s.erase(std::remove(s.begin(), s.end(), '"'), s.end());
}

// Strip outer quotes if present, unescape "" -> ", then remove stray quotes
static std::string cleanTitle(std::string s) {
    s = trim(std::move(s));

    // strip outer quotes
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        s = s.substr(1, s.size() - 2);
    }

    // unescape doubled quotes
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

    // remove any remaining " characters (dataset oddities)
    removeAllDoubleQuotes(out);
    return trim(std::move(out));
}

// rating string -> numeric tenths for comparisons
static int ratingStrToRating10(const std::string& s) {
    // parse like: 6, 6.0, 6.7
    int whole = 0, frac = 0;
    size_t i = 0;

    while (i < s.size() && std::isdigit((unsigned char)s[i])) {
        whole = whole * 10 + (s[i] - '0');
        i++;
    }
    if (i < s.size() && s[i] == '.') {
        i++;
        if (i < s.size() && std::isdigit((unsigned char)s[i])) {
            frac = s[i] - '0';
        }
    }
    return whole * 10 + frac;
}

// Normalize rating for printing:
// "6.0" -> "6", "6.7" -> "6.7", "10.0" -> "10"
static std::string normalizeRatingOut(std::string r) {
    r = trim(std::move(r));
    // remove trailing zeros and trailing dot
    if (r.find('.') != std::string::npos) {
        while (!r.empty() && r.back() == '0') r.pop_back();
        if (!r.empty() && r.back() == '.') r.pop_back();
    }
    return r;
}

// Find last comma outside quotes (robust on big dataset)
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

static Movie parseMovieLine(const std::string& rawLine) {
    std::string line = rawLine;
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();

    size_t commaPos = find_last_comma_outside_quotes(line);
    if (commaPos == std::string::npos) {
        throw std::runtime_error("Bad CSV line: " + line);
    }

    std::string nameField   = line.substr(0, commaPos);
    std::string ratingField = line.substr(commaPos + 1);

    Movie m;
    m.name = cleanTitle(nameField);

    // Keep rating formatting like expected output
    m.rating_out = normalizeRatingOut(ratingField);

    // Numeric key for sorting by rating desc
    m.rating10 = ratingStrToRating10(m.rating_out);

    return m;
}

/* ---------- API ---------- */

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
        // keep internal whitespace, just trim ends / CRLF
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