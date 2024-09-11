#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#define TRUE 1
#define FALSE 0
#define MAX_CLASS_NUM 1024

class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

template <typename T>
class Stack {
private:
  T* elts;
  int  num;
public:
  Stack(int max_size) {
    elts  = new T[max_size];
    num = 0;
  }
  void push(T elt) {
    elts[num++] = elt;
  }
  T pop() {
    return elts[--num];
  }
  bool is_empty() {
    return num == 0;
  }
  bool has_element(T elt) {
    for (int i = 0; i < num; i++) 
      if (elts[i] == elt)
        return true;
    return false;
  }

};


typedef struct Method {
  Symbol class_name;
  Symbol method_name;
  bool operator==(const Method& other) const{
    return class_name == other.class_name && method_name == other.method_name;
  }
} Method;


class ClassTable {
private:
  int semant_errors;
  int num_classes;
  void install_basic_classes();
  int get_index_by_type(Symbol type);
  bool dfs(int, bool*, int*, Stack<Symbol>*);
  ostream& error_stream;
  SymbolTable<Symbol, Class__class> *map; // symbol => class
  Symbol types[MAX_CLASS_NUM];

public:
  ClassTable(Classes classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
  int setup_env(Class_ new_class);
  void reset_env(int num_scopes);
  bool has_cycle();

  Class__class* get(Symbol type) {
    return map->lookup(type);
  };
  // Add Symbol => Class
  void add(Symbol type, Class_ new_class) {
    map->addid(type, new_class);
    types[num_classes] = type;
    num_classes++;
  };
  Symbol get_parent(Symbol type) {
    return get(type)->get_parent();
  };
  // Whether type is a subtype of other_type
  bool is_subtype(Symbol type, Symbol other_type);
  // the least type C such that A < C and B < C
  Symbol join_type(Symbol type, Symbol other_type) {

    if (!type)
      return other_type;
    if (!other_type)
      return type;

    Symbol cur = type;
    while(true) {
      if (is_subtype(other_type, cur))
        return cur;
      cur = get_parent(cur);
    }
    printf("join_type error\n");
    return NULL;
  }
};




// (Class, method) => signature (arguments and return type)
class MethodEnv : public SymbolTable<Method, Signature>{
public:
  Signature* lookup(Method m);
};

// Idetifier => Type
class ObjectEnv : public SymbolTable<Symbol, Symbol>{
};

class ClassEnv{
private:
  class__class* cur_class;
public:
  ClassEnv() {
    cur_class = NULL;
  }
  void set(class__class* new_class) {
    cur_class = new_class;
  }
  class__class* get(){
    return cur_class;
  }

};

class TypeEnv{
public:
  ObjectEnv* O;
  MethodEnv* M;
  ClassEnv* C;
  TypeEnv() {
    O = new ObjectEnv();
    M = new MethodEnv();
    C = new ClassEnv();
  }
};

#endif

