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

sudo apt install bear
cd project_2026_1st
bear make clean all  (this  commands uses make and copies the compilation commands to a json file used by intelissense) 

the file created by bear is called compile_commands.json by default. Dont rename it as, renaming it will push it to the repository. 