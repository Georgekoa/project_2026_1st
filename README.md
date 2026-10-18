# project_2026_1st
Project of Algorithmic problems software 2026-27


We started by understanding the assignment and making sure our setups are ready to accomodate this project.
we first tested the g++ compiler locally and before starting with the actual photo sift extraction we added a gitignore, with the compiled files the .o object files the build directories the data folder which is going to be used for testing purposes and the core files. This is to prevent accidental large pushes of data and/or crash files to github.

i had trouble with opencv on my WSL windows 11 PC, while on native linux version 4.6 of opencv everythiong worked. 
i made a compile_commands.json using bear to help me point vscode to my local opencv version 4.6 installation (wsl used 4.2 where the SIFT desrcitpors werent recognized)\

in the makefile we are going to leave anyone use their local opencv version. 
added the compile_commands.json to gitignore, as it it for local use only

for intelissense issues:


1) Clone the repo and install the project dependencies, including a compatible OpenCV version. (prefferably native 4.6.0)
2) Open the project in VS Code.
3) Install Bear if it isn't installed already.
4) Generate the local compilation database.

sudo apt install bear <br>
cd project_2026_1st  <br>
bear make clean all  (this  commands uses make and copies the compilation commands to a json file used by intelissense) <br>

the file created by bear is called compile_commands.json by default. Dont rename it as, renaming it will push it to the repository. 

the makefile already uses LDLIBS = $(shell pkg-config --libs opencv4) so anyones opencv library will be found automatically 





## Building guide

You can use make clean and make regulary. 
if vscode has issues with SIFT you can use 
bear make clean all to ignore these errors (supposed your opencv version is compatible)

OR on newer build 
bear -- make clean all


## Building of project proccess 

First we tested SIFT function and usability on a hardcoded image.
we will now create a dedicated function to make it reusbale for hundreds of images.


 downloaded the mirflickr data set using this command wget  http://press.liacs.nl/mirflickr/mirflickr25k.v3b/mirflickr25k.zip

 and hpatches by this repository : https://github.com/hpatches/hpatches-dataset

 ## implemeantion

 std::vector<std::string> txt_reader(std::string& file_path); 
 this func will just read the the split file (it will be hardcoded in the beggining, maybe all the way to the end) and returns a vector of strings where we are going to store the images name to.  
 we chose to use the >> operator to split the split.txt file as it will stop at whitespaces.

void vector_combine(cv::Mat& new_matrix,std::vector<std::vector<float>>& master_vector);<br>
void collect_training_descriptors(std::vector<std::string>  training_sequences,std::vector<std::vector<float>>& master_vector);<br>

for those two functions the logic is basically collect_training_descriptors being the coordinator who calls vector_combine on each image to get the first 500 descs to the master vector where we will keep our training data.<br>

in detail, txt_reader leaves us with a vector of strings with the training sequences names. so collect builds the full path names using the generic root path and adding the strings from the txt vector + the number of the image and the .ppm in the end. that leaves us with a clear path name that we can call sift_extractor to. <br>
So we iterate through the whole training sequences, we build the path, we call sift extractors which returns a matrix and then vector_combine converts those matrices to vector of vectors and adds it to our master vector

after that the master vector consist of 128 collumns, which is expected and 206122 rows. A number which is most likely correct as the maximum numbers of sift descriptors after our 500 limit cutoff is 210.000
(70 sequences * 6 images * 500 descriptors)
Assuming some images wont have 500 descriptors but less, this number makes sense.


**initialize_centers** just takes the master vector, build a vector of the same size with the cells containing just an indexing (0,1,2,....,~200k), suffles it and then we pick the first 256 cells, after which we make another vector of vectors to represent our centers, and we push back the master_vector contents from tyhe first 256 indexes (so ultimately we get random distinct push backs)
