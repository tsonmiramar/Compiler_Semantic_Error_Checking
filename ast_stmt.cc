/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"
#include "symtable.h"

Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    printf("\n");
}

void Program::Check() {
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */

    // sample test - not the actual working code
    // replace it with your own implementation
    
    symbolTable->push(); //Add a global scope table 

    if ( decls->NumElements() > 0 ) {
      for ( int i = 0; i < decls->NumElements(); ++i ) {
        Decl *d = decls->Nth(i);
        d->Check();
	/*
         * Basically you have to make sure that each declaration is 
         * semantically correct.
         */
      }
    }

    symbolTable->pop(); //Pop the global scope table
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

void StmtBlock::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    stmts->PrintAll(indentLevel+1);
}

void StmtBlock::Check(){
	for ( int i = 0; i < decls->NumElements(); i++ ){
		decls->Nth(i)->Check();
	}

	for ( int i = 0; i < stmts->NumElements(); i++ ){
		Stmt* stmt = stmts->Nth(i);
		StmtBlock* stmtBlk = dynamic_cast<StmtBlock*>(stmt);
		if ( stmtBlk != NULL ){
			symbolTable->push();
		}

		stmt->Check();

		if ( stmtBlk != NULL){
			symbolTable->pop();
		}
	}
}

DeclStmt::DeclStmt(Decl *d) {
    Assert(d != NULL);
    (decl=d)->SetParent(this);
}

//Semantic Check for DeclStmt
void DeclStmt::Check(){
	decl->Check();
}

void DeclStmt::PrintChildren(int indentLevel) {
    decl->Print(indentLevel+1);
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && b != NULL);
    (init=i)->SetParent(this);
    step = s;
    if ( s )
      (step=s)->SetParent(this);
}

void ForStmt::PrintChildren(int indentLevel) {
    init->Print(indentLevel+1, "(init) ");
    test->Print(indentLevel+1, "(test) ");
    if ( step )
      step->Print(indentLevel+1, "(step) ");
    body->Print(indentLevel+1, "(body) ");
}

//Semantic Check for For Stmt
void ForStmt::Check(){
	symbolTable->push();
	loop_switchStack->push(this);
	
	init->Check();
	test->Check();

	if(!(test->type == Type::boolType)) {
		ReportError::TestNotBoolean(test);
		test->type = Type::errorType;
	}

	if(step != NULL) {
		step->Check();
	}

	body->Check();

	loop_switchStack->pop();
	symbolTable->pop();
}

void WhileStmt::PrintChildren(int indentLevel) {
    test->Print(indentLevel+1, "(test) ");
    body->Print(indentLevel+1, "(body) ");
}

//Semantic Check for While Stmt
void WhileStmt::Check(){
	symbolTable->push();
	loop_switchStack->push(this);
	test->Check();
	
	if ( !(test->type == Type::boolType) ){
		ReportError::TestNotBoolean(test);
		test->type = Type::errorType;
	}

	body->Check();
	loop_switchStack->pop();
	symbolTable->pop();
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

void IfStmt::PrintChildren(int indentLevel) {
    if (test) test->Print(indentLevel+1, "(test) ");
    if (body) body->Print(indentLevel+1, "(then) ");
    if (elseBody) elseBody->Print(indentLevel+1, "(else) ");
}

//Semantic Check for IfStmt
void IfStmt::Check(){
	test->Check();
	if ( test->type != Type::boolType ){
		ReportError::TestNotBoolean(test);
		test->type = Type::errorType;
	}

	symbolTable->push(); //Push new scope for If body expr
	body->Check();
	symbolTable->pop(); //Pop If body expr scope

	if (elseBody != NULL ){
		symbolTable->push();
		elseBody->Check();
		symbolTable->pop();
	}
}
ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    expr = e;
    if (e != NULL) expr->SetParent(this);
}

void ReturnStmt::PrintChildren(int indentLevel) {
    if ( expr ) 
      expr->Print(indentLevel+1);
}

SwitchLabel::SwitchLabel(Expr *l, Stmt *s) {
    Assert(l != NULL && s != NULL);
    (label=l)->SetParent(this);
    (stmt=s)->SetParent(this);
}

SwitchLabel::SwitchLabel(Stmt *s) {
    Assert(s != NULL);
    label = NULL;
    (stmt=s)->SetParent(this);
}

void SwitchLabel::PrintChildren(int indentLevel) {
    if (label) label->Print(indentLevel+1);
    if (stmt)  stmt->Print(indentLevel+1);
}

SwitchStmt::SwitchStmt(Expr *e, List<Stmt *> *c, Default *d) {
    Assert(e != NULL && c != NULL && c->NumElements() != 0 );
    (expr=e)->SetParent(this);
    (cases=c)->SetParentAll(this);
    def = d;
    if (def) def->SetParent(this);
}

void SwitchStmt::PrintChildren(int indentLevel) {
    if (expr) expr->Print(indentLevel+1);
    if (cases) cases->PrintAll(indentLevel+1);
    if (def) def->Print(indentLevel+1);
}

//Semactic check for switch stmt
void SwitchStmt::Check(){
	symbolTable->push(); //Push scope
	loop_switchStack->push(this); 
	expr->Check();
	for ( int i = 0; i < cases->NumElements(); i++ ){
		AssignExpr* assignExpr = dynamic_cast<AssignExpr*>(cases->Nth(i));
		if ( assignExpr != NULL )
			continue; //Skip semantic checking of assignexpr inside switch statement
		cases->Nth(i)->Check();
	}
	loop_switchStack->pop();
	symbolTable->pop(); //Pop scope
}

//Semantic Check for Case stmt
void Case::Check(){
	label->Check();
	StmtBlock* stmtBlock = dynamic_cast<StmtBlock*>(stmt);
	if ( stmtBlock != NULL)	
		symbolTable->push();

	stmt->Check();

	if (stmtBlock != NULL )
		symbolTable->pop();
}

//Semantic Check for Default Stmt
void Default::Check(){
	StmtBlock* stmtBlock = dynamic_cast<StmtBlock*>(stmt);
	if ( stmtBlock != NULL )
		symbolTable->push();
	
	stmt->Check();

	if ( stmtBlock != NULL )
		symbolTable->pop();
}

//Semantic Check for Break Stmt
void BreakStmt::Check(){
	if ( !(loop_switchStack->insideLoop()) && !(loop_switchStack->insideSwitch())){
		ReportError::BreakOutsideLoop(this);
	}
}

// Semantic Check for Continue Stmt

void ContinueStmt::Check(){
	if ( !loop_switchStack->insideLoop() ){
		ReportError::ContinueOutsideLoop(this);
	}
}
