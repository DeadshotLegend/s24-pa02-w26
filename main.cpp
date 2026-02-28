// Shivansh Goel
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <exception>

#include "movies.h"
#include "utilities.h"

using namespace std;

static void printMovieLine(const Movie& m) {
    cout << m.name << ", " << (m.rating10 / 10) << "." << (m.rating10 % 10) << "\n";
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

    
    if (argc == 2) {
        for (const Movie& m : movies) {
            printMovieLine(m);
        }
        return 0;
    }

  
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

        vector<vector<const Movie*>> buckets(101);

        for (auto jt = it; jt != movies.end(); ++jt) {
            if (!startsWith(jt->name, prefix)) break;

            int r = jt->rating10;
            if (r < 0) r = 0;
            if (r > 100) r = 100;
            buckets[r].push_back(&(*jt));
        }

        bool any = false;

        for (int r = 100; r >= 0; --r) {
            for (const Movie* mp : buckets[r]) {
                printMovieLine(*mp);
                any = true;
            }
        }

        if (!any) {
            cout << "No movies found with prefix " << prefix << "\n";
            continue; 
        }

        cout << "\n";

        const Movie* best = nullptr;
        for (int r = 100; r >= 0; --r) {
            if (!buckets[r].empty()) {
                best = buckets[r][0];
                break;
            }
        }
        bestResults.push_back({prefix, best});
    }

    for (const auto& br : bestResults) {
        cout << "Best movie with prefix " << br.prefix
             << " is: " << br.bestMovie->name
             << " with rating " << (br.bestMovie->rating10 / 10) << "." << (br.bestMovie->rating10 % 10)
             << "\n";
    }

    return 0;
}

