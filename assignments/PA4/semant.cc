

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"


extern int semant_debug;
extern char *curr_filename;

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

// bool ClassTable::has_cycle() {

//     bool* visited = new bool[num_classes];

//     int num_visited = 0;
//     while(num_visited != num_classes) {
//         for (int node_index = 0; node_index < num_classes; node_index++){
//             if (visited[node_index])
//                 continue;
//             Stack<Symbol>* rec_stack = new Stack<Symbol>(MAX_CLASS_NUM);
//             if (this->dfs(node_index, visited, &num_visited, &rec_stack))
//                 return true;
//         }
//         return false;
//     }
// }

bool ClassTable::dfs(int node_index, bool* visited, int *num_visited, Stack<Symbol> *rec_stack) {
    Symbol node = types[node_index];
    if (node == No_class)
        return false;
    rec_stack->push(node);
    visited[node_index] = true;
    num_visited++;

    Symbol parent = get_parent(node);
    if (visited[node_index])
        return false;
    if (rec_stack->has_element(parent))
        return true;
    
    return dfs(get_index_by_type(parent), visited, num_visited, rec_stack);
}

int ClassTable::get_index_by_type(Symbol type) {
    for (int i = 0; i < num_classes; i++)
        if (types[i] == type)
            return i;
    return -1;
}

bool ClassTable::is_subtype(Symbol type, Symbol other_type) {
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

TypeEnv *env;
ClassTable *classtable;

Signature nil_sig() {
    return new nil_node<SigType*>();
}

Signature single_sig(Symbol s) {
    return new single_list_node<SigType*>(new SigType(s));
}

Signature append_sig(Signature s1, Signature s2) {
    return new append_node<SigType*>(s1, s2);
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
    // cout << sig->nth(i)->get_type()->get_string();

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
    mytest();
    cout << "=====\n";
    return;

    // if (classtable->has_cycle()) {
    //     classtable->semant_error() << "classes form a cycle" << endl;
    //     exit(1);
    // }

    /* some semantic analysis code may go here */
    env = new TypeEnv();
    for(int i = classes->first(); classes->more(i); i = classes->next(i))
        classes->nth(i)->semant();

    if (classtable->errors()) {
	cerr << "Compilation halted due to static semantic errors." << endl;
	exit(1);
    }
}

void class__class::semant()
{
    env->C->set(this);
    env->O->enterscope();
    env->M->enterscope();
    for (int i = features->first(); features->more(i); i = features->next(i))
        features->nth(i)->semant();
    env->O->exitscope();
    env->M->exitscope();
}

// Features
void method_class::semant()
{
    env->O->enterscope();
    env->O->add(self, env->C->get()->get_name());
    // Signature sig = new list_node<Symbol>;
    // TODO: Type checking
    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
        // formals->nth(i)->semant(sig);
    }

    Symbol expr_type = expr->semant();
    if (expr_type == SELF_TYPE) {
        if (!classtable->is_subtype(expr_type, env->C->get()->get_name())) {
            classtable->semant_error() << "method has invalid type";
            return;
        }
    } else {
        if (!classtable->is_subtype(expr_type, return_type)) {
            classtable->semant_error() << "method has invalid type";
            return;
        }
    }
    env->O->exitscope();
    // sig = sig->append(sig, single_list_node<Symbol>(return_type));
    // Signature sig;
    // env->M->add(env->C->get()->get_name(), name, sig);

    return;
}

void attr_class::semant()
{
    if (init == no_expr()){
        env->O->add(name, type_decl);
    } else {
        // TODO, TYPE checking
        env->O->enterscope();
        env->O->add(self, env->C->get()->get_name());
        Symbol init_type = init->semant();
        if (!classtable->is_subtype(init->semant(), type_decl)) {
            classtable->semant_error() << "attr has invalid type";
            return;
        }
        env->O->exitscope();
        env->O->add(name, type_decl);
    }
}

void formal_class::semant(Signature sig) {
    return;
}
// Formals
// void formal_class::semant(Signature sig)
// {
//     env->O->add(name, type_decl);
//     sig = sig.append(sig, single_list_node<Symbol>(type_decl));
// }

// Expressions

Symbol assign_class::semant() {
    Symbol id_type = env->O->get(name);
    Symbol expr_type = expr->semant();
    if (!classtable->is_subtype(expr_type, id_type)) {
        classtable->semant_error() << "assign has invalid type";
        return Object;
    }
    return expr_type;
}

Symbol string_const_class::semant(){
    return Str;
}

Symbol bool_const_class::semant() {
    return Bool;
}

Symbol int_const_class::semant() {
    return Int;
}

Symbol object_class::semant() {
    Symbol name_type = env->O->get(name);
    if (name_type == NULL) {
        classtable->semant_error() << name << " has invalid type\n";
        return Object;
    }
    return name_type;
}

Symbol new__class::semant() {
    Symbol new_type = type_name;
    if (new_type == SELF_TYPE)
        return env->C->get()->get_name();
    return new_type;
}

Symbol dispatch_class::semant() {
    Symbol expr_type = expr->semant();
    for(int i = actual->first(); actual->more(i); i = actual->next(i)) {
        actual->nth(i)->semant();
    }
    if (expr_type == SELF_TYPE) 
        expr_type = env->C->get()->get_name();

    // Signature sig = env->M->get(expr_type, name);
    // if (sig == NULL) {
    //     classtable->semant_error() << "dispatch has invalid signature";
    //     return Object;
    // }
    // int i;
    // for(i = actual->first(); actual->more(i); i = actual->next(i)) {
    //     if (!classtable->is_subtype(actual->nth(i)->semant(), sig->nth(i))) {
    //         classtable->semant_error() << "dispatch violates subtype inheritance";
    //         return Object;
    //     }
    // }
    // if (sig->nth(i) == SELF_TYPE)
        // return expr_type;
    // return sig->nth(i);
    return NULL;

}

Symbol static_dispatch_class::semant() {
    Symbol expr_type = expr->semant();
    for(int i = actual->first(); actual->more(i); i = actual->next(i)) {
        actual->nth(i)->semant();
    }
    if (!classtable->is_subtype(expr_type, type_name)) {
            classtable->semant_error() << "statis dispatch has invalid type";
            return Object;
    }
    // Signature sig = env->M->get(type_name, name);
    // int i = 0;
    // for(i = actual->first(); actual->more(i); i = actual->next(i)) {
    //     if (!classtable->is_subtype(actual->nth(i)->semant(), sig->nth(i))) {
    //         classtable->semant_error() << "static dispatch violates subtype inheritance";
    //         return Object;
    //     }
    // }
    // if (sig->nth(i) == SELF_TYPE)
    //     return expr_type;
    // return sig->nth(i);
    return Object;

}

Symbol cond_class::semant() {
    Symbol pred_type = pred->semant();
    Symbol then_type = then_exp->semant();
    Symbol else_type = else_exp->semant();
    if (pred_type != Bool)  {
        classtable->semant_error() << "cond pred not type bool";
        return Object;
    }

    return classtable->join_type(then_type, else_type);
}

Symbol block_class::semant() {
    int i;
    for (i = body->first(); body->more(i); i = body->next(i)) {
        body->nth(i)->semant();
    }
    return body->nth(i)->semant();
}

Symbol let_class::semant() {
    if (type_decl == SELF_TYPE)
        type_decl = env->C->get()->get_name();
    if (init != no_expr()) {
        Symbol init_type = init->semant();
        if (classtable->is_subtype(init_type, type_decl)) {
            classtable->semant_error() << "let has invalid type";
            return Object;
        }
    }
    env->O->enterscope();
    env->O->add(identifier, type_decl);
    Symbol body_type = body->semant();
    env->O->exitscope();
    return body_type;
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
Symbol typcase_class::semant() {
    return Object;
}


// // Modify case_type
// Symbol branch_class::semant(Symbol* case_type)  {
//     env->O->enterscope();
//     env->O->add(branch->name, branch->type_decl);
//     case_type = classtable->join_type(*case_type, branch->expr->semant());
//     env->O->exitscope();
//     return NULL;
// }

Symbol loop_class::semant() {
    Symbol pred_type = pred->semant();
    Symbol body_type = body->semant();
    if (pred_type != Bool) {
        classtable->semant_error() << "loop pred not type bool";
        return Object;
    }
    return Object;
}

Symbol isvoid_class::semant() {
    e1->semant();
    return Bool;
}

Symbol neg_class::semant() {
    if (e1->semant() != Bool) {
        classtable->semant_error() << "neg expr has invalid type";
        return Object;
    }
    return Bool;
}

Symbol plus_class::semant() {
    if (e1->semant() != Int) {
        classtable->semant_error() << "plus expr has invalid type";
        return Object;
    }
    if (e2->semant() != Int) {
        classtable->semant_error() << "plus expr has invalid type";
        return Object;
    }
    return Int;
}

Symbol sub_class::semant() {
    if (e1->semant() != Int) {
        classtable->semant_error() << "sub expr has invalid type";
        return Object;
    }
    if (e2->semant() != Int) {
        classtable->semant_error() << "sub expr has invalid type";
        return Object;
    }
    return Int;
}

Symbol mul_class::semant() {
    if (e1->semant() != Int) {
        classtable->semant_error() << "mul expr has invalid type";
        return Object;
    }
    if (e2->semant() != Int) {
        classtable->semant_error() << "mul expr has invalid type";
        return Object;
    }
    return Int;
}

Symbol divide_class::semant() {
    if (e1->semant() != Int) {
        classtable->semant_error() << "div expr has invalid type";
        return Object;
    }
    if (e2->semant() != Int) {
        classtable->semant_error() << "div expr has invalid type";
        return Object;
    }
    return Int;
}

Symbol lt_class::semant() {
    if (e1->semant() != Int) {
        classtable->semant_error() << "lt expr has invalid type";
        return Object;
    }
    if (e2->semant() != Int) {
        classtable->semant_error() << "lt expr has invalid type";
        return Object;
    }
    return Bool;
}

Symbol eq_class::semant() {
    if (e1->semant() != Int) {
        classtable->semant_error() << "eq expr has invalid type";
        return Object;
    }
    if (e2->semant() != Int) {
        classtable->semant_error() << "eq expr has invalid type";
        return Object;
    }
    return Bool;
}

Symbol leq_class::semant() {
    if (e1->semant() != Int) {
        classtable->semant_error() << "leq expr has invalid type";
        return Object;
    }
    if (e2->semant() != Int) {
        classtable->semant_error() << "leq expr has invalid type";
        return Object;
    }
    return Bool;
}

// aka not
Symbol comp_class::semant() {
    if (e1->semant() != Bool) {
        classtable->semant_error() << "not expr has invalid type";
        return Object;
    }
    return Bool;
}

Symbol no_expr_class::semant() {
    return Object;
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