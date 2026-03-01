#include "utilities.h"

#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

static void rstrip_newlines(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
}

// Find the last comma that is NOT inside CSV quotes.
static size_t find_last_comma_outside_quotes(const std::string& line) {
    bool inQuotes = false;
    size_t last = std::string::npos;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                i++; // escaped quote inside quoted region
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            last = i;
        }
    }
    return last;
}

// Trim only spaces/tabs around field
static void trim_edges_ws(std::string& s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(s.begin());
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();
}

// Unquote if wrapped in quotes; unescape "" -> "
static std::string unquoteCSVField(std::string s) {
    rstrip_newlines(s);
    trim_edges_ws(s);

    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        s = s.substr(1, s.size() - 2);

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
    return s;
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

// Normalize rating for printing: "6.0"->"6", keep "6.7", keep "0"
static std::string normalizeRatingOut(std::string r) {
    rstrip_newlines(r);
    trim_edges_ws(r);

    auto dot = r.find('.');
    if (dot != std::string::npos) {
        while (!r.empty() && r.back() == '0') r.pop_back();
        if (!r.empty() && r.back() == '.') r.pop_back();
    }
    return r;
}

static Movie parseMovieLine(std::string line) {
    rstrip_newlines(line);

    size_t commaPos = find_last_comma_outside_quotes(line);
    if (commaPos == std::string::npos) {
        throw std::runtime_error("Bad CSV line: " + line);
    }

    std::string nameField = line.substr(0, commaPos);
    std::string ratingField = line.substr(commaPos + 1);

    Movie m;
    m.name = unquoteCSVField(std::move(nameField));
    trim_edges_ws(m.name);

    m.rating_out = normalizeRatingOut(std::move(ratingField));
    m.rating10 = ratingStrToRating10(m.rating_out);

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

// IMPORTANT: prefixes may include whitespace — do NOT trim.
// Only strip trailing CR.
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