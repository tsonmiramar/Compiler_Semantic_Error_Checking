/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "symtable.h"        
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}

VarDecl::VarDecl(Identifier *n, Type *t, Expr *e) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
    typeq = NULL;
}

VarDecl::VarDecl(Identifier *n, TypeQualifier *tq, Expr *e) : Decl(n) {
    Assert(n != NULL && tq != NULL);
    (typeq=tq)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
    type = NULL;
}

VarDecl::VarDecl(Identifier *n, Type *t, TypeQualifier *tq, Expr *e) : Decl(n) {
    Assert(n != NULL && t != NULL && tq != NULL);
    (type=t)->SetParent(this);
    (typeq=tq)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
}
  
void VarDecl::PrintChildren(int indentLevel) { 
   if (typeq) typeq->Print(indentLevel+1);
   if (type) type->Print(indentLevel+1);
   if (id) id->Print(indentLevel+1);
   if (assignTo) assignTo->Print(indentLevel+1, "(initializer) ");
}

//Semantic check for VarDecl
void VarDecl::Check(){
	//Check if this Variable is declared before in the scope table
	Symbol varsym(this->GetIdentifier()->GetName(),this,E_VarDecl);
	Symbol* preVarsym = symbolTable->find(varsym.name);
	if ( preVarsym != NULL ){
		ReportError::DeclConflict(this,preVarsym->decl);
		symbolTable->remove(*preVarsym);
	}

	symbolTable->insert(varsym);

	if (assignTo != NULL){
		assignTo->Check();

		if ( (this->GetType() != assignTo->type ) && ( assignTo->type != Type::errorType ) ){ 
			ReportError::InvalidInitialization(this->GetIdentifier(), this->GetType(), assignTo->type);
		}
	}
}

FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
    returnTypeq = NULL;
}

FnDecl::FnDecl(Identifier *n, Type *r, TypeQualifier *rq, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r != NULL && rq != NULL&& d != NULL);
    (returnType=r)->SetParent(this);
    (returnTypeq=rq)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

void FnDecl::PrintChildren(int indentLevel) {
    if (returnType) returnType->Print(indentLevel+1, "(return type) ");
    if (id) id->Print(indentLevel+1);
    if (formals) formals->PrintAll(indentLevel+1, "(formals) ");
    if (body) body->Print(indentLevel+1, "(body) ");
}

//Semantic Check for Function Declaration
void FnDecl::Check(){
	//Check if this function is already declared in the current scope
	Symbol Fnsym(this->GetIdentifier()->GetName(),this,E_FunctionDecl);
        Symbol* preFnsym = symbolTable->find(Fnsym.name);
        if ( preFnsym != NULL ){
                ReportError::DeclConflict(this,preFnsym->decl);
                symbolTable->remove(*preFnsym);
        }

        symbolTable->insert(Fnsym);
	
	if ( this->GetBody() != NULL ){
		
		symbolTable->push(); //push new scoped table	
		List<VarDecl*>* formals = this->GetFormals();
		for ( int i = 0; i < formals->NumElements(); i++ ){
			formals->Nth(i)->Check();
		}
		
		this->GetBody()->Check();
		
		symbolTable->pop(); //Pop the current scoped table		
	}
}

