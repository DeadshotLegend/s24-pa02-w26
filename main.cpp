// Shivansh Goel - CS24 PA02

#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "movies.h"
#include "utilities.h"

using namespace std;

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

    // Part 1 requires alphabetical order by the *printed* movie name.
    sortByName(movies);

    /**************************************************************************
     * PART 1
     **************************************************************************/
    if (argc == 2) {
        for (const Movie& m : movies) {
            cout << m.print_name << ", " << m.rating_out << "\n";
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

    // lower_bound comparator: compare Movie.print_name to prefix key
    auto cmpMovieToKey = [](const Movie& m, const string& key) {
        return m.print_name < key;
    };

    bool printedAnyMatchBlock = false;

    for (const string& prefix : prefixes) {
        auto it = lower_bound(movies.begin(), movies.end(), prefix, cmpMovieToKey);

        vector<const Movie*> matches;
        matches.reserve(64);

        for (auto jt = it; jt != movies.end(); ++jt) {
            if (!startsWith(jt->print_name, prefix)) break;
            matches.push_back(&(*jt));
        }

        if (matches.empty()) {
            // IMPORTANT: exact wording, and NO blank line after this message.
            cout << "No movies found with prefix " << prefix << "\n";
            continue;
        }

        // Sort matches by rating desc, then name asc (using print_name)
        sort(matches.begin(), matches.end(), betterForPrefixOutput);

        // IMPORTANT formatting:
        // - One blank line BETWEEN successful match blocks
        // - No extra blank line immediately after the last block
        if (printedAnyMatchBlock) cout << "\n";
        printedAnyMatchBlock = true;

        for (const Movie* mp : matches) {
            cout << mp->print_name << ", " << mp->rating_out << "\n";
        }

        bestResults.push_back({prefix, matches.front()});
    }

    // IMPORTANT formatting:
    // - Best-movie lines are at the END
    // - The examples show one blank line before the best section (after the last match block),
    //   but NOT after any "No movies found..." lines.
    if (!bestResults.empty() && printedAnyMatchBlock) cout << "\n";

    for (const auto& br : bestResults) {
        cout << "Best movie with prefix " << br.prefix
             << " is: " << br.bestMovie->print_name
             << " with rating " << br.bestMovie->rating_out
             << "\n";
    }

    return 0;
}

/******************************************************************************
 * PART 3a: Time Complexity (Worst Case)
 *
 * Data structure/algorithm used for Part 2:
 *   - Movies stored in a vector and sorted once by print_name.
 *   - For each prefix:
 *       1) lower_bound on sorted vector to find first candidate.
 *       2) scan forward while titles start with prefix, collecting up to k matches.
 *       3) sort the k matches by rating desc, then name asc.
 *
 * Let:
 *   n = number of movies
 *   m = number of prefixes
 *   k = max number of movies that match a prefix
 *   l = max length of a movie name / prefix (string comparisons)
 *
 * Worst-case time for Part 2:
 *   For each prefix:
 *     lower_bound: O(log n * l)
 *     scanning:    O(k * l)
 *     sort k:      O(k log k) comparisons, each O(l) => O(k log k * l)
 *
 * Total: O( m * (log n * l + k * l + k log k * l) )
 *      = O( m * (log n + k + k log k) * l )
 *
 * PART 3b: Space Complexity (Worst Case)
 *
 *   - Store n movies (names up to length l): O(n*l)
 *   - Store m prefixes (length up to l): O(m*l)
 *   - For a single prefix, store up to k pointers in matches: O(k)
 *   - Store up to m best results (prefix + pointer): O(m*l + m)
 *
 * Total: O(n*l + m*l + k)
 *
 * PART 3c: Time/Space Tradeoffs
 *
 * Designed primarily for low time complexity with moderate space:
 *   - Sort once to enable binary search per prefix.
 *   - Only store pointers for matches per prefix (O(k) extra).
 * Tradeoff:
 *   - Need per-prefix sorting of matches to meet required output ordering,
 *     which dominates when k is large.
 ******************************************************************************/