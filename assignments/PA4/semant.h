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




class MethodEnv{
private:

public:
  void enterscope() {

  };
  void exitscope() {

  };
};
// class MethodEnv{
// private:
//   SymbolTable<Method, Signature> *map;
// public:
//   MethodEnv() {
//     map = new SymbolTable<Method, Signature>();
//   }
//   void add(Symbol class_name, Symbol method_name, Signature sig) {
//     Method method = {class_name, method_name};
//     map->addid(method, &sig);
//   }
//   bool has(Symbol class_name, Symbol method_name) {
//     Method method = {class_name, method_name};
//     // TODO comparsion of signature
//     // return map->lookup(method) != NULL;
//     return false;
//   }
//   void enterscope() {
//     map->enterscope();
//   }
//   void exitscope() {
//     map->exitscope();
//   }
//   // TODO: to be implemented
//   Signature get(Symbol class_name, Symbol method_name) {
//     Method method = {class_name, method_name};
//     // Signature* sig = map->lookup(method);
//     // if (sig == NULL)
//     //   return NULL;
//     return NULL;
//   };

// };

class ObjectEnv{
private:
  SymbolTable<Symbol, Symbol> *map; // Identifier => Type

public:
  ObjectEnv() {
    map = new SymbolTable<Symbol, Symbol>();
  }
  void add(Symbol identifier, Symbol type){
    map->addid(identifier, &type);
  }
  // If var `identifier` has type 
  bool has(Symbol identifier) {
    return map->lookup(identifier) != NULL;
  }

  void enterscope() {
    map->enterscope();
  }
  void exitscope() {
    map->exitscope();
  }
  Symbol get(Symbol identifier){
    Symbol* type = map->lookup(identifier);
    if (type == NULL)
      return NULL;
    return *type;
  }

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

