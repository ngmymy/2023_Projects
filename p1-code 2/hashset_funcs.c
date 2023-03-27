// hashset_funcs.c: utility functions for operating on hash sets. Most
// functions are used in the hashset_main.c which provides an
// application to work with the functions.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashset.h"

// PROVIDED: Compute a simple hash code for the given character
// string. The code is "computed" by casting the first 8 characters of
// the string to a numbers and returning their xor. The empty string
// has hash code 0. Longer strings will have numbers which are the
// integer interpretation of up to their first 8 bytes.  ADVANTAGE:
// constant time to compute the hash code. DISADVANTAGE: poor
// distribution for long strings; all strings with same first 8 chars
// hash to the same location.
int hashcode(char key[]){
  int hc = 0;
  for(int i=0; key[i]!='\0'; i++){
    hc = hc*31 + key[i];
  }
  return hc;
}

// Initialize the hash set 'hs' to have given size and elem_count
// 0. Ensures that the 'table' field is initialized to an array of
// size 'table_size' and is filled with NULLs. Also ensures that the
// first/last pointers are initialized to NULL
void hashset_init(hashset_t *hs, int table_size){ 
  hs->elem_count = 0;
  hs->table_size = table_size;
  hs->order_first = NULL; // first
  hs->order_last = NULL; // last pointers to NULL
  hs->table = malloc(sizeof(hashnode_t*) * table_size); // allocates table

  for(int i= 0; i < table_size; i++){ // makes all elements NULL
    hs->table[i] = NULL;
  }
}

// Returns 1 if the parameter `elem` is in the hash set and 0
// otherwise. Uses hashcode() and field `table_size` to determine
// which index in table to search.  Iterates through the list at that
// table index using strcmp() to check for `elem`. NOTE: The
// `hashcode()` function may return positive or negative
// values. Negative values are negated to make them positive. The
// "bucket" (index in hs->table) for `elem` is determined by with
// 'hashcode(key) modulo table_size'.
int hashset_contains(hashset_t *hs, char elem[]){
  int hc = hashcode(elem);
  if(hc < 0)
    hc *= -1;

  int index = hc % hs->table_size; // modulo to fit into a smaller index
  hashnode_t *curr_node = hs->table[index]; // correct index for first node of table
  while(curr_node != NULL){
    if(strcmp(elem, curr_node->elem) == 0){ // compare elem to the one of the current node we are at
      return 1;
    }
    curr_node = curr_node->table_next; // iterate so that it moves to next
  }
  return 0;
}

// If the element is already present in the hash set, makes no changes
// to the hash set and returns 0. hashset_contains() may be used for
// this. Otherwise determines the bucket to add `elem` at via the same
// process as in hashset_contains() and adds it to the FRONT of the
// list at that table index. Adjusts the `hs->order_last` pointer to
// append the new element to the ordered list of elems. If this is the
// first element added, also adjsuts the `hs->first` pointer. Updates the
// `elem_count` field and returns 1 to indicate a successful addition.
//
// NOTE: Adding elems at the front of each bucket list allows much
// simplified logic that does not need any looping/iteration.
int hashset_add(hashset_t *hs, char elem[]){
  if(hashset_contains(hs, elem)){
    return 0;
  }
  int hc = hashcode(elem);
  if(hc < 0)                                  // if negative multiply by -1 to get a positive
    hc *= -1;

  int index = hc % hs->table_size;            // determines bucket
  hashnode_t *newNode = malloc(sizeof(hashnode_t));
  strcpy(newNode->elem, elem);

  if(hs->elem_count == 0){                    //if order_first is NULL (or empty)
    hs->order_first = newNode;                // make the first node be the one that was made
  }else{
    hs->order_last->order_next = newNode;       // otherwise set the LATEST one (accessed through last->next) as new node
  }

  newNode->order_next = NULL;                         // initializes empty next for the new node
  hs->order_last = newNode;

  if(hs->table[index] == NULL){               // Test case where table at index is NULL
    hs->table[index] = newNode;
    newNode->table_next = NULL;
  }else{
    newNode->table_next = hs->table[index];
    hs->table[index] = newNode;
  }
  hs->elem_count++;                              // iterate elem_count
  return 1;
}

// De-allocates nodes/table for `hs`. Iterates through the ordered
// list of the hash set starting at the `order_first` field and
// de-allocates all nodes in the list. Also free's the `table`
// field. Sets all relevant fields to 0 or NULL as appropriate to
// indicate that the hash set has no more usable space. Does NOT
// attempt to de-allocate the `hs` itself as it may not be
// heap-allocated (e.g. in the stack or a global).
void hashset_free_fields(hashset_t *hs){
  hashnode_t *current = hs->order_first;
  while(current != NULL){
    hashnode_t *next = current->order_next;
    free(current);
    current = next;
  }
  free(hs->table); // frees table field

  hs->order_last = NULL; 
  hs->order_first = NULL;
  hs->elem_count = 0; 
  hs->table_size = 0;

}

// Displays detailed structure of the hash set. Shows stats for the
// hash set as below including the load factor (element count divided
// by table_size) to 4 digits of accuracy.  Then shows each table
// array index ("bucket") on its own line with the linked list of
// elems in the bucket on the same line. 
// 
// EXAMPLE:
// elem_count: 4
// table_size: 5
// order_first: Rick
// order_last : Tinyrick
// load_factor: 0.8000
// [ 0] : {7738144525137111380 Tinyrick >>NULL} 
// [ 1] : 
// [ 2] : 
// [ 3] : {125779953153363 Summer >>Tinyrick} {1801677138 Rick >>Morty} 
// [ 4] : {521644699469 Morty >>Summer} 
//
// NOTES:
// - Uses format specifier "[%2d] : " to print the table indices
// - Nodes in buckets have the following format:
//   {1415930697 IceT >>Goldenfold}
//    |          |       |        
//    |          |       +-> order_next->elem OR NULL if last node
//    |          +->`elem` string     
//    +-> hashcode("IceT"), print using format "%ld" for 64-bit longs
// 
void hashset_show_structure(hashset_t *hs){
  printf("elem_count: %d\n", hs->elem_count);
  printf("table_size: %d\n", hs->table_size);

  if(hs->order_first->elem == NULL){
    printf("order_first: %s\n", "NULL");
  }else{
    printf("order_first: %s\n", hs->order_first->elem);
  }
  if(hs->order_last->elem == NULL){
    printf("order_last : %s\n", "NULL");
  }else{
    printf("order_last : %s\n", hs->order_last->elem);
  }

  double load_fact = (double)hs->elem_count / (double)hs->table_size;
  printf("load_factor: %.4f\n", load_fact);

  for(int i = 0; i < hs->table_size; i++){
    if(i <= 9){                           // if 1 digit,
      if(hs->table[i] == NULL){               // if bucket is empty, print with spacing and line break
        printf("[ %d] :\n", i);
      }else{                                   // else, this spacing
        printf("[ %d] : ", i);
      }
    }else{                                // if more than one digit
      if(hs->table[i] == NULL){
        printf("[%d] :\n", i);
      }else{
        printf("[%d] : ", i);
      }
    }
    hashnode_t *current_arr = hs->table[i];
    while(current_arr != NULL){                                   // current bucket that we are working accessing

      printf("{%d %s >>", hashcode(current_arr->elem), current_arr->elem);
      if(current_arr->order_next == NULL){
        printf("NULL} ");
      }else{
        printf("%s} ", current_arr->order_next->elem);
      }
      current_arr = current_arr->table_next;                      // if the bucket has no more nodes to print, go to next line
      if(current_arr == NULL){
        printf("\n");
      }
      
    }
  }
}

// Outputs all elements of the hash set according to the order they
// were added. Starts at the `order_first` field and iterates through
// the list defined there. Each element is printed on its own line
// preceded by its add position with 1 for the first elem, 2 for the
// second, etc. Prints output to `FILE *out` which should be an open
// handle. NOTE: the output can be printed to the terminal screen by
// passing in the `stdout` file handle for `out`.
void hashset_write_elems_ordered(hashset_t *hs, FILE *out){
  hashnode_t *current = hs->order_first;
  int order_num = 1;
  while(current != NULL){                                       // if current hs->order_first isnt NULL
    fprintf(out, "   %d %s\n", order_num, current->elem);       // print order num and element of current hs->order_first
    current = current->order_next;                              // iterates through
    order_num++;
  }
}

// Writes the given hash set to the given `filename` so that it can be
// loaded later.  Opens the file and writes its 'table_size' and
// 'elem_count' to the file. Then uses the hashset_write_elems_ordered()
// function to output all elems in the hash set into the file.
// EXAMPLE FILE:
// 
// 5 6 
//   1 Rick
//   2 Morty
//   3 Summer
//   4 Jerry
//   5 Beth
//   6 Tinyrick
// 
// First two numbers are the 'table_size' and 'elem_count' field and
// remaining text is the output of hashset_write_elems_ordered();
// e.g. insertion position and element.
void hashset_save(hashset_t *hs, char *filename){
  FILE *file = fopen(filename, "w");
  if(file == NULL){
    printf("ERROR: could not open file '%s'\n", filename);
  }else{
    fprintf(file, "%d %d\n", hs->table_size, hs->elem_count);       // print table size and elem count into that file
    hashset_write_elems_ordered(hs, file);                          // use to write elems
    fclose(file);
  }
}

// Loads a hash set file created with hashset_save(). If the file
// cannot be opened, prints the message
// 
// ERROR: could not open file 'somefile.hs'
//
// and returns 0 without changing anything. Otherwise clears out the
// current hash set `hs`, initializes a new one based on the size
// present in the file, and adds all elems from the file into the new
// hash set. Ignores the indices at the start of each line and uses
// hashset_add() to insert elems in the order they appear in the
// file. Returns 1 on successful loading (FIXED: previously indicated
// a different return value on success) . This function does no error
// checking of the contents of the file so if they are corrupted, it
// may cause an application to crash or loop infinitely.
int hashset_load(hashset_t *hs, char *filename){   
  FILE *file = fopen(filename, "r");
  if(file == NULL){
    printf("ERROR: could not open file '%s'\n", filename);
    return 0;
  }
  int size;
  int count;
  fscanf(file, "%d %d", &size, &count);                   // reads in size and count from file
  hashset_free_fields(hs);                                // frees fields of current hs
  hashset_init(hs, size);                                 // initialize new hs to correct size
  for(int i = 0; i < count; i++){                         // iterate through count pulled from the data file
    char elem[128];                                       // initializing char item[byte]
    fscanf(file, "%*d %s", elem);
    hashset_add(hs, elem);                                // add elem to hs
  }
  fclose(file);
  return 1;
}

// If 'num' is a prime number, returns 'num'. Otherwise, returns the
// first prime that is larger than 'num'. Uses a simple algorithm to
// calculate primeness: check if any number between 2 and (num/2)
// divide num. If none do, it is prime. If not, tries next odd number
// above num. Loops this approach until a prime number is located and
// returns this. Used to ensure that hash table_size stays prime which
// theoretically distributes elements better among the array indices
// of the table.
int next_prime(int num){
  int cur_num = num;
  for(int i = 2; i < (cur_num/2); i++){

    if(cur_num % i == 0){                               // if ... go try next i in for loop

      if(cur_num % 2 == 0){                             // if cur_num is divisible by 2, recursively try next_prime with THAT number plus 1
        return next_prime(cur_num+1);
      }else{
        return next_prime(cur_num+2);                   // is !divisible by 2 then recursively try with plus 2
      }
    }
  }
  return num;
}

// Allocates a new, larger area of memory for the `table` field and
// re-adds all current elems to it. The size of the new table is
// next_prime(2*table_size+1) which keeps the size prime.  After
// allocating the new table, all table entries are initialized to NULL
// then the old table is iterated through and all elems are added to
// the new table according to their hash code. The memory for the old
// table is de-allocated and the new table assigned to the hash set
// fields "table" and "table_size".  This function increases
// "table_size" while keeping "elem_count" the same thereby reducing
// the load of the hash table. Ensures that the memory associated with
// the old table is free()'d. Makes NO special effort to preserve old
// nodes: re-adds everything into the new table and then frees the old
// one along with its nodes. Uses functions such as hashset_init(),
// hashset_add(), hashset_free_fields() to accomplish the transfer.
void hashset_expand(hashset_t *hs){
  hashset_t new_hash;
  hashset_init(&new_hash, next_prime(2*hs->table_size+1));                
  hashnode_t *current = hs->order_first;

  while(current != NULL){                                   // while current isnt NUll, keep adding
    hashset_add(&new_hash, current->elem);
    current = current->order_next;
  }

  hashset_free_fields(hs);                                  // frees
  *hs = new_hash;                                           // sets pointer to new hashset that we made
}