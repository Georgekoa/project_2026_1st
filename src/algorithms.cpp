#include <iostream>
#include <fstream>  //needed for txt util
#include <sstream>
#include <string>
#include <filesystem> //needed for constructing the file name paths 
#include <random>
#include <vector>
#include <algorithm>
#include <limits>
#include <algorithms.h>
#include <functions.h>
#include <kmeans.h>


std::vector<Searchresult> exact_search(std::vector<float>& histogram, std::vector<ImageData>& database, int total_results){
    std::vector<Searchresult> results; //a vector to store id and disatance in each cell.
    
   
    
    for(int i = 0 ; i < database.size(); i ++){   //database.size() corresponds to the total number of images we are going to proccess
        Searchresult curr_image;
        curr_image.distance = calculate_distance(histogram, database[i].histogram, 256);
        curr_image.image_id = database[i].image_id;
        results.push_back(curr_image);
        
    }
    
    std::vector<Searchresult> leaderboard;
    std::sort(results.begin(), results.end(), compare_distance);
    for(int j = 0 ; j < total_results; j++){
        leaderboard.push_back(results[j]);
    }   
    return leaderboard;
}


bool compare_distance(Searchresult& a, Searchresult& b){   //compare the diatance of 2 searchresult objects.
    return a.distance < b.distance;
}

LSHIndex::LSHIndex(std::vector<ImageData>& database, int k_val, int L_val, double w_val, int seed)
    : k(k_val), L(L_val), w(w_val), db_ptr(&database) {

    if (database.empty()) {
        dimensions = 0;
        table_size = 1;
        return;
    }

    dimensions = database[0].histogram.size();
    table_size = std::max(1, static_cast<int>(database.size() / 8));

    std::mt19937 gen(seed);
    std::normal_distribution<float> normal_dist(0.0f, 1.0f);
    std::uniform_real_distribution<double> uniform_t(0.0, w);
    std::uniform_int_distribution<uint32_t> uniform_r(1, 10000);

    v.resize(L, std::vector<std::vector<float>>(k, std::vector<float>(dimensions)));
    t.resize(L, std::vector<double>(k));

    for (int l = 0; l < L; l++) {
        for (int i = 0; i < k; i++) {
            for (int d = 0; d < dimensions; d++) {
                v[l][i][d] = normal_dist(gen);
            }
            t[l][i] = uniform_t(gen);
        }
    }

    r.resize(k);
    for (int i = 0; i < k; i++) {
        r[i] = uniform_r(gen);
    }

    tables.resize(L, std::vector<std::vector<BucketEntry>>(table_size));

    for (size_t idx = 0; idx < database.size(); idx++) {
        if (database[idx].histogram.empty()) continue;

        for (int l = 0; l < L; l++) {
            uint32_t id = compute_id(database[idx].histogram, l);
            int bucket = id % table_size;
            tables[l][bucket].push_back({static_cast<int>(idx), id});
        }
    }
}

uint32_t LSHIndex::compute_id(const std::vector<float>& vec, int table_idx) const {
    const uint64_t M = 4294967291ULL; // 2^32 - 5
    int64_t sum = 0;

    for (int i = 0; i < k; i++) {
        double dot = 0.0;
        for (int d = 0; d < dimensions; d++) {
            dot += static_cast<double>(vec[d]) * v[table_idx][i][d];
        }

        int64_t h_i = static_cast<int64_t>(std::floor((dot + t[table_idx][i]) / w));

        int64_t term = (static_cast<int64_t>(r[i]) * h_i) % static_cast<int64_t>(M);
        if (term < 0) term += M;

        sum = (sum + term) % static_cast<int64_t>(M);
    }

    return static_cast<uint32_t>(sum);
}

std::vector<Searchresult> LSHIndex::query(std::vector<float>& query_histogram, int total_results) {
    std::vector<Searchresult> results;
    if (db_ptr == nullptr || db_ptr->empty()) {
        return results;
    }

    std::vector<ImageData>& database = *db_ptr;
    std::vector<bool> visited(database.size(), false);

    for (int l = 0; l < L; l++) {
        uint32_t q_id = compute_id(query_histogram, l);
        int bucket = q_id % table_size;

        for (const auto& entry : tables[l][bucket]) {
            int img_idx = entry.image_index;
            if (!visited[img_idx]) {
                visited[img_idx] = true;

                Searchresult candidate;
                candidate.image_id = database[img_idx].image_id;
                candidate.distance = calculate_distance(query_histogram, database[img_idx].histogram, dimensions);
                results.push_back(candidate);
            }
        }
    }

    std::sort(results.begin(), results.end(), compare_distance);

    int limit = std::min(total_results, static_cast<int>(results.size()));
    std::vector<Searchresult> leaderboard(results.begin(), results.begin() + limit);

    return leaderboard;
}