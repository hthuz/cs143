
//**************************************************************
//
// Code generator SKELETON
//
// Read the comments carefully. Make sure to
//    initialize the base class tags in
//       `CgenClassTable::CgenClassTable'
//
//    Add the label for the dispatch tables to
//       `IntEntry::code_def'
//       `StringEntry::code_def'
//       `BoolConst::code_def'
//
//    Add code to emit everyting else that is needed
//       in `CgenClassTable::code'
//
//
// The files as provided will produce code to begin the code
// segments, declare globals, and emit constants.  You must
// fill in the rest.
//
//**************************************************************

#include "cgen.h"
#include "cgen_gc.h"

extern void emit_string_constant(ostream &str, char *s);
extern int cgen_debug;

#define INIT_METHOD NULL

//
// Three symbols from the semantic analyzer (semant.cc) are used.
// If e : No_type, then no code is generated for e.
// Special code is generated for new SELF_TYPE.
// The name "self" also generates code different from other references.
//
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
Symbol
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
	arg = idtable.add_string("arg");
	arg2 = idtable.add_string("arg2");
	Bool = idtable.add_string("Bool");
	concat = idtable.add_string("concat");
	cool_abort = idtable.add_string("abort");
	copy = idtable.add_string("copy");
	Int = idtable.add_string("Int");
	in_int = idtable.add_string("in_int");
	in_string = idtable.add_string("in_string");
	IO = idtable.add_string("IO");
	length = idtable.add_string("length");
	Main = idtable.add_string("Main");
	main_meth = idtable.add_string("main");
	//   _no_class is a symbol that can't be the name of any
	//   user-defined class.
	No_class = idtable.add_string("_no_class");
	No_type = idtable.add_string("_no_type");
	Object = idtable.add_string("Object");
	out_int = idtable.add_string("out_int");
	out_string = idtable.add_string("out_string");
	prim_slot = idtable.add_string("_prim_slot");
	self = idtable.add_string("self");
	SELF_TYPE = idtable.add_string("SELF_TYPE");
	Str = idtable.add_string("String");
	str_field = idtable.add_string("_str_field");
	substr = idtable.add_string("substr");
	type_name = idtable.add_string("type_name");
	val = idtable.add_string("_val");
}

static char *gc_init_names[] =
	{"_NoGC_Init", "_GenGC_Init", "_ScnGC_Init"};
static char *gc_collect_names[] =
	{"_NoGC_Collect", "_GenGC_Collect", "_ScnGC_Collect"};

//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);

//*********************************************************
//
// Define method for code generation
//
// This is the method called by the compiler driver
// `cgtest.cc'. cgen takes an `ostream' to which the assembly will be
// emmitted, and it passes this and the class list of the
// code generator tree to the constructor for `CgenClassTable'.
// That constructor performs all of the work of the code
// generator.
//
//**********************************************************
class Method {
public:
  Symbol class_name;
  Symbol method_name;
  Method(Symbol c, Symbol m) {
	class_name = c;
	method_name = m;
  }
  bool operator==(const Method& other) const{
    return class_name == other.class_name && method_name == other.method_name;
  }
  char* to_disp_label() {
	char* res = new char[strlen(class_name->get_string()) + strlen(method_name->get_string()) + 2];
	strcpy(res, class_name->get_string());
	res[strlen(class_name->get_string())] = '.';
	strcpy(res + strlen(class_name->get_string()) + 1, method_name->get_string());
  return res;

  }
};

bool equal_method_name(Method* m1, Method* m2) {
	return (m1->method_name == m2->method_name);
}

// Type => dispatch table(with only method symbol)
// Bottom: methods of current Object
// Top: methods of Object
class DispMap : public SymbolTable<Symbol, Stack<Method*> > {
public:
	int get_method_offset(Symbol type, Symbol method) {
		Stack<Method*>* stack = this->lookup(type);
		for (int i = 0; i < stack->size(); i++) {
			if (stack->at(i)->method_name == method)
				return stack->size() - i - 1;
		}
		return -1;
	}
};

// class => attribute layout
class AttrMap : public SymbolTable<Symbol, Stack<Feature> > {
public:
	int get_attr_offset(Symbol type, Symbol identifer) {
		Stack<Feature>* stack = this->lookup(type);
		for (int i = 0; i < stack->size(); i++) {
			if (stack->at(i)->get_name() == identifer) {
				return ATTR0_OFFSET + stack->size() - i - 1;
			}
		}
		return -1;
	}
};

// identifier => offset to fp
class LocalMap : public SymbolTable<Symbol, int> {
};

class SelfObject {
private:
	CgenNodeP node; // class
	Feature cur_method;
public:
	void set_node(CgenNodeP n) {node = n;}
	CgenNodeP get_node() {return node;}
	void set_method(Feature m) {cur_method = m;}
	Feature get_method() {return cur_method;}
};

class CgenEnv {
public:
  SelfObject* so;
  DispMap* disp_map; // type => dispatch table
  AttrMap* attr_map; // type => attribute(feature)
  LocalMap* local_map;
  CgenEnv() {
    so = new SelfObject();
	disp_map = new DispMap();
	attr_map = new AttrMap();
	local_map = new LocalMap();
	disp_map->enterscope();
	attr_map->enterscope();
	local_map->enterscope();
  }
};

CgenEnv* env;
CgenClassTable* codegen_classtable;
int label_num = 0;

void program_class::cgen(ostream &os)
{
	// spim wants comments to start with '#'
	os << "# start of generated code\n";

	initialize_constants();
	codegen_classtable = new CgenClassTable(classes, os);
	codegen_classtable->code_init();
	codegen_classtable->code_method();

	os << "\n# end of generated code\n";
}

//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

static void emit_load(char *dest_reg, int offset, char *source_reg, ostream &s)
{
	s << LW << dest_reg << " " << offset * WORD_SIZE << "(" << source_reg << ")"
	  << endl;
}

static void emit_store(char *source_reg, int offset, char *dest_reg, ostream &s)
{
	s << SW << source_reg << " " << offset * WORD_SIZE << "(" << dest_reg << ")"
	  << endl;
}

static void emit_load_imm(char *dest_reg, int val, ostream &s)
{
	s << LI << dest_reg << " " << val << endl;
}

static void emit_load_address(char *dest_reg, char *address, ostream &s)
{
	s << LA << dest_reg << " " << address << endl;
}

static void emit_partial_load_address(char *dest_reg, ostream &s)
{
	s << LA << dest_reg << " ";
}

static void emit_load_bool(char *dest, const BoolConst &b, ostream &s)
{
	emit_partial_load_address(dest, s);
	b.code_ref(s);
	s << endl;
}

static void emit_load_string(char *dest, StringEntry *str, ostream &s)
{
	emit_partial_load_address(dest, s);
	str->code_ref(s);
	s << endl;
}

static void emit_load_int(char *dest, IntEntry *i, ostream &s)
{
	emit_partial_load_address(dest, s);
	i->code_ref(s);
	s << endl;
}

static void emit_move(char *dest_reg, char *source_reg, ostream &s)
{
	s << MOVE << dest_reg << " " << source_reg << endl;
}

static void emit_neg(char *dest, char *src1, ostream &s)
{
	s << NEG << dest << " " << src1 << endl;
}

static void emit_add(char *dest, char *src1, char *src2, ostream &s)
{
	s << ADD << dest << " " << src1 << " " << src2 << endl;
}

static void emit_addu(char *dest, char *src1, char *src2, ostream &s)
{
	s << ADDU << dest << " " << src1 << " " << src2 << endl;
}

static void emit_addiu(char *dest, char *src1, int imm, ostream &s)
{
	s << ADDIU << dest << " " << src1 << " " << imm << endl;
}

static void emit_div(char *dest, char *src1, char *src2, ostream &s)
{
	s << DIV << dest << " " << src1 << " " << src2 << endl;
}

static void emit_mul(char *dest, char *src1, char *src2, ostream &s)
{
	s << MUL << dest << " " << src1 << " " << src2 << endl;
}

static void emit_sub(char *dest, char *src1, char *src2, ostream &s)
{
	s << SUB << dest << " " << src1 << " " << src2 << endl;
}

static void emit_sll(char *dest, char *src1, int num, ostream &s)
{
	s << SLL << dest << " " << src1 << " " << num << endl;
}

static void emit_jalr(char *dest, ostream &s)
{
	s << JALR << "\t" << dest << endl;
}

static void emit_jal(char *address, ostream &s)
{
	s << JAL << address << endl;
}

static void emit_return(ostream &s)
{
	s << RET << endl;
}

static void emit_gc_assign(ostream &s)
{
	s << JAL << "_GenGC_Assign" << endl;
}

static void emit_disptable_ref(Symbol sym, ostream &s)
{
	s << sym << DISPTAB_SUFFIX;
}

static void emit_init_ref(Symbol sym, ostream &s)
{
	s << sym << CLASSINIT_SUFFIX;
}

static void emit_label_ref(int l, ostream &s)
{
	s << "label" << l;
}

static void emit_protobj_ref(Symbol sym, ostream &s)
{
	s << sym << PROTOBJ_SUFFIX;
}

static void emit_method_ref(Symbol classname, Symbol methodname, ostream &s)
{
	s << classname << METHOD_SEP << methodname;
}

static void emit_label_def(int l, ostream &s)
{
	emit_label_ref(l, s);
	s << ":" << endl;
}

static void emit_beqz(char *source, int label, ostream &s)
{
	s << BEQZ << source << " ";
	emit_label_ref(label, s);
	s << endl;
}

static void emit_beq(char *src1, char *src2, int label, ostream &s)
{
	s << BEQ << src1 << " " << src2 << " ";
	emit_label_ref(label, s);
	s << endl;
	
#define INIT_METHOD NULL
}

static void emit_bne(char *src1, char *src2, int label, ostream &s)
{
	s << BNE << src1 << " " << src2 << " ";
	emit_label_ref(label, s);
	s << endl;
}

static void emit_bleq(char *src1, char *src2, int label, ostream &s)
{
	s << BLEQ << src1 << " " << src2 << " ";
	emit_label_ref(label, s);
	s << endl;
}

static void emit_blt(char *src1, char *src2, int label, ostream &s)
{
	s << BLT << src1 << " " << src2 << " ";
	emit_label_ref(label, s);
	s << endl;
}

static void emit_blti(char *src1, int imm, int label, ostream &s)
{
	s << BLT << src1 << " " << imm << " ";
	emit_label_ref(label, s);
	s << endl;
}

static void emit_bgti(char *src1, int imm, int label, ostream &s)
{
	s << BGT << src1 << " " << imm << " ";
	emit_label_ref(label, s);
	s << endl;
}

static void emit_branch(int l, ostream &s)
{
	s << BRANCH;
	emit_label_ref(l, s);
	s << endl;
}

//
// Push a register on the stack. The stack grows towards smaller addresses.
//
static void emit_push(char *reg, ostream &str)
{
	emit_store(reg, 0, SP, str);
	emit_addiu(SP, SP, -4, str);
}

//
// Fetch the integer value in an Int object.
// Emits code to fetch the integer value of the Integer object pointed
// to by register source into the register dest
//
static void emit_fetch_int(char *dest, char *source, ostream &s)
{
	emit_load(dest, DEFAULT_OBJFIELDS, source, s);
}

//
// Emits code to store the integer value contained in register source
// into the Integer object pointed to by dest.
//
static void emit_store_int(char *source, char *dest, ostream &s)
{
	emit_store(source, DEFAULT_OBJFIELDS, dest, s);
}

static void emit_test_collector(ostream &s)
{
	emit_push(ACC, s);
	emit_move(ACC, SP, s);	// stack end
	emit_move(A1, ZERO, s); // allocate nothing
	s << JAL << gc_collect_names[cgen_Memmgr] << endl;
	emit_addiu(SP, SP, 4, s);
	emit_load(ACC, 0, SP, s);
}

static void emit_gc_check(char *source, ostream &s)
{
	if (source != (char *)A1)
		emit_move(A1, source, s);
	s << JAL << "_gc_check" << endl;
}

static void emit_setup_frame(int num_local, ostream &s)
{
	// allocate stack
	emit_addiu(SP, SP, -(3 + num_local) * WORD_SIZE, s);
	// store old $fp, $s0 and $ra
	emit_store(FP, 3 + num_local , SP, s);
	emit_store(SELF, 2 + num_local, SP, s);
	emit_store(RA, 1 + num_local, SP, s);
	// setup new $fp and $s0
	emit_addiu(FP, SP, WORD_SIZE, s);
	emit_move(SELF, ACC, s); // store $a0 to $s0
}

static void emit_tear_frame(int num_local, int num_args, ostream &s)
{
	// restore $fp, $s0 and $ra
	emit_load(FP, 3 + num_local, SP, s);
	emit_load(SELF,2 + num_local, SP, s);
	emit_load(RA, 1 + num_local, SP, s);
	// deallocate stack
	emit_addiu(SP, SP, (3 + num_local + num_args) * WORD_SIZE, s);
	// jump to return address
	emit_return(s);
}

///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Cool has three kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// All string constants are listed in the global "stringtable" and have
// type StringEntry.  StringEntry methods are defined both for String
// constant definitions and references.
//
// All integer constants are listed in the global "inttable" and have
// type IntEntry.  IntEntry methods are defined for Int
// constant definitions and references.
//
// Since there are only two Bool values, there is no need for a table.
// The two booleans are represented by instances of the class BoolConst,
// which defines the definition and reference methods for Bools.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream &s)
{
	s << STRCONST_PREFIX << index;
}


//
// Emit code for a constant String.
// You should fill in the code naming the dispatch table.
//

void StringEntry::code_def(ostream &s, int stringclasstag)
{
	IntEntryP lensym = inttable.add_int(len);

	// Add -1 eye catcher
	s << WORD << "-1" << endl;

	code_ref(s);
	s << LABEL																// label
	  << WORD << stringclasstag << endl										// tag
	  << WORD << (DEFAULT_OBJFIELDS + STRING_SLOTS + (len + 4) / 4) << endl // size
	  << WORD;

	/***** Add dispatch information for class String ******/

	s << "String" << DISPTAB_SUFFIX << endl; // dispatch table
	s << WORD;
	lensym->code_ref(s);
	s << endl;					  // string length
	emit_string_constant(s, str); // ascii string
	s << ALIGN;					  // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the
// stringtable.
//
void StrTable::code_string_table(ostream &s, int stringclasstag)
{
	for (List<StringEntry> *l = tbl; l; l = l->tl()) {
		l->hd()->code_def(s, stringclasstag);
	}
}

//
// Ints
//
void IntEntry::code_ref(ostream &s)
{
	s << INTCONST_PREFIX << index;
}

//
// Emit code for a constant Integer.
// You should fill in the code naming the dispatch table.
//

void IntEntry::code_def(ostream &s, int intclasstag)
{
	// Add -1 eye catcher
	s << WORD << "-1" << endl;

	code_ref(s);
	s << LABEL											 // label
	  << WORD << intclasstag << endl					 // class tag
	  << WORD << (DEFAULT_OBJFIELDS + INT_SLOTS) << endl // object size
	  << WORD;

	/***** Add dispatch information for class Int ******/

	s << "Int" << DISPTAB_SUFFIX << endl; // dispatch table
	s << WORD << str << endl; // integer value
}

//
// IntTable::code_string_table
// Generate an Int object definition for every Int constant in the
// inttable.
//
void IntTable::code_string_table(ostream &s, int intclasstag)
{
	for (List<IntEntry> *l = tbl; l; l = l->tl()) 
		l->hd()->code_def(s, intclasstag);
}

//
// Bools
//
BoolConst::BoolConst(int i) : val(i) { assert(i == 0 || i == 1); }

void BoolConst::code_ref(ostream &s) const
{
	s << BOOLCONST_PREFIX << val;
}

//
// Emit code for a constant Bool.
// You should fill in the code naming the dispatch table.
//

void BoolConst::code_def(ostream &s, int boolclasstag)
{
	// Add -1 eye catcher
	s << WORD << "-1" << endl;

	code_ref(s);
	s << LABEL											  // label
	  << WORD << boolclasstag << endl					  // class tag
	  << WORD << (DEFAULT_OBJFIELDS + BOOL_SLOTS) << endl // object size
	  << WORD;

	/***** Add dispatch information for class Bool ******/

	s << "Bool" << DISPTAB_SUFFIX << endl;				  // dispatch table
	s << WORD << val << endl; // value (0 or 1)
}

//////////////////////////////////////////////////////////////////////////////
//
//  CgenClassTable methods
//
//////////////////////////////////////////////////////////////////////////////

//***************************************************
//
//  Emit code to start the .data segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_data()
{
	Symbol main = idtable.lookup_string(MAINNAME);
	Symbol string = idtable.lookup_string(STRINGNAME);
	Symbol integer = idtable.lookup_string(INTNAME);
	Symbol boolc = idtable.lookup_string(BOOLNAME);

	str << "\t.data\n"
		<< ALIGN;
	//
	// The following global names must be defined first.
	//
	str << GLOBAL << CLASSNAMETAB << endl;
	str << GLOBAL;
	emit_protobj_ref(main, str);
	str << endl;
	str << GLOBAL;
	emit_protobj_ref(integer, str);
	str << endl;
	str << GLOBAL;
	emit_protobj_ref(string, str);
	str << endl;
	str << GLOBAL;
	falsebool.code_ref(str);
	str << endl;
	str << GLOBAL;
	truebool.code_ref(str);
	str << endl;
	str << GLOBAL << INTTAG << endl;
	str << GLOBAL << BOOLTAG << endl;
	str << GLOBAL << STRINGTAG << endl;

	//
	// We also need to know the tag of the Int, String, and Bool classes
	// during code generation.
	//
	str << INTTAG << LABEL
		<< WORD << get_node_by_type(Int)->get_class_tag() << endl;
	str << BOOLTAG << LABEL
		<< WORD << get_node_by_type(Bool)->get_class_tag() << endl;
	str << STRINGTAG << LABEL
		<< WORD << get_node_by_type(Str)->get_class_tag() << endl;
}

//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_text()
{
	str << GLOBAL << HEAP_START << endl
		<< HEAP_START << LABEL
		<< WORD << 0 << endl
		<< "\t.text" << endl
		<< GLOBAL;
	emit_init_ref(idtable.add_string("Main"), str);
	str << endl
		<< GLOBAL;
	emit_init_ref(idtable.add_string("Int"), str);
	str << endl
		<< GLOBAL;
	emit_init_ref(idtable.add_string("String"), str);
	str << endl
		<< GLOBAL;
	emit_init_ref(idtable.add_string("Bool"), str);
	str << endl
		<< GLOBAL;
	emit_method_ref(idtable.add_string("Main"), idtable.add_string("main"), str);
	str << endl;
}

void CgenClassTable::code_bools(int boolclasstag)
{
	falsebool.code_def(str, boolclasstag);
	truebool.code_def(str, boolclasstag);
}

void CgenClassTable::code_select_gc()
{
	//
	// Generate GC choice constants (pointers to GC functions)
	//
	str << GLOBAL << "_MemMgr_INITIALIZER" << endl;
	str << "_MemMgr_INITIALIZER:" << endl;
	str << WORD << gc_init_names[cgen_Memmgr] << endl;
	str << GLOBAL << "_MemMgr_COLLECTOR" << endl;
	str << "_MemMgr_COLLECTOR:" << endl;
	str << WORD << gc_collect_names[cgen_Memmgr] << endl;
	str << GLOBAL << "_MemMgr_TEST" << endl;
	str << "_MemMgr_TEST:" << endl;
	str << WORD << (cgen_Memmgr_Test == GC_TEST) << endl;
}

//********************************************************
//
// Emit code to reserve space for and initialize all of
// the constants.  Class names should have been added to
// the string table (in the supplied code, is is done
// during the construction of the inheritance graph), and
// code for emitting string constants as a side effect adds
// the string's length to the integer table.  The constants
// are emmitted by running through the stringtable and inttable
// and producing code for each entry.
//
//********************************************************

void CgenClassTable::code_constants()
{
	//
	// Add constants that are required by the code generator.
	//
	stringtable.add_string("");
	inttable.add_string("0");

	stringtable.code_string_table(str, get_node_by_type(Str)->get_class_tag());
	inttable.code_string_table(str, get_node_by_type(Int)->get_class_tag());
	code_bools(get_node_by_type(Bool)->get_class_tag());
}

void CgenClassTable::code_class_nameTab()
{
	str << CLASSNAMETAB <<  ":" << endl;
	for (int i = 0; i < total_class_tag_num; i++) {
		str << WORD;
		stringtable.lookup_string(get_node_by_class_tag(i)->get_name()->get_string())->code_ref(str);
		str << endl;
	}
}


void CgenClassTable::code_class_objTab() 
{
	str << CLASSOBJTAB << ":" << endl;
	for (int i = 0; i < total_class_tag_num; i++) {
		char* classname = get_node_by_class_tag(i)->get_name()->get_string();
		str << WORD << classname << PROTOBJ_SUFFIX << endl;
		str << WORD << classname << CLASSINIT_SUFFIX << endl;
	}
}

void CgenClassTable::code_dispTab() 
{

	for (List<CgenNode>* l = nds; l; l = l->tl()) {
		if (l->hd()->get_class_tag() < 0)
			continue;
		l->hd()->code_dispTab(str);
	}

}

void CgenClassTable::code_protObj()
{
	for (List<CgenNode>* l = nds; l; l = l->tl()) {
		if (l->hd()->get_class_tag() < 0)
			continue;
		l->hd()->code_protObj(str);
	}
}

void CgenClassTable::code_init()
{
	for (List<CgenNode>* l = nds; l; l = l->tl()) {
		if (l->hd()->get_class_tag() < 0)
			continue;
		l->hd()->code_init(str);
	}
}

void CgenClassTable::code_method()
{
	for (List<CgenNode>* l = nds; l; l = l->tl()) {
		if (l->hd()->get_class_tag() < 0 || l->hd()->basic())
			continue;
		l->hd()->code_method(str);
	}
}

CgenClassTable::CgenClassTable(Classes classes, ostream &s) : nds(NULL), str(s)
{
	// objectclasstag = 0;
	// ioclasstag = 1;
	// intclasstag = 2 /* Change to your Int class tag here */;
	// boolclasstag = 3 /* Change to your Bool class tag here */;
	// stringclasstag = 4 /* Change to your String class tag here */;

	enterscope();
	if (cgen_debug)
		cout << "Building CgenClassTable" << endl;
	install_basic_classes();
	install_classes(classes);
	build_inheritance_tree();
	set_class_tag();

	code();
	exitscope();
}


void CgenClassTable::install_basic_classes()
{

	// The tree package uses these globals to annotate the classes built below.
	// curr_lineno  = 0;
	Symbol filename = stringtable.add_string("<basic class>");

	//
	// A few special class names are installed in the lookup table but not
	// the class list.  Thus, these classes exist, but are not part of the
	// inheritance hierarchy.
	// No_class serves as the parent of Object and the other special classes.
	// SELF_TYPE is the self class; it cannot be redefined or inherited.
	// prim_slot is a class known to the code generator.
	//
	addid(No_class,
		  new CgenNode(class_(No_class, No_class, nil_Features(), filename),
					   Basic, this));
	addid(SELF_TYPE,
		  new CgenNode(class_(SELF_TYPE, No_class, nil_Features(), filename),
					   Basic, this));
	addid(prim_slot,
		  new CgenNode(class_(prim_slot, No_class, nil_Features(), filename),
					   Basic, this));

	//
	// The Object class has no parent class. Its methods are
	//        cool_abort() : Object    aborts the program
	//        type_name() : Str        returns a string representation of class name
	//        copy() : SELF_TYPE       returns a copy of the object
	//
	// There is no need for method bodies in the basic classes---these
	// are already built in to the runtime system.
	//
	install_class(
		new CgenNode(
			class_(Object,
				   No_class,
				   append_Features(
					   append_Features(
						   single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
						   single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
					   single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
				   filename),
			Basic, this));

	//
	// The IO class inherits from Object. Its methods are
	//        out_string(Str) : SELF_TYPE          writes a string to the output
	//        out_int(Int) : SELF_TYPE               "    an int    "  "     "
	//        in_string() : Str                    reads a string from the input
	//        in_int() : Int                         "   an int     "  "     "
	//
	install_class(
		new CgenNode(
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
				   filename),
			Basic, this));

	//
	// The Int class has no methods and only a single attribute, the
	// "val" for the integer.
	//
	install_class(
		new CgenNode(
			class_(Int,
				   Object,
				   single_Features(attr(val, prim_slot, no_expr())),
				   filename),
			Basic, this));

	//
	// Bool also has only the "val" slot.
	//
	install_class(
		new CgenNode(
			class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())), filename),
			Basic, this));

	//
	// The class Str has a number of slots and operations:
	//       val                                  ???
	//       str_field                            the string itself
	//       length() : Int                       length of the string
	//       concat(arg: Str) : Str               string concatenation
	//       substr(arg: Int, arg2: Int): Str     substring
	//
	install_class(
		new CgenNode(
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
				   filename),
			Basic, this));
}

// CgenClassTable::install_class
// CgenClassTable::install_classes
//
// install_classes enters a list of classes in the symbol table.
//
void CgenClassTable::install_class(CgenNodeP nd)
{
	Symbol name = nd->get_name();

	if (probe(name))
	{
		return;
	}

	// The class name is legal, so add it to the list of classes
	// and the symbol table.
	nds = new List<CgenNode>(nd, nds);
	addid(name, nd);
}

void CgenClassTable::install_classes(Classes cs)
{
	for (int i = cs->first(); cs->more(i); i = cs->next(i))
		install_class(new CgenNode(cs->nth(i), NotBasic, this));
}

//
// CgenClassTable::build_inheritance_tree
//
void CgenClassTable::build_inheritance_tree()
{
	for (List<CgenNode> *l = nds; l; l = l->tl())
		set_relations(l->hd());
}

//
// CgenClassTable::set_relations
//
// Takes a CgenNode and locates its, and its parent's, inheritance nodes
// via the class table.  Parent and child pointers are added as appropriate.
//
void CgenClassTable::set_relations(CgenNodeP nd)
{
	CgenNode *parent_node = probe(nd->get_parent());
	nd->set_parentnd(parent_node);
	parent_node->add_child(nd);
}

void CgenClassTable::set_class_tag() {
	probe(No_class)->set_class_tag(-1);
	probe(prim_slot)->set_class_tag(-1);
	probe(SELF_TYPE)->set_class_tag(-1);

	Stack<CgenNodeP>* stack = new Stack<CgenNodeP>(1024);
	int cur_tag = 0;
	CgenNodeP root = this->root();
	CgenNodeP node;
	stack->push(root);
	while(!stack->is_empty()) {
		node = stack->pop();
		node->set_class_tag(cur_tag++);
		for(List<CgenNode>* l = node->get_children(); l ; l = l->tl()) {
			stack->push(l->hd());
		}
	}
	total_class_tag_num = cur_tag;
}

void CgenNode::add_child(CgenNodeP n)
{
	children = new List<CgenNode>(n, children);
}

void CgenNode::set_parentnd(CgenNodeP p)
{
	assert(parentnd == NULL);
	assert(p != NULL);
	parentnd = p;
}

void CgenClassTable::code()
{
	if (cgen_debug)
		cout << "coding global data" << endl;
	code_global_data();

	if (cgen_debug)
		cout << "choosing gc" << endl;
	code_select_gc();

	if (cgen_debug)
		cout << "coding constants" << endl;
	code_constants();

	//                 Add your code to emit
	//                   - prototype objects
	//                   - class_nameTab
	//                   - dispatch tables
	//
	env = new CgenEnv();
	code_class_nameTab();
	code_class_objTab();
	code_dispTab();
	code_protObj();

	if (cgen_debug)
		cout << "coding global text" << endl;
	code_global_text();

	//                 Add your code to emit
	//                   - object initializer
	//                   - the class methods
	//                   - etc...
}


CgenNode* CgenClassTable::get_node_by_class_tag(int class_tag) {
	for(List<CgenNode> *l = nds; l; l=l->tl()) {
		if (l->hd()->get_class_tag() == class_tag) 
			return l->hd();
	}
	return NULL;
}

CgenNode* CgenClassTable::get_node_by_type(Symbol type) {
	for(List<CgenNode> *l = nds; l; l = l->tl()) {
		if(l->hd()->get_name() == type)
			return l->hd();
	}
	return NULL;
}

CgenNodeP CgenClassTable::root()
{
	return probe(Object);
}

///////////////////////////////////////////////////////////////////////
//
// CgenNode methods
//
///////////////////////////////////////////////////////////////////////

CgenNode::CgenNode(Class_ nd, Basicness bstatus, CgenClassTableP ct) : class__class((const class__class &)*nd),
																	   parentnd(NULL),
																	   children(NULL),
																	   basic_status(bstatus)
{
	stringtable.add_string(name->get_string()); // Add class name to string table
}

int CgenNode::get_size() {
	int obj_size = 0;
	CgenNodeP cur = this;
	while(cur) {
		for (int i = cur->features->first(); cur->features->more(i); i = cur->features->next(i)) {
			if (cur->features->nth(i)->is_method())
				continue;
			obj_size++;
		}
		cur = cur->get_parentnd();
	}
	return obj_size + DEFAULT_OBJFIELDS;
}


void CgenNode::code_dispTab(ostream &s) {
	s << this->get_name()->get_string() << DISPTAB_SUFFIX << ":" << endl;
	CgenNodeP cur = this;
	Stack<Method*>* stack = new Stack<Method*>(1024);
	while (cur) {
		for (int i = cur->features->len() - 1; i >= 0 ; i--) {
			if(!cur->features->nth(i)->is_method())
				continue;
			// Concatenate
			Method* method = new Method(cur->get_name(), cur->features->nth(i)->get_name());
			Method** equal_elt_ptr = stack->equal_elt(method, equal_method_name);
			if (equal_elt_ptr != NULL ) {
				Method* equal_elt = new Method(NULL, NULL);
				equal_elt = *equal_elt_ptr;
				stack->pop_elt(equal_elt);
				stack->push(equal_elt);
			} else {
				stack->push(method);
			}
		}
		cur = cur->get_parentnd();
	}
	env->disp_map->addid(get_name(), stack->copy());
	// Print the stack
	while (!stack->is_empty()) {
		s << WORD << stack->pop()->to_disp_label() << endl;
	}
	delete stack;
}

void CgenNode::code_protObj(ostream &s) {
	s << WORD << "-1" << endl;
	s << get_name()->get_string() << PROTOBJ_SUFFIX << ":" << endl;
	s << WORD << class_tag << endl;
	s << WORD << get_size() << endl;
	s << WORD << get_name()->get_string() << DISPTAB_SUFFIX << endl;

	CgenNodeP cur = this;
	Stack<Feature>* stack = new Stack<Feature>(1024);
	while(cur) {
		for(int i = cur->features->len() - 1; i >= 0; i--) {
			if (cur->features->nth(i)->is_method())
				continue;
			stack->push(cur->features->nth(i));
		}
		cur = cur->get_parentnd();
	}
	env->attr_map->addid(get_name(), stack->copy());
	while(!stack->is_empty()) {
		Feature feat = stack->pop();
		s << WORD;
		// Int: int_const0
		// Str: string_const 10
		// Bool: bool_const 0
		// Object, IO, etc: all 0
		if (feat->get_type_decl() == Int)  {
			char default_int[] = "0";
			inttable.lookup_string(default_int)->code_ref(s);
			s << endl;
			continue;
		}
		if (feat->get_type_decl() == Str) {
			char default_str[] = "";
			stringtable.lookup_string(default_str)->code_ref(s);
			s << endl;
			continue;
		}
		if (feat->get_type_decl() == Bool) {
			falsebool.code_ref(s);
			s << endl;
			continue;
		}
		s << 0 << endl;
	}
	delete stack;
}

void CgenNode::code_init(ostream& s) {
	env->so->set_node(this);
	env->so->set_method(INIT_METHOD);
	s << get_name()->get_string() << CLASSINIT_SUFFIX << ":" << endl;

	// get num of locals required
	int num_local = 0;
	for (int i = features->first(); features->more(i); i = features->next(i)) {
		if (features->nth(i)->is_method()) continue;
		num_local = MAX(num_local, features->nth(i)->get_init()->get_local_var_num());
	}
	emit_setup_frame(num_local, s);

	// Call parent init method
	char* parent_init = new char[strlen(get_parent()->get_string()) + strlen(CLASSINIT_SUFFIX)];
	strncpy(parent_init, get_parent()->get_string(), get_parent()->get_len());
	strcat(parent_init, CLASSINIT_SUFFIX);
	if (get_name() != Object)
		emit_jal(parent_init, s);

	// Initialize attributes
	for (int i = features->first(); features->more(i); i = features->next(i)) {
		if (features->nth(i)->is_method())
			continue;
		// Unlike let, if an attribute is not initialized, no need to generate code for them
		// cause the proto object already initialize them to 0
		if (features->nth(i)->get_init()->is_no_expr())
			continue;
		features->nth(i)->get_init()->code(s);
		int attr_offset = env->attr_map->get_attr_offset(get_name(), features->nth(i)->get_name());
		emit_store(ACC, attr_offset, SELF, s);
	}

	emit_move(ACC, SELF, s); // return value (return SELF)
	emit_tear_frame(num_local, 0, s);
}

void CgenNode::code_method(ostream& s) {
	env->so->set_node(this);
	for (int i = features->first(); features->more(i); i = features->next(i)) {
		if (!features->nth(i)->is_method()) {
			continue;
		}
		env->so->set_method(features->nth(i));
		s << get_name()->get_string() << "." << features->nth(i)->get_name()->get_string() << ":" << endl;
		features->nth(i)->code(s);
	}
}


void method_class::code(ostream& s) {
	emit_setup_frame(env->so->get_method()->get_local_var_num(), s);
	expr->code(s);
	emit_tear_frame(env->so->get_method()->get_local_var_num(), formals->len(),s);
}


//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `cool-tree.h'  Sample code for
//   constant integers, strings, and booleans are provided.
//
//*****************************************************************

void assign_class::code(ostream &s)
{
	expr->code(s);
	// local variable from let
	int* local_offset = env->local_map->lookup(name);
	if (local_offset != NULL)  {
		emit_store(ACC, *local_offset, FP, s);
		return;
	}

	// method parameter
	if (env->so->get_method() != INIT_METHOD) {
		int arg_offset = env->so->get_method()->get_arg_offset(name);
		int local_num = env->so->get_method()->get_local_var_num();
		if (arg_offset != -1) {
			emit_store(ACC, local_num + arg_offset, FP, s);
			return;
		}
	}

	// attribute
	int attr_offset = env->attr_map->get_attr_offset(env->so->get_node()->get_name(), name);
	if (attr_offset != -1) {
		emit_store(ACC,attr_offset, SELF, s );
		return;
	}

}

void static_dispatch_class::code(ostream &s)
{
	// Push parameters onto the stack
	for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
		actual->nth(i)->code(s);
		emit_store(ACC, 0, SP, s);
		emit_addiu(SP, SP, -4, s);
	}
	expr->code(s);

	// check dispatch abort error
	emit_bne(ACC, ZERO, label_num, s);
	// TOCHECK: str_const0 always point to file name?
	emit_load_address(ACC, "str_const0", s) ;
	emit_load_imm(T1, this->get_line_number(), s);
	emit_jal(DISPATCH_ABORT, s);
	emit_label_def(label_num++, s);

	// Dispatch table
	Symbol expr_type = this->type_name;
	if (expr_type == SELF_TYPE) {
		expr_type = env->so->get_node()->get_name();
	}
	int tag = codegen_classtable->get_node_by_type(expr_type)->get_class_tag();
	emit_load_address(T1, CLASSOBJTAB, s);
	emit_load_imm(T2, tag, s);
	emit_sll(T2, T2, 3, s);
	emit_addu(T1, T1, T2, s);
	emit_load(T1, 0, T1, s);
	// T1 now holds proto object
	emit_load(T1, DISPTABLE_OFFSET, T1, s);
	// T1 now holds dispatch table address


	emit_load(T1, env->disp_map->get_method_offset(expr_type, this->name), T1, s);
	emit_jalr(T1, s);

}

void dispatch_class::code(ostream &s)
{
	// Push parameters onto the stack
	for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
		actual->nth(i)->code(s);
		emit_store(ACC, 0, SP, s);
		emit_addiu(SP, SP, -4, s);
	}
	expr->code(s);

	// check dispatch abort error
	emit_bne(ACC, ZERO, label_num, s);
	// TOCHECK: str_const0 always point to file name?
	emit_load_address(ACC, "str_const0", s) ;
	emit_load_imm(T1, this->get_line_number(), s);
	emit_jal(DISPATCH_ABORT, s);
	emit_label_def(label_num++, s);

	// Dispatch table
	emit_load(T1, 2, ACC, s);
	Symbol expr_type = expr->get_type();
	if (expr_type == SELF_TYPE) {
		expr_type = env->so->get_node()->get_name();
	}
	// cout << "name: " << name->get_string() << " " << "expr_type: " << expr_type->get_string() << " offset " << env->disp_map->get_method_offset(expr_type, this->name)<<  endl;
	emit_load(T1, env->disp_map->get_method_offset(expr_type, this->name), T1, s);
	emit_jalr(T1, s);

}

// beqz else_label
// [true body]
// b end_label
// else_label:
// [else body]
// end_label:
void cond_class::code(ostream &s)
{
	pred->code(s);
	emit_load(T1, ATTR0_OFFSET, ACC, s);
	int else_label_num = label_num++;
	emit_beqz(T1, else_label_num, s);
	then_exp->code(s);

	int end_label_num = label_num++;
	emit_branch(end_label_num, s);
	emit_label_def(else_label_num, s);
	else_exp->code(s);
	emit_label_def(end_label_num, s);
}

// pred_label:
// [pred body]
// beq zero end_label:
// [loop body]
// b pred_label
// end_label:

void loop_class::code(ostream &s)
{
	int pred_label_num = label_num++;
	emit_label_def(pred_label_num, s);
	pred->code(s);
	int end_label_num = label_num++;
	emit_load(T1, ATTR0_OFFSET, ACC, s);
	emit_beq(T1, ZERO, end_label_num, s);

	body->code(s);
	emit_branch(pred_label_num, s);
	emit_label_def(end_label_num, s);
	emit_move(ACC, ZERO, s);
}

// bne zero case1_label
// [abort2 body]
// case1_label
//     			 => blt, bgt case2_label
// 				 => branch end_label
// case2_label
//  			 => blt, bgt abort_label
// 				 => branch end_label
// abort_label
// [abort label]
// end_label
// [end body]

int cur_local = 0;
void typcase_class::code(ostream &s)
{
	int end_label = label_num++;
	expr->code(s);
	// abort2 body
	emit_bne(ACC, ZERO, label_num, s);
	emit_load_address(ACC, "str_const0", s);
	emit_load_imm(T1, this->get_line_number(), s);
	emit_jal(CASE_ABORT2, s);


	// local variable
	if (cur_local == 0) {
		env->local_map->enterscope();
	}
	// Show select case based on class tag in descending order. selection sort
	int prev_max_tag = 1024;
	bool local_added_flag = false;
	int next_case_label = label_num;
	for(int k = 0; k < cases->len(); k++) {

		CgenNodeP max_node = codegen_classtable->get_node_by_type(Object);
		Case max_case;
		for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
			CgenNodeP node = codegen_classtable->get_node_by_type(cases->nth(i)->get_type_decl());
			if (node->get_class_tag() > max_node->get_class_tag() && node->get_class_tag() < prev_max_tag) {
				max_case = cases->nth(i);
				max_node = node;
			}
		}
		emit_label_def(next_case_label, s);
		emit_load(T1, TAG_OFFSET, ACC, s);
		label_num++;
		next_case_label = label_num;
		emit_blti(T1,max_node->get_class_tag(),next_case_label, s);
		emit_bgti(T1, max_node->get_max_tag_child()->get_class_tag(), next_case_label, s);
		label_num++;

		int* offset;
		// allocate space for expr
		if (!local_added_flag) {
			offset = new int(cur_local);
			env->local_map->addid(max_case->get_name(), offset);
			cur_local++;
			local_added_flag = true;
		}
		emit_store(ACC, *offset, FP, s);

		max_case->get_expr()->code(s);
		emit_branch(end_label, s);

		// update
		prev_max_tag = max_node->get_class_tag();

	}
	if (--cur_local == 0) {
		env->local_map->exitscope();
	}

	// abort body
	emit_label_def(next_case_label, s);
	emit_jal(CASE_ABORT, s);

	// end label
	emit_label_def(end_label, s);


}

void block_class::code(ostream &s)
{
	for(int i = body->first(); body->more(i); i = body->next(i)) {
		body->nth(i)->code(s);
	}
}
void let_class::code(ostream &s)
{
	// init expr
	if (init->is_no_expr()) {
		if (type_decl == Int) {
			char default_int[] = "0";
			emit_partial_load_address(ACC, s); inttable.lookup_string(default_int)->code_ref(s); s << endl;
		} else if (type_decl == Str) {
			char default_str[] = "";
			emit_partial_load_address(ACC, s); stringtable.lookup_string(default_str)->code_ref(s); s << endl;
		} else if (type_decl == Bool) {
			emit_partial_load_address(ACC, s); falsebool.code_ref(s); s << endl;
		} else {
			emit_move(ACC, ZERO, s);
		}
	} else {
		init->code(s);
	}

	// allocate a new space
	if (cur_local == 0) {
		env->local_map->enterscope();
	}
	int* offset = new int(cur_local);
	env->local_map->addid(identifier, offset);
	emit_store(ACC, *offset, FP, s);
	cur_local++;

	body->code(s);
	if (--cur_local == 0) {
		env->local_map->exitscope();
	}
}

void plus_class::code(ostream &s)
{
	e1->code(s);
	// Load the int 
	emit_load(T1,ATTR0_OFFSET, ACC, s);
	emit_store(T1, 0, SP, s);
	emit_addiu(SP, SP, -4, s);
	e2->code(s);
	// Create a new Int address for operation result
	emit_jal(OBJECT_COPY, s);
	emit_load(T1, 1, SP, s);
	emit_load(T2, ATTR0_OFFSET, ACC, s);
	emit_add(T1, T1, T2, s);
	// Store opeartion result into specified location
	emit_store(T1, ATTR0_OFFSET, ACC, s);

	emit_addiu(SP, SP, 4, s);
}

void sub_class::code(ostream &s)
{
	e1->code(s);
	// Load the int 
	emit_load(T1,ATTR0_OFFSET, ACC, s);
	emit_store(T1, 0, SP, s);
	emit_addiu(SP, SP, -4, s);
	e2->code(s);
	// Create a new Int address for operation result
	emit_jal(OBJECT_COPY, s);
	emit_load(T1, 1, SP, s);
	emit_load(T2, ATTR0_OFFSET, ACC, s);
	emit_sub(T1, T1, T2, s);
	// Store opeartion result into specified location
	emit_store(T1, ATTR0_OFFSET, ACC, s);

	emit_addiu(SP, SP, 4, s);
}

void mul_class::code(ostream &s)
{
	e1->code(s);
	// Load the int 
	emit_load(T1,ATTR0_OFFSET, ACC, s);
	emit_store(T1, 0, SP, s);
	emit_addiu(SP, SP, -4, s);
	e2->code(s);
	// Create a new Int address for operation result
	emit_jal(OBJECT_COPY, s);
	emit_load(T1, 1, SP, s);
	emit_load(T2, ATTR0_OFFSET, ACC, s);
	emit_mul(T1, T1, T2, s);
	// Store opeartion result into specified location
	emit_store(T1, ATTR0_OFFSET, ACC, s);

	emit_addiu(SP, SP, 4, s);
}

void divide_class::code(ostream &s)
{
	e1->code(s);
	// Load the int 
	emit_load(T1,ATTR0_OFFSET, ACC, s);
	emit_store(T1, 0, SP, s);
	emit_addiu(SP, SP, -4, s);
	e2->code(s);
	// Create a new Int address for operation result
	emit_jal(OBJECT_COPY, s);
	emit_load(T1, 1, SP, s);
	emit_load(T2, ATTR0_OFFSET, ACC, s);
	emit_div(T1, T1, T2, s);
	// Store opeartion result into specified location
	emit_store(T1, ATTR0_OFFSET, ACC, s);

	emit_addiu(SP, SP, 4, s);
}

void neg_class::code(ostream &s)
{
	e1->code(s);
	// Create a new Int address for op result
	emit_jal(OBJECT_COPY, s);
	emit_load(T1, ATTR0_OFFSET, ACC, s);
	emit_neg(T1, T1, s);
	emit_store(T1, ATTR0_OFFSET, ACC, s);
}

// 
// acc <- true value
// blt label
// acc <- false value
// label:
// [next part]
void lt_class::code(ostream &s)
{
	e1->code(s);
	emit_load(T1, ATTR0_OFFSET, ACC, s);
	emit_store(T1, 0, SP, s);
	emit_addiu(SP, SP, -4, s);
	e2->code(s);

	// Compare
	emit_load(T1, 1, SP, s);
	emit_load(T2, ATTR0_OFFSET, ACC, s);
	emit_partial_load_address(ACC, s); truebool.code_ref(s); s << endl;
	emit_blt(T1, T2, label_num, s);
	emit_partial_load_address(ACC, s); falsebool.code_ref(s); s << endl;

	// Next step
	emit_label_def(label_num++, s);
	emit_addiu(SP, SP, 4, s);
}

void eq_class::code(ostream &s)
{
	if (e1->get_type() != Int && e1->get_type() != Str && e1->get_type() != Bool) {
		e1->code(s);
		emit_store(ACC, 0, SP, s);
		emit_addiu(SP, SP, -4, s);
		e2->code(s);

		// Compare
		emit_load(T1, 1, SP, s);
		emit_move(T2, ACC, s);
		emit_partial_load_address(ACC, s); truebool.code_ref(s); s << endl;
		emit_beq(T1, T2, label_num, s);
		emit_partial_load_address(ACC, s); falsebool.code_ref(s); s << endl;

		// Next step
		emit_label_def(label_num++, s);
		emit_addiu(SP, SP, 4, s);
		return;
	}

	e1->code(s);
	emit_store(ACC, 0, SP, s);
	emit_addiu(SP, SP, -4, s);
	e2->code(s);

	// Compare
	emit_load(T1, 1, SP, s);
	emit_move(T2, ACC, s);
	emit_partial_load_address(ACC, s); truebool.code_ref(s); s << endl;
	emit_beq(T1, T2, label_num, s);

	emit_partial_load_address(A1, s); falsebool.code_ref(s); s << endl;

	emit_jal(EQUALITY_TEST, s);

	// Next step
	emit_label_def(label_num++, s);
	emit_addiu(SP, SP, 4, s);
}

void leq_class::code(ostream &s)
{
	e1->code(s);
	emit_load(T1, ATTR0_OFFSET, ACC, s);
	emit_store(T1, 0, SP, s);
	emit_addiu(SP, SP, -4, s);
	e2->code(s);

	// Compare
	emit_load(T1, 1, SP, s);
	emit_load(T2, ATTR0_OFFSET, ACC, s);
	emit_partial_load_address(ACC, s); truebool.code_ref(s); s << endl;
	emit_bleq(T1, T2, label_num, s);
	emit_partial_load_address(ACC, s); falsebool.code_ref(s); s << endl;

	// Next step
	emit_label_def(label_num++, s);
	emit_addiu(SP, SP, 4, s);
}

void comp_class::code(ostream &s)
{
	e1->code(s);
	emit_load(T1, ATTR0_OFFSET, ACC, s);
	emit_partial_load_address(ACC, s); truebool.code_ref(s); s << endl; 
	emit_beqz(T1, label_num, s);
	emit_partial_load_address(ACC, s); falsebool.code_ref(s); s << endl;

	emit_label_def(label_num++, s);
}

void int_const_class::code(ostream &s)
{
	//
	// Need to be sure we have an IntEntry *, not an arbitrary Symbol
	//
	emit_load_int(ACC, inttable.lookup_string(token->get_string()), s);
}

void string_const_class::code(ostream &s)
{
	emit_load_string(ACC, stringtable.lookup_string(token->get_string()), s);
}

void bool_const_class::code(ostream &s)
{
	emit_load_bool(ACC, BoolConst(val), s);
}

void new__class::code(ostream &s)
{
	Symbol new_type = type_name;
	if (new_type == SELF_TYPE) {
		emit_load_address(T1, CLASSOBJTAB, s);
		emit_load(T2, TAG_OFFSET, SELF, s);
		emit_sll(T2, T2, 3, s);
		emit_addu(T1, T1, T2, s);
		// T1 nows holds the pointer to proto object
		emit_store(T1, 0, SP, s);
		emit_addiu(SP, SP, -4, s);
		emit_load(ACC, 0, T1, s);
		emit_jal(OBJECT_COPY, s);
		// Init method
		emit_load(T1, 1, SP, s);
		emit_addiu(SP, SP, 4, s);
		emit_load(T1, 1, T1, s);
		emit_jalr(T1, s);
		return;
	}

	emit_partial_load_address(ACC, s); s << new_type->get_string() << PROTOBJ_SUFFIX << endl;
	emit_jal(OBJECT_COPY, s);

	char* class_init = new char[strlen(new_type->get_string()) + strlen(CLASSINIT_SUFFIX)];
	strncpy(class_init, new_type->get_string(), new_type->get_len());
	strcat(class_init, CLASSINIT_SUFFIX);
	emit_jal(class_init, s);
	
}

void isvoid_class::code(ostream &s)
{
	e1->code(s);
	emit_move(T1, ACC, s);
	emit_partial_load_address(ACC, s); truebool.code_ref(s); s << endl;
	emit_beqz(T1, label_num, s);
	emit_partial_load_address(ACC, s); falsebool.code_ref(s); s << endl;

	emit_label_def(label_num++, s);
}

void no_expr_class::code(ostream &s)
{
}

void object_class::code(ostream &s)
{
	// self
	if (name == self) {
		emit_move(ACC, SELF, s);
		return;
	}
	// local variable from let
	int* local_offset = env->local_map->lookup(name);
	if (local_offset != NULL)  {
		emit_load(ACC, *local_offset, FP, s);
		return;
	}
	// method parameter
	if (env->so->get_method() != INIT_METHOD) {
		int arg_offset = env->so->get_method()->get_arg_offset(name);
		int local_num = env->so->get_method()->get_local_var_num();
		if (arg_offset != -1) {
			emit_load(ACC, local_num + arg_offset, FP, s);
			return;
		}
	}
	// object attribute
	// cout << "object " << name->get_string() << " class" <<  env->so->get_node()->get_name()->get_string() << endl;
	int attr_offset = env->attr_map->get_attr_offset(env->so->get_node()->get_name(), name);
	if (attr_offset != -1) {
		emit_load(ACC, attr_offset, SELF, s);
		return;
	}
}

int method_class::get_arg_offset(Symbol arg) {
	for(int i = formals->first(); formals->more(i); i = formals->next(i)) {
		if (formals->nth(i)->get_name() == arg) {
			return FRAME_ARG0_OFFSET + formals->len() - i - 1;
		}
	}
	return -1;
}
