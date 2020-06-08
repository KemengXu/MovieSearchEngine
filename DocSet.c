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

#include "DocSet.h"
#include "Hashtable.h"
#include "Util.h"

#define HUNDRED 100

int CompareRowId(void* id1, void* id2) {
  if ((int*)id1 == (int*)id2) { return 0; }
  if ((int*)id1 < (int*)id2) {
    return -1;
  } else {
    return 1;
  }
}

// helper function if the docId is new to the set
int AddIfNoDocId(DocumentSet set,  uint64_t docId, int rowId) {
  HTKeyValue kv;
  HTKeyValue old_kv;
  kv.key = docId;
  kv.value = CreateLinkedList();
  if (kv.value == NULL) {return -1;}
  PutInHashtable(set->doc_index, kv, &old_kv);
  int* payLoad = (int*)malloc(HUNDRED * sizeof(int));
  *payLoad = rowId;
  AppendLinkedList(kv.value, payLoad);
  return 0;
}

// helper function if the docId already exists in the set
int AddIfHasDocId(DocumentSet set,  uint64_t docId, int rowId, HTKeyValue* tmp) {
  LLIter iter = CreateLLIter(tmp->value);
  int* cur_row;
  LLIterGetPayload(iter, (void**)&cur_row);
  if (cur_row == &rowId) {
    DestroyLLIter(iter);
    return -1;
  }
  while (LLIterHasNext(iter)) {
    LLIterNext(iter);
    LLIterGetPayload(iter, (void**)&cur_row);
    if (cur_row == &rowId) {
      DestroyLLIter(iter);
      return -1;
    }
  }
  int* payLoad = (int*)malloc(HUNDRED * sizeof(int));
  *payLoad = rowId;
  AppendLinkedList(tmp->value, payLoad);
  DestroyLLIter(iter);
  return 0;
}

int AddDocInfoToSet(DocumentSet set,  uint64_t docId, int rowId) {
// one thing to notice: the docIdSet HT starts with 1, rowIds start with 0
  // STEP 4: Implement AddDocInfoToSet.
  // Make sure there are no duplicate rows or docIds.
  HTKeyValue tmp;
  int res = LookupInHashtable(set->doc_index, docId, &tmp);
  if (res != 0) {
    return AddIfNoDocId(set, docId, rowId);
  } else {
    return AddIfHasDocId(set, docId, rowId, &tmp);
  }
}

int DocumentSetContainsDoc(DocumentSet set, uint64_t docId) {
  // STEP 5: Implement DocumentSetContainsDoc
  HTKeyValue tmp;
  if (LookupInHashtable(set->doc_index, docId, &tmp) == 0) {return 0;}
  return -1;
}

void PrintOffsetList(LinkedList list) {
  printf("Printing offset list\n");
  LLIter iter = CreateLLIter(list);
  int* payload;
  while (LLIterHasNext(iter) != 0) {
    LLIterGetPayload(iter, (void**)&payload);
    printf("%d\t", *((int*)payload));
    LLIterNext(iter);
  }
}


DocumentSet CreateDocumentSet(char *desc) {
  DocumentSet set = (DocumentSet)malloc(sizeof(struct docSet));
  if (set == NULL) {
    // Out of memory
    printf("Couldn't malloc for movieSet %s\n", desc);
    return NULL;
  }
  int len = strlen(desc);
  set->desc = (char*)malloc((len + 1) *  sizeof(char));
  if (set->desc == NULL) {
    printf("Couldn't malloc for movieSet->desc");
    return NULL;
  }
  snprintf(set->desc, len + 1, "%s", desc);
  set->doc_index = CreateHashtable(16);
  return set;
}


void DestroyOffsetList(void *val) {
  LinkedList list = (LinkedList)val;
  DestroyLinkedList(list, &SimpleFree);
}

void DestroyDocumentSet(DocumentSet set) {
  // Free desc
  free(set->desc);
  // Free doc_index
  DestroyHashtable(set->doc_index, &DestroyOffsetList);
  // Free set
  free(set);
}
