// main.cpp
// Shivansh Goel

#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "movies.h"
#include "utilities.h"

using namespace std;

static string escapeQuotesForOutput(const string& s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '"') out += "\"\"";
        else out += c;
    }
    return out;
}

static void printMovieLine(const Movie& m) {
    cout << escapeQuotesForOutput(m.name) << ", " << m.rating_out << "\n";
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
    try {
        movies = readMoviesCSV(movieFilename);
    } catch (const exception& e) {
        cerr << e.what() << "\n";
        return 1;
    }

    sortByName(movies);

    // ----------------
    // PART 1
    // ----------------
    if (argc == 2) {
        for (const Movie& m : movies) {
            printMovieLine(m);
        }
        return 0;
    }

    // ----------------
    // PART 2
    // ----------------
    const string prefixFilename = argv[2];
    vector<string> prefixes;
    try {
        prefixes = readPrefixes(prefixFilename);
    } catch (const exception& e) {
        cerr << e.what() << "\n";
        return 1;
    }

    struct BestResult {
        string prefix;
        const Movie* bestMovie;
    };
    vector<BestResult> bestResults;
    bestResults.reserve(prefixes.size());

    auto cmpMovieNameToKey = [](const Movie& m, const string& key) {
        return m.name < key;
    };

    for (const string& prefix : prefixes) {
        auto it = lower_bound(movies.begin(), movies.end(), prefix, cmpMovieNameToKey);

        vector<const Movie*> buckets[101];

        const Movie* bestMovie = nullptr;
        int bestRating10 = -1;

        for (auto jt = it; jt != movies.end(); ++jt) {
            if (!startsWith(jt->name, prefix)) break;

            int r = jt->rating10;
            if (r < 0) r = 0;
            if (r > 100) r = 100;

            buckets[r].push_back(&(*jt));

            if (r > bestRating10) {
                bestRating10 = r;
                bestMovie = &(*jt);
            }
        }

        if (bestMovie == nullptr) {
            cout << "No movies found with prefix " << prefix << "\n";
            continue;
        }

        // ✅ FIX: use printMovieLine so quotes get escaped correctly
        for (int r = 100; r >= 0; --r) {
            for (const Movie* mp : buckets[r]) {
                printMovieLine(*mp);
            }
        }
        cout << "\n";

        bestResults.push_back({prefix, bestMovie});
    }

    for (const auto& br : bestResults) {
        cout << "Best movie with prefix " << br.prefix
             << " is: " << escapeQuotesForOutput(br.bestMovie->name)
             << " with rating " << br.bestMovie->rating_out
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