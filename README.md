# OSProject3
< Seth Polen , Teresa Weaver , Ruben Cortes >

///////// FILE LISTING //////////
project3.c	c file implementing fat32 
makefile 	makefile for project3.c
gitlog.txt	text file of our gitlog

///////// MAKEFILE DESCRIPTION ////////
builds the executable "project3" and accepts a fat32.img as param
compiles with c99 flag as a std, automatically cleans by 
rm project3.o


///////// DIVISION OF LABOR /////////
exit : Seth
info : Teresa
size : ruben
ls : seth pair programmed with Ruben and Teresa 
cd : ruben pair programmed with Seth and Teresa
creat: Teresa pair programmed with Ruben and Seth
mkdir: Seth pair programmed with Ruben and Teresa
mv: Teresa pair programmed with Seth and Ruben
open: Ruben pair programmed with Teresa and Seth
close: Seth pair programmed with Ruben and Teresa
lseek: not implemented 
write: not implemented
read: not implemented
rm: not implemented
cp FILENAME TO: not implemented
rmdir: not implented
cp -r FROM TO : not implemented


README.md and GITLOG : Seth 

/////////// GIT LOG ///////////
PROVIDED IN TAR 
may not accurately reflect how equally we worked on the assignment
everytime we worked on it we met on zoom, screensharing so that we could
pair program and tackle the problems together, other than exit info and size
We equally kept each other accountable for sections and just code readability

/////////// HOW TO COMPILE //////////
'make'
//// to run ////
project3 fat32.img



////////// KNOWN BUGS //////////
unimplemented sections listed above in division of labor
creat: unable to create a new cluster if the current one is full. In this case a directory will not be made. 
size: wont look past the first cluster of the directory.
cd: wont look past the first cluster of the directory to find what its looking for.





