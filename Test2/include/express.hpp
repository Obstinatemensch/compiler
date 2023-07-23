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
    Write,
    assign,
    cond
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

struct varExpr
{
    char *varName;
    bool isArray;
    expr *index;
    types type;
};

struct funCall
{
    char *funName;
    expr *exprList;
};

struct opSymbol
{
    char sym;
    types type;
};

struct result
{
    types type;
    void *value; 
};

struct value
{
    char *name;
    void *val; 
    types type;
    bool isArray;
};

struct declarationNode
{
    value *var;
    declarationNode *next;
};

struct arrayInfo
{
    void *arr;
    int size;
};

struct declaration
{
    types type;
    declarationNode *list;
    declaration *next;
};

struct statement
{
    void *stmt;
    stmtTypes stmtType;
    statement *next;
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

struct cond_stmt
{
    condStmtType type;
    void *cond_stmt_attrs;
};

struct if_stmt
{
    expr *cond;
    statement *stmtList;
};

struct if_else_stmt
{
    expr *condition;
    statement *ifStatementList;
    statement *elseStatementList;
};

struct while_stmt
{
    expr *condition;
    statement *statementList;
};

expr *addLeafExpr(int val);
expr *addLeafExpr(bool val);
expr *addLeafExpr(varExpr *var);
expr *addNonLeafExpr(expr *left, char opSymbol, expr *right);
result *solve(expr *root);
varExpr *createVarExpr(char *varName);
varExpr *createVarExpr(char *varName, expr *index);
str_expr *createStrExpr(char *name);
str_expr *createStrExpr(char *name, str_expr *strExp);

// declaration statement functions
value *createVal(char *varName);
value *createVal(char *varName, int maxSize);
declarationNode *createDeclNode(char *var);
declarationNode *createDeclNode(char *var, int size);

declaration *createDeclaration(types type, declarationNode *top);

declarationNode *createDeclList(declarationNode *node, declarationNode *list);

declaration *createDeclarationList(declaration *node, declaration *list);

void addDeclarationBlock(declaration *root);
void printGlobalDecl(declaration *root);

// assignment statement functions

statement *createAssignStmt(varExpr *var, expr *exp);
void printAssignStmt(assign_stmt *stmt);

// write statement functions

statement *createWriteStmt(expr *expr);
statement *createWriteStmt(str_expr *strExpr);
void printWriteStmt(write_stmt *stmt);

// conditional statement functions
statement *createIfStmt(expr *cond_expr, statement *stmt_list);
statement *createIfElseStmt(expr *cond_expr, statement *if_stmt_list, statement *else_stmt_list);
statement *createWhileStmt(expr *cond_expr, statement *stmt_list);
void printCondStmt(cond_stmt *stmt);

// helper function for statements
statement *createStmtList(statement *stmt, statement *stmtList);
void printStmtList(statement *stmt);
void printStmt(statement *stmt);

// main block helper functions
main_block *createMainBlock(types ret_type, declaration *local_decl, statement *stmt_list, expr *ret_stmt);
void addMainBlock(main_block *mainBlock);

// function for printing main block and code execution
void execProg();
void execGlobalDeclaration();
void execMainBlock();

void printMainBlock();
void PrintProgram();
void printGlobalSymbolTable();

// Some functions for printing
void printResult(result *val);