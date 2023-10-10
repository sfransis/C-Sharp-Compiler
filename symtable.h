//
// Symbol Table Module Interface
//
#ifndef SYMTABLE_H
#define SYMTABLE_H

typedef enum { T_STRING, T_INT, T_LONG } DataType;
typedef enum { V_GLOBAL, V_PARAM, V_LOCAL, V_GLARRAY } VariableKind;

typedef struct symbol_s {
   int scopeLevel;     // 0 for globals, 1 for params and locals
   DataType type;
   VariableKind varKind; //not used yet...
   unsigned int size;  // 0 if simple var, N if array (N is num elems)
   int offset;         // stack offset for local vars and params
   char* name;
   struct symbol_s* next;
} Symbol;

typedef struct {
   int index;
   Symbol* lastsym;
} SymbolTableIter;

Symbol** newSymbolTable();
int addSymbol(Symbol** table, char* name, int scopeLevel, DataType type,
              unsigned int size, int offset);
Symbol* findSymbol(Symbol** table, char* name);
Symbol* iterSymbolTable(Symbol** table, int scopeLevel, SymbolTableIter* iter);
void freeAllSymbols(Symbol** table);
int delScopeLevel(Symbol** table, int scopeLevel);
/* 
NOT USED
int delSymbol(Symbol** table, char* name, int scopeLevel);
*/

#endif

