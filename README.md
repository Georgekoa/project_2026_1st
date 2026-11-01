# project_2026_1st
Project of Algorithmic Problems — 2026–27

This repository contains our image retrieval project. The goal is to represent images with SIFT descriptors and a Bag-of-Visual-Words (BoVW) model, and then compare five different search methods.

The methods we implement are:

- Exact Search
- Locality-Sensitive Hashing (LSH)
- Hypercube
- IVFFlat
- IVFPQ

The main comparison is between retrieval quality and query time. We use HPatches for the queries and relevant images, and MIRFlickr-25K as a source of distractor images.

---

## 1. Setup and dependencies

We use C++17 and OpenCV. The project Makefile gets the OpenCV compiler and linker flags through `pkg-config`, so it should use the OpenCV installation available on the machine.

Check that OpenCV is available:

```
pkg-config --modversion opencv4
```

Then build the project:

```
make clean all
```

You can also use:

```
make
```

If VS Code IntelliSense does not recognize OpenCV or SIFT correctly, this does not mean that the project itself cannot compile. 

For IntelliSense, we use Bear to generate a local compilation database:

```
sudo apt install bear
cd project_2026_1st
bear -- make clean all
```

This creates `compile_commands.json`. VS Code can use it to understand the actual compiler and include paths used by the Makefile.

`compile_commands.json` is local to each machine, so it should stay in `.gitignore`. The same applies to compiled binaries, object files, build directories, downloaded datasets and crash/core files. We do not want to push large datasets or machine-specific build files to GitHub.

### OpenCV note

SIFT must be available in the installed OpenCV version. If the compiler cannot find `cv::SIFT`, first check which OpenCV installation `pkg-config` is returning:

```
pkg-config --modversion opencv4
pkg-config --cflags opencv4
pkg-config --libs opencv4
```

---

## 2. Datasets

We use two datasets.

### HPatches

HPatches contains 116 sequences, with six images in each sequence.

For every sequence:

- Image `1.ppm` is the query.
- Images `2.ppm` to `6.ppm` are the five relevant images.

The split file keeps each sequence in exactly one group:

| Split | Number of sequences |
|---|---:|
| Training | 70 |
| Validation | 23 |
| Test | 23 |

We use the training split to build the visual vocabulary. Validation is used to compare methods and select parameters. The test split is kept for the final evaluation. We should not use test results to tune parameters.

The split file is called `split.txt`. Each line contains the sequence name followed by its split:

```
i_example train
v_example validation
i_another test
```

The program reads the two values from each line and stores the sequence name in the corresponding vector.

### MIRFlickr-25K

MIRFlickr images are used as distractors. They are not used to train the BoVW vocabulary.

The images in our local folder are named in order, for example:

```
im1.jpg
im2.jpg
im3.jpg
...
```

For a run with a selected value of `D`, the program starts from `im1.jpg` and continues in filename order. If an image has no SIFT descriptors, it is skipped and the next image is tried. We stop after adding `D` valid images.

This is important: `D` is the number of valid distractors that enter the database, not just the number of files we try to open.

---

## 3. Project structure

The main files are separated by what they do:

```
project_2026_1st/
├── include/
│   ├── algorithms.h
│   ├── evaluations.h
│   ├── functions.h
│   └── kmeans.h
├── src/
│   ├── algorithms.cpp
│   ├── evaluations.cpp
│   ├── functions.cpp
│   ├── kmeans.cpp
│   └── main.cpp
├── data/
│   ├── hpatches-sequences-release/
│   └── mirflickr/
├── split.txt
├── vocabulary_30.txt
├── Makefile
└── README.md
```

The `data` directory is local and should not be pushed to the repository.

### What each source file does

- `main.cpp`: reads the command-line arguments, loads the split and vocabulary, creates the query set and database, builds the selected index, runs the evaluation and writes the output file.
- `functions.cpp`: contains the SIFT extraction, split-file reading, descriptor collection and histogram creation functions.
- `kmeans.cpp`: contains the k-means implementation and the functions used to initialize, assign and update centers. It also contains the vocabulary save/load functions.
- `algorithms.cpp`: contains Exact Search, LSH, Hypercube, IVFFlat and IVFPQ.
- `evaluations.cpp`: runs the selected search method for each query and calculates the evaluation metrics.

The matching declarations are kept in the header files under `include/`.

---

## 4. How we build the image representation

### SIFT descriptors

We first extract SIFT descriptors from an image. Each descriptor is a vector with 128 values.

For vocabulary training, we use the 70 HPatches training sequences. Each sequence has six images, so there are 420 training images in total.

We select at most `S = 500` descriptors from each image. The descriptor selection is shuffled, and the same random generator is passed between images so that we do not restart the same random sequence for every image.

The theoretical maximum number of selected descriptors is:

```
70 sequences * 6 images * 500 descriptors = 210,000 descriptors
```

In our run, we collected 206,122 descriptors, each with 128 dimensions. This is expected because some images have fewer than 500 SIFT descriptors.

### K-means and the visual vocabulary

We use k-means to group the training descriptors into `K = 256` centers. These centers form the visual vocabulary.

The k-means process is split into a few simple steps:

1. `initialize_centers` selects 256 descriptors to use as the initial centers.
2. `assign_centers` finds the nearest center for every descriptor.
3. `update_centers` recalculates each center using the average of the descriptors assigned to it.
4. `kmeans` repeats the assignment and update steps until it reaches the iteration limit or the assignments stop changing.

We already have a vocabulary file called `vocabulary_30.txt`. We checked that it contains 256 centers, each with 128 values. The current evaluation program loads this file instead of training the vocabulary again every time we run an experiment.

The vocabulary must come from the training split only. MIRFlickr images and validation/test images must not be used to train it.

### Image histograms

Once the vocabulary is available, we represent every image with a histogram:

1. Extract the image's SIFT descriptors.
2. Find the nearest vocabulary center for each descriptor.
3. Count how many descriptors were assigned to each center.
4. Normalize the histogram by the total number of descriptors.

The result is a 256-dimensional vector for every image. These histogram vectors are what the search algorithms compare.

---

## 5. Building the query set and database

We build the evaluation set from either validation or test, depending on `-set`.

For every selected HPatches sequence:

- We extract descriptors from image `1.ppm` and create one query histogram.
- We extract descriptors from images `2.ppm` to `6.ppm`.
- Those five images are added to the database.
- Their IDs are stored as the relevant IDs for that query.

The query image itself is not added to the database.

After HPatches has been loaded, we add `D` valid MIRFlickr images to the same database.

For 23 validation or test sequences, the database size is:

```
23 sequences * 5 HPatches database images = 115 HPatches images
115 + D MIRFlickr images = total database size
```

For example:

| D | HPatches images | MIRFlickr images | Total database |
|---:|---:|---:|---:|
| 1,000 | 115 | 1,000 | 1,115 |
| 5,000 | 115 | 5,000 | 5,115 |
| 10,000 | 115 | 10,000 | 10,115 |
| 25,000 | 115 | 25,000 | 25,115 |

Every query has exactly five relevant images. MIRFlickr images are distractors, so they are never considered relevant.

---

## 6. Search methods

All methods work with the same 256-dimensional image histograms. The difference is how they find candidates from the database.

### Exact Search

Exact Search compares the query against every database histogram and ranks all database images by distance. It is our baseline because it checks the whole database rather than selecting only some candidates.

### Locality-Sensitive Hashing (LSH)

LSH uses random projections and hash functions to place similar vectors into the same buckets.

We build `L` hash tables, with `k` hash functions used for each table. At query time, we check the matching buckets, collect the candidate images and rank them by their actual histogram distance.

The main parameters are:

- `k`: number of hash functions per table.
- `L`: number of hash tables.
- `w`: hash window size.

### Hypercube

Hypercube maps each database vector to a vertex in a binary hypercube. At query time, we find the query vertex and visit nearby vertices in order of their Hamming distance.

We do not search the whole database. We stop when we reach the candidate limit `M` or the vertex limit `probes`.

The main parameters are:

- `kproj`: number of projections / bits used for the vertex.
- `w`: hash window size.
- `M`: maximum number of candidate images examined.
- `probes`: maximum number of vertices examined.

A run may examine fewer than `M` candidates. `M` is a maximum, not a target. If the selected vertices do not contain enough images, the search returns fewer candidates.

### IVFFlat

IVFFlat uses k-means to split the database histograms into clusters. Each cluster has an inverted list containing the database images assigned to it.

At query time, we find the nearest cluster centers and search only the selected inverted lists. The distances to the candidate images are calculated using their full histograms.

The main parameters are:

- `kclusters`: number of clusters. If not specified, our program uses `ceil(sqrt(n))`, where `n` is the database size.
- `nprobe`: number of nearest clusters to search.

This k-means is separate from the k-means used to create the BoVW vocabulary. It works on the database image histograms, not on the original SIFT descriptors.

### IVFPQ

IVFPQ also uses inverted-file clustering, but it compresses the database histograms with Product Quantization.

Our histogram has 256 dimensions. With `M = 16`, it is divided into 16 subvectors, each with 16 dimensions. Each subvector has its own codebook.

With `nbits = 8`, each subvector codebook contains `2^8 = 256` centers. Each database image is represented by one code per subvector.

At query time, the method searches the selected inverted lists and uses lookup tables to estimate distances from the compressed codes.

The main parameters are:

- `kclusters`: number of inverted-file clusters. The default is `ceil(sqrt(n))`.
- `nprobe`: number of clusters to search.
- `M`: number of subvectors.
- `nbits`: number of bits per subvector code. The default value of 8 gives 256 codebook centers.

The histogram dimension must be divisible by `M`. With our current setup, 256 / 16 = 16 dimensions per subvector.

---

## 7. Building and running the program

Build the project:

```bash
make clean all
```

The executable is called `search`.

The program accepts the dataset paths, vocabulary settings, selected split, output filename, random seed and algorithm parameters as command-line arguments.

### General command format

```bash
./search -hp <hpatches_path> -mir <mirflickr_path> \
-split split.txt -vocab 256 -S 500 -D 1000 \
-set validation -o output.txt -seed 214 -exact
```

The paths can be omitted if the default paths in `ProgramConfiguration` match the local folder layout.

### Main arguments

| Argument | Meaning |
|---|---|
| `-hp` | HPatches dataset directory |
| `-mir` | MIRFlickr image directory |
| `-split` | Path to `split.txt` |
| `-vocab` | Vocabulary size (`256`) |
| `-S` | Maximum SIFT descriptors selected per training image (`500`) |
| `-D` | Number of valid MIRFlickr distractors |
| `-set` | `validation` or `test` |
| `-o` | Output filename |
| `-seed` | Random seed |

### Select an algorithm

Exactly one algorithm flag should be supplied:

| Flag | Method |
|---|---|
| `-exact` | Exact Search |
| `-lsh` | LSH |
| `-hypercube` | Hypercube |
| `-ivfflat` | IVFFlat |
| `-ivfpq` | IVFPQ |

### Algorithm parameters

| Argument | Used by | Meaning | Initial value |
|---|---|---|---:|
| `-k` | LSH | Hash functions per table | 4 |
| `-L` | LSH | Number of hash tables | 5 |
| `-w` | LSH / Hypercube | Hash window | 0.05 |
| `-kproj` | Hypercube | Number of projections | 10 |
| `-M` | Hypercube | Candidate limit | 500 |
| `-probes` | Hypercube | Vertex limit | 10 |
| `-kclusters` | IVFFlat / IVFPQ | Number of IVF clusters | `ceil(sqrt(n))` |
| `-nprobe` | IVFFlat / IVFPQ | Number of clusters searched | 5 |
| `-M` | IVFPQ | Number of subvectors | 16 |
| `-nbits` | IVFPQ | Bits per subvector code | 8 |

`-M` has a different meaning in Hypercube and IVFPQ. For that reason, we pass it explicitly in the run command.

### Example commands for D = 1,000

```bash
./search -exact -set validation -D 1000 -o exact_validation.txt

./search -lsh -set validation -D 1000 -o lsh_validation.txt

./search -hypercube -set validation -D 1000 -M 500 -probes 10 -o hypercube_validation.txt

./search -ivfflat -set validation -D 1000 -nprobe 5 -o ivfflat_validation.txt

./search -ivfpq -set validation -D 1000 -M 16 -nbits 8 -nprobe 5 -o ivfpq_validation.txt
```

For another database size, we only need to change `-D` and the output filename.

For example, `-D 5000` creates a database of 5,115 images when the selected split contains 23 sequences.

---

## 8. Evaluation and output

For each query, the search method returns up to the first ten database images.

The evaluation code compares those results against the five relevant HPatches images for that query.

### Metrics

**Recall@5 and Recall@10**

We count how many relevant images appear in the first 5 or 10 results and divide by 5.

**AP@10**

AP@10 also considers the position of the relevant results. A relevant image near rank 1 contributes more than one near rank 10.

**mAP@10**

We calculate the average AP@10 across all queries.

**Average query time**

We measure the time from when the query histogram is available until the search method returns its results. Image loading, SIFT extraction, histogram creation and index construction are excluded. The time is reported in milliseconds.

If an approximate method returns fewer than 10 results, the missing positions count as non-relevant. We also report how many queries returned fewer than 10 results.

### Output file

The `-o` argument controls where the results are saved.

The file contains the method name, the top ten image IDs for every query, the per-query metrics, and the mean metrics:

```
<METHOD NAME>
Query: <sequence_id>
Rank-1: <image_id>
Rank-2: <image_id>
...
Rank-10: <image_id>
Recall@5: <value>
Recall@10: <value>
AP@10: <value>

...

Mean Recall@5: <value>
Mean Recall@10: <value>
mAP@10: <value>
Average query time: <value>
```

HPatches image IDs are written as `sequence_name/image_number.ppm`, for example `v_bird/2.ppm`. MIRFlickr image IDs are written as filenames, for example `im123.jpg`.

---

## 9. Experiment plan

We use validation first and test only after selecting the parameters.

### Step 1: Validation experiments

Run all five methods for:

```
D = 1000
D = 5000
D = 10000
D = 25000
```

Start with the initial parameter values described above. This gives us a baseline comparison for every database size.

### Step 2: Parameter selection

Use the validation results to tune the approximate methods.

The goal is not only to get the highest recall. We also need to compare how much query time each method saves.

For example:

- For LSH, try different values of `k` and `L`.
- For Hypercube, try different `M` and `probes` values.
- For IVFFlat, try different `nprobe` values and, if needed, different `kclusters`.
- For IVFPQ, try different `nprobe` values and compare compression/search settings if time allows.

We should change one parameter at a time where possible, so we can understand what caused the result to change.

### Step 3: Final test experiments

After selecting the parameters using validation, run the test set with those selected values.

We should not use the test results to continue tuning. The test set is only for the final measurement.

### Step 4: Separate illumination and viewpoint results

The report must show results for all queries together and separately for illumination and viewpoint sequences.

The sequence prefixes identify the two groups in our HPatches split:

- `i_` for illumination sequences.
- `v_` for viewpoint sequences.

We need to calculate the same metrics separately for these groups and include them in the report.

### Step 5: Write the report

The final report should include:

- A short description of the datasets and split.
- How SIFT and BoVW are used to represent the images.
- How each of the five search methods works.
- The parameters used in the experiments.
- Tables comparing Recall@5, Recall@10, mAP@10 and average query time.
- Overall results and separate illumination/viewpoint results.
- A discussion of the speed-versus-quality trade-off.
- A conclusion based on the actual test results.

---

## 10. Current validation results

The following results are from our first validation run with `D = 1,000`. These use the initial parameters and are not the final tuned results.

| Method | Recall@5 | Recall@10 | mAP@10 | Query time (ms) |
|---|---:|---:|---:|---:|
| Exact | 0.756522 | 0.808696 | 0.779037 | 0.287944 |
| LSH | 0.382609 | 0.391304 | 0.378985 | 0.025071 |
| Hypercube | 0.156522 | 0.156522 | 0.153623 | 0.083963 |
| IVFFlat | 0.756522 | 0.800000 | 0.775000 | 0.171000 |
| IVFPQ | 0.695652 | 0.773913 | 0.724965 | 0.289721 |

The first results already show the main trade-off we want to discuss. LSH is much faster in this run, but its recall is lower. IVFFlat is close to Exact Search in retrieval quality and is faster for this database size. Hypercube currently returns fewer than ten results for some queries, so its candidate and probe limits need attention. IVFPQ has lower retrieval quality than Exact Search in this run and is not faster at this database size.

These are only the initial results for `D = 1,000`. We still need the larger database experiments, parameter selection and final test evaluation before drawing conclusions.

---

## 11. Dataset download references

HPatches dataset:

https://github.com/hpatches/hpatches-dataset

MIRFlickr-25K dataset:

http://press.liacs.nl/mirflickr/mirflickr25k.v3b/mirflickr25k.zip


