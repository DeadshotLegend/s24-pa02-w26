#include "utilities.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

/* ---------- helpers ---------- */

static std::string rstrip_cr(std::string s) {
    if (!s.empty() && s.back() == '\r') s.pop_back();
    return s;
}

static std::string trim_edges_ws_crlf(std::string s) {
    auto is_ws = [](unsigned char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };
    while (!s.empty() && is_ws((unsigned char)s.front())) s.erase(s.begin());
    while (!s.empty() && is_ws((unsigned char)s.back())) s.pop_back();
    return s;
}

// Unquote ONLY if the entire field is wrapped in quotes, and unescape "" -> "
static std::string unquoteCSVField(std::string s) {
    s = trim_edges_ws_crlf(std::move(s));

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

// rating string -> numeric tenths for comparisons (0..100)
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
    return whole * 10 + frac; // e.g., 8.1 -> 81
}

// Normalize rating for printing: "6.0"->"6", keep "6.7", keep "0"
static std::string normalizeRatingOut(std::string r) {
    r = trim_edges_ws_crlf(std::move(r));
    if (r.find('.') != std::string::npos) {
        while (!r.empty() && r.back() == '0') r.pop_back();
        if (!r.empty() && r.back() == '.') r.pop_back();
    }
    return r;
}

// Find last comma not inside quoted CSV region
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
    m.name = unquoteCSVField(std::move(nameField));     // preserves internal quotes
    m.name = trim_edges_ws_crlf(std::move(m.name));     // trim edges only

    m.rating_out = normalizeRatingOut(std::move(ratingField));
    m.rating10   = ratingStrToRating10(m.rating_out);

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

// IMPORTANT: Do NOT trim spaces—prefixes may include whitespace.
// Only remove trailing CR from Windows line endings.
std::vector<std::string> readPrefixes(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Could not open prefix file: " + filename);

    std::vector<std::string> prefixes;
    std::string line;
    while (std::getline(in, line)) {
        prefixes.push_back(rstrip_cr(line));
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