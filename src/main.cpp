#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include "functions.h"
#include "kmeans.h"
#include "algorithms.h"
#include "evaluations.h"

struct ProgramConfiguration {    //for argument parsing   //we create the default options in case someone doesnt pass an argument on the command line
    std::string hp_path="data/hpatches-sequences-release/hpatches-sequences-release";
    std::string mir_path="data/mirflickr25k/mirflickr";
    std::string vocabulary_path = "vocabulary_30.txt";
    std::string split_path="split.txt";
    std::string output_path="output.txt";

    std::string dataset_set="validation";
    std::string algorithm;

    int vocab = 256;
    int S = 500;
    int D = 1000;
    int seed = 214;

    int k = 4;  //lsh
    int L = 5;

    int kproj = 10;    //hypercube

    double w = 0.05;   //shared 
    int M = 0;
    int probes = 10;

    int kclusters = 0;  //ivfflat / ivfpq
    int nprobe = 5;
    int nbits = 8;
};

ProgramConfiguration parse_arguments(int argc, char* argv[]) {   //PARSER

    ProgramConfiguration config;

    for (int i = 1; i < argc; i++) {   //for every argument

        std::string argument = argv[i]; //parse the i-th argument

        if (argument == "-hp") { 
            config.hp_path = argv[++i];
        }
        else if (argument == "-mir") {
            config.mir_path = argv[++i];
        }
        else if (argument == "-split") {
            config.split_path = argv[++i];
        }
        else if (argument == "-vocab") {
            config.vocab = std::stoi(argv[++i]);
        }
        else if (argument == "-S") {
            config.S = std::stoi(argv[++i]);
        }
        else if (argument == "-D") {
            config.D = std::stoi(argv[++i]);
        }
        else if (argument == "-set") {
            config.dataset_set = argv[++i];
        }
        else if (argument == "-o") {
            config.output_path = argv[++i];
        }
        else if (argument == "-seed") {
            config.seed = std::stoi(argv[++i]);
        }
        else if (argument == "-exact") {
            config.algorithm = "exact";
        }
        else if (argument == "-lsh") {
            config.algorithm = "lsh";
        }
        else if (argument == "-hypercube") {
            config.algorithm = "hypercube";
        }
        else if (argument == "-ivfflat") {
            config.algorithm = "ivfflat";
        }
        else if (argument == "-ivfpq") {
            config.algorithm = "ivfpq";
        }
        else if (argument == "-k") {
            config.k = std::stoi(argv[++i]);
        }
        else if (argument == "-L") {
            config.L = std::stoi(argv[++i]);
        }
        else if (argument == "-kproj") {
            config.kproj = std::stoi(argv[++i]);
        }
        else if (argument == "-w") {
            config.w = std::stod(argv[++i]);
        }
        else if (argument == "-M") {
            config.M = std::stoi(argv[++i]);
        }
        else if (argument == "-probes") {
            config.probes = std::stoi(argv[++i]);
        }
        else if (argument == "-kclusters") {
            config.kclusters = std::stoi(argv[++i]);
        }
        else if (argument == "-nprobe") {
            config.nprobe = std::stoi(argv[++i]);
        }
        else if (argument == "-nbits") {
            config.nbits = std::stoi(argv[++i]);
        }
    }

    return config;
}


int main(int argc, char* argv[]) {
    
    ProgramConfiguration config = parse_arguments(argc, argv);   //parse command line arguments
    std::mt19937 generator(config.seed);   //create the random seed we are going to create throuout main. (214)
    DatasetSplit dataset_split = read_dataset_split(config.split_path);  
    std::vector<std::string> evaluation_sequences;

    if (config.dataset_set == "validation") {
        evaluation_sequences = dataset_split.validation;   //the validation part is a vector of strings so we put it on evaluation_sequences
    }   
    else {
        evaluation_sequences = dataset_split.test;
    }

    std::cout << "training sequences: " << dataset_split.training.size() << "\n";
    std::cout << "validation sequences: " << dataset_split.validation.size() << "\n";
    std::cout << "test sequences: " << dataset_split.test.size() << "\n";

    // std::mt19937 generator(config.seed);  //instead of hardcoding 214. (though its quite the same)      //we dont need this part as we are using the prerecoreder vocabulary_30.txt file 
    // std::vector<std::vector<float>> master_vector;
    // collect_training_descriptors(dataset_split.training,master_vector, config.S);
    // std::cout << "total SIFT descriptors: " << master_vector.size() << "\n";

    // if (!master_vector.empty()) {  //safe handling master_vector[0]
    //     std::cout << "Descriptor dimensions: " << master_vector[0].size() << "\n";
    // }

    std::filesystem::path vocabulary_file = config.vocabulary_path;   //this basically converts the string to std::filesystem::path format
    std::vector<std::vector<float>> centers = load_centers(vocabulary_file);

    std::cout << "vocabulary centers: " << centers.size() << "\n";
    std::cout << "center dimensions: " << centers[0].size() << "\n";

    std::vector<ImageData> database;    //create the databases
    std::vector<QueryData> queries;
    for (int i = 0; i < evaluation_sequences.size(); i++) {   //for each one of our testing images

        std::string sequence = evaluation_sequences[i];
        std::string query_path = config.hp_path + "/" + sequence + "/1.ppm";   //create the query using image 1

        cv::Mat query_descriptors = SIFT_extractor(query_path);

        if (query_descriptors.empty()) {
            std::cerr << "Error: No descriptors found in " << query_path << "\n";
            return 1;
        }

        QueryData query_data;

        query_data.query.image_id = sequence + "/1.ppm";
        query_data.query.histogram = create_histogram(centers, query_descriptors);

        for (int j = 2; j <= 6; j++) {  //add images 2-6 to the database 

            std::string image_path = config.hp_path + "/" + sequence + "/" + std::to_string(j) + ".ppm";

            cv::Mat descriptors = SIFT_extractor(image_path);

            if (descriptors.empty()) {
                std::cerr << "error: no descriptors found in " << image_path << "\n";
                return 1;
            }
            ImageData image_data;

            image_data.image_id = sequence + "/" + std::to_string(j) + ".ppm";
            image_data.histogram = create_histogram(centers, descriptors);

            database.push_back(image_data);
            query_data.relevant_ids.push_back(image_data.image_id);//these five images are relevant to the current query
        }

        queries.push_back(query_data);
    }   


    std::cout << "Evaluation sequences: " << evaluation_sequences.size() << "\n";
    std::cout << "Queries: " << queries.size() << "\n";
    std::cout << "HPatches database images: " << database.size() << "\n";

    if (!queries.empty()) {
        std::cout << "Relevant images for first query: " << queries[0].relevant_ids.size() << "\n";
    }

    if (!database.empty()) {
        std::cout << "Histogram dimensions: " << database[0].histogram.size() << "\n";
    }    

    int hpatches_images = database.size();  //keep track of our hpatches images used.                    
    int mirflickr_added = 0;                //keep track of our mirflickr images    
    
    for (int i = 1; i <= 25000 && mirflickr_added < config.D; i++) {   //for 25k images or for lower if requested.

        std::string image_path = config.mir_path + "/im" + std::to_string(i) + ".jpg"; 

        cv::Mat descriptors = SIFT_extractor(image_path);

        if (descriptors.empty()) {
            continue;
        }

        ImageData image_data;

        image_data.image_id = "im" + std::to_string(i) + ".jpg";   //creating the i-th image id (its path)
        image_data.histogram = create_histogram(centers, descriptors);

        database.push_back(image_data);

        mirflickr_added++;
    }

    std::vector<ImageData> empty_database;
    LSHIndex lsh_index(empty_database);
    IVFFlatIndex ivf_index;
    IVFPQIndex pq_index;
    HypercubeIndex hypercube_index;

    int centers_count = config.kclusters;   //calucalate the ivf centers
    if (centers_count == 0) {
        centers_count = ceil(sqrt(database.size()));
    }
    std::cout << "evaluation sequences: " << evaluation_sequences.size() << "\n";    //keep track of results
    std::cout << "queries: " << queries.size() << "\n";  //23
    std::cout << "hpatches database images: " << hpatches_images << "\n";  //should be 115 nontheless   (23 seqs*5 images each)
    std::cout << "MIRFlickr images added: " << mirflickr_added << "\n";  //D
    std::cout << "Total database images: " << database.size() << "\n";  //115 + D
    if (config.algorithm == "lsh"){
        lsh_index = LSHIndex(database, config.k, config.L, config.w, config.seed);          //build the indexes based on the requested algoritmh 
    }else if (config.algorithm == "hypercube"){
        hypercube_index = build_hypercube_index(database, config.kproj, config.w, generator);
    }else if (config.algorithm == "ivfflat"){
        ivf_index = build_ivff_index(database, centers_count);

    }else if (config.algorithm == "ivfpq"){
        pq_index = build_ivfpq_index(database, centers_count, config.M, config.nbits);
}


    EvaluationResults results = evaluate_algorithm( queries, database, config.algorithm, lsh_index, ivf_index, pq_index ,hypercube_index ,config.nprobe,config.M , config.probes, generator);
   
    std::cout << "\nEvaluation results\n";   //printing evaluation results
    std::cout << "Algorithm: " << config.algorithm << "\n";
    std::cout << "Recall@5: " << results.mean_recall5 << "\n";
    std::cout << "Recall@10: " << results.mean_recall10 << "\n";
    std::cout << "mAP@10: " << results.map10 << "\n";
    std::cout << "Average query time: " << results.average_query_time << " ms\n";
    std::cout << "Queries with less than 10 results: " << results.queries_with_less_than_10 << "\n";



    std::ofstream output_file(config.output_path);     //write results to output file

    if (!output_file.is_open()) {
        std::cerr << "Could not open output file\n";
        return 1;
    }

    output_file << config.algorithm << "\n";

    for (int i = 0; i < results.query_results.size(); i++) {

        std::string query_id = results.query_results[i].query_id;

    // keep only the sequence name
        query_id = query_id.substr(0, query_id.find('/'));

        output_file << "Query: " << query_id << "\n";

    for (int j = 0; j < results.query_results[i].results.size(); j++) {

        output_file << "Rank-" << j + 1 << ": "
                    << results.query_results[i].results[j].image_id << "\n";
    }

    output_file << "Recall@5: " << results.query_results[i].recall5 << "\n";
    output_file << "Recall@10: " << results.query_results[i].recall10 << "\n";
    output_file << "AP@10: " << results.query_results[i].ap10 << "\n\n";
}

    output_file << "Mean Recall@5: " << results.mean_recall5 << "\n";
    output_file << "Mean Recall@10: " << results.mean_recall10 << "\n";
    output_file << "mAP@10: " << results.map10 << "\n";
    output_file << "Average query time: " << results.average_query_time << "\n";

    output_file.close();

    std::cout << "Results saved to: " << config.output_path << "\n";
        return 0;
    }


