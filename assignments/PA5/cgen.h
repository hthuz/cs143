#include <assert.h>
#include <stdio.h>
#include "emit.h"
#include "cool-tree.h"
#include "symtab.h"

enum Basicness
{
    Basic,
    NotBasic
};
#define TRUE 1
#define FALSE 0

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
  // For debugging
  T at(int index) {
    return elts[index];
  }
  int size() {
    return num;
  }
  Stack<T>* copy() {
    Stack<T>* stack = new Stack<T>(this->num);
    for (int i = 0; i < this->num; i++) {
      stack->push(this->elts[i]);
    }
    return stack;
  }

};

class CgenClassTable;
typedef CgenClassTable *CgenClassTableP;

class CgenNode;
typedef CgenNode *CgenNodeP;

class CgenClassTable : public SymbolTable<Symbol, CgenNode>
{
private:
    List<CgenNode> *nds;
    ostream &str;
    int total_class_tag_num;
    int objectclasstag;
    int ioclasstag;
    int stringclasstag;
    int intclasstag;
    int boolclasstag;

    // The following methods emit code for
    // constants and global declarations.

    void code_global_data();
    void code_global_text();
    void code_bools(int);
    void code_select_gc();
    void code_constants();
    void code_class_nameTab();
    void code_class_objTab();
    void code_dispTab();
    void code_protObj();
    void code_init();
    void code_method();

    // The following creates an inheritance graph from
    // a list of classes.  The graph is implemented as
    // a tree of `CgenNode', and class names are placed
    // in the base class symbol table.

    void install_basic_classes();
    void install_class(CgenNodeP nd);
    void install_classes(Classes cs);
    void build_inheritance_tree();
    void set_relations(CgenNodeP nd);
    void set_class_tag();
    

    // auxiliary methods
    CgenNode* get_node_by_class_tag(int class_tag);

public:
    CgenClassTable(Classes, ostream &str);
    void code();
    CgenNodeP root();
};

class CgenNode : public class__class
{
private:
    CgenNodeP parentnd;       // Parent of class
    List<CgenNode> *children; // Children of class
    Basicness basic_status;   // `Basic' if class is basic
                              // `NotBasic' otherwise
    int class_tag;

public:
    CgenNode(Class_ c,
             Basicness bstatus,
             CgenClassTableP class_table
            );

    void add_child(CgenNodeP child);
    List<CgenNode> *get_children() { return children; }
    void set_parentnd(CgenNodeP p);
    CgenNodeP get_parentnd() { return parentnd; }
    int basic() { return (basic_status == Basic); }

    int get_class_tag() {return class_tag;}
    void set_class_tag(int tag) {class_tag = tag;}
    int get_size();
    void code_dispTab(ostream& s);
    void code_protObj(ostream& s);
    void code_init(ostream& s);
    void code_method(ostream& s);
};

class BoolConst
{
private:
    int val;

public:
    BoolConst(int);
    void code_def(ostream &, int boolclasstag);
    void code_ref(ostream &) const;
};

