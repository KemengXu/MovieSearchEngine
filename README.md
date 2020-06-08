## This is the project in Computer System course
## See [instruction page](https://course.ccs.neu.edu/cs5007su19-seattle/assignments/a9.html) for all the details! 
## Some Files: 
* ```FileParser```: Responsible for reading in a file, checking for errors, and parse out movies.
* ```Movie```: A struct holding all the Movie info (id, title, type, genre, etc.)
* ```MovieIndex```: A hashtable that indexes movies according to specified field. Basically wraps a hashtable with functionality specific to adding/removing/destroying with MovieSets. The key is the description for the document set, and the value is the document set.
* ```MovieReport```: Responsible for printing out a report given a MovieIndex, to the command line or file.
* ```MovieSet```: A set of Movies that are connected somehow: they have the same value for a given field. If the field is Year, all the movies in the set are made in the same year. It consists of a description and a LinkedList of Movies.
##### for more info please see the files' documentation
### notes
  * Use hashtable inside hashtable, and use fileId and rowId to improve time complexity
  * Not build Movie struct for every line we see in every file, instead, build what we need by specific fileId and rowId to improve space complexity
### improvements
  * In CreateMovieFromFileRow() in main.c, I don't think it is very Time efficient, it's O(n), because we loop from the beginning row of the file every time we get a (file, rowId) pair. What I think would work better is we use another Hashtable to store (rowId, row) pairs of each file, just like what we've been doning throughout the whole project, so that we make it O(1)