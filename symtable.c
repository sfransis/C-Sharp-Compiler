//
// Symbol Table Module
// - the symbol table is implemented as a hash table that uses
//   chaining (linked lists) to resolve collisions. Thus, each
//   table entry is a pointer to a linked list of symbols that
//   hashed to that entry (index value).
//
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include <stdbool.h>

// odd and especially prime values are best for hash tables
#define TABLESIZE 97

// Table hash function
// - just adds up all chars in the string and then
//   mods by table size to get 0 to (size-1) index value
static int hash(char *str)
{
   int h = 0;
   int i;
   for (i=0; i < strlen(str); i++)
      h += str[i];
   h = h % TABLESIZE;
   return h;
}

// Create a new symbol table and return pointer to it
// - each table entry will be a pointer to a linked list
//   of symbols that hashed to that entry; a NULL entry
//   means no symbols have yet hashed to that entry
Symbol** newSymbolTable()
{
   int i;
   Symbol** table;
   table = (Symbol**) malloc(sizeof(Symbol*)*TABLESIZE);
   for (i=0; i < TABLESIZE; i++)
      table[i] = 0;
   return table;
}

// Add a new symbol to the given symbol table
// - name is the symbol name string (must strdup() it in here to store)
// - scopeLevel is the scoping level of the symbol (0 is global)
// - type is its data type
// - this function must hash the symbol name to find the correct
//   table entry to put it on; each table entry is a pointer to a linked
//   list of symbols that hash to that index; symbols must be added to
//   the head of the list
// - this function must allocate a new Symbol structure, it must
//   strdup() the name to save its own copy, and must set all structure
//   fields appropiately
// - return 0 on success, any other on failure (generally, negative)
int addSymbol(Symbol** table, char* name, int scopeLevel, DataType type,
              unsigned int size, int offset)
{
   // your implementation should be less than 10 lines long -- keep it simple!

    Symbol* symName = (Symbol*) malloc (sizeof(Symbol));
    symName -> name = strdup(name);
    symName -> scopeLevel= 0;
    symName -> type = type;
    symName -> size = 0;
    symName -> offset = 0;
    int location = hash(name);
    symName -> next = NULL;
    symName -> next = table [location];
    table [location] = symName;
    return 0;
}

// Lookup a symbol name to see if it is in the symbol table
// - returns a pointer to the symbol record, or NULL if not found
// - it should return the first symbol record that exists with the
//   given name; there is no need to look further once you find one
// - pseudocode: hash the name to get table index, then look through
//               linked list to see if the name exists as a symbol
Symbol* findSymbol(Symbol** table, char* name)
{
   // this function should also have a pretty simple implementation
    int location = hash(name);
    Symbol* cur = table[location];
    while (cur != NULL){
        if((strcmp(cur -> name, name) == 0)){
            return cur;
        }// end of if
        cur = cur -> next;
    }// end of while
    return NULL;
}

// Iterator over entire symbol table
// - caller must declare iter as actual structure, not a pointer (pass with &)
// - caller must initialize iter.index to be -1 before first call
// - caller then calls this function until it returns NULL, meaning end
//   of all symbols; each return value is a pointer to a symbol in the table
// - parameter scopeLevel is not currently used (just pass a 0 in)
Symbol* iterSymbolTable(Symbol** table, int scopeLevel, SymbolTableIter* iter)
{
   Symbol* cur;
   if (iter->index == -1) {
      // start at index 0
      iter->index = 0;
      cur = table[iter->index];
   } else {
      // start where we left off
      cur = iter->lastsym->next;
   }
   // if we have another symbol already, use it (loop will be skipped)
   // otherwise, search for next index that has symbols (is not empty)
   while (!cur && iter->index < TABLESIZE-1) {
      iter->index++;
      cur = table[iter->index];
   }
   // update iterator position and return current symbol
   iter->lastsym = cur;
   return cur;
}

// Walk the table and the lists and free all symbol names
// and symbol structs
// - does not free the table (array) itself
//  - caller should probably free the table after this!
// - zeroing pointers in next and table elements makes memory
//   a bit safe, cannot accidentally follow and old pointer
void freeAllSymbols(Symbol** table)
{
   int i;
   Symbol *cur, *stmp;
   for (i=0; i < TABLESIZE; i++) {
      cur = table[i];
      while (cur) {
         stmp = cur;
         cur = cur->next;
         stmp->next = 0; // safety
         free(stmp->name);
         free(stmp);
      }
      table[i] = 0; // safety
   }
}

// Deletes all symbols that are at a given scope level and above
// - both the symbol string and the structure
// - relinks the linked list so that nothing is lost
int delScopeLevel(Symbol** table, int scopeLevel)
{
   int i;
   Symbol *prev=0, *cur=0, *t;
   for (i=0; i < TABLESIZE; i++) {
      prev = 0;
      cur = table[i];
      while (cur) {
         if (cur->scopeLevel >= scopeLevel) {
            t = cur;
            if (prev)
               prev->next = cur->next;
            else
               table[i] = cur->next;
            cur = cur->next;
            free(t->name);
            t->name = 0; // safety
            t->next = 0; // safety
            free(t);
         } else {
            prev = cur;
            cur = cur->next;
         }
      }
   }
   return 0;
}

/*int main(){
  Symbol** table = newSymbolTable();
}*/

/*** NOT USED SO REMOVED WITHOUT DELETING CODE
// Delete a specific symbol at a specific scope level
// from the symbol table
int delSymbol(Symbol** table, char* name, int scopeLevel)
{
   int i;
   Symbol *prev=0, *cur=0;
   i = hash(name);
   cur = table[i];
   while (cur) {
      if (!strcmp(cur->name, name) &&
          cur->scopeLevel == scopeLevel)
         break;
      prev = cur;
      cur = cur->next;
   }
   if (!cur)
      return 1;
   if (prev)
      prev->next = cur->next;
   else
      table[i] = cur->next;
   free(cur->name);
   cur->name = 0;
   cur->next = 0;
   free(cur);
   return 0;
}
***/
