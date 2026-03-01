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

static void printMovieLine(const Movie& m) {
    cout << m.name << ", " << m.rating_out << "\n";
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
        // Find first position where name >= prefix
        auto it = lower_bound(movies.begin(), movies.end(), prefix, cmpMovieNameToKey);

        // Bucket movies by rating10 (0..50 for 0.0..5.0)
        vector<const Movie*> buckets[51];

        const Movie* bestMovie = nullptr;
        int bestRating10 = -1;

        // Scan forward only while titles start with prefix
        for (auto jt = it; jt != movies.end(); ++jt) {
            if (!startsWith(jt->name, prefix)) break;

            int r = jt->rating10;             // 0..50
            if (r < 0) r = 0;
            if (r > 50) r = 50;

            buckets[r].push_back(&(*jt));     // preserves alphabetical order within same rating

            // Best movie: higher rating wins; tie breaks by alphabetical (first seen in name-sorted scan)
            if (r > bestRating10) {
                bestRating10 = r;
                bestMovie = &(*jt);
            }
        }

        if (bestMovie == nullptr) {
            cout << "No movies found with prefix " << prefix << "\n";
            continue; // no blank line after "No movies found..."
        }

        // Print in decreasing rating; within bucket already alphabetical
        for (int r = 50; r >= 0; --r) {
            for (const Movie* mp : buckets[r]) {
                cout << mp->name << ", " << mp->rating_out << "\n";
            }
        }
        cout << "\n"; // blank line after successful match block

        bestResults.push_back({prefix, bestMovie});
    }

    // Print best movies at the very end
    for (const auto& br : bestResults) {
        cout << "Best movie with prefix " << br.prefix
             << " is: " << br.bestMovie->name
             << " with rating " << br.bestMovie->rating_out
             << "\n";
    }

    return 0;
}

/******************************************************************************
 * PART 3a: Time Complexity (Worst Case) — Part 2 ONLY
 *
 * Data structure/algorithm used for Part 2:
 *   - Movies stored in a vector and sorted once by name.
 *   - For each prefix:
 *       1) lower_bound to find the first candidate.
 *       2) scan forward to collect all matches (contiguous in name-sorted order).
 *       3) bucket matches by rating (0.0–5.0 in tenths => 51 buckets) and print from high to low.
 *
 * Let:
 *   n = number of movies in the dataset
 *   m = number of prefixes
 *   k = maximum number of movies that match a prefix
 *   l = maximum length of a movie name
 *
 * For each prefix:
 *   - lower_bound: O(log n * l)  (string comparisons)
 *   - scan matches: O(k * l)     (startsWith checks + comparisons)
 *   - bucketing/printing: O(k)   (each match goes into exactly one bucket)
 *
 * Worst-case time for Part 2:
 *   O( m * (log n * l + k * l + k) )
 * = O( m * (log n + k) * l )   (dominant terms)
 *
 * NOTE: Output (I/O) time can dominate in practice because many lines may be printed.
 *
 * PART 3b: Space Complexity (Worst Case) — Part 2 ONLY
 *
 *   - Movies stored: O(n * l) overall storage for titles
 *   - Prefixes stored: O(m * l)
 *   - Per prefix: buckets store up to k pointers total: O(k)
 *   - Best results: up to m pointers + prefix strings: O(m * l)
 *
 * Worst-case total: O(n*l + m*l + k)
 *
 * PART 3c: Time/Space Tradeoffs
 *
 * Goal:
 *   Prioritize low time complexity while keeping space reasonable.
 *
 * Approach:
 *   Sorting movies once enables fast prefix searches via binary search + linear scan over only
 *   the matching range. To satisfy required output order (rating desc, then name asc) efficiently,
 *   we avoid sorting k matches and instead bucket by rating (51 buckets), which runs in O(k).
 *
 * Tradeoff:
 *   We use O(k) extra pointer storage per prefix for buckets to reduce time from O(k log k)
 *   (sorting matches) down to O(k). Low time complexity was harder to achieve than low space
 *   complexity because output ordering constraints can force expensive per-prefix work unless
 *   carefully optimized.
 ******************************************************************************/