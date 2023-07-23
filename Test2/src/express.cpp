#include "../include/express.hpp"
#include <bits/stdc++.h>
using namespace std;

map<string, value *> symbol_table;
Program prog = {NULL, NULL};

void addVar(char *name, types type, map<string, value *> &sym_tab = symbol_table);
void addVar(char *name, types type, int a, map<string, value *> &sym_tab = symbol_table);

void update(char *name, int val, map<string, value *> &sym_tab = symbol_table);
void update(char *name, int index, int val, map<string, value *> &sym_tab = symbol_table);

result *lookup(char *name, map<string, value *> &sym_tab = symbol_table);
result *lookup(char *name, int index, map<string, value *> &sym_tab = symbol_table);

void execStmtList(statement *stmt_list);

varExpr *createVarExpr(char *varName)
{
    varExpr *a = new varExpr;
    a->type = Unknown;
    a->varName = strndup(varName, strlen(varName));
    a->isArray = false;
    a->index = NULL;
    return a;
}

varExpr *createVarExpr(char *varName, expr *index)
{
    varExpr *a = new varExpr;
    a->type = Unknown;
    a->varName = strndup(varName, strlen(varName));
    a->isArray = true;
    a->index = index;
    return a;
}

expr *addLeafExpr(varExpr *var)
{
    expr *newNode = new expr;
    newNode->attr = var;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->type = Leaf;
    newNode->valType = Variable;

    return newNode;
}

expr *addLeafExpr(bool val)
{
    cout << "bool terminal made!!\n";

    boolExpr *a = new boolExpr;
    a->type = Bool;
    a->val = val;

    expr *newPtr = new expr;
    newPtr->attr = move(a);
    newPtr->left = NULL;
    newPtr->right = NULL;
    newPtr->type = Leaf;
    newPtr->valType = Bool;

    return newPtr;
}

expr *addLeafExpr(int val)
{
    intExpr *a = new intExpr;
    a->type = Int;
    a->val = val;

    expr *newNode = new expr;
    newNode->attr = move(a);
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->type = Leaf;
    newNode->valType = Int;

    return newNode;
}

types opType(char sym)
{
    switch (sym)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '<':
    case '>':
    case 'g':
    case 'l':
    case 'n':
    case 'e':
        return Int;
    case '!':
    case '&':
    case '|':
        return Bool;
    default:
        cerr << "Invalid Operator";
        exit(0);
    }
    return Unknown;
}

expr *addNonLeafExpr(expr *left, char opSym, expr *right)
{
    opSymbol *a = new opSymbol;
    a->type = opType(opSym);
    a->sym = opSym;

    expr *newNode = new expr;
    newNode->attr = move(a);
    newNode->left = left;
    newNode->right = right;
    newNode->type = NonLeaf;
    newNode->valType = Unknown;

    return newNode;
}

result *solve(expr *node)
{
    result *a = new result;
    if (node->type == Leaf)
    {
        if (node->valType == Int)
        {
            a->type = ((intExpr *)(node->attr))->type;
            a->value = &((intExpr *)(node->attr))->val;
        }
        else if (node->valType == Bool)
        {
            a->type = ((boolExpr *)(node->attr))->type;
            a->value = &((boolExpr *)(node->attr))->val;
        }
        else if (node->valType == Variable)
        {
            varExpr *varE = (varExpr *)(node->attr);
            if (!(varE->isArray))
            {
                result *val = lookup(varE->varName);
                a = val;
            }
            else
            {
                result *i = solve(varE->index);
                if (i->type == Int)
                {
                    result *val = lookup(varE->varName, *((int *)(i->value)));
                    a = val;
                }
                else
                {
                    cout << "Expression giving bool in the index of the array." << endl;
                    exit(1);
                }
            }
        }
        else
        {
            cout << "invalid Type for a Terminal";
            exit(0);
        }
    }
    else if (node->type == NonLeaf)
    {
        if (((opSymbol *)(node->attr))->type == Bool)
        {
            switch (((opSymbol *)(node->attr))->sym)
            {
            case '&':
                a->value = new bool((*(bool *)(solve(node->left)->value)) & (*(bool *)(solve(node->right)->value)));
                a->type = Bool;
                break;
            case '|':
                a->value = new bool((*(bool *)(solve(node->left)->value)) | (*(bool *)(solve(node->right)->value)));
                a->type = Bool;
                break;
            case '!':
                a->value = new bool(!(*(bool *)(solve(node->right)->value)));
                a->type = Bool;
                break;
            default:
                cout << "Invalid operator";
                exit(1);
            }
        }
        else if (((opSymbol *)(node->attr))->type == Int)
        {
            switch (((opSymbol *)(node->attr))->sym)
            {
            // for the operations that have int as their type
            case '+':
                a->value = new int((*(int *)(solve(node->left)->value)) + (*(int *)(solve(node->right)->value)));
                a->type = Int;
                break;
            case '-':
                a->value = new int((*(int *)(solve(node->left)->value)) - (*(int *)(solve(node->right)->value)));
                a->type = Int;
                break;
            case '/':
                a->value = new int((*(int *)(solve(node->left)->value)) / (*(int *)(solve(node->right)->value)));
                a->type = Int;
                break;
            case '*':
                a->value = new int((*(int *)(solve(node->left)->value)) * (*(int *)(solve(node->right)->value)));
                a->type = Int;
                break;
            case '%':
                a->value = new int((*(int *)(solve(node->left)->value)) % (*(int *)(solve(node->right)->value)));
                a->type = Int;
                break;

            // operations that have bool as their type
            case '>':
                a->value = new bool((*(int *)(solve(node->left)->value)) > (*(int *)(solve(node->right)->value)));
                a->type = Bool;
                break;
            case '<':
                a->value = new bool((*(int *)(solve(node->left)->value)) < (*(int *)(solve(node->right)->value)));
                a->type = Bool;
                break;
            case 'g':
                a->value = new bool((*(int *)(solve(node->left)->value)) >= (*(int *)(solve(node->right)->value)));
                a->type = Bool;
                break;
            case 'l':
                a->value = new bool((*(int *)(solve(node->left)->value)) <= (*(int *)(solve(node->right)->value)));
                a->type = Bool;
                break;
            case 'n':
                a->value = new bool((*(int *)(solve(node->left)->value)) != (*(int *)(solve(node->right)->value)));
                a->type = Bool;
                break;
            case 'e':
                a->value = new bool((*(int *)(solve(node->left)->value)) == (*(int *)(solve(node->right)->value)));
                a->type = Bool;
                break;
            default:
                cerr << "Invalid operator";
                exit(1);
            }
        }
        else
        {
            cerr << "Invalid Expr Node detected";
            exit(0);
        }
    }
    return a;
}

void printExpression(expr *node)
{
    result *a = new result;

    if (node->type == Leaf)
    {
        if (node->valType == Int)
        {
            cout << "NUM ";
        }
        else if (node->valType == Bool)
        {
            cout << "BOOL ";
        }
        else if (node->valType == Variable)
        {
            varExpr *varE = (varExpr *)(node->attr);
            if (!(varE->isArray))
            {
                cout << "VAR ";
            }
            else
            {
                cout << "ARRREF VAR ";
                printExpression(varE->index);
            }
        }
        else
        {
            cout << "Invalid Type";
            exit(0);
        }
    }
    else if (node->type == NonLeaf)
    {
        if (((opSymbol *)(node->attr))->type == Bool)
        {
            switch (((opSymbol *)(node->attr))->sym)
            {
            case '&':
                cout << "LOGICAL_AND ";
                printExpression(node->left);
                printExpression(node->right);
                break;
            case '|':
                cout << "LOGICAL_OR ";
                printExpression(node->left);
                printExpression(node->right);
                break;
            case '!':
                cout << "LOGICAL_NOT ";
                printExpression(node->right);
                break;
            default:
                cout << "Invalid operator ";
                exit(1);
            }
        }
        else if (((opSymbol *)(node->attr))->type == Int)
        {
            switch (((opSymbol *)(node->attr))->sym)
            {
            // operations that result in int
            case '+':
                cout << "PLUS ";
                break;
            case '-':
                cout << "SUB ";
                break;
            case '/':
                cout << "DIV ";
                break;
            case '*':
                cout << "MUL ";
                break;
            case '%':
                cout << "MOD ";
                break;

            // operations that result in bool
            case '>':
                cout << "MORE ";
                break;
            case '<':
                cout << "LESS ";
                break;
            case 'g':
                cout << "GREATERTHANOREQUAL ";
                break;
            case 'l':
                cout << "LESSTHANOREQUAL ";
                break;
            case 'n':
                cout << "NOTEQUAL ";
                break;
            case 'e':
                cout << "EQUALEQUAL ";
                break;
            default:
                cout << "Invalid operator";
                exit(1);
            }
            printExpression(node->left);
            printExpression(node->right);
        }
        else
        {
            cerr << "Invalid Expresssion Node detected";
            exit(0);
        }
    }
}

// Symbol table functions

void addVar(char *name, types type, map<string, value *> &sym_tab)
{
    if (sym_tab.count(name) > 0)
    {
        cerr << "Multiple declarations of the same variable " << name;
        exit(1);
    }
    else
    {
        value *v = createVal(name);
        v->type = type;
        sym_tab[name] = v;
    }
}

void addVar(char *name, types type, int a, map<string, value *> &sym_tab)
{
    if (sym_tab.count(name) > 0)
    {
        cerr << "Multiple declarations of the same variable " << name;
        exit(1);
    }
    else
    {
        value *v = createVal(name, a);
        v->type = type;
        sym_tab[name] = v;
    }
}

result *lookup(char *name, map<string, value *> &sym_tab)
{
    if (sym_tab.count(name) == 0)
    {
        cout << "Undeclared Variable " << name << endl;
        exit(1);
    }
    else
    {
        value *v = sym_tab[name];
        result *out = new result;
        if (v->type == Int)
        {
            if (v->isArray)
            {
                cout << name << " The value of the array cannot be changed without the index." << endl;
                exit(1);
            }
            else
            {
                out->type = Int;
                out->value = (int *)(v->val);
                return out;
            }
        }
        else if (v->type == Bool)
        {
            if (v->isArray)
            {
                cout << name << " The value of the array cannot be changed without the index." << endl;
                exit(1);
            }
            else
            {
                out->type = Bool;
                out->value = (int *)(v->val);
                return out;
            }
        }
        else
        {
            cout << "Invalid type for variable " << name << endl;
            exit(1);
        }
    }
    return nullptr;
}

result *lookup(char *name, int index, map<string, value *> &sym_tab)
{
    if (sym_tab.count(name) == 0)
    {
        cout << "Undeclared Variable " << name << endl;
        exit(1);
    }
    else
    {
        value *v = sym_tab[name];
        result *out = new result;
        if (v->type == Int)
        {
            if (!v->isArray)
            {
                cout << name << " is not indexable. Variable not an array. " << endl;
                exit(1);
            }
            else
            {
                arrayInfo *arr = (arrayInfo *)(v->val);
                if (index < 0 || index >= arr->size)
                {
                    cerr << "Index out of range for array " << name << " " << endl;
                    exit(1);
                }
                else
                {
                    out->type = Int;
                    out->value = &(((int *)(arr->arr))[index]);
                    return out;
                }
            }
        }
        else if (v->type == Bool)
        {
            if (!v->isArray)
            {
                cout << name << " is not indexable. Variable not an array. " << endl;
                exit(1);
            }
            else
            {
                arrayInfo *arr = (arrayInfo *)(v->val);
                if (index < 0 || index >= arr->size)
                {
                    cerr << "Index out of range for array " << name << " " << endl;
                    exit(1);
                }
                else
                {
                    out->type = Bool;
                    out->value = &(((int *)(arr->arr))[index]);
                    return out;
                }
            }
        }
        else
        {
            cout << "Invalid type for variable " << name << endl;
            exit(1);
        }
    }
    return nullptr;
}
void update(char *name, int index, int val, std::map<std::string, value *> &sym_tab)
{
    if (sym_tab.count(name) <= 0)
    {
        std::cerr << "Undeclared Variable: " << name << std::endl;
        exit(1);
    }
    value *v = sym_tab[name];
    if (v->type == Int)
    {
        if (!(v->isArray))
        {
            std::cerr << name << " is not indexable. Variable not an array." << std::endl;
            exit(1);
        }
        else
        {
            arrayInfo *arr = (arrayInfo *)(v->val);
            if (index >= arr->size)
            {
                std::cerr << "Index out of range for array " << name << std::endl;
                exit(1);
            }
            else
            {
                int *arrPtr = (int *)(arr->arr);
                arrPtr[index] = val;
            }
        }
    }
    else
    {
        std::cerr << "Need to complete for bool" << std::endl;
        exit(1);
    }
}

void update(char *name, int val, map<string, value *> &sym_tab)
{
    auto it = sym_tab.find(name);
    if (it == sym_tab.end())
    {
        cerr << "Undeclared Variable. " << name << endl;
        exit(1);
    }

    value *v = it->second;
    if (v->type == Int)
    {
        if (v->isArray)
        {
            cerr << name << " because its a array, its value cannot be assigned without an index." << endl;
            exit(1);
        }
        else
        {
            *((int *)(v->val)) = val;
        }
    }
    else
    {
        cerr << "Unsupported data type." << endl;
        exit(1);
    }
}

value *createVal(char *varName)
{
    value *v = new value;
    v->name = strndup(varName, strlen(varName));
    v->type = Int;
    v->val = new int;
    v->isArray = false;
    return v;
}
value *createVal(char *varName, int maxSize)
{
    value *v = new value;
    v->name = strndup(varName, strlen(varName));
    v->type = Int;
    arrayInfo *a = new arrayInfo;
    a->arr = new int[maxSize];
    a->size = maxSize;

    v->val = a;
    v->isArray = true;
    return v;
}

declarationNode *createDeclNode(char *varName)
{
    value *var = createVal(varName);

    declarationNode *newPtr = new declarationNode;
    newPtr->var = var;
    newPtr->next = NULL;
    return newPtr;
}

declarationNode *createDeclNode(char *varName, int size)
{
    value *var = createVal(varName, size);

    declarationNode *newPtr = new declarationNode;
    newPtr->var = var;
    newPtr->next = NULL;
    return newPtr;
}

declarationNode *createDeclList(declarationNode *node, declarationNode *lst)
{
    node->next = lst;
    return node;
}

declaration *createDeclaration(types type, declarationNode *top)
{
    declaration *newPtr = new declaration;
    newPtr->type = type;
    newPtr->list = top;
    newPtr->next = NULL;

    return newPtr;
}

declaration *createDeclarationList(declaration *node, declaration *list)
{
    node->next = list;

    return node;
}

void addDeclarationBlock(declaration *root)
{
    prog.global_declarations = root;
}

void printDeclVarNodes(declarationNode *list)
{
    for (declarationNode *ptr = list; ptr != nullptr; ptr = ptr->next)
    {
        std::cout << "VAR" << ' ';
    }
}

void printDeclstmt(const declaration *root)
{
    const char *typeStr = "invalid type";
    switch (root->type)
    {
    case Int:
        std::cout << "INT" << ' ';
        break;
    case Bool:
        std::cout << "BOOL" << ' ';
        break;
    }
    printDeclVarNodes(root->list);
    std::cout << endl;
}

void printGlobalDecl(declaration *root)
{
    for (declaration *ptr = root; ptr != nullptr; ptr = ptr->next)
    {
        printDeclstmt(ptr);
    }
}

// assignment statement helper function

statement *createAssignStmt(varExpr *var, expr *exp)
{
    assign_stmt *newPtr = new assign_stmt;
    newPtr->var = var;
    newPtr->exp = exp;

    statement *newStmt = new statement;
    newStmt->stmt = newPtr;
    newStmt->stmtType = assign;
    newStmt->next = NULL;
    return newStmt;
}

void printAssignStmt(assign_stmt *stmt)
{
    cout << "ASSIGN ";
    if (stmt->var->isArray)
    {
        cout << "ARRREF VAR ";
        printExpression(stmt->var->index);
    }
    else
    {
        cout << "VAR ";
    }

    printExpression(stmt->exp);
}

str_expr *createStrExpr(char *name)
{
    str_expr *newPtr = new str_expr;
    newPtr->var = strndup((name), strlen(name));
    newPtr->next = NULL;
    return newPtr;
}

str_expr *createStrExpr(char *name, str_expr *strExp)
{
    str_expr *newPtr = new str_expr;
    newPtr->var = strndup((name), strlen(name));
    newPtr->next = strExp;
    return newPtr;
}

void printStrExprList(str_expr *list)
{
    if (list == NULL)
    {
        cout << "the list object is NULL\n";
    }
    while (list != NULL)
    {
        if (list->var == NULL)
        {
            cout << "null string";
        }
        cout << list->var << " ";
        list = list->next;
    }
    cout << endl;
}

// print ast functions

statement *createWriteStmt(expr *expr)
{
    write_stmt *newPtr = new write_stmt;
    newPtr->val = expr;
    newPtr->isExpr = true;

    statement *newStmt = new statement;
    newStmt->next = NULL;
    newStmt->stmtType = Write;
    newStmt->stmt = newPtr;
    return newStmt;
}

statement *createWriteStmt(str_expr *strExpr)
{
    write_stmt *newPtr = new write_stmt;
    newPtr->val = strExpr;
    newPtr->isExpr = false;

    statement *newStmt = new statement;
    newStmt->next = NULL;
    newStmt->stmtType = Write;
    newStmt->stmt = newPtr;
    return newStmt;
}

void printWriteStmt(write_stmt *stmt)
{
    cout << "FUNC ";
    if (stmt->isExpr)
    {
        printExpression((expr *)(stmt->val));
    }
    else
    {
        printStrExprList((str_expr *)(stmt->val));
    }
}

statement *createIfStmt(expr *cond_expr, statement *stmt_list)
{
    if_stmt *newIf = new if_stmt;
    newIf->cond = cond_expr;
    newIf->stmtList = stmt_list;

    cond_stmt *newCondStmt = new cond_stmt;
    newCondStmt->cond_stmt_attrs = newIf;
    newCondStmt->type = If;

    statement *newStmt = new statement;
    newStmt->stmt = newCondStmt;
    newStmt->stmtType = cond;
    newStmt->next = NULL;
    return newStmt;
}

void printIfStmt(if_stmt *stmt)
{
    cout << "\nIF ";
    printExpression(stmt->cond);
    cout << endl
         << " THEN" << endl;
    printStmtList(stmt->stmtList);
    cout << "ENDIF" << endl;
}

statement *createIfElseStmt(expr *cond_expr, statement *if_stmt_list, statement *else_stmt_list)
{
    if_else_stmt *newIfElse = new if_else_stmt;
    newIfElse->condition = cond_expr;
    newIfElse->ifStatementList = if_stmt_list;
    newIfElse->elseStatementList = else_stmt_list;

    cond_stmt *newCondStmt = new cond_stmt;
    newCondStmt->cond_stmt_attrs = newIfElse;
    newCondStmt->type = IfElse;

    statement *newStmt = new statement;
    newStmt->stmt = newCondStmt;
    newStmt->stmtType = cond;
    newStmt->next = NULL;
    return newStmt;
}

void printIfElseStmt(if_else_stmt *stmt)
{
    cout << "\nIF ";
    printExpression(stmt->condition);
    cout << " THEN " << endl;
    printStmtList(stmt->ifStatementList);
    cout << "ELSE" << endl;
    printStmtList(stmt->elseStatementList);
    cout << "ENDIF" << endl;
}

statement *createWhileStmt(expr *cond_expr, statement *stmt_list)
{
    while_stmt *newWhile = new while_stmt;
    newWhile->condition = cond_expr;
    newWhile->statementList = stmt_list;

    cond_stmt *newCondStmt = new cond_stmt;
    newCondStmt->cond_stmt_attrs = newWhile;
    newCondStmt->type = While;

    statement *newStmt = new statement;
    newStmt->stmt = newCondStmt;
    newStmt->stmtType = cond;
    newStmt->next = NULL;
    return newStmt;
}

void printWhileStmt(while_stmt *stmt)
{
    cout << "\nWHILE ";
    printExpression(stmt->condition);
    cout << " DO " << endl;
    printStmtList(stmt->statementList);
    cout << "ENDWHILE" << endl;
}

statement *createStmtList(statement *stmt, statement *stmtList)
{
    stmt->next = stmtList;
    return stmt;
}

void printStmt(statement *stmt)
{
    switch (stmt->stmtType)
    {
    case Write:
        printWriteStmt((write_stmt *)(stmt->stmt));
        break;
    case assign:
        printAssignStmt((assign_stmt *)(stmt->stmt));
        break;
    case cond:
        printCondStmt((cond_stmt *)(stmt->stmt));
        break;
    default:
        printf("Unknown statement type\n");
        break;
    }
}

void printCondStmt(cond_stmt *stmt)
{
    switch (stmt->type)
    {
    case While:
        printWhileStmt(static_cast<while_stmt *>(stmt->cond_stmt_attrs));
        break;
    case If:
        printIfStmt(static_cast<if_stmt *>(stmt->cond_stmt_attrs));
        break;
    case IfElse:
        printIfElseStmt(static_cast<if_else_stmt *>(stmt->cond_stmt_attrs));
        break;
    default:
        cout << "Invalid conditional statement" << endl;
        break;
    }
}

// functions to implement the main block

main_block *createMainBlock(types ret_type, declaration *local_decl, statement *stmt_list, expr *ret_stmt)
{
    main_block *mainBlock = new main_block{ret_type, local_decl, stmt_list, ret_stmt};
    return mainBlock;
}

void addMainBlock(main_block *mainBlock)
{
    prog.mainBlock = mainBlock;
}

// functions to execute

void execProg()
{
    cout<<endl;

    cout<<"----------------------------"<<endl;
    cout<<"        Output:";
    execGlobalDeclaration();
    execMainBlock();
}

void addDeclaration(map<string, value *> &sym_tab, declaration *decl)
{
    declarationNode *node = decl->list;
    while (node != NULL)
    {
        if (!(node->var->isArray))
        {
            addVar(node->var->name, decl->type);
        }
        else
        {
            addVar(node->var->name, decl->type, ((arrayInfo *)(node->var->val))->size);
        }
        node = node->next;
    }
}

void addDeclarationList(map<string, value *> &sym_tab, declaration *decl_list)
{
    while (decl_list != NULL)
    {
        addDeclaration(sym_tab, decl_list);
        decl_list = decl_list->next;
    }
}

void execGlobalDeclaration()
{
    addDeclarationList(symbol_table, prog.global_declarations);
}

void printStmtList(statement *stmt)
{

    while (stmt != NULL)
    {
        printStmt(stmt);
        stmt = stmt->next;
        cout << endl;
    }
}
void printType(types type)
{
    if (type == Int)
    {
        cout << "int";
    }
    else if (type == Bool)
    {
        cout << "bool";
    }
}

void printBoolean(bool b)
{
    if (b)
    {
        cout << "true ";
    }
    else
    {
        cout << "false ";
    }
}

void printBoolArray(bool *lst, int size)
{
    for (int i = 0; i < size; i++)
    {
        printBoolean(lst[i]);
    }
}

void printIntArray(int *lst, int size)
{
    for (int i = 0; i < size; i++)
    {
        cout << lst[i] << " ";
    }
}

void printSymbolTable(const map<std::string, value *> &sym_tab)
{
    for (auto it = symbol_table.begin(); it != symbol_table.end(); it++)
    {
        cout << it->first << "\t: ";
        value *variable = it->second;

        if (variable->isArray)
        {
            arrayInfo *arr = (arrayInfo *)(it->second->val);
            cout << "(";
            printType(variable->type);
            cout << ") ";

            if (variable->type == Int)
            {
                printIntArray((int *)(arr->arr), arr->size);
            }
            else if (variable->type == Bool)
            {
                printBoolArray((bool *)(arr->arr), arr->size);
            }

            cout << "[" << arr->size << "]" << endl;
        }
        else
        {
            cout << "(";
            printType(variable->type);
            cout << ") ";

            if (variable->type == Int)
            {
                cout << *(int *)(variable->val) << endl;
            }
            else if (variable->type == Bool)
            {
                cout << ") ";
                printBoolean(*(bool *)(variable->val));
                cout << endl;
            }
        }
    }
}

void printGlobalSymbolTable()
{
    cout << "----------------------------" << endl;
    cout << "       Symbol Table         \n";
    cout << "----------------------------\n";
    printSymbolTable(symbol_table);
    cout<<endl;
}

// execution functions

void executeWriteStatement(write_stmt *writeStmt)
{
    if (writeStmt->isExpr)
    {
        result *value = solve((expr *)(writeStmt->val));
        if (value->type == Int)
        {
            cout << *(int *)(value->value) << endl;
        }
        else if (value->type == Bool)
        {
            printBoolean(*(int *)(value->value));
            cout << endl;
        }
    }
    else
    {
        str_expr *list = (str_expr *)(writeStmt->val);
        printStrExprList(list);
    }
}

void execAssignStmt(assign_stmt *assignStmt)
{
    result *res = solve(assignStmt->exp);
    varExpr *var = assignStmt->var;

    if (var->isArray)
    {
        result *index = solve(var->index);
        if (index->type == Int)
            update(var->varName, *((int *)(index->value)), *((int *)(res->value)), symbol_table);
        else
        {
            cerr << "Index should be of Int type." << endl;
            exit(1);
        }
    }
    else
    {
        update(var->varName, *((int *)(res->value)), symbol_table);
    }
}

void executeIfElseStatement(if_else_stmt *ifElseStmt)
{
    result *conditionValue = solve(ifElseStmt->condition);
    if (conditionValue->type != Bool)
    {
        cerr << "Condition should be of type bool. Int type condition not supported" << std::endl;
        exit(1);
    }

    bool condition = *(bool *)(conditionValue->value);
    if (condition)
    {
        execStmtList(ifElseStmt->ifStatementList);
    }
    else
    {
        execStmtList(ifElseStmt->elseStatementList);
    }
    delete conditionValue;
}

void executeWhileStatement(while_stmt *whileStmt)
{
    result *conditionValue = solve(whileStmt->condition);
    if (conditionValue->type != Bool)
    {
        cerr << "Condition should be of type bool. Int type conditions are not supported" << std::endl;
        exit(1);
    }

    while (*(bool *)(conditionValue->value))
    {
        execStmtList(whileStmt->statementList);
        delete conditionValue;
        conditionValue = solve(whileStmt->condition);

        if (conditionValue->type != Bool)
        {
            cerr << "Condition should be of type bool. Int type conditions are not supported" << std::endl;
            exit(1);
        }
    }

    delete conditionValue;
}

void exectuteCondStatement(cond_stmt *condStmt)
{
    if (condStmt->type == IfElse)
    {
        executeIfElseStatement((if_else_stmt *)(condStmt->cond_stmt_attrs));
    }
    else if (condStmt->type == While)
    {
        executeWhileStatement((while_stmt *)(condStmt->cond_stmt_attrs));
    }
    else
    {
        cout << "Invalid conditional statement type." << endl;
        exit(1);
    }
}

void execStmt(statement *stmt)
{
    if (stmt->stmtType == Write)
    {
        executeWriteStatement((write_stmt *)(stmt->stmt));
    }
    else if (stmt->stmtType == assign)
    {
        execAssignStmt((assign_stmt *)(stmt->stmt));
    }
    else if (stmt->stmtType == cond)
    {
        exectuteCondStatement((cond_stmt *)(stmt->stmt));
    }
    else
    {
        cout << "Invalid statement type";
        exit(1);
    }
}

void execStmtList(statement *stmt_list)
{
    while (stmt_list != NULL)
    {
        execStmt(stmt_list);
        stmt_list = stmt_list->next;
    }
}

void execMainBlock()
{
    execStmtList(prog.mainBlock->stmt_list);
}

void printMainBlock()
{
    const main_block *mainBlock = prog.mainBlock;

    std::cout << "FUN ";
    if (mainBlock->return_type == Int)
    {
        std::cout << "INT ";
    }
    else
    {
        throw std::runtime_error("Unsupported data type. Only Int type supported for return type of main function.");
    }

    std::cout << "MAIN" << std::endl;
    printStmtList(mainBlock->stmt_list);

    std::cout << "RET ";
    printExpression(mainBlock->return_stmt);
    std::cout << std::endl;

    std::cout << "END MAIN" << std::endl;
}

void PrintProgram()
{
    cout << "----------------------------" << endl;
    cout << "   Abstract Syntax Tree           \n";
    cout << "----------------------------" << endl;
    printGlobalDecl(prog.global_declarations);
    printMainBlock();

}

// printing the result
void printResult(result *val)
{
    if (val->type == Int)
    {
        cout << *(int *)(val->value);
    }
    else if (val->type == Bool)
    {
        cout << *(bool *)(val->value);
    }
}