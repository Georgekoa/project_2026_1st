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
#include <evaluations.h>


float calculate_recall(std::vector<Searchresult>& results, std::vector<std::string>& relevant_ids, int mode){   //mode refers to recall@5 or recall@10
   
    if (relevant_ids.empty()) {  
        return 0.0f;
    }
   
    int limit = std::min(mode, static_cast<int>(results.size()));
    int count = 0 ;
   

    for (int i = 0; i < limit; i++) {   //either mode(5 or 10) or less if the results are less.
        if (std::find(relevant_ids.begin(), relevant_ids.end(), results[i].image_id) != relevant_ids.end()) {
            count++;
        }
    }
    return static_cast<float>(count) / relevant_ids.size();
}


float calculate_ap10(std::vector<Searchresult>& results,std::vector<std::string>& relevant_ids){
    if (relevant_ids.empty()){   //error handling for empty input
        return 0.0f;
    }
    
    int relevant_found = 0 ;
    float sum = 0.0f;
    int limit = std::min(10, static_cast<int>(results.size()));   //no mode here, only 10.
    for (int i = 0; i < limit; i++) {
        if (std::find(relevant_ids.begin(), relevant_ids.end(), results[i].image_id) != relevant_ids.end()) {
            relevant_found++;
            sum += static_cast<float>(relevant_found) / (i + 1);
        }
    }

    return sum / relevant_ids.size();
    
}

// std::vector<ImageData> relevant_imgs(std::vector<ImageData>& database, ImageData& image){
    
//     std::vector<ImageData> relevant_images;
//     std::filesystem::path image_path(image.image_id);
    
//     std::string sequence_name = image_path.parent_path().filename().string(); //parent path strips the file name, file name gets only the last portion of the path, and string turn it back into a string
    
    
//     for(int i = 0 ; i < database.size(); i ++){
        
//        if (database[i].image_id.find(sequence_name) != std::string::npos && database[i].image_id != image.image_id) {  //if the stripped sequence name is is inside a database entry and exluding the same query image
//             relevant_images.push_back(database[i]);
//         }   
//     }
//     return relevant_images;
// }



// float calculate_map10(std::vector<ImageData>& database, std::string algorithm){    //not needed for the time being
//     if(algorithm == "exact"){
//         float map = 0.0f;
//         for (int i = 0 ; i < database.size();i++){
//             std::vector<ImageData> current_relevant_imgs = relevant_imgs(database, database[i]);
//             std::vector<Searchresult> current_result = exact_search(database[i], database, 10);
//             float current_ap10 = calculate_ap10(current_result, current_relevant_imgs);
//             map += current_ap10;
//         }
//         return map / database.size();
//     }


//      return 0.0f; //for unsupported algorithm names
// }

// float calculate_query_time(std::vector<ImageData>& database, std::string algorithm){
//     double total_time = 0.0;              //not needed for the time being
//     if(algorithm=="exact"){
        
//         for(int i =0 ; i < database.size();i++){
//             auto start = std::chrono::steady_clock::now();

//             std::vector<Searchresult> results = exact_search(database[i], database, 10);

//             auto end = std::chrono::steady_clock::now();
//             total_time += std::chrono::duration<double, std::milli>(end - start).count();  //cast it to douvle so we can count in milliseconds.
//         }
//         float average = total_time/database.size();
//         return average;
//     }

//     return 0.0f; //again for unsupported names
// }

std::vector<Searchresult> search_algorithm(QueryData& query, std::vector<ImageData>& database, std::string algorithm, LSHIndex& lsh_index, IVFFlatIndex& ivf_index, IVFPQIndex& pq_index, HypercubeIndex& hypercube_index, int nprobe, int M, int probes, std::mt19937& generator) {

    if (algorithm == "exact") {
        return exact_search(query.query, database, 10);
    }
    else if (algorithm == "lsh") {
        return lsh_index.query(query.query.histogram, 10);
    }
    else if (algorithm == "hypercube") {
        return hypercube_search(query.query.histogram, query.query.image_id, database, hypercube_index, M, probes, 10, generator);
    }
    else if (algorithm == "ivfflat") {
        return ivfflat_search(query.query.histogram, query.query.image_id, database, ivf_index, nprobe, 10);
    }
    else if (algorithm == "ivfpq") {
        return ivfpq_search(query.query.histogram, query.query.image_id, database, pq_index, nprobe, 10);
    }

    std::cerr<< "unknown algorithm: " <<  algorithm;
    return {}; //for cases of unsopported algoritmh names

}


EvaluationResults evaluate_algorithm(std::vector<QueryData>& queries, std::vector<ImageData>& database, std::string algorithm, LSHIndex& lsh_index, IVFFlatIndex& ivf_index, IVFPQIndex& pq_index, HypercubeIndex& hypercube_index, int nprobe, int M, int probes, std::mt19937& generator) {

    EvaluationResults evaluation;

    evaluation.mean_recall5 = 0.0f;
    evaluation.mean_recall10 = 0.0f;
    evaluation.map10 = 0.0f;
    evaluation.average_query_time = 0.0;
    evaluation.queries_with_less_than_10 = 0;

    if (queries.empty()) {
        return evaluation;
    }

    for (int i = 0; i < queries.size(); i++) {

        QueryEvaluation current;

        current.query_id = queries[i].query.image_id;

        auto start = std::chrono::steady_clock::now();

        current.results = search_algorithm(queries[i], database, algorithm, lsh_index, ivf_index, pq_index, hypercube_index, nprobe, M, probes, generator);

        auto end = std::chrono::steady_clock::now();

        current.query_time = std::chrono::duration<double, std::milli>(end - start).count();

        current.recall5 = calculate_recall(current.results, queries[i].relevant_ids, 5);
        current.recall10 = calculate_recall(current.results, queries[i].relevant_ids, 10);
        current.ap10 = calculate_ap10(current.results, queries[i].relevant_ids);

        if (current.results.size() < 10) {
            evaluation.queries_with_less_than_10++;
        }

        evaluation.mean_recall5 += current.recall5;
        evaluation.mean_recall10 += current.recall10;
        evaluation.map10 += current.ap10;
        evaluation.average_query_time += current.query_time;

        evaluation.query_results.push_back(current);
    }

    int total_queries = queries.size();

    evaluation.mean_recall5 /= total_queries;
    evaluation.mean_recall10 /= total_queries;
    evaluation.map10 /= total_queries;
    evaluation.average_query_time /= total_queries;

    return evaluation;
}

