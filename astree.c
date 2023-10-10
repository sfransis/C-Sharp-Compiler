//
// Abstract Syntax Tree Implementation
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "astree.h"

// Create a new AST node
// - allocates space and initializes node type, zeros other stuff out
// - returns pointer to node
ASTNode* newASTNode(ASTNodeType type)
{
   int i;
   ASTNode* node = (ASTNode*) malloc(sizeof(ASTNode));
   node->type = type;
   node->valType = T_INT;
   node->varKind = V_GLOBAL;
   node->ival = 0;
   node->strval = 0;
   node->strNeedsFreed = 0;
   node->next = 0;
   for (i=0; i < ASTNUMCHILDREN; i++)
      node->child[i] = 0;
   return node;
}

// Generate an indentation string prefix, for use
// in printing the abstract syntax tree with indentation
// used to indicate tree depth.
// -- NOT thread safe! (uses a static char array to hold prefix)
#define INDENTAMT 3
static char* levelPrefix(int level)
{
   static char prefix[128]; // static so that it can be returned safely
   int i;
   for (i=0; i < level*INDENTAMT && i < 126; i++)
      prefix[i] = ' ';
   prefix[i] = '\0';
   return prefix;
}

//
// Free an entire ASTree, along with string data it has
//
void freeASTree(ASTNode* node)
{
   if (!node)
      return;
   freeASTree(node->child[0]);
   freeASTree(node->child[1]);
   freeASTree(node->child[2]);
   freeASTree(node->next);
   if (node->strNeedsFreed && node->strval)
      free(node->strval);
   free(node);
}

// Print the abstract syntax tree starting at the given node
// - this is a recursive function, your initial call should
//   pass 0 in for the level parameter
// - comments in code indicate types of nodes and where they
//   are expected; this helps you understand what the AST looks like
// - out is the file to output to, can be "stdout" or other
void printASTree(ASTNode* node, int level, FILE *out)
{
   if (!node)
      return;
   fprintf(out,"%s",levelPrefix(level)); // note: no newline printed here!
   switch (node->type) {
    case AST_PROGRAM:
       fprintf(out,"Program\n");
       printASTree(node->child[0],level+1,out);  // child 0 is gobal var decls
       fprintf(out,"%s--functions--\n",levelPrefix(level+1));
       printASTree(node->child[1],level+1,out);  // child 1 is function defs
       break;
    case AST_VARDECL:
       fprintf(out,"Variable declaration (%s)",node->strval); // var name
       if (node->valType == T_INT)
          if (node->varKind != V_GLARRAY)
             fprintf(out," type int\n");
          else
             fprintf(out," type int array size %d\n",node->ival);
       else if (node->valType == T_LONG)
          fprintf(out," type long\n");
       else if (node->valType == T_STRING)
          fprintf(out," type string\n");
       else
          fprintf(out," type unknown (%d)\n", node->valType);
       break;
    case AST_FUNCTION:
       fprintf(out,"Function def (%s)\n",node->strval); // function name
       fprintf(out,"%s--params--\n",levelPrefix(level+1));
       printASTree(node->child[0],level+1,out); // child 0 is param list
       fprintf(out,"%s--locals--\n",levelPrefix(level+1));
       printASTree(node->child[2],level+1,out); // child 2 is local vars
       fprintf(out,"%s--body--\n",levelPrefix(level+1));
       printASTree(node->child[1],level+1,out); // child 1 is body (stmt list)
       break;
    case AST_SBLOCK:
       fprintf(out,"Statement block\n");
       printASTree(node->child[0],level+1,out);  // child 0 is statement list
       break;
    case AST_FUNCALL:
       fprintf(out,"Function call (%s)\n",node->strval); // func name
       printASTree(node->child[0],level+1,out);  // child 0 is argument list
       break;
    case AST_ARGUMENT:
       fprintf(out,"Funcall argument\n");
       printASTree(node->child[0],level+1,out);  // child 0 is argument expr
       break;
    case AST_ASSIGNMENT:
       fprintf(out,"Assignment to (%s) ", node->strval);
       if (node->varKind == V_GLARRAY) { //child[1]) {
          fprintf(out,"array var\n");
          fprintf(out,"%s--index--\n",levelPrefix(level+1));
          printASTree(node->child[1],level+1,out);
       } else
          fprintf(out,"simple var\n");
       fprintf(out,"%s--right hand side--\n",levelPrefix(level+1));
       printASTree(node->child[0],level+1,out);  // child 1 is right hand side
       break;
    case AST_WHILE:
       fprintf(out,"While loop\n");
       printASTree(node->child[0],level+1,out);  // child 0 is condition expr
       fprintf(out,"%s--body--\n",levelPrefix(level+1));
       printASTree(node->child[1],level+1,out);  // child 1 is loop body
       break;
    case AST_IFTHEN:
       fprintf(out,"If then\n");
       printASTree(node->child[0],level+1,out);  // child 0 is condition expr
       fprintf(out,"%s--ifpart--\n",levelPrefix(level+1));
       printASTree(node->child[1],level+1,out);  // child 1 is if body
       fprintf(out,"%s--elsepart--\n",levelPrefix(level+1));
       printASTree(node->child[2],level+1,out);  // child 2 is else body
       break;
    case AST_EXPRESSION: // only for binary op expression
       fprintf(out,"Expression (op %d,%c)\n",node->ival,node->ival);
       printASTree(node->child[0],level+1,out);  // child 0 is left side
       printASTree(node->child[1],level+1,out);  // child 1 is right side
       break;
    case AST_RELEXPR: // only for relational op expression
       fprintf(out,"Relational Expression (op %d,%c)\n",node->ival,node->ival);
       printASTree(node->child[0],level+1,out);  // child 0 is left side
       printASTree(node->child[1],level+1,out);  // child 1 is right side
       break;
    case AST_VARREF:
       fprintf(out,"Variable ref (%s)",node->strval); // var name
       if (node->varKind == V_GLARRAY) { //child[0]) {
          fprintf(out," array ref\n");
          printASTree(node->child[0],level+1,out);
       } else
          fprintf(out,"\n");
       break;
    case AST_CONSTANT: // for both int and string constants
       if (node->valType == T_INT)
          fprintf(out,"Int Constant = %d\n",node->ival);
       else if (node->valType == T_STRING)
          fprintf(out,"String Constant = (%s)\n",node->strval);
       else
          fprintf(out,"Unknown Constant\n");
       break;
    default:
       fprintf(out,"Unknown AST node!\n");
   }
   // IMPORTANT: walks down sibling list (for nodes that form lists, like
   // declarations, functions, parameters, arguments, and statements)
   printASTree(node->next,level,out);
}

//
// Below here id code for generating our output assembly code from
// an AST. You will probably want to move some things from the
// grammar file (.y file) over here, since you will no longer be
// generating code in the grammar file. You may have some global
// stuff that needs accessed from both, in which case declare it in
// one and then use "extern" to reference it in the other.

// In my code, I moved over this stuff:
//void outputConstSec(FILE* out);
int argnum=0;
char *argregs[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
Symbol* sym;
extern Symbol** table;

// Used for labels inside code, for loops and conditionals
static int getUniqueLabelID()
{
   static int lid = 100; // you can start at 0, it really doesn't matter
   return lid++;
}

// Generate assembly code from AST
// - this function should look _alot_ like the print function;
//   indeed, the best way to start would be to copy over the
//   code from printASTree() and change all the recursive calls
//   to this function; then, instead of printing info, we are
//   going to print assembly code. Easy!
// - param node is the current node being processed
// - param count is a counting parameter (similar to level in
//   the printASTree() function) that can be used to keep track
//   of a position in a list -- I use it only in two places, to keep
//   track of arguments and then to use the correct argument register
//   (count is my index into my argregstr[] array); and to keep a
//   label ID for conditional jumps on AST_RELEXPR nodes; otherwise
//   this can just be 0
// - param out is the output file handle. Use "fprintf(out,..."
//   instead of printf(...); call it with "stdout" for terminal output
//   (see printASTree() code for how it uses the output file handle)
//
void genCodeFromASTree(ASTNode* node, int count, FILE *out)
{
  char * instr = (char*) malloc (100);
  // keeping count of the labels to jump to 
  int a;
  int b;
   if (!node){
     return;
  }
  fprintf(out,"%s",levelPrefix(count)); // note: no newline printed here!
  switch (node->type) {
   case AST_PROGRAM:
      fprintf(out,"\n\t.text\n");
      genCodeFromASTree(node->child[0],count+1,out);  // child 0 is gobal var decls
      fprintf(out, "\n\t.section\t.rodata\n\n");
      genCodeFromASTree(node->child[1],count+1,out);  // child 1 is function defs
      break;

   case AST_VARDECL:
      // fprintf(out, "vardecl!\n");
       if (node->valType == T_INT && node->ival > 0) {

          if (node->varKind != V_GLARRAY) {
         // global int variable reference
             fprintf(out, "\t.comm\t%s, 4, 4\n", node->strval);
      } else {
         // global array variable reference
             fprintf(out, "\t.comm\t%s, %d, 32\n", node->strval, 4*node->ival);
      } // end inner if/else

       } else if (node->valType == T_INT && node->ival == 0) {

      // parameter variable reference
      sym = findSymbol(table, node->strval);
          fprintf(out, "\t.comm\t%s, 4, 4\n", node->strval);

       } else if (node->valType == T_INT && node->ival < 0) {

      // local variable reference
      fprintf(out, "\t.comm\t%s, 4, 4\n", node->strval);

       } else if (node->valType == T_STRING) {
          fprintf(out, "\t.comm\t%s, 4, 4\n", node->strval);
       } else {
          fprintf(out,"\tUNKNOWN TYPE: %d\n", node->valType);
       } // end if/elseif/if
      break;

   case AST_FUNCTION:
      puts("");
      fprintf(out,"\t.text\n\t.global\t%s\n\t.type\t%s, @function\n%s:\n\tpushq\t%%rbp\n\tmovq\t%%rsp, %%rbp\n\n",node->strval, node->strval, node->strval); // function name
      genCodeFromASTree(node->child[0],count+1,out); // child 0 is param list
      genCodeFromASTree(node->child[2],count+1,out); // child 2 is local vars
      genCodeFromASTree(node->child[1],count+1,out); // child 1 is body (stmt list)

      if(strcmp(node->strval, "main") == 0){
         // there was a change here about the leave and rbp shit 
         fprintf(out, "\n\tmovl\t$0, %%eax\n\tleave\n\tret\n\n");
      }// end of if
      else{
         fprintf(out,"\n\tpopq\t%%rbp\n\tret\n\n");
      }// end of else
      break;

   case AST_SBLOCK:
      fprintf(out,"Statement block\n");
      genCodeFromASTree(node->child[0],count+1,out);  // child 0 is statement list
      break;
   case AST_FUNCALL:
      genCodeFromASTree(node->child[0],count+1,out);  // child 0 is argument list
      fprintf(out,"\tcall\t%s\n",node->strval); // func name
      argnum = 0;
      break;
   case AST_ASSIGNMENT:

      genCodeFromASTree(node->child[0],count+1,out);  // child 0 is right hand side in any assignment
 
        if (node->varKind == V_GLARRAY) {
       fprintf(out, "\tpushq\t%%rax\n");    // saving the right hand side

           genCodeFromASTree(node->child[1], count+1, out);
           fprintf(out, "\tpopq\t%%rcx\n");
           fprintf(out, "\tcltq\n\tmovl\t%%ecx, %s(,%%rax,4)\n", node->strval); // convert 32->64 bit value (eax->rax)
        } // end if

        else if (node->ival == 0) {
       // =0 indicates that it is a global variable element
       fprintf(out, "\tmovl\t%%eax,\t%s(%%rip)\n", node->strval);
       } else if (node->ival < 0) {
       // <0 indicates that it is a local variable element
       fprintf(out, "\tmovl\t%%eax,\t%d(%%rbp)\n", node->ival); // reference to space on the stack where locVal is stored
       } else if (node->ival > 0) {
       // >0 indicates that it is a parameter element
       // fprintf(out, "AST_ASSIGNMENT: Parameter");
           fprintf(out, "\tmovq\t%%rax,\t%%%s\n", argregs[node->ival]); // rax == eax
       } // end if/else if
      break;

   case AST_ARGUMENT:
      // fprintf(out,"Funcall argument\n");
      genCodeFromASTree(node->child[0],count+1,out);  // child 0 is argument expr
      fprintf(out, "\tmovq\t%%rax,\t%s\n", argregs[argnum]);
      argnum++;  
      break;
   case AST_WHILE:
      a = getUniqueLabelID();
      b = getUniqueLabelID();

      // fprintf(out,"While loop\n");

      fprintf(out, "\tjmp LL%d\n", b);
      fprintf(out, "LL%d:\n", a);
      genCodeFromASTree(node->child[1],count+1,out);  // child 1 is loop body
      fprintf(out, "LL%d:\n", b);
      genCodeFromASTree(node->child[0],a,out);  // child 0 is condition expr
      // fprintf(out, "%s\tLL%d\n", instr, a); //supposed to be done in the relexpr
      break;
   case AST_IFTHEN:
      // fprintf(out,"If then\n");
      a = getUniqueLabelID();
      b = getUniqueLabelID();

      genCodeFromASTree(node->child[0],a,out);  // child 0 is condition expr
      // fprintf(out, "\t%s\tLL%d\n", instr, a); is done in the relexpr

      genCodeFromASTree(node->child[2],count+1,out);  // child 1 is if body
      fprintf(out, "\tjmp\tLL%d\n", b);
      fprintf(out, "LL%d:\n", a);

      genCodeFromASTree(node->child[1],count+1,out);  // child 2 is else body
      fprintf(out, "LL%d:\n", b);
      break;
   case AST_EXPRESSION: // only for binary op expression
      genCodeFromASTree(node->child[0],count+1,out);  // child 0 is left side
      fprintf(out,"\n\tpushq\t%%rax");
      if(node -> ival == '+'){
         genCodeFromASTree(node->child[1],count+1,out);  // child 1 is right side
         fprintf(out, "\tpopq\t%%rbx\n\taddl\t%%ebx,\t%%eax\n");
         break;
      }// end of if 
      if(node -> ival == '-'){
         genCodeFromASTree(node->child[1],count+1,out);  // child 1 is right side
         fprintf(out, "\tpopq\t%%rbx\n\tsubl\t%%ebx,\t%%eax\n");
         break;
      }// end of if 
      break;
   case AST_RELEXPR:
        // fprintf(out,"Relational Expression (op %d,%c)\n",node->ival,node->ival);
       genCodeFromASTree(node->child[0],0,out);  // child 0 is left side
       fprintf(out,"\tpushq\t%%rax\n");
       genCodeFromASTree(node->child[1],0,out);  // child 1 is right side
       fprintf(out,"\tpopq\t%%rcx\n");
       fprintf(out,"\tcmpl\t%%eax,%%ecx\n");
       switch (node->ival) {
         case '<': instr = "jl"; break;
         case '>': instr = "jg"; break;
         case '!': instr = "jne"; break;
         case '=': instr = "je"; break;
         default: instr = "unknown relop";
       }
       fprintf(out,"\t%s\tLL%d\n",instr,count);
       break;
   case AST_VARREF:
      // fprintf(out,"\n\tmovl\t%s (%%rip),\t%%eax",node->strval); // var name
      if (node->varKind == V_GLARRAY) {
            genCodeFromASTree(node->child[0], count+1, out);
            fprintf(out, "\tcltq\n\tmovl\t%s(,%%rax,4),\t%%eax\n", node->strval);
         } // end if

         else if (node->ival == 0) {
            // =0 indicates that it is a global variable element
            fprintf(out, "\n\tmovl\t%s(%%rip),\t%%eax\n", node->strval);
        } else if (node->ival < 0) {
            // <0 indicates that it is a local variable element
            fprintf(out, "\tmovl\t%d(%%rbp),\t%%eax\n", node->ival); // reference to space on the stack where locVal is stored
        } else if (node->ival > 0) {
            // >0 indicates that it is a parameter element
        // fprintf(out, "AST_VARREF: Parameter");
            fprintf(out, "\tmovq\t%%%s,\t%%rax\n", argregs[node->ival]); // rax == eax
        } // end if/else if
      break;

   case AST_CONSTANT: // for both int and string constants
      if (node->valType == T_INT){
         fprintf(out,"\n\tmovl\t$%d, %%eax\n", node->ival);
      }
      else if (node->valType == T_STRING){
         fprintf(out,"\n\tmovl\t$.LC%d, %%eax\n",node -> ival);
      }
      else
         fprintf(out,"Unknown Constant\n");
      
      break;
   default:
      fprintf(out,"Unknown AST node!\n");
  }
  // IMPORTANT: walks down sibling list (for nodes that form lists, like
  // declarations, functions, parameters, arguments, and statements)
  genCodeFromASTree(node->next,count,out);
}
