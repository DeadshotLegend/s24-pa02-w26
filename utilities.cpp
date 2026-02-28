// utilities.cpp (FINAL - robust for large dataset)
#include "utilities.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <vector>

static inline bool is_ascii_ws(unsigned char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static std::string trim_all(std::string s) {
    while (!s.empty() && is_ascii_ws((unsigned char)s.front())) s.erase(s.begin());
    while (!s.empty() && is_ascii_ws((unsigned char)s.back())) s.pop_back();

    while (s.size() >= 2 &&
           (unsigned char)s[0] == 0xC2 &&
           (unsigned char)s[1] == 0xA0) {
        s.erase(0, 2);
        while (!s.empty() && is_ascii_ws((unsigned char)s.front())) s.erase(s.begin());
    }

    while (s.size() >= 2 &&
           (unsigned char)s[s.size() - 2] == 0xC2 &&
           (unsigned char)s[s.size() - 1] == 0xA0) {
        s.erase(s.size() - 2, 2);
        while (!s.empty() && is_ascii_ws((unsigned char)s.back())) s.pop_back();
    }

    return s;
}

static std::string unescape_doubled_quotes(const std::string& s) {
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

static std::string unquoteCSVField(std::string s) {
    s = trim_all(std::move(s));

    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        s = s.substr(1, s.size() - 2);
    }

    s = unescape_doubled_quotes(s);
    return trim_all(std::move(s));
}

static int ratingOutToRating10(const std::string& out) {
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

// Find the last comma that is NOT inside quotes.
// This is robust to:
// - Proper CSV: "title, with comma",7.1
// - Broken CSV:  title, with comma,7.1   (unquoted commas)
// We assume the rating is after the final separator.
static size_t find_last_comma_outside_quotes(const std::string& line) {
    bool inQuotes = false;
    size_t last = std::string::npos;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '"') {
            // Toggle quotes, but skip doubled quotes inside a quoted field
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                i++; // skip second quote
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            last = i;
        }
    }
    return last;
}

static Movie parseMovieLine(const std::string& lineRaw) {
    std::string line = lineRaw;
    // keep internal chars, just remove trailing newline-like stuff
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();

    size_t commaPos = find_last_comma_outside_quotes(line);
    if (commaPos == std::string::npos) {
        throw std::runtime_error("Bad CSV line (no separator comma): " + line);
    }

    std::string nameField   = line.substr(0, commaPos);
    std::string ratingField = line.substr(commaPos + 1);

    Movie m;
    m.name = unquoteCSVField(std::move(nameField));

    std::string ratingStr = trim_all(std::move(ratingField));

    long double d;
    try {
        d = std::stold(ratingStr);
    } catch (...) {
        throw std::runtime_error("Bad rating value: " + ratingStr);
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << d;
    m.rating_out = oss.str();

    m.rating10 = ratingOutToRating10(m.rating_out);

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
        prefixes.push_back(trim_all(line));
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