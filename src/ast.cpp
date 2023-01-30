#include "lexer.h"
#include "ast.h"

//===---------------------------------------------------------------------===//
// AST - This module implements the Abstract Syntax Tree (AST) for Anx
//
// The structure of the AST follows the following heirarchy:
// Node - The base type for AST nodes
//   GroupNode - A node that consists of a list of other nodes
//   DeclNode - A node declaring something
//     FunctionDecl - A function declaration
//     VariableDecl - A variable declaration
//   StmtNode - A node that represents a value or evaluates in some way
//     ReturnStmt - A return statement
//     IfStmt - An if statement
//     BinOpStmt - A binary operation statement
//     AssignStmt - A variable assignment statement
//     CallStmt - A function call statement
//     IntStmt - An integer literal statement
//     BoolStmt - A boolean literal statement
//     VoidStmt - A void literal statement
//     IdentifierStmt - An identifier
//     CastStmt - A typecast statement
//===---------------------------------------------------------------------===//
