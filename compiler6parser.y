/*
// prog: declarations functions
// functions: /* empty */ //  | function functions
// function: ID LPAREN parameters RPAREN LBRACE localdecls statements RBRACE
// statements: /* empty */  | statement statements
// statement: funcall SEMICOLON  | assignment SEMICOLON  | whileloop  | ifthen  | ifthenelse
// assignment: ID EQUALS expression | ID LBRACKET expression RBRACKET EQUALS expression 
// funcall: ID LPAREN arguments RPAREN
// whileloop: KWWHILE LPAREN relexpr RPAREN LBRACE statements RBRACE
// ifthen: KWIF LPAREN relexpr RPAREN LBRACE statements RBRACE
// ifthenelse: KWIF LPAREN relexpr RPAREN LBRACE statements RBRACE KWELSE LBRACE statements RBRACE
// arguments: /* empty */  | argument  | argument COMMA arguments
// argument: expression
// expression: NUMBER  | STRING  | ID  | ID LBRACKET expression RBRACKET  | expression ADDOP expression
// relexpr: expression RELOP expression
// parameters: /* empty */  | vardecl  | vardecl COMMA parameters
// declarations: /* empty */  | vardecl SEMICOLON declarations
// localdecls: /* empty */  | vardecl SEMICOLON localdecls
// vardecl: KWINT ID  | KWSTRING ID  | KWINT ID LBRACKET NUMBER RBRACKET
// 

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "astree.h"
#include "symtable.h"

int addString(char* passedString);
void printPreamble();
void printMiddle();
void printGlobal();

int addSymbol(Symbol** table, char* name, int scopeLevel, DataType type,
              unsigned int size, int offset);
Symbol** newSymbolTable();
Symbol* iterSymbolTable(Symbol** table, int scopeLevel, SymbolTableIter* iter);

int yyerror(char *s);
int yylex(void);
int debug = 0;

// global variables

int stat;
Symbol** table;
ASTNode* root; 
char* string [128];
int currentIndex = 0;
int sid = 0;
int argNum=0;
char *argRegStr[] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
int lOffset = -4; 
int paramPosition = 1;
%}

/* token value data types */
%union { 
   int ival;  // for most scanner tokens
   char* str; // tokens that need a string, like ID and STRING
   struct astnode_s * astnode; // for all grammar nonterminals
}

/* Starting non-terminal */
%start prog
/* All nonterminals "return" an ASTNode pointer */
%type <astnode> functions function statements statement funcall arguments
%type <astnode> argument expression parameters declarations assignment
%type <astnode> varDecl prog whileLoop ifThen ifThenElse relExpr localDecls

/* Token types -- tokens either have an int value or a string value */
%token <ival> NUMBER COMMA SEMICOLON LPAREN RPAREN LBRACE RBRACE RBRACKET LBRACKET
%token <ival> ADDOP EQUALS KWINT KWSTRING KWWHILE KWIF KWELSE RELOP
%token <str>  ID STRING 

%%

// AST_PROGRAM -- root node for whole program
//                child[0] is global var decls; child[1] is function decls

prog:  declarations functions

    {
      // printGlobal();
      $$ = newASTNode(AST_PROGRAM); 
      // $$ -> valType = T_STRING;
      $$ -> child[0] = $1;
      $$ -> child[1] = $2;
      root = $$;
      printMiddle();
    }

functions: /*empty*/
    { $$ = 0;}

    |function functions

    {
      $1->next = $2;
      $$ = $1;
    }

// AST_FUNCTION - root node for function definition
//                child[0] is param decls; child[1] is function body

function: ID LPAREN parameters RPAREN  LBRACE localDecls statements RBRACE

    {
      $$ = newASTNode(AST_FUNCTION);
      $$->valType = T_STRING;
      $$->strval = $1;
      $$->child[0] = $3;
      $$->child[1] = $7;
      $$->child[2] = $6;

      // restet the lOffset and paramPosition 
      lOffset = -4;
      paramPosition = 1;
      // removing all symbols associated with the funciton, in other words a "make clean" so that you start off with a blank slate  
      delScopeLevel(table, 1);
    }

statements: /*empty*/
    { $$ = 0; }

    | statement statements

    {
      $1->next = $2;
      $$ = $1;
    }

statement: funcall SEMICOLON

    {
      $$ = $1;
    }


    |assignment SEMICOLON

    {
      $$ = $1;
    }

    |whileLoop

    {
      $$ = $1;
    }

    |ifThen

    {
      $$ = $1;
    }

    |ifThenElse

    {
      $$ = $1;
    }

// AST_FUNCALL -- function call node; strval is function name;
//                child[0] is arguments

funcall: ID LPAREN arguments RPAREN

    {
      $$ = newASTNode(AST_FUNCALL);
      // might not need this $$ -> valType = T_STRING;
      $$ -> strval = $1;
      $$ -> child[0] = $3;
      // gonna have to check if this bitch even needs to be set to 0
      // argNum = 0;
    }

// AST_ASSIGNMENT - assignment statement; strval is variable name
//                child[0] is right hand side expression

assignment: ID EQUALS expression

    {
      $$ = newASTNode(AST_ASSIGNMENT);
      $$ -> valType = T_STRING;
      $$ -> strval = $1;
      $$ -> child[0] = $3;

      // finding ID in the table 
      Symbol* foundSymbol = findSymbol(table, $1); 
      $$->ival = foundSymbol -> offset;
      $$->valType = foundSymbol->type; 
    }
    // Buckets 

    | ID LBRACKET expression RBRACKET EQUALS expression

    {
      $$ = newASTNode(AST_ASSIGNMENT);
      $$-> strval = $1;
      $$->child[0] = $6;
      $$->child[1] = $3;
      $$->ival = 0;
      $$->valType = T_INT;
      $$->varKind = V_GLARRAY;
    }

arguments: /*empty*/
    { $$ = 0; }

    | argument
    {
      $$ = $1;
    }

    | argument COMMA  arguments

    {
      $1-> next = $3;
      $$ = $1;
    }

// AST_ARGUMENT - function call argument; child[0] is expression of arg;
//                next is the next argument

argument: expression
    {
      $$ = newASTNode(AST_ARGUMENT);
      $$ -> child[0] = $1;
      // argNum++;
    }

// AST_EXPRESSION - expression node; ival is the operator id number
//                child[0] is left subexpr, child[1] is right subexpr

expression: NUMBER
    {
      $$ = newASTNode(AST_CONSTANT);
      $$ -> valType = T_INT;
      $$ -> ival = $1;
    }

    | ID
    {
      $$ = newASTNode(AST_VARREF);
      Symbol* symId = findSymbol(table, $1);
      $$ -> valType = T_STRING;
      $$ -> strval = $1;
      $$->ival = symId -> offset;
    }

    |STRING
    {
      sid = addString($1);
      $$ = newASTNode(AST_CONSTANT);
      $$ -> valType = T_STRING;
      $$ -> strval = $1;
      $$ -> ival = sid;
      // she also still has this shit in the argument. 
      // you might needs this hoe?
      // argNum++;
    }

    | expression ADDOP expression
    {
      $$ = newASTNode(AST_EXPRESSION);
      // $$ -> valType = T_INT; 
      $$ -> ival = $2;
      $$ -> child[0] = $1;
      $$ -> child[1] = $3;
      // $1->next = $3;
      // $$ = $1;
    
    }

    // Buckets 

    | ID LBRACKET expression RBRACKET

    {
      $$ = newASTNode(AST_VARREF);
      $$->valType = T_INT;
      $$->varKind = V_GLARRAY;
      $$->strval = $1;
      $$->child[0] = $3;
      $$->ival = 0;
    }

declarations: /*empty*/
    { $$ = 0; }

    | varDecl SEMICOLON declarations

    {
      addSymbol(table, $1->strval, 0, $1->valType, $1->ival, 0);
      // $$ -> valType = T_STRING;
      $1->next = $3;
      $$ = $1;

    }

// AST_VARDECL -- variable declaration; strval is var name; ival will be used
//                for local var offsets, array sizes, etc.

varDecl: KWINT ID

    {
      $$ = newASTNode(AST_VARDECL);
      $$ -> valType = T_INT;
      $$ -> strval = $2;
      // $$ -> ival = addSymbol(table, $2, 0, T_INT,0,0);
    }

    | KWSTRING ID

    {
      // $$ -> ival = addSymbol(table, $2, 0, T_STRING,0,0);
      $$ = newASTNode(AST_VARDECL);
      $$ -> valType = T_STRING;
      $$ -> strval = $2;
      
    }

    // Buckets 

    | KWINT ID LBRACKET NUMBER RBRACKET

    {
      $$ = newASTNode(AST_VARDECL);
      $$->valType = T_INT;
      $$->varKind = V_GLARRAY;
      $$->strval = $2;
      $$->ival = $4;
    }

// Buckets 

localDecls: /*empty*/
    { $$ = 0; }

    | varDecl SEMICOLON declarations

    {
      addSymbol(table, $1->strval, 1, $1->valType, 0, lOffset);
      $1->ival = lOffset;
      lOffset = lOffset - 4;
      $1->next = $3;
    }

// Buckets 

parameters: /*empty*/
    { $$ = 0; }

    | varDecl

    {
      addSymbol(table, $1->strval, 1, $1->valType, 0, paramPosition);
      $1->ival = paramPosition;
      paramPosition++;
      $$ = $1;
    }

    | varDecl COMMA parameters

    {
      addSymbol(table, $1->strval, 1, $1->valType, 0, paramPosition);
      $1->ival = paramPosition;
      paramPosition++;
      $1->next = $3;
      $$ = $1;
    }

// AST_IFTHEN  -- if-then-else statement; child[0] is condition expression
//                child[1] is if block, child[2] is else block
ifThenElse: KWIF LPAREN relExpr RPAREN LBRACE statements RBRACE KWELSE LBRACE statements RBRACE

    {
      $$ = newASTNode(AST_IFTHEN);
      $$ -> child[0] = $3;
      $$ -> child[1] = $6;
      $$ -> child[2] = $10;
    }

// AST_RELEXPR  - a relational expr; child[0] and child[1] are expressions
//                (or varrefs or constants); ival is op id; strval is jump
//                label?
relExpr: expression RELOP expression

    {
      $$ = newASTNode(AST_RELEXPR);
      $$ -> ival = $2;
      $$ -> child[0] = $1;
      $$ -> child[1] = $3;
      
    }

// AST_WHILE   -- while loop statement
//                child[0] is condition expression; child[1] is loop body
whileLoop: KWWHILE LPAREN relExpr RPAREN LBRACE statements RBRACE

    {
      $$ = newASTNode(AST_WHILE);
      $$ -> child[0] = $3;
      $$ -> child[1] = $6;
    }

// AST_IFTHEN  -- if-then-else statement; child[0] is condition expression
//                child[1] is if block, child[2] is else block
ifThen: KWIF LPAREN relExpr RPAREN LBRACE statements RBRACE

    {
      $$ = newASTNode(AST_IFTHEN);
      $$ -> child[0] = $3; 
      $$ -> child[1] = $6;
      $$ -> child[2] = 0;
    }
    ;
%%

extern FILE *yyin; // from lex

int main(int argc, char **argv)
{
  // FILE *inf;
   if (argc==2) {
      yyin = fopen(argv[1],"r");
      if (!yyin) {
         printf("Error: unable to open file (%s)\n",argv[1]);
         return(1);
      }
   }
   int doAssembly = 1;
	 table = newSymbolTable();
   yyparse();
   if (!doAssembly) {
      printASTree(root,0,stdout);
      return 0;
   }
   genCodeFromASTree(root,0,stdout);
   return 0;
}

int addString(char* passedString){
  string[currentIndex] = passedString;
  return currentIndex++;
}

void printPreamble(){
  printf("\n\t.text\n\t.section\t.rodata\n\n");
}

void printMiddle(){
  int i = 0;
  while (i < currentIndex){
  printf(".LC%d:\n\t.string\t\t%s\n", i, string[i]);
  i++;
  }
}

void printGlobal(){
	SymbolTableIter iter;
	iter.index = -1;
	Symbol * cur = iterSymbolTable(table, 0, &iter);
	while( cur!= NULL){
		printf("\t.comm ");
		printf("%s", cur->name);
		printf(",4,4\n");
		cur = iterSymbolTable(table, 0, &iter);
	}
}

extern int yylineno; // from lex

int yyerror(char *s)
{
   fprintf(stderr, "Error: line %d: %s\n",yylineno,s);
   return 0;
}

int yywrap()
{
   return(1);
}
