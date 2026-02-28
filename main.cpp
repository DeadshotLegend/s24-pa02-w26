//Shivansh Goel
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <exception>

#include "movies.h"
#include "utilities.h"

using namespace std;

static void printMovieLine(const Movie& m) {
    cout << m.name << ", " << fixed << setprecision(1) << m.rating << "\n";
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    /**************************************************************************
     * CONSTANT / DECLARATION
     **************************************************************************/
    if (argc != 2 && argc != 3) {
        // Spec doesn’t define error format; keep minimal.
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

    // Sort once by name (supports Part 1 and fast prefix searching).
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

    // Store best results (only for prefixes with >= 1 match) in input order.
    struct BestResult {
        string prefix;
        const Movie* bestMovie;
    };
    vector<BestResult> bestResults;
    bestResults.reserve(prefixes.size());

    // For lower_bound, compare Movie.name vs a string key.
    auto cmpMovieNameToKey = [](const Movie& m, const string& key) {
        return m.name < key;
    };

    for (const string& prefix : prefixes) {
        // Find first position where name >= prefix
        auto it = lower_bound(movies.begin(), movies.end(), prefix, cmpMovieNameToKey);

        vector<const Movie*> matches;
        // Scan forward only while titles start with prefix
        for (auto jt = it; jt != movies.end(); ++jt) {
            if (!startsWith(jt->name, prefix)) break;
            matches.push_back(&(*jt));
        }

        if (matches.empty()) {
            cout << "No movies found with prefix " << prefix << "\n";
            continue; // NOTE: no blank line after "No movies found..."
        }

        // Sort matches by rating desc, name asc
        sort(matches.begin(), matches.end(), betterForPrefixOutput);

        // Print match block
        for (const Movie* mp : matches) {
            printMovieLine(*mp);
        }
        cout << "\n";  // blank line after successful match block

        // Record best (first after sorting)
        bestResults.push_back({prefix, matches.front()});
    }

    // Print best movies at the very end
    for (const auto& br : bestResults) {
        cout << "Best movie with prefix " << br.prefix
             << " is: " << br.bestMovie->name
             << " with rating " << fixed << setprecision(1) << br.bestMovie->rating
             << "\n";
    }

    return 0;
}

