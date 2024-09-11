

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"


extern int semant_debug;
extern char *curr_filename;

TypeEnv *env;
ClassTable *classtable;
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}



ClassTable::ClassTable(Classes classes) : semant_errors(0), error_stream(cerr) {
    /* Fill this in */
    num_classes = 0;
    map = new SymbolTable<Symbol, Class__class>;
    map->enterscope();
    install_basic_classes();
    for(int i = classes->first(); classes->more(i); i = classes->next(i))
        add(classes->nth(i)->get_name(), classes->nth(i));
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
	class_(IO, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
	class_(Str, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat, 
								      single_Formals(formal(arg, Str)),
								      Str, 
								      no_expr()))),
			       single_Features(method(substr, 
						      append_Formals(single_Formals(formal(arg, Int)), 
								     single_Formals(formal(arg2, Int))),
						      Str, 
						      no_expr()))),
	       filename);

    add(Object, Object_class);
    add(IO, IO_class);
    add(Int, Int_class);
    add(Bool, Bool_class);
    add(Str, Str_class);



}

// Return number of scopes entered
int ClassTable::setup_env(Class_ new_class) {
    Stack<Class_>* stack = new Stack<Class_>(MAX_CLASS_NUM);
    Class_ cur_class = new_class;
    // All classes until Object (Object included)
    while(cur_class) {
        stack->push(cur_class);
        cur_class = this->get(cur_class->get_parent());
    }

    // for(int i = 0; i < stack->size(); i++) {
    //     cout << stack->at(i)->get_name()->get_string() << endl;
    // }

    int num_scopes = 0;
    while(!stack->is_empty()) {
        cur_class = stack->pop();
        env->O->enterscope();
        env->M->enterscope();
        Features features = cur_class->get_features();
        for(int i = features->first(); features->more(i); i = features->next(i)) {
            if (features->nth(i)->is_method()) {
                Method method = {cur_class->get_name(), features->nth(i)->get_name()};
                Signature* sig_ptr = new Signature;
                *sig_ptr = features->nth(i)->get_signature();
                env->M->addid(method, sig_ptr);
            } else {
                // No justification of type of identifier in this stage
                Symbol* type = new Symbol;
                *type = features->nth(i)->get_type_decl();
                env->O->addid(features->nth(i)->get_name(), type);
            }
        }
        num_scopes++;
    }

    // Method method = {IO, out_string};
    // Signature* sig_ptr = env->M->lookup(method);
    // cout << "===look up result===" << endl;
    // (*sig_ptr)->dump(cout, 0);
    // cout << (*sig_ptr)->len() << endl;
    return num_scopes;
}

void ClassTable::reset_env(int num_scopes) {
    for (int i = 0; i < num_scopes; i++) {
        env->O->exitscope();
        env->M->exitscope();
    }
}

bool ClassTable::has_cycle() {

    bool* visited = new bool[num_classes];

    int num_visited = 0;
    while(num_visited != num_classes) {
        for (int node_index = 0; node_index < num_classes; node_index++){
            if (visited[node_index])
                continue;
            Stack<Symbol>* rec_stack = new Stack<Symbol>(num_classes);
            if (this->dfs(node_index, visited, num_visited, rec_stack))
                return true;
        }
        return false;
    }
    return false;
}

bool ClassTable::dfs(int node_index, bool* visited, int& num_visited, Stack<Symbol> *rec_stack) {
    Symbol node = types[node_index];

    rec_stack->push(node);
    visited[node_index] = true;
    num_visited++;

    if (node == Object)
        return false;
    Symbol parent = get_parent(node);
    int parent_index = get_index_by_type(parent);
    if (rec_stack->has_element(parent)) {
        return true;
    }
    
    return dfs(get_index_by_type(parent), visited, num_visited, rec_stack);
}

int ClassTable::get_index_by_type(Symbol type) {
    for (int i = 0; i < num_classes; i++)
        if (types[i] == type)
            return i;
    return -1;
}

bool ClassTable::is_subtype(Symbol type, Symbol other_type) {
    if (type == SELF_TYPE) 
        type = env->C->get()->get_name();
    if (other_type == SELF_TYPE) 
        other_type = env->C->get()->get_name();
    Symbol cur = type;
    while(cur != No_class) {
        if (cur == other_type) 
            return true;
        cur = get_parent(cur);
    }
    return false;
}



////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 



/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */


Signature nil_sig() {
    return new nil_node<SigType*>();
}

Signature single_sig(Symbol s) {
    return new single_list_node<SigType*>(new SigType(s));
}

Signature append_sig(Signature s1, Signature s2) {
    return new append_node<SigType*>(s1, s2);
}

Signature* MethodEnv::lookup(Method method) {
    Symbol cur_class_name = method.class_name;
    Method new_method = method;
    while (cur_class_name != Object) {
        new_method.class_name = cur_class_name;
        Signature* sig_ptr = SymbolTable<Method, Signature>::lookup(new_method);
        if (sig_ptr != NULL) {
            return sig_ptr;
        }
        // keep searching
        cur_class_name = classtable->get_parent(cur_class_name);
    }
    return NULL;
}


void mytest() {
    Signature sig =  nil_sig();
    sig = append_sig(sig, single_sig(Object));
    sig = append_sig(sig, single_sig(Int));
    sig->dump(cout, 0);

    int i = 0;
    for (i = sig->first(); sig->more(i); i = sig->next(i)) {
        cout << sig->nth(i)->get_type()->get_string() << " ";
    }
    cout << endl;
    cout << sig->nth(sig->len()-1)->get_type()->get_string() << endl;

    // SymbolTable<Method, Signature>* map = new SymbolTable<Method, Signature>;
    // map->enterscope();
    // Method method = {Object, Int};
    // Method method2 = {Object, Object};
    // map->addid(method, &sig);


}

void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    classtable = new ClassTable(classes);

    if (classtable->has_cycle()) {
        classtable->semant_error() << "classes form a cycle" << endl;
	    cerr << "Compilation halted due to static semantic errors." << endl;
	    exit(1);
    }

    /* some semantic analysis code may go here */
    env = new TypeEnv();
    for(int i = classes->first(); classes->more(i); i = classes->next(i)) {
        int num_scopes = classtable->setup_env(classes->nth(i));
        classes->nth(i)->semant();
        classtable->reset_env(num_scopes);
    }

    if (classtable->errors()) {
	cerr << "Compilation halted due to static semantic errors." << endl;
	exit(1);
    }
}

void class__class::semant()
{
    env->C->set(this);
    for (int i = features->first(); features->more(i); i = features->next(i))
        features->nth(i)->semant();
}

void method_class::semant()
{
    env->O->enterscope();
    env->O->addid(self, &SELF_TYPE);

    // Method method = {env->C->get()->get_name(), this->name};
    // Signature* sig_ptr = env->M->lookup(method);
    // if (sig_ptr == NULL) {
    //     classtable->semant_error(env->C->get_filename(), this) 
    //     << "method has invalid type";
    //     return;
    // }
    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
        formals->nth(i)->semant();
    }

    expr->semant();
    if (!classtable->is_subtype(expr->get_type(), return_type)) {
        classtable->semant_error(env->C->get_filename(), this)
        << "Inferred return type "
        << expr->get_type()->get_string()
        << " of method "
        << this->get_name()->get_string()
        << " does not conform to declared return type "
        << return_type->get_string()
        << ".\n";
        return;
    }
    env->O->exitscope();

}

void attr_class::semant()
{
    if (name == self) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "'" << name << "'"
        << " cannot be the name of an attribute.\n";
        return;
    }
    if (init->is_no_expr()){
    } else {
        Symbol class_symbol = env->C->get()->get_name();
        init->semant();
        if (!classtable->is_subtype(init->get_type(), type_decl)) {
            classtable->semant_error(env->C->get_filename(), this)
            << "attr " 
            << name << " : " << type_decl
            << " has invalid type\n";
            return;
        }
    }
}

void formal_class::semant() {
    env->O->addid(name, &type_decl);
}

// Expressions

void assign_class::semant() {
    Symbol* id_type = env->O->lookup(name);
    if (id_type == NULL) {
        classtable->semant_error(env->C->get_filename(), this)
        << "assign has invalid type\n";
        type = Object;
        return;
    }
    expr->semant();
    // cout << (*id_type)->get_string() << endl;
    if (!classtable->is_subtype(expr->get_type(), *id_type)) {
        classtable->semant_error(env->C->get_filename(), this)
        << "assign has invalid type\n";
        type = Object;
        return;
    }
    type = expr->get_type();
}

void string_const_class::semant(){
    type = Str;
}

void bool_const_class::semant() {
    type = Bool;
}

void int_const_class::semant() {
    type = Int;
}

void object_class::semant() {
    Symbol* name_type = env->O->lookup(name);
    if (name_type == NULL) {
        classtable->semant_error() << "object " << name << " has invalid type\n";
        type = Object;
        return;
    }
    type = *name_type;
}

void new__class::semant() {
    if (type_name == SELF_TYPE){
        type = env->C->get()->get_name();
        return;
    }
    type = type_name;
}

void dispatch_class::semant() {
    expr->semant();
    for(int i = actual->first(); actual->more(i); i = actual->next(i)) {
        actual->nth(i)->semant();
    }
    Symbol expr_type = expr->get_type();
    if (expr_type == SELF_TYPE) 
        expr_type = env->C->get()->get_name();

    Method method = {expr_type, name};
    Signature* sig_ptr = env->M->lookup(method);
    if (sig_ptr == NULL) {
        classtable->semant_error(env->C->get_filename(), this)
        << "Dispatch to undefined method "
        << name
        << ".\n";
        type = Object;
        return;
    }

    Signature sig = *sig_ptr;
    for(int i = actual->first(); actual->more(i); i = actual->next(i)) {
        if (!classtable->is_subtype(actual->nth(i)->get_type(), sig->nth(i)->get_type())) {
            classtable->semant_error(env->C->get_filename(), this)
            << "In call of method "
            << name->get_string()
            << ", type "
            << actual->nth(i)->get_type()
            << " of parameter arg does not conform to declared type "
            << sig->nth(i)->get_type()
            << ".\n";
            type = Object;
            return;
        }
    }
    if (sig->nth(sig->len()-1)->get_type() == SELF_TYPE)
        type = expr->get_type();
    else
        type = sig->nth(sig->len()-1)->get_type();
}

void static_dispatch_class::semant() {
    expr->semant();
    for(int i = actual->first(); actual->more(i); i = actual->next(i)) {
        actual->nth(i)->semant();
    }
    if (!classtable->is_subtype(expr->get_type(), type_name)) {
        classtable->semant_error() << "statis dispatch has invalid type\n";
        type = Object;
        return ;
    }
    Method method = {type_name, name};
    Signature* sig_ptr = env->M->lookup(method);
    if (sig_ptr == NULL) {
        classtable->semant_error(env->C->get_filename(), this) << "static dispatch has invalid signature";
        type = Object;
        return;
    }

    Signature sig = *sig_ptr;
    for(int i = actual->first(); actual->more(i); i = actual->next(i)) {
        if (!classtable->is_subtype(actual->nth(i)->get_type(), sig->nth(i)->get_type())) {
            classtable->semant_error(env->C->get_filename(), this)
            << "In call of method "
            << name->get_string()
            << ", type "
            << actual->nth(i)->get_type()
            << " of parameter arg does not conform to declared type "
            << sig->nth(i)->get_type()
            << ".\n";
            type = Object;
            return;
        }
    }
    if (sig->nth(sig->len()-1)->get_type() == SELF_TYPE)
        type = expr->get_type();
    else
        type = sig->nth(sig->len()-1)->get_type();
}

void cond_class::semant() {
    pred->semant();
    then_exp->semant();
    else_exp->semant();
    if (pred->get_type() != Bool)  {
        classtable->semant_error() << "cond pred not type bool\n";
        type = Object;
        return;
    }

    type = classtable->join_type(then_exp->get_type(), else_exp->get_type());
}

void block_class::semant() {
    for (int i = body->first(); body->more(i); i = body->next(i)) {
        body->nth(i)->semant();
    }
    type = body->nth(body->len()-1)->get_type();
}

void let_class::semant() {
    if (type_decl == SELF_TYPE)
        type_decl = env->C->get()->get_name();
    if (init != no_expr()) {
        init->semant();
        if (classtable->is_subtype(init->get_type(), type_decl)) {
            classtable->semant_error() << "let has invalid type\n";
            type = Object;
            return;
        }
    }
    env->O->enterscope();
    env->O->addid(identifier, &type_decl);
    body->semant();
    env->O->exitscope();
    type = body->get_type();
}

// Symbol typcase_class::semant() {
//     Symbol expr_type = expr->semant();

//     Symbol case_type = NULL;
//     for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
//         branch_class* branch = dynamic_cast<branch_class*>(cases->nth(i));
//         if (!branch) {
//             printf("dynamic cast fail\n");
//         }
//         branch->semant(&case_type);
//     }
//     return case_type;
// }
void typcase_class::semant() {
    type = Object;
}


// // Modify case_type
// Symbol branch_class::semant(Symbol* case_type)  {
//     env->O->enterscope();
//     env->O->add(branch->name, branch->type_decl);
//     case_type = classtable->join_type(*case_type, branch->expr->semant());
//     env->O->exitscope();
//     return NULL;
// }

void loop_class::semant() {
    pred->semant();
    body->semant();
    if (pred->get_type() != Bool) {
        classtable->semant_error() << "loop pred not type bool\n";
        type = Object;
        return;
    }
    type = Object;
    return;
}

void isvoid_class::semant() {
    e1->semant();
    type = Bool;
    return;
}

void neg_class::semant() {
    e1->semant();
    if (e1->get_type() != Bool) {
        classtable->semant_error() << "neg expr has invalid type\n";
        type = Object;
        return;
    }
    type = Bool;
    return;
}

void plus_class::semant() {
    e1->semant();
    e2->semant();
    if (e1->get_type() != Int) {
        classtable->semant_error() << "plus expr has invalid type\n";
        type = Object;
        return;
    }
    if (e2->get_type() != Int) {
        classtable->semant_error() << "plus expr has invalid type\n";
        type = Object;
        return;
    }
    type = Int;
    return;
}

void sub_class::semant() {
    e1->semant();
    e2->semant();
    if (e1->get_type() != Int) {
        classtable->semant_error() << "sub expr has invalid type\n";
        type = Object;
        return;
    }
    if (e2->get_type() != Int) {
        classtable->semant_error() << "sub expr has invalid type\n";
        type = Object;
        return;
    }
    type = Int;
    return;
}

void mul_class::semant() {
    e1->semant();
    e2->semant();

    if (e1->get_type() != Int) {
        classtable->semant_error() << "mul expr has invalid type\n";
    }
    if (e2->get_type() != Int) {
        classtable->semant_error() << "mul expr has invalid type\n";
        type = Object;
        return;
    }
    type = Int;
    return;
}

void divide_class::semant() {
    e1->semant();
    e2->semant();
    if (e1->get_type() != Int) {
        classtable->semant_error() << "div expr has invalid type\n";
        type = Object;
        return;
    }
    if (e2->get_type() != Int) {
        classtable->semant_error() << "div expr has invalid type\n";
        type = Object;
        return;
    }
    type = Int;
}

void lt_class::semant() {
    e1->semant();
    e2->semant();
    if (e1->get_type() != Int) {
        classtable->semant_error() << "lt expr has invalid type\n";
        type = Object;
        return;
    }
    if (e2->get_type() != Int) {
        classtable->semant_error() << "lt expr has invalid type\n";
        type = Object;
        return;
    }
    type = Bool;
    return;
}

void eq_class::semant() {
    e1->semant();
    e2->semant();
    if (e1->get_type() != Int) {
        classtable->semant_error() << "eq expr has invalid type\n";
        type = Object;
        return;
    }
    if (e2->get_type() != Int) {
        classtable->semant_error() << "eq expr has invalid type\n";
        type = Object;
        return;
    }
    type = Bool;
}

void leq_class::semant() {
    e1->semant();
    e2->semant();
    if (e1->get_type() != Int) {
        classtable->semant_error() << "leq expr has invalid type\n";
        type = Object;
        return;
    }
    if (e2->get_type() != Int) {
        classtable->semant_error() << "leq expr has invalid type\n";
        type = Object;
        return;
    }
    type = Bool;
}

// aka not
void comp_class::semant() {
    e1->semant();
    if (e1->get_type() != Bool) {
        classtable->semant_error() << "not expr has invalid type\n";
        type = Object;
        return;
    }
    type = Bool;
}

void no_expr_class::semant() {
    type = Object;
}

// Symbol eq_class::semant() {
//     Symbol e1_type = e1->semant();
//     Symbol e2_type = e2->semant();
//     if ((e1_type == Int || e2_type == Int) && e1_type != e2_type) {
//         classtable->semant_error() << "invalid type comparsion";
//         return Object;
//     }
//     if ((e1_type == Str || e2_type == Str) && e1_type != e2_type) {
//         classtable->semant_error() << "invalid type comparsion";
//         return Object;
//     }
//     if ((e1_type == Bool || e2_type == Bool) && e1_type != e2_type) {
//         classtable->semant_error() << "invalid type comparsion";
//         return Object;
//     }
//     return Bool;
// }