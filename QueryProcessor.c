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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "QueryProcessor.h"
#include "MovieTitleIndex.h"
#include "LinkedList.h"
#include "Hashtable.h"
#include "DocSet.h"
#include "Assert007.h"

SearchResultIter CreateSearchResultIter(DocumentSet set) {
  SearchResultIter iter =
    (SearchResultIter)malloc(sizeof(struct searchResultIter));
    // STEP 7: Implement the initialization of the iter.
  iter->doc_iter = CreateHashtableIterator((Hashtable)set->doc_index);
  HTKeyValue kv;
  HTIteratorGet(iter->doc_iter, &kv);
  iter->cur_doc_id = kv.key;
  iter->offset_iter = CreateLLIter(kv.value);
  return iter;
}

void DestroySearchResultIter(SearchResultIter iter) {
  // Destroy LLIter
  if (iter->offset_iter != NULL) {
    DestroyLLIter(iter->offset_iter);
  }
  // Destroy doc_iter
  DestroyHashtableIterator(iter->doc_iter);

  free(iter);
}



SearchResultIter FindMovies(MovieTitleIndex index, char *term) {
  DocumentSet set = GetDocumentSet(index, term);
  if (set == NULL) {
    return NULL;
  }
  printf("Getting docs for movieset term: \"%s\"\n", set->desc);
  SearchResultIter iter = CreateSearchResultIter(set);
  return iter;
}


int SearchResultGet(SearchResultIter iter, SearchResult output) {
  // STEP 9: Implement SearchResultGet
  output->doc_id = iter->cur_doc_id;
  int* res;
  LLIterGetPayload(iter->offset_iter, (void**)&res);
  output->row_id = *res;
  return 0;
}

int SearchResultNext(SearchResultIter iter) {
  // STEP 8: Implement SearchResultNext
  if (SearchResultIterHasMore(iter) == 0) {return 0;}
  if (LLIterHasNext(iter->offset_iter)) {
    LLIterNext(iter->offset_iter);
  } else {
    DestroyLLIter(iter->offset_iter);
    HTIteratorNext(iter->doc_iter);
    HTKeyValue kv;
    HTIteratorGet(iter->doc_iter, &kv);
    iter->offset_iter = CreateLLIter(kv.value);
    iter->cur_doc_id = kv.key;
  }
  return 1;
}

// Return 0 if no more
int SearchResultIterHasMore(SearchResultIter iter) {
  if (iter->doc_iter == NULL) {
    return 0;
  }
  if (LLIterHasNext(iter->offset_iter) == 0) {
    return (HTIteratorHasMore(iter->doc_iter));
  }

  return 1;
}
