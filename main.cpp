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

    /**************************************************************************
     * CONSTANT / DECLARATION
     **************************************************************************/
    if (argc != 2 && argc != 3) {
        cerr << "Usage: ./runMovies movieFilename [prefixFilename]\n";
        return 1;
    }

    const string movieFilename = argv[1];

    /**************************************************************************
     * INITIALIZATION
     **************************************************************************/
    vector<Movie> movies;
    try {
        movies = readMoviesCSV(movieFilename);
    } catch (const exception& e) {
        cerr << e.what() << "\n";
        return 1;
    }

    // Sort once by movie name (supports Part 1 and prefix searching).
    sortByName(movies);

    /**************************************************************************
     * PART 1
     **************************************************************************/
    if (argc == 2) {
        for (const Movie& m : movies) {
            printMovieLine(m);
        }
        return 0;
    }

    /**************************************************************************
     * PART 2
     **************************************************************************/
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

    // For lower_bound: compare Movie.name vs a string key.
    auto cmpMovieNameToKey = [](const Movie& m, const string& key) {
        return m.name < key;
    };

    for (const string& prefix : prefixes) {
        auto it = lower_bound(movies.begin(), movies.end(), prefix, cmpMovieNameToKey);

        // Ratings are in tenths; support 0.0 .. 10.0 => 0..100
        vector<const Movie*> buckets[101];

        const Movie* bestMovie = nullptr;
        int bestRating10 = -1;

        // Scan forward while titles match prefix (contiguous in name-sorted order)
        for (auto jt = it; jt != movies.end(); ++jt) {
            if (!startsWith(jt->name, prefix)) break;

            int r = jt->rating10;   // expected 0..100
            if (r < 0) r = 0;
            if (r > 100) r = 100;

            // preserves alphabetical order within the same rating (since scan is name-sorted)
            buckets[r].push_back(&(*jt));

            // Best movie: higher rating wins; ties -> alphabetically first (first seen)
            if (r > bestRating10) {
                bestRating10 = r;
                bestMovie = &(*jt);
            }
        }

        if (bestMovie == nullptr) {
            cout << "No movies found with prefix " << prefix << "\n";
            continue;
        }

        // Print in decreasing rating; within bucket already alphabetical
        for (int r = 100; r >= 0; --r) {
            for (const Movie* mp : buckets[r]) {
                cout << mp->name << ", " << mp->rating_out << "\n";
            }
        }
        cout << "\n";

        bestResults.push_back({prefix, bestMovie});
    }

    // Print best movies at the very end
    for (const auto& br : bestResults) {
       cout << "Best movie with prefix " << br.prefix
     << " is: " << escapeQuotesForOutput(br.bestMovie->name)
     << " with rating " << br.bestMovie->rating_out
     << "\n"; 
    }

    return 0;
}

/******************************************************************************
 * PART 3a: Time Complexity (Worst Case) — Part 2 ONLY
 *
 * Let:
 *   n = number of movies
 *   m = number of prefixes
 *   k = max number of movies matching a prefix
 *   l = max length of a movie name
 *
 * Movies are pre-sorted by name once (outside Part 2).
 *
 * For each prefix:
 *   - lower_bound: O(log n * l)
 *   - scan matches: O(k * l)
 *   - bucket print: O(k)
 *
 * Worst-case Part 2 time:
 *   O( m * (log n + k) * l )
 *
 * PART 3b: Space Complexity (Worst Case) — Part 2 ONLY
 *   - movies storage: O(n*l)
 *   - prefixes storage: O(m*l)
 *   - per prefix buckets store k pointers total: O(k)
 *   - bestResults: O(m)
 *
 * Total: O(n*l + m*l + k)
 *
 * PART 3c: Tradeoffs
 *   Optimized for time by avoiding per-prefix sort (O(k log k)).
 *   We use 101 small buckets (0..10.0 in tenths) to output in required order in O(k).
 ******************************************************************************/