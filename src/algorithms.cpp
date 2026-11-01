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


std::vector<Searchresult> exact_search(ImageData& query, std::vector<ImageData>& database, int total_results){
    std::vector<Searchresult> results; //a vector to store id and disatance in each cell.
    
   
    
    for(int i = 0 ; i < database.size(); i ++){   //database.size() corresponds to the total number of images we are going to proccess
        if (database[i].image_id == query.image_id) {
            continue;
        }
        Searchresult curr_image{database[i].image_id, calculate_distance(query.histogram, database[i].histogram, 256)}; //id and dtsnace fields
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

std::vector<std::vector<float>> create_histogram_database(std::vector<ImageData>& database){
    std::vector<std::vector<float>> histogram_database;
    
    for(int i = 0 ; i < database.size();i++){
        histogram_database.push_back(database[i].histogram);    //ImageData already has histograms, so we copy them inside another vector to help our new kmeans
    }
    return histogram_database;
}

IVFFlatIndex build_ivff_index(std::vector<ImageData>& database , int centers_count) {
    
    IVFFlatIndex index;

    std::vector<std::vector<float>> histogram_database = create_histogram_database(database);
    index.centers = kmeans(histogram_database, 256, 5, centers_count);
    index.inverted_lists.resize(centers_count);  //create an empty vector to accomodate every center

    std::vector<int> assignments = assign_centers(histogram_database, index.centers); //assign every histogram to its nearest center..... again for clarity. assign centers returns a vector with size # of histograms

    for (int i = 0; i < assignments.size(); i++) {   //distribute database images into their lists 
        int center_index = assignments[i];
        index.inverted_lists[center_index].push_back(i);   //we just create a vector with integers (their index)
    }

    return index;
}

std::vector<int> find_nearest_centers(std::vector<float>& query, IVFFlatIndex& index, int nprobe) {  //query = image histogram, probe is the centers we want to return.   //function to return the nprobes nearest centers of our query image
    std::vector<int> nearest_centers;
    std::vector<bool> selected(index.centers.size(), false);  //track which centers are used

    if (nprobe > index.centers.size()) {
        nprobe = index.centers.size();   //in case the centers are less that our selected nprobe, make nprobe the maximum.
    }

    for (int i = 0 ; i < nprobe ; i++) {   \

        float min_distance = std::numeric_limits<float>::max();
        int nearest_center = -1;

        
        for (int j = 0; j < index.centers.size(); j++) {   //find the closest center that has not already been selected

            if (selected[j] == false) {

                float distance = calculate_distance(query, index.centers[j], 256);

                if (distance < min_distance) {
                    min_distance = distance;
                    nearest_center = j;
                }
            }
        }
        
        if (nearest_center != -1) {   //save the center index and mark it as selected
            nearest_centers.push_back(nearest_center);
            selected[nearest_center] = true;
        }
    }
    return nearest_centers;
}


std::vector<Searchresult> ivfflat_search(std::vector<float>& query, std::string& query_image_id ,std::vector<ImageData>& database, IVFFlatIndex& index, int nprobe, int total_results){
    std::vector<Searchresult> results;
    
    std::vector<int> nearest_centers = find_nearest_centers(query, index, nprobe);  //find the nprobes nearest centers
    for (int i = 0; i < nearest_centers.size(); i++) {    //search the inverted lists of those centers

        int center_index = nearest_centers[i];        
        std::vector<int> candidates = index.inverted_lists[center_index];   //get the database indices belonging to this center(index.inverted_lists[center_index] refers to the assignment vector, so we have which images belong to our current center)

        for (int j = 0; j < candidates.size(); j++) {   //calculate the distance to every candidate

            int image_index = candidates[j];
            
        if (database[image_index].image_id == query_image_id) {   //exclude the query image from the results
            continue;
        }
            Searchresult result;
            result.image_id = database[image_index].image_id;
            result.distance = calculate_distance(query, database[image_index].histogram,256 );
            results.push_back(result);
        }
    }

    
    std::sort(results.begin(), results.end(), compare_distance);  //sort results by distance, smallest (better) first

    
    if (results.size() > total_results) {  //keep only the requested number of results
        results.resize(total_results);
    }

    return results;
}

IVFPQIndex build_ivfpq_index(std::vector<ImageData>& database, int centers_count, int M, int nbits){
    IVFPQIndex index;
    int dimensions = 256;
    int subvector_size = dimensions / M;
    int pq_centers_count = pow(2, nbits);

    std::vector<std::vector<float>> histogram_database = create_histogram_database(database);  //extract all database histograms

    index.centers = kmeans(histogram_database, dimensions,5 , centers_count);  //building the ivf index
    index.inverted_lists.resize(centers_count);  //creating the size beforehand so we can access each cell without segfaults
    
    std::vector<int> assignments = assign_centers(histogram_database, index.centers); //assign every image to its nearest ivf center (same as before)

    for (int i = 0; i < assignments.size(); i++) {
        int center_index = assignments[i];
        index.inverted_lists[center_index].push_back(i);
    }

    std::vector<std::vector<float>> residuals;    //calculate residuals (difference from the center)
    for (int i = 0; i < histogram_database.size(); i++) {

        int center_index = assignments[i];
        std::vector<float> residual;

        for (int j = 0; j < dimensions; j++) {
            float value =
                histogram_database[i][j] - index.centers[center_index][j];

            residual.push_back(value);
        }
        residuals.push_back(residual);
    }

    index.codebooks.resize(M);   //train the codebooks

    for (int m = 0; m < M; m++) {

        std::vector<std::vector<float>> subvector_database;

        for (int i = 0; i < residuals.size(); i++) {      //extract the corresponding subvector from every residual

            std::vector<float> subvector;
            for (int j = 0; j < subvector_size; j++) {
                int position = m * subvector_size + j;
                subvector.push_back(residuals[i][position]);
            }
            subvector_database.push_back(subvector);
        }

        index.codebooks[m] = kmeans(subvector_database, subvector_size, 5, pq_centers_count);   //train one codebook for this subvector position
    }

    

    index.compressed_codes.resize(database.size());   //compression
    for (int i = 0; i < residuals.size(); i++) {
        
        for (int m = 0; m < M; m++) {    //every image receives M integer codes

            std::vector<float> subvector;
            for (int j = 0; j < subvector_size; j++) {

                int position = m * subvector_size + j;

                subvector.push_back(residuals[i][position]);
            }
            int code = find_nearest_center(subvector, index.codebooks[m]);    //find the closest codebook center
            index.compressed_codes[i].push_back(code);
        }
    }

    return index;
}

std::vector<Searchresult> ivfpq_search(std::vector<float>& query, std::string& query_image_id, std::vector<ImageData>& database, IVFPQIndex& index, int nprobe, int total_results){
    std::vector<Searchresult> results;

    int M = index.codebooks.size(); //get the number of subvectors
    int subvector_size = 256 / M; //calculate the dimensions of each subvector
    int pq_centers_count = index.codebooks[0].size(); //get the number of centers in each PQ codebook

    std::vector<int> nearest_centers;   //find the nprobe nearest IVF centers
    std::vector<bool> selected(index.centers.size(), false);

    if (nprobe > index.centers.size()) {  //same check as before
        nprobe = index.centers.size();
    }

    for (int i = 0; i < nprobe; i++) {
        float min_distance = std::numeric_limits<float>::max();
        int nearest_center = -1;

        for (int j = 0; j < index.centers.size(); j++) {
            if (selected[j] == false) {
                float distance = calculate_distance(query, index.centers[j], 256);

                if (distance < min_distance) {
                    min_distance = distance;
                    nearest_center = j;
                }
            }
        }

        if (nearest_center != -1) {
            nearest_centers.push_back(nearest_center);
            selected[nearest_center] = true;
        }
    }

    for (int i = 0; i < nearest_centers.size(); i++) {    //search the selected inverted lists
        int center_index = nearest_centers[i];
        std::vector<float> query_residual;   //calculate the query residual for this ivf center

        for (int j = 0; j < 256; j++) {
            float value = query[j] - index.centers[center_index][j];
            query_residual.push_back(value);
        }

        std::vector<std::vector<float>> lookup_table;   //build the distance lookup tables

        for (int m = 0; m < M; m++) {
            std::vector<float> query_subvector;

            for (int j = 0; j < subvector_size; j++) {
                int position = m * subvector_size + j;
                query_subvector.push_back(query_residual[position]);
            }

            std::vector<float> distances;

            for (int c = 0; c < pq_centers_count; c++) {
                float distance = calculate_distance(query_subvector, index.codebooks[m][c], subvector_size);
                distances.push_back(distance);
            }

            lookup_table.push_back(distances);
        }

        std::vector<int> candidates = index.inverted_lists[center_index];    //retrieve the candidates from the inverted list

        for (int j = 0; j < candidates.size(); j++) {
            int image_index = candidates[j];

            
            if (database[image_index].image_id == query_image_id) {    //exclude the query image from the results (that why we included the query image id as an argument)
                continue;
            }
            float total_distance = 0;
            for (int m = 0; m < M; m++) {    //calculate the approximate distance using the pq codes
                int code = index.compressed_codes[image_index][m];
                total_distance += lookup_table[m][code];
            }

            Searchresult result;
            result.image_id = database[image_index].image_id;
            result.distance = total_distance;
            results.push_back(result);
        }
    }

    std::sort(results.begin(), results.end(), compare_distance);   //sort the results and keep only the requested number

    if (results.size() > total_results) {
        results.resize(total_results);
    }

    return results;
}

int get_hypercube_vertex(std::vector<float>& point, HypercubeIndex& index, std::mt19937& generator){
    int vertex = 0;
    std::uniform_int_distribution<int> bit_distribution(0, 1);    //either 1 or 0 with 50% each, so we assign a numbet o each mapping the first imr and store it in bit_mappings

    for (int i = 0; i < index.kproj; i++) {  //for each projection 
        double projection = 0;

        for (int j = 0; j < index.dimensions; j++) {    //calculate the dot product between the image histogram and the random projection vector
            projection += point[j] * index.projections[i][j];
        }

        long long bucket = static_cast<long long>(std::floor((projection + index.shifts[i]) / index.w));   //add the random shift and divide by w and take the floor. this gives us the hash bucket
 
        if (index.bit_mappings[i].find(bucket) == index.bit_mappings[i].end()) {  //we look up that bucket in bit_mappings
            index.bit_mappings[i][bucket] = bit_distribution(generator);   //if we haven't encountered it before we assign it a random bit (0 or 1)
        }

        int bit = index.bit_mappings[i][bucket]; //use that bit to construct the vertex number

        vertex = vertex | (bit << i);
    }

    return vertex;
}

HypercubeIndex build_hypercube_index(std::vector<ImageData>& database, int kproj, double w, std::mt19937& generator){  //building the struct index
    HypercubeIndex index;

    index.kproj = kproj;      //init the index
    index.w = w;
    if (database.empty() || kproj <= 0 || w <= 0){  //error handling
        std::cerr<< "invalid parameters\n";
        return index;
    }
    index.dimensions = database[0].histogram.size();  

    std::normal_distribution<float> normal_distribution(0.0, 1.0);        //define the random distributions
    std::uniform_real_distribution<double> shift_distribution(0.0, w);

    index.projections.resize(kproj);  //preparing the storage
    index.shifts.resize(kproj);
    index.bit_mappings.resize(kproj);
    index.vertices.resize(1 << kproj);

    for (int i = 0; i < kproj; i++) {   //generating the projections
        index.projections[i].resize(index.dimensions);

        for (int j = 0; j < index.dimensions; j++) {
            index.projections[i][j] = normal_distribution(generator);
        }

        index.shifts[i] = shift_distribution(generator); 
    }

    for (int i = 0; i < database.size(); i++) {    //assign database images to vertices
        int vertex = get_hypercube_vertex(database[i].histogram, index, generator);

        index.vertices[vertex].push_back(i);
    }

    return index;
}

std::vector<Searchresult> hypercube_search(std::vector<float>& query, std::string& query_image_id, std::vector<ImageData>& database, HypercubeIndex& index, int M, int probes, int total_results, std::mt19937& generator){  //acxtual search function
    std::vector<Searchresult> results;

    if (database.empty() || index.vertices.empty() || query.size() != index.dimensions) { //check if the index and query are valid
        return results;
    }

    int query_vertex = get_hypercube_vertex(query, index, generator);   //find the vertex that the query belongs to

    std::vector<std::pair<int, int>> vertex_distances;   //store the hamming distance and vertex id for every vertex

    for (int i = 0; i < index.vertices.size(); i++) {   //calculate the hamming distance between the query vertex and every other vertex
        int difference = query_vertex ^ i;
        int hamming_distance = 0;

        while (difference > 0) {   //count the number of different bits between the two vertices
            hamming_distance += difference & 1;
            difference >>= 1;
        }

        vertex_distances.push_back({hamming_distance, i});
    }

    std::sort(vertex_distances.begin(), vertex_distances.end());   //sort vertices by hamming distance so we visit the closest ones first

    int examined = 0;
    
    for (int i = 0; i < probes && i < vertex_distances.size(); i++) {    //visit at most probes vertices
        int vertex = vertex_distances[i].second;

        for (int j = 0; j < index.vertices[vertex].size(); j++) {    //examine the database images stored in the current vertex
            int image_index = index.vertices[vertex][j];

            if (database[image_index].image_id == query_image_id) {    //skip the query image if it is also part of the database
                continue;
            }
            if (examined >= M) {     //stop if we have examined the maximum number of candidates
                break;
            }
            float distance = calculate_distance(query, database[image_index].histogram, index.dimensions); //calculate the distance between the query and the candidate image
            
            results.push_back({database[image_index].image_id, distance}); //store the candidate image id and its distance
            examined++;
        }

        if (examined >= M) {   //stop visiting vertices once we reach the candidate limit
            break;
        }
    }

    std::cout<< "examined " <<examined << "\n"; //see the number o candidates out of which the top 10 will appear
    std::sort(results.begin(), results.end(), compare_distance);  //sort by distance,we use the same compare distance as before
    if (results.size() > total_results) {    //keep only the requested number of results
        results.resize(total_results);
    }

    return results;
}