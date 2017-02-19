/**
 * File: symtable.h
 * ----------- 
 *  This file defines a class for symbol table and scoped table table.
 *
 *  Scoped table is to hold all declarations in a nested scope. It simply
 *  uses the standard C++ map.
 *
 *  Symbol table is implemented as a vector, where each vector entry holds
 *  a pointer to the scoped table.
 */

#ifndef _H_symtable
#define _H_symtable

#include <map>
#include <vector>
#include <iostream>
#include <string.h>
#include "errors.h"

using namespace std;

class Decl;
class Stmt;

enum EntryKind {
  E_FunctionDecl,
  E_VarDecl,
};

struct Symbol {
  char *name;
  Decl *decl;
  EntryKind kind;
  int someInfo;

  Symbol() : name(NULL), decl(NULL), kind(E_VarDecl), someInfo(0) {}
  Symbol(char *n, Decl *d, EntryKind k, int info = 0) :
        name(n),
        decl(d),
        kind(k),
        someInfo(info) {}
};

struct lessStr {
  bool operator()(const char* s1, const char* s2) const
  { return strcmp(s1, s2) < 0; }
};
 
typedef map<const char *, Symbol, lessStr>::iterator SymbolIterator;

class ScopedTable {
  map<const char *, Symbol, lessStr> symbols;

  public:
    ScopedTable();
    ~ScopedTable();

    void insert(Symbol &sym); 
    void remove(Symbol &sym);
    Symbol *find(const char *name);
};
   
class SymbolTable {
  std::vector<ScopedTable *> tables;
 
  public:
    SymbolTable();
    ~SymbolTable();

    void push();
    void pop();

    void insert(Symbol &sym);
    void remove(Symbol &sym);
    Symbol *find(const char *name);

};    

class MyStack {
    vector<Stmt *> stmtStack;

  public:
    void push(Stmt *s) { stmtStack.push_back(s); }
    void pop()         { if (stmtStack.size() > 0 ) stmtStack.pop_back(); }
    bool insideLoop();
    bool insideSwitch();
};

#endif
