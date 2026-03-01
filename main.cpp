// Shivansh Goel - CS24 PA02

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "movies.h"
#include "utilities.h"

using namespace std;

// Local startsWith so we don't depend on utilities.h declaring it
static inline bool startsWithLocal(const string& s, const string& prefix) {
    if (prefix.size() > s.size()) return false;
    for (size_t i = 0; i < prefix.size(); i++) {
        if (s[i] != prefix[i]) return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc != 2 && argc != 3) {
        cerr << "Usage: ./runMovies movieFilename [prefixFilename]\n";
        return 1;
    }

    const string movieFilename = argv[1];

    vector<Movie> movies;
    movies = readMoviesCSV(movieFilename);

    // Part 1: alphabetical order by movie name (use the printable name field)
    sortByName(movies);

    // =========================
    // Part 1
    // =========================
    if (argc == 2) {
        for (const Movie& m : movies) {
            cout << m.name << ", " << m.rating_out << "\n";
        }
        return 0;
    }

    // =========================
    // Part 2
    // =========================
    const string prefixFilename = argv[2];
    vector<string> prefixes = readPrefixes(prefixFilename);

    struct BestResult {
        string prefix;
        const Movie* best;
    };

    vector<BestResult> bestResults;
    bestResults.reserve(prefixes.size());

    // lower_bound comparator: compare Movie.name to prefix key
    auto cmpMovieToKey = [](const Movie& m, const string& key) {
        return m.name < key;
    };

    bool printedAnySuccessfulBlock = false;

    for (const string& prefix : prefixes) {
        auto it = lower_bound(movies.begin(), movies.end(), prefix, cmpMovieToKey);

        vector<const Movie*> matches;
        matches.reserve(32);

        for (auto jt = it; jt != movies.end(); ++jt) {
            if (!startsWithLocal(jt->name, prefix)) break;
            matches.push_back(&(*jt));
        }

        if (matches.empty()) {
            cout << "No movies found with prefix " << prefix << "\n";
            continue; // no blank line after this
        }

        // Sort by rating desc (rating10), then name asc
        sort(matches.begin(), matches.end(),
             [](const Movie* a, const Movie* b) {
                 if (a->rating10 != b->rating10) return a->rating10 > b->rating10;
                 return a->name < b->name;
             });

        // Blank line BETWEEN successful blocks only
        if (printedAnySuccessfulBlock) cout << "\n";
        printedAnySuccessfulBlock = true;

        for (const Movie* mp : matches) {
            cout << mp->name << ", " << mp->rating_out << "\n";
        }

        bestResults.push_back({prefix, matches.front()});
    }

    // Exactly one blank line before Best section if we printed any successful block
    if (printedAnySuccessfulBlock && !bestResults.empty()) cout << "\n";

    for (const auto& br : bestResults) {
        cout << "Best movie with prefix " << br.prefix
             << " is: " << br.best->name
             << " with rating " << br.best->rating_out
             << "\n";
    }

    return 0;
}

/******************************************************************************
 * PART 3a: Time Complexity (Worst Case)
 *
 * n = number of movies, m = number of prefixes, k = max matches per prefix,
 * l = max title/prefix length
 *
 * For each prefix:
 *   lower_bound: O(log n * l)
 *   scan matches: O(k * l)
 *   sort matches: O(k log k * l)
 *
 * Total: O(m * (log n + k + k log k) * l)
 *
 * PART 3b: Space Complexity (Worst Case)
 *   movies: O(n*l)
 *   prefixes: O(m*l)
 *   matches: O(k)
 *   bestResults: O(m)
 * Total: O(n*l + m*l + k)
 *
 * PART 3c: Tradeoffs
 *   Optimized for time using sorting + binary search per prefix.
 ******************************************************************************/