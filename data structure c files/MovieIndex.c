/*
 *  Adrienne Slaughter
 *  5007 Spr 2020
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
#include <ctype.h>

#include "MovieIndex.h"
#include "htll/LinkedList.h"
#include "htll/Hashtable.h"
#include "Movie.h"
#include "MovieSet.h"

#define LEN 10

char* MyCheckAndAllocateString(char* token) {
  if (strcmp("-", token) == 0) {
    return NULL;
  } else {
    char *out = (char *) malloc((strlen(token) + 1) * sizeof(char));
    snprintf(out, strlen(token) + 1, "%s", token);
    return out;
  }
}

void DestroyMovieSetWrapper(void *movie_set) {
  DestroyMovieSet((MovieSet)movie_set);
}

void toLower(char *str, int len) {
  for (int i = 0; i < len; i++) {
    str[i] = tolower(str[i]);
  }
}

Index BuildMovieIndex(LinkedList movies, enum IndexField field_to_index) {
  Index movie_index = CreateIndex();

  // STEP 4(Student): Check that there is at least one movie
  // What happens if there is not at least one movie?
  if (NumElementsInLinkedList(movies) == 0) {
    DestroyIndex(movie_index);
    return NULL;
  }
  LLIter iter = CreateLLIter(movies);
  Movie* cur_movie;
  LLIterGetPayload(iter, (void**)&cur_movie);

  AddMovieToIndex(movie_index, cur_movie, field_to_index);

  while (LLIterHasNext(iter)) {
    LLIterNext(iter);
    LLIterGetPayload(iter, (void**)&cur_movie);
    AddMovieToIndex(movie_index, cur_movie, field_to_index);
  }
  DestroyLLIter(iter);
  return movie_index;
}

int Contains(MovieSet s, Movie* m) {
  if (NumElementsInLinkedList(s->movies) == 0) {return 0;}
  LLIter iter = CreateLLIter(s->movies);
  Movie* item;
  LLIterGetPayload(iter, (void**)&item);
  if (item == m) {
    DestroyLLIter(iter);
    return 1;
  }
  while (LLIterHasNext(iter) == 1) {
    LLIterNext(iter);
    LLIterGetPayload(iter, (void**)&item);
    if (item == m) {
      DestroyLLIter(iter);
      return 1;
    }
  }
  DestroyLLIter(iter);
  return 0;
}

Movie* copy(Movie* m) {
  Movie* mov = CreateMovie();
  mov->star_rating = m->star_rating;
  mov->title = MyCheckAndAllocateString(m->title);
  mov->content_rating = MyCheckAndAllocateString(m->content_rating);
  mov->genre = MyCheckAndAllocateString(m->genre);
  mov->duration = m->duration;
  mov->num_actors = m->num_actors;
  char** actors = (char**)malloc(sizeof(char*) * m->num_actors);
  for (int i = 0; i < m->num_actors; i++) {
    actors[i] = MyCheckAndAllocateString(m->actor_list[i]);
  }
  mov->actor_list = actors;
  return mov;
}

int AddMovieActorsToIndex(Index index, Movie *movie) {
  HTKeyValue kvp;
  HTKeyValue old_kvp;

  // STEP 6(Student): Add movies to the index via actors.
  //  Similar to STEP 5.
  for (int i = 0; i < movie->num_actors; i++) {
    kvp.key = ComputeKey(movie, Actor, i);
    if (LookupInHashtable(index, kvp.key, &kvp) != 0) {
      kvp.value = CreateMovieSet(movie->actor_list[i]);
      if (PutInHashtable(index, kvp, &old_kvp) != 0) {return 1;}
    }
    if (Contains(kvp.value, movie) != 0) {continue;}
    Movie* curr = copy(movie);
    if (AddMovieToSet((MovieSet)kvp.value, curr) != 0) {return 1;}
  }
  DestroyMovie(movie);
  return 0;
}

int AddMovieToIndex(Index index, Movie *movie, enum IndexField field) {
  if (field == Actor) {
    return AddMovieActorsToIndex(index, movie);
  }

  HTKeyValue kvp;
  kvp.key = ComputeKey(movie, field, 0);
  // STEP 5(Student): How do we add movies to the index?
  // The general idea:
  // Check hashtable to see if relevant MovieSet already exists
  // If it does, grab access to it from the hashtable
  // If it doesn't, create the new MovieSet and get the pointer to it
  // Put the new MovieSet into the Hashtable.
  if (LookupInHashtable(index, kvp.key, &kvp) != 0) {     // field not in index
    MovieSet curr;
    char rating[LEN];
    switch (field) {
      case Genre:
        curr = CreateMovieSet(movie->genre);
        break;
      case StarRating:
        snprintf(rating, LEN, "%f", movie->star_rating);
        curr = CreateMovieSet(rating);
        break;
      case ContentRating:
        curr = CreateMovieSet(movie->content_rating);
        break;
      default:
        printf("Wrong field!!!");
        break;
    }
    kvp.value = curr;
    HTKeyValue old_kvp;
    if (PutInHashtable(index, kvp, &old_kvp) != 0) {return 1;}
  }
  if (Contains(kvp.value, movie) != 0) {return 1;}
  // After we either created or retrieved the MovieSet from the Hashtable:
  if (AddMovieToSet((MovieSet)kvp.value, movie) != 0) {return 1;}
  return 0;
}

uint64_t ComputeKey(Movie* movie, enum IndexField which_field,
int which_actor) {
  char rating_str[LEN];
  switch (which_field) {
    case Genre:
      return FNVHash64((unsigned char*)movie->genre, strlen(movie->genre));
    case StarRating:
      snprintf(rating_str, LEN, "%f", movie->star_rating);
      return FNVHash64((unsigned char*)rating_str, strlen(rating_str));
    case ContentRating:
      return FNVHash64((unsigned char*)movie->content_rating,
                       strlen(movie->content_rating));
    case Actor:
      if (which_actor < movie->num_actors) {
        return FNVHash64((unsigned char*)movie->actor_list[which_actor],
                         strlen(movie->actor_list[which_actor]));
      }
      break;
  }
  return -1u;
}

// Removed for simplicity
// MovieSet GetMovieSet(Index index, const char *term){}

int DestroyIndex(Index index) {
  DestroyHashtable(index, &DestroyMovieSetWrapper);
  return 0;
}

Index CreateIndex() {
  return CreateHashtable(128);
}
