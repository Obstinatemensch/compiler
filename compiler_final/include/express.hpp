enum exprType
{
    Leaf,
    NonLeaf,
    FunCall
};
enum types
{
    Int,
    Bool,
    Unknown,
    Variable
};
enum stmtTypes
{
    write,
    read,
    assign,
    cond,
    funccall
};
enum condStmtType
{
    If,
    IfElse,
    While,
    For
};

struct expr
{
    expr *left;
    expr *right;

    void *attr;
    exprType type;
    types valType;
};

struct intExpr
{
    int val;
    types type;
};

struct boolExpr
{
    bool val;
    types type;
};

struct exprList
{
    expr *exp;
    exprList *next;
};

struct varExpr
{

    char *varName;
    bool isArray;
    exprList *startIndex;
    exprList *endIndex;
};

struct func_call
{
    char *funName;
    exprList *exprlist;
};

struct opSymbol
{
    char sym;
    types type;
};

struct result
{
    types type;
    void *value; // should be int * or bool *
};

struct result_list
{
    result *res;
    result_list *next;
};

struct value
{
    char *name;
    void *val; // int * or arrayInfo *
    types type;
    bool isArray;
};

struct indexList
{
    int i;
    indexList *next;
};

struct declVarNode
{
    char *name;
    indexList *startIndex;
    indexList *endIndex;
    bool isArray;
};

struct var_list
{
    char *name;
    var_list *next;
};

struct arg
{
    types type;
    var_list *varList;
};

struct arg_list
{
    arg *Arg;
    arg_list *next;
};

struct func
{
    char *name;
    arg_list *argList;
};

struct arrayInfo
{
    void *arr; // should be int * or bool *
    indexList *indexLst;
    int arrayDim;
};

struct Glist
{

    void *node; // declVarNode * or func*
    bool isGid;
    Glist *next;
};

struct declaration
{
    types type;
    Glist *list;
    declaration *next;
};

struct statement
{
    void *stmt;
    stmtTypes stmtType;
    statement *next;
};

struct Fdef
{
    types retType;
    char *funcName;
    arg_list *FargList;
    declaration *LdeclSec;
    statement *stmtList;
    expr *retStmt;
    Fdef *next;
};

struct funcDetails
{
    func *Func;
    Fdef *fdef;
    bool isDefSet;
    types declType;
};

struct main_block
{
    types return_type;
    declaration *local_declaration;
    statement *stmt_list;
    expr *return_stmt;
};

struct Program
{
    declaration *global_declarations;
    Fdef *fdef_sec;
    main_block *mainBlock;
};

struct str_expr
{
    char *var;
    str_expr *next;
};

// structure for statements
struct assign_stmt
{
    varExpr *var;
    expr *exp;
};

struct write_stmt
{
    bool isExpr;
    void *val;
};

struct read_stmt
{
    varExpr *varexp;
};

struct cond_stmt
{
    condStmtType type;
    void *cond_stmt_attrs; // if_stmt * , if_else_stmt *,  while_stmt * or for_loop *
};

struct if_stmt
{
    expr *cond;
    statement *stmtList;
};

struct if_else_stmt
{
    expr *cond;
    statement *if_stmtList;
    statement *else_stmtList;
};

struct while_stmt
{
    expr *cond;
    statement *stmtList;
};

struct for_loop
{
    assign_stmt *startAssign;
    expr *cond;
    assign_stmt *updateAssign;
    statement *stmtList;
};

expr *addLeafExpr(int val);
expr *addLeafExpr(bool val);
expr *addLeafExpr(varExpr *var);
expr *addLeafExpr(func_call *_func_call);

expr *addNonLeafExpr(expr *left, char opSymbol, expr *right);
varExpr *createVarExpr(char *varName);
varExpr *createVarExpr(varExpr *varExp, expr *index);

str_expr *createStrExpr(char *name);
str_expr *createStrExpr(char *name, str_expr *strExp);

// declaration statement functions
value *createValue(char *varName, types type);
value *createValue(char *varName, types type, indexList *startIndex);
declVarNode *createDeclNode(char *var);
declVarNode *createDeclNode(declVarNode *node, int size);

declaration *createDeclaration(types type, Glist *top);

declaration *createDeclarationList(declaration *node, declaration *list);

var_list *createVarList(char *name);

var_list *createVarList(char *name, var_list *varList);

arg *createArg(types type, var_list *varList);

arg_list *createArgList(arg *Arg);
arg_list *createArgList(arg *Arg, arg_list *argList);

exprList *createExprList(expr *exp);
exprList *createExprList(expr *exp, exprList *exprlist);

func *createFunc(char *name, arg_list *argList);

void addGlobalDeclarationBlock(declaration *root);
void printGlobalDecl(declaration *root);

Glist *createGlist(declVarNode *node);
Glist *createGlist(declVarNode *node, Glist *glist);

Glist *createGlist(func *node);
Glist *createGlist(func *node, Glist *glist);

Fdef *createFdef(types ret_type, char *name, arg_list *argList, declaration *local_decl, statement *stmt_list, expr *ret_stmt);
Fdef *createFdefSec(Fdef *fdef, Fdef *fdefSec);
void addFdef_sec(Fdef *fdefSec);

// assignment statement functions

statement *createAssignStmt(varExpr *var, expr *exp);
void printAssignStmt(assign_stmt *stmt);

// write statement functions

statement *createWriteStmt(expr *expr);
statement *createWriteStmt(str_expr *strExpr);
void printWriteStmt(write_stmt *stmt);

// read statement functions
statement *createReadStmt(varExpr *varexp);

// conditional statement functions
statement *createIfStmt(expr *cond_expr, statement *stmt_list);
statement *createIfElseStmt(expr *cond_expr, statement *if_stmt_list, statement *else_stmt_list);
statement *createWhileStmt(expr *cond_expr, statement *stmt_list);
statement *createForStmt(statement *startAssign, expr *condition, statement *updateAssign, statement *stmtList);
void printCondStmt(cond_stmt *stmt);

// func_call statement functions
func_call *createfunc_call(char *name, exprList *exprlist);
statement *createFuncStmt(func_call *funcCall);

// helper function for statements
statement *createStmtList(statement *stmt, statement *stmtList);
void printStmtList(statement *stmt);
void printStmt(statement *stmt);

// main block helper functions
main_block *createMainBlock(types ret_type, declaration *local_decl, statement *stmt_list, expr *ret_stmt);
void addMainBlock(main_block *mainBlock);

void PrintProgram();

// Printing functions
void printResult(result *val);
void generate_c_code();
