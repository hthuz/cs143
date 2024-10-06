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

  T* equal_elt(T elt, bool (*compare)(T,T) ) {
    for(int i = 0; i < num; i++) {
      if (compare(elts[i], elt)) {
        return &elts[i];
      }
    }
    return NULL;
  }

  // void swap(T& elt1, T& elt2) {
  //   T& tmp = elt1;
  //   elt1 = elt2;
  //   elt2 = tmp;
  // }

  void pop_elt(T elt) {
    for(int i = 0; i < num; i++) {
      if (elts[i] == elt) {
        if (i == num-1) {
          pop();
          return;
        }

        for (int j = i; j < num - 1; j++) {
          elts[j] = elts[j + 1];
        }
        break;
      }
    }
    num--;
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
    // int objectclasstag;
    // int ioclasstag;
    // int stringclasstag;
    // int intclasstag;
    // int boolclasstag;

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
    CgenNode* get_node_by_type(Symbol type);
    // Why code_method needs to be public:
    // in code_method case expr requires inheritance graph of classtable
    // so it needs to refer to classtable.
    // classtable must be initialized before case expr can access it
    // so put code_method separately instead of inside its constructor
    void code_method(); 
    void code_init();
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
    CgenNodeP get_max_tag_child() {
      Stack<CgenNodeP>* stack = new Stack<CgenNodeP>(1024);
      stack->push(this);
      int max_tag = this->class_tag;
      CgenNodeP max_child;
      CgenNodeP node;

      while (!stack->is_empty()) {
        node = stack->pop();
        if (node->get_class_tag() >= max_tag) {
          max_child = node;
          max_tag = node->get_class_tag();
        }
        for(List<CgenNode>* l = node->get_children(); l; l = l->tl()) {
          stack->push(l->hd());
        }
      }
      return max_child;
    }
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

