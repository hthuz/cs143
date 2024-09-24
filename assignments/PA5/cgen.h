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

    // The following creates an inheritance graph from
    // a list of classes.  The graph is implemented as
    // a tree of `CgenNode', and class names are placed
    // in the base class symbol table.

    void install_basic_classes();
    void install_class(CgenNodeP nd);
    void install_classes(Classes cs);
    void build_inheritance_tree();
    void set_relations(CgenNodeP nd);
    

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
             CgenClassTableP class_table,
             int class_tag);

    void add_child(CgenNodeP child);
    List<CgenNode> *get_children() { return children; }
    void set_parentnd(CgenNodeP p);
    CgenNodeP get_parentnd() { return parentnd; }
    int basic() { return (basic_status == Basic); }
    int get_class_tag() {return class_tag;}
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
