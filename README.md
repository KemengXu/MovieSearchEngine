## This is the project in Computer System course
### See [instruction page](https://course.ccs.neu.edu/cs5007su19-seattle/assignments/a9.html) for all the details! 
## WHAT DOES THE OVERALL SYSTEM LOOK LIKE?
#### The following picture is an overview of the entire system:
![Image of system](https://github.com/KemengXu/MovieSearchEngine/blob/master/Whole.png)
#### The following picture shows the core data structure:
![Image of MovieTitleIndex](https://github.com/KemengXu/MovieSearchEngine/blob/master/MovieTitleIndex.png)
#### The following picture shows the protocol between server and client:
![Image of QueryProtocol](https://github.com/KemengXu/MovieSearchEngine/blob/master/QueryProtocol.png)
##### for more info please see the files' documentation
### notes
  * Use hashtable inside hashtable, and use fileId and rowId to improve time complexity
  * Not build Movie struct for every line we see in every file, instead, build what we need by specific fileId and rowId to improve space complexity
### improvements
  * In CreateMovieFromFileRow() in main.c, I don't think it is very Time efficient, it's O(n), because we loop from the beginning row of the file every time we get a (file, rowId) pair. What I think would work better is we use another Hashtable to store (rowId, row) pairs of each file, just like what we've been doning throughout the whole project, so that we make it O(1)