

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
    for(int i = classes->first(); classes->more(i); i = classes->next(i)) {
        if (this->get(classes->nth(i)->get_name()) != NULL) {
            this->semant_error(classes->nth(i)->get_filename(), classes->nth(i)) 
            << "Class "
            << classes->nth(i)->get_name()->get_string()
            << " was previously defined.\n";
        }
        add(classes->nth(i)->get_name(), classes->nth(i));
    }
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


bool ClassTable::is_defined_type(Symbol type) {
    if (type == No_class) 
        return true;
    for(int i = 0; i < num_classes; i++)
        if (types[i] == type)
            return true;
    return false;
}

// For example, class A inherits B, but B is not defined
bool ClassTable::has_undefined_class() {
    for(int i = 0; i < num_classes; i++) {
        Class_ cur_class = this->get(types[i]);
        if (!classtable->is_defined_type(cur_class->get_parent()))  {
            classtable->semant_error(cur_class->get_filename(), cur_class) 
            << "Class " 
            << cur_class->get_name()->get_string()
            << " inherits from an undefined class "
            << cur_class->get_parent()->get_string()
            << ".\n";
            return true;
        }
    }
    return false;
}

// Can be improved
void ClassTable::setup_method_env() {
    Class_ cur_class;
    // Setup method env for all classes
    env->M->enterscope();
    for(int i = 0; i < this->num_classes; i++) {
        cur_class = this->get(types[i]);
        Features features = cur_class->get_features();
        for(int j = features->first(); features->more(j); j = features->next(j)) {
            if (features->nth(j)->is_method()) {
                Method method = {cur_class->get_name(), features->nth(j)->get_name()};


                Signature* sig_ptr = new Signature;
                *sig_ptr = features->nth(j)->get_signature();
                env->M->addid(method, sig_ptr);
            }
        }
    }

    // check for method inheritance
    for(int i = 0; i < this->num_classes; i++) {
        cur_class = this->get(types[i]);
        Features features = cur_class->get_features();
        for(int j = features->first(); features->more(j); j = features->next(j)) {
            if (!features->nth(j)->is_method())
                continue;
            Method cur_method = {types[i], features->nth(j)->get_name()};
            Signature* cur_sig_ptr = env->M->lookup(cur_method);
            Signature cur_sig = *cur_sig_ptr;
            // Check if this method appears in the parent class
            Class_ parent_class = this->get(cur_class->get_parent());
            // Search until No_class
            while(parent_class != NULL) {
                Method parent_method = {parent_class->get_name(), cur_method.method_name};
                Signature* parent_sig_ptr = env->M->lookup(parent_method);
                if (parent_sig_ptr == NULL) {
                    parent_class = this->get(parent_class->get_parent());
                    continue;
                }
                Signature parent_sig = *parent_sig_ptr;
                // Check if signatures are the same
                if (cur_sig->len() != parent_sig->len()) {
                    this->semant_error(cur_class->get_filename(), cur_class)
                    << "Incompatible number of formal parameters in redefined method "
                    << cur_method.method_name
                    << ".\n";
                    return;
                }
                for(int k = cur_sig->first(); cur_sig->more(k); k = cur_sig->next(k)) {
                    if (cur_sig->nth(k)->get_type() != parent_sig->nth(k)->get_type()) {
                        this->semant_error(cur_class->get_filename(), cur_class) 
                        << "In redefined method "
                        << cur_method.method_name->get_string()
                        << ", paramter type "
                        << cur_sig->nth(k)->get_type()->get_string()
                        << " is different from original type "
                        << parent_sig->nth(k)->get_type()->get_string()
                        << "\n";
                        return;
                    }
                }
                parent_class = this->get(parent_class->get_parent());
            }

        }

    }
}

// Return number of scopes entered
// Return -1 if any semantic erros happen
int ClassTable::setup_object_env(Class_ new_class) {
    Stack<Class_>* stack = new Stack<Class_>(MAX_CLASS_NUM);
    Class_ cur_class = new_class;
    // All classes until Object (Object included)
    while(cur_class) {
        stack->push(cur_class);
        cur_class = this->get(cur_class->get_parent());
    }




    // Setup object env for inheritance classes
    int num_scopes = 0;
    while(!stack->is_empty()) {
        cur_class = stack->pop();
        env->O->enterscope();
        Features features = cur_class->get_features();
        for(int i = features->first(); features->more(i); i = features->next(i)) {
            if (!features->nth(i)->is_method()) {
                // Illegal to redefine attribute
                if (env->O->lookup(features->nth(i)->get_name()) != NULL) {
                    this->semant_error(env->C->get_filename(), features->nth(i))
                    << "Attribute "
                    << features->nth(i)->get_name()->get_string()
                    << " is an attribute of an inherited class.\n";
                    return -1;
                }

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

void ClassTable::reset_object_env(int num_scopes) {
    for (int i = 0; i < num_scopes; i++) {
        env->O->exitscope();
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
    if (other_type == SELF_TYPE) {
        if(type == SELF_TYPE)
            return true;
        else
            return false;
    }
    if (type == SELF_TYPE) 
        type = env->C->get()->get_name();

    Symbol cur = type;
    while(cur != No_class) {
        if (cur == other_type) 
            return true;
        cur = get_parent(cur);
    }
    return false;
}


Symbol ClassTable::join_type(Symbol type, Symbol other_type){
    if (!type)
        return other_type;
    if (!other_type)
        return type;
    if (type == SELF_TYPE && other_type == SELF_TYPE)
        return SELF_TYPE;

    if (type == SELF_TYPE)
        type = env->C->get()->get_name();
    if (other_type == SELF_TYPE)
        other_type = env->C->get()->get_name();

    Symbol cur = type;
    while(true) {
        if (is_subtype(other_type, cur))
            return cur;
        cur = get_parent(cur);
    }
    printf("join_type error\n");
    return NULL;
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

// use the method in the closest class
// search from current class up until Object class
Signature* MethodEnv::lookup(Method method) {
    Symbol cur_class_name = method.class_name;
    Method new_method = method;
    do {
        new_method.class_name = cur_class_name;
        Signature* sig_ptr = SymbolTable<Method, Signature>::lookup(new_method);
        if (sig_ptr != NULL) {
            return sig_ptr;
        }
        // keep searching
        cur_class_name = classtable->get_parent(cur_class_name);
    } while(cur_class_name != No_class);
    return NULL;
}

void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    classtable = new ClassTable(classes);

    if (!classtable->has_undefined_class() && classtable->has_cycle()) {
        classtable->semant_error() << "classes form a cycle" << endl;
    }

    if (!classtable->is_defined_type(Main)) {
        classtable->semant_error() << "Class Main is not defined." << endl;
    }

    if (classtable->errors()) {
	cerr << "Compilation halted due to static semantic errors." << endl;
	exit(1);
    }

    /* some semantic analysis code may go here */
    env = new TypeEnv();
    classtable->setup_method_env();
    for(int i = classes->first(); classes->more(i); i = classes->next(i)) {
        int num_scopes = classtable->setup_object_env(classes->nth(i)); 
        if (classes->nth(i)->get_name() == SELF_TYPE) {
            classtable->semant_error(classes->nth(i)->get_filename(), classes->nth(i))
            << "Redefinition of basic class "
            << classes->nth(i)->get_name()->get_string()
            << ".\n";
        }
        if (classtable->errors()) {
	        cerr << "Compilation halted due to static semantic errors." << endl;
	        exit(1);
        }

        classes->nth(i)->semant();
        classtable->reset_object_env(num_scopes);
    }

    if (classtable->errors()) {
	cerr << "Compilation halted due to static semantic errors." << endl;
	exit(1);
    }
}

void class__class::semant()
{
    env->C->set(this);

    if (this->get_parent() == Int ||
        this->get_parent() == Bool ||
        this->get_parent() == Str) {
            classtable->semant_error(get_filename(), this) 
            << "Class "
            << name->get_string()
            << " cannot inherit class "
            << this->get_parent()->get_string()
            << ".\n";
        return;
    }
    for (int i = features->first(); features->more(i); i = features->next(i))
        features->nth(i)->semant();
}

void method_class::semant()
{
    env->O->enterscope();
    Symbol* self_type = new Symbol;
    *self_type = SELF_TYPE;
    env->O->addid(self, self_type);

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


    if (!classtable->is_defined_type(type_decl) && type_decl != SELF_TYPE) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "Class "
        << type_decl->get_string()
        << " of attribute "
        << name->get_string()
        << " is undefined.\n";
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
    if (type_decl == SELF_TYPE) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "Formal parameter "
        << name->get_string()
        << "cannot have type "
        << type_decl->get_string()
        << ".\n";
        return;
    }
    if (env->O->probe(name) != NULL) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "Formal parameter "
        << name->get_string()
        << " is multiply defined.\n";
        return;
    }
    Symbol* symbol = new Symbol;
    *symbol = type_decl;
    env->O->addid(name, symbol);
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

    if (name == self) {
        classtable->semant_error(env->C->get_filename(), this)
        << "Cannot assign to "
        << "'" << name->get_string() << "'"
        << ".\n";
        type = expr->get_type();
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

    if (name == self) {
        type = SELF_TYPE;
        return;
    }
    Symbol* name_type = env->O->lookup(name);
    if (name_type == NULL) {
        classtable->semant_error(env->C->get_filename(), this)
        << "Undeclared identifier "
        << name
        << ".\n";
        type = Object;
        return;
    }
    type = *name_type;
}

void new__class::semant() {
    // TODO: what if new a type that doesn't exist

    if (type_name == SELF_TYPE){
        type = type_name;
        return;
    }

    if (!classtable->is_defined_type(type_name)) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "'new' used with undefined class "
        << type_name->get_string()
        << ".\n";
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
    // cout << env->C->get()->get_name()->get_string() << " " << expr_type->get_string() << " " << name->get_string() << endl;
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
        classtable->semant_error(env->C->get_filename(), this)
        << "Expression type "
        << expr->get_type()->get_string()
        << " does not conform to declared static dispatch type "
        << type_name->get_string()
        << ".\n";
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
    if (identifier == self) {
        classtable->semant_error(env->C->get_filename(), this)
        << "'" << identifier->get_string() << "'"
        << " cannot be bound in a 'let' expression.\n";
        // type = Object;
        // return;
    }
    if (!init->is_no_expr()) {
        init->semant();
        if (!classtable->is_subtype(init->get_type(), type_decl)) {
            classtable->semant_error(env->C->get_filename(), this) << "let has invalid type\n";
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


void typcase_class::semant() {
    expr->semant();
    SymbolTable<Symbol, bool>* all_types = new SymbolTable<Symbol, bool>;
    all_types->enterscope();
    Symbol case_type = NULL;
    bool exist = true;
    for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
        env->O->enterscope();

        Symbol* type_decl = new Symbol;
        *type_decl = cases->nth(i)->get_type_decl();
        // check for branch of the same type_decl
        if (all_types->lookup(*type_decl) != NULL) {
            classtable->semant_error(env->C->get_filename(), this) 
            << "Duplicate branch "
            << (*type_decl)->get_string()
            << " in case statement.\n";
        }
        all_types->addid(*type_decl, &exist);

        env->O->addid(cases->nth(i)->get_name(), type_decl);

        Expression expr = cases->nth(i)->get_expr();
        expr->semant();

        env->O->exitscope();
        case_type = classtable->join_type(case_type, expr->get_type());
    }
    type = case_type;
}



void loop_class::semant() {
    pred->semant();
    body->semant();
    if (pred->get_type() != Bool) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "Loop condition does not have type Bool.\n";
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
    if (e1->get_type() != Int) {
        classtable->semant_error(env->C->get_filename(), this) << "neg expr has invalid type\n";
        type = Object;
        return;
    }
    type = Int;
    return;
}

void plus_class::semant() {
    e1->semant();
    e2->semant();
    if (e1->get_type() != Int || e2->get_type() != Int) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "non-Int arguments: "
        << e1->get_type()->get_string()
        << " + "
        << e2->get_type()->get_string()
        << "\n";
        type = Object;
        return;
    }
    type = Int;
    return;
}

void sub_class::semant() {
    e1->semant();
    e2->semant();
    if (e1->get_type() != Int || e2->get_type() != Int) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "non-Int arguments: "
        << e1->get_type()->get_string()
        << " - "
        << e2->get_type()->get_string()
        << "\n";
        type = Object;
        return;
    }
    type = Int;
    return;
}

void mul_class::semant() {
    e1->semant();
    e2->semant();

    if (e1->get_type() != Int || e2->get_type() != Int) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "non-Int arguments: "
        << e1->get_type()->get_string()
        << " * "
        << e2->get_type()->get_string()
        << "\n";
        type = Object;
        return;
    }
    type = Int;
    return;
}

void divide_class::semant() {
    e1->semant();
    e2->semant();
    if (e1->get_type() != Int || e2->get_type() != Int) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "non-Int arguments: "
        << e1->get_type()->get_string()
        << " / "
        << e2->get_type()->get_string()
        << "\n";
        type = Object;
        return;
    }
    type = Int;
}

void lt_class::semant() {
    e1->semant();
    e2->semant();
    if (e1->get_type() != Int || e2->get_type() != Int) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "non-Int arguments: "
        << e1->get_type()->get_string()
        << " < "
        << e2->get_type()->get_string()
        << "\n";
        type = Object;
        return;
    }
    type = Bool;
    return;
}

void eq_class::semant() {
    e1->semant();
    e2->semant();
    if ((e1->get_type() == Int || e1->get_type() == Str || e1->get_type() == Bool ||
         e2->get_type() == Int || e2->get_type() == Str || e2->get_type() == Bool) && 
         (e1->get_type() != e2->get_type())) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "non-Int arguments: "
        << e1->get_type()->get_string()
        << " = "
        << e2->get_type()->get_string()
        << "\n";
        type = Object;
        return;
    }
    type = Bool;
}

void leq_class::semant() {
    e1->semant();
    e2->semant();
    if (e1->get_type() != Int || e2->get_type() != Int) {
        classtable->semant_error(env->C->get_filename(), this) 
        << "non-Int arguments: "
        << e1->get_type()->get_string()
        << " <= "
        << e2->get_type()->get_string()
        << "\n";
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
