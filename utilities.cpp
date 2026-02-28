// utilities.cpp 
#include "utilities.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <vector>

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

// Strip outer quotes if present, and ALWAYS unescape doubled quotes: "" -> "
static std::string unquoteCSVField(std::string s) {
    // trim spaces/tabs/CR
    auto is_ws = [](unsigned char c){ return c == ' ' || c == '\t' || c == '\r'; };
    while (!s.empty() && is_ws((unsigned char)s.front())) s.erase(s.begin());
    while (!s.empty() && is_ws((unsigned char)s.back()))  s.pop_back();

    // strip outer ASCII quotes
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
    return out;
}

// Convert formatted rating string like "7.7" or "10.0" to integer tenths (77, 100)
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

// Split a CSV line into exactly 2 fields (name, rating) by finding the first comma not in quotes.
// Handles quoted titles with commas and doubled quotes inside.
static void splitTwoCSVFields(const std::string& lineRaw, std::string& field1, std::string& field2) {
    std::string line = rtrim(lineRaw);

    bool inQuotes = false;
    std::string a;
    size_t i = 0;

    for (; i < line.size(); i++) {
        char c = line[i];

        if (c == '"') {
            // "" inside quotes means a literal quote
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                a.push_back('"');
                i++; // skip second quote
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            // separator between name and rating
            i++; // skip comma
            break;
        } else {
            a.push_back(c);
        }
    }

    field1 = a;
    field2 = (i <= line.size()) ? line.substr(i) : "";
}

static Movie parseMovieLine(const std::string& lineRaw) {
    std::string nameField, ratingField;
    splitTwoCSVFields(lineRaw, nameField, ratingField);

    Movie m;
    m.name = unquoteCSVField(nameField);

    std::string ratingStr = trim(ratingField);

    long double d;
    try {
        d = std::stold(ratingStr);
    } catch (...) {
        throw std::runtime_error("Bad rating value: " + ratingStr);
    }

    // Format EXACTLY like the spec output: fixed, 1 decimal
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << d;
    m.rating_out = oss.str();

    // Make numeric key consistent with printed value
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
        prefixes.push_back(rtrim(line)); // keep whitespace inside prefix intact
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