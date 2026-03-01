#include "utilities.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

// Remove only trailing CR/LF characters
static void rstrip_newlines(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
}

// Normalize rating for printing: "6.0"->"6", keep "6.7", keep "0"
static std::string normalizeRatingOut(std::string r) {
    rstrip_newlines(r);

    // trim spaces/tabs around rating (safe)
    while (!r.empty() && (r.front() == ' ' || r.front() == '\t')) r.erase(r.begin());
    while (!r.empty() && (r.back() == ' ' || r.back() == '\t')) r.pop_back();

    // drop trailing .0
    auto dot = r.find('.');
    if (dot != std::string::npos) {
        while (!r.empty() && r.back() == '0') r.pop_back();
        if (!r.empty() && r.back() == '.') r.pop_back();
    }
    return r;
}

// rating string like "8.1" -> 81, "0" -> 0
static int ratingStrToRating10(const std::string& s) {
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

static Movie parseMovieLine(std::string line) {
    rstrip_newlines(line);

    // rating is always after the LAST comma
    size_t commaPos = line.rfind(',');
    if (commaPos == std::string::npos) {
        throw std::runtime_error("Bad CSV line: " + line);
    }

    Movie m;
    m.name = line.substr(0, commaPos);                 // KEEP RAW TITLE (including quotes)
    std::string ratingField = line.substr(commaPos + 1);

    m.rating_out = normalizeRatingOut(ratingField);
    m.rating10   = ratingStrToRating10(m.rating_out);

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

// IMPORTANT: prefixes may include whitespace; do NOT trim.
// Only strip trailing '\r' if present.
std::vector<std::string> readPrefixes(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Could not open prefix file: " + filename);

    std::vector<std::string> prefixes;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        prefixes.push_back(line);
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