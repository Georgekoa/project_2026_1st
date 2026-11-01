#ifndef EVALUATIONS_H
#define EVALUATIONS_H
#include <vector>
#include <string>
#include <random>
#include "algorithms.h"

struct QueryEvaluation {
    std::string query_id;
    std::vector<Searchresult> results;
    float recall5;
    float recall10;
    float ap10;
    double query_time;
};

struct EvaluationResults {
    float mean_recall5;
    float mean_recall10;
    float map10;
    double average_query_time;
    int queries_with_less_than_10;
    std::vector<QueryEvaluation> query_results;
};


struct QueryData {   //necessary for rebuilding the evaluation functions to accomodate all algorithms
    ImageData query;
    std::vector<std::string> relevant_ids;
};





float calculate_recall(std::vector<Searchresult>& results, std::vector<std::string>& relevant_ids, int mode);
float calculate_ap10(std::vector<Searchresult>& results, std::vector<std::string>& relevant_ids);
std::vector<ImageData> relevant_imgs(std::vector<ImageData>& database, ImageData& image);
// float calculate_map10(std::vector<ImageData>& database, std::string algorithm);
// float calculate_query_time(std::vector<ImageData>& database, std::string algorithm);
std::vector<Searchresult> search_algorithm(QueryData& query, std::vector<ImageData>& database, std::string algorithm, LSHIndex& lsh_index, IVFFlatIndex& ivf_index, IVFPQIndex& pq_index, HypercubeIndex& hypercube_index, int nprobe, int M, int probes, std::mt19937& generator);
EvaluationResults evaluate_algorithm(std::vector<QueryData>& queries, std::vector<ImageData>& database, std::string algorithm, LSHIndex& lsh_index, IVFFlatIndex& ivf_index, IVFPQIndex& pq_index, HypercubeIndex& hypercube_index, int nprobe, int M, int probes, std::mt19937& generator);

#endif