#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hashset.h"

int main(int argc, char *argv[]){
  int echo = 0;                                // controls echoing, 0: echo off, 1: echo on
  if(argc > 1 && strcmp("-echo",argv[1])==0) { // turn echoing on via -echo command line option
    echo=1;
  }

  printf("Hashset Application\n");
  printf("Commands:\n");
  printf("  hashcode <elem>  : prints out the numeric hash code for the given key (does not change the hash set)\n");
  printf("  contains <elem>  : prints the value associated with the given element or NOT PRESENT\n");
  printf("  add <elem>       : inserts the given element into the hash set, reports existing element\n");
  printf("  print            : prints all elements in the hash set in the order they were addded\n");
  printf("  structure        : prints detailed structure of the hash set\n");
  printf("  clear            : reinitializes hash set to be empty with default size\n");
  printf("  save <file>      : writes the contents of the hash set to the given file\n");
  printf("  load <file>      : clears the current hash set and loads the one in the given file\n");
  printf("  next_prime <int> : if <int> is prime, prints it, otherwise finds the next prime and prints it\n");
  printf("  expand           : expands memory size of hash set to reduce its load factor\n");
  printf("  quit             : exit the program\n");
  
  char cmd[128];
  hashset_t hash;
  int success;
  hashset_init(&hash, HASHSET_DEFAULT_TABLE_SIZE);

  while(1){
    printf("HS>> ");                 // print prompt
    success = fscanf(stdin,"%s",cmd); // read a command
    if(success==EOF){                 // check for end of input
      printf("\n");                   // found end of input
      break;                          // break from loop
    }

    if(strcmp("quit", cmd)==0 ){     // check for quit command
      if(echo){
        printf("quit\n");
      }
      break;                          // break from loop
    }

    else if(strcmp("structure", cmd)==0 ){ // check for structure
      if(echo){
        printf("structure\n");
      }
      hashset_show_structure(&hash);
    }

    else if(strcmp("hashcode", cmd)==0 ){ // hashcode command
      fscanf(stdin,"%s", cmd); 
      if(echo){
        printf("hashcode %s\n", cmd);
      }
      int hash = hashcode(cmd);        
      printf("%d\n", hash);               // prints int for hashcode of string entered
    }
    
    // add case for "contains" command which uses the hashset_contains() function
    
    else if(strcmp("contains", cmd)==0){           // contains command
      fscanf(stdin,"%s",cmd);                       // reads string to check      
      if(echo){
        printf("contains %s\n",cmd);
      }
      success = hashset_contains(&hash,cmd);      // calls contain func
      if(success == 0){                               // checks for success
        printf("NOT PRESENT\n");
      }else{
        printf("FOUND: %s\n", cmd);
      }
    }

    else if(strcmp("add", cmd)==0){               // add command
      fscanf(stdin,"%s",cmd);                       // reads string to check      
      if(echo){
        printf("add %s\n",cmd);
      }
      success = hashset_add(&hash, cmd);           // call list function
      if(!success){                                 // check for success
        printf("Elem already present, no changes made\n");
      }
    }
    else if(strcmp("save", cmd)==0){              // save command
      fscanf(stdin,"%s",cmd);                       // reads string to check      
      if(echo){
        printf("save %s\n",cmd);
      }
      hashset_save(&hash, cmd);
    }

    else if(strcmp("load", cmd)==0){           // load command
      fscanf(stdin,"%s",cmd);                       // reads string to check      
      if(echo){
        printf("load %s\n",cmd);
      }
      success = hashset_load(&hash, cmd);        // call list function
      if(!success){                             // check for success
        printf("load failed\n");
      }
    }

    else if(strcmp("next_prime", cmd)==0){           // next_prime command
      int num;
      fscanf(stdin,"%d",&num);                       // reads string to check      
      if(echo){
        printf("next_prime %d\n",num);
      }
      printf("%d\n", next_prime(num));
    }

    else if( strcmp("expand", cmd)==0 ){   // expand command
      if(echo){
        printf("expand\n");
      }
      hashset_expand(&hash);
    }

    else if( strcmp("clear", cmd)==0 ){   // clear command
      if(echo){
        printf("clear\n");
      }
      hashset_free_fields(&hash);
      hashset_init(&hash, HASHSET_DEFAULT_TABLE_SIZE);
    }

    else if( strcmp("print", cmd)==0 ){   // print command
      if(echo){
        printf("print\n");
      }
      hashset_write_elems_ordered(&hash, stdout);
    }
    
    else{
      if(echo){
        printf("unknown command %s\n", cmd);
      }
    }
  }  
  // end main while loop
  hashset_free_fields(&hash);                      // clean up the list
  return 0;
}