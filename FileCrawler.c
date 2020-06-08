/*
 *  Created by Adrienne Slaughter
 *  CS 5007 Summer 2019
 *  Northeastern University, Seattle
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  See <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "FileCrawler.h"
#include "DocIdMap.h"
#include "LinkedList.h"

char* filenameMallocer(char* token) {
  char *out = (char *) malloc((strlen(token) + 1) * sizeof(char));
  snprintf(out, strlen(token) + 1, "%s", token);
  return out;
}

void CrawlFilesToMap(const char *dir, DocIdMap map) {
  // STEP 3: Implement the file crawler.
  // Use namelist (declared below) to find files and put in map.
  // NOTE: There may be nested folders.
  // Be sure to lookup how scandir works. Don't forget about memory use.
  struct stat s;
  struct dirent **namelist;
  int n;
  n = scandir(dir, &namelist, 0, alphasort);

  if (n < 0) {
    perror("scandir");
  } else {
    while (n--) {
      if (strcmp(namelist[n]->d_name, ".") == 0) {
        free(namelist[n]);
        continue;
      }
      if (strcmp(namelist[n]->d_name, "..") == 0) {
        free(namelist[n]);
        continue;
      }
      char path[50];
      strcpy(path, dir);
      if (path[strlen(path)-1] !='/') strcat(path, "/");
      strcat(path, namelist[n]->d_name);
      if (!(lstat(path, &s) >= 0)) {continue;}
      if (S_ISREG(s.st_mode)) {
        // should use pathname here! not file name
        PutFileInMap(filenameMallocer(path), map);
      } else if (S_ISDIR(s.st_mode)) {
        CrawlFilesToMap((const char*)&path[0], map);
      }
      free(namelist[n]);
    }
    free(namelist);
  }
}
