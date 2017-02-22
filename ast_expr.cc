/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "symtable.h"

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}
void IntConstant::PrintChildren(int indentLevel) { 
    printf("%d", value);
}

//Semantic check for intconstant
void IntConstant::Check(){
	this->type = Type::intType;
}

FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
    printf("%g", value);
}

//Semantic check for float constant
void FloatConstant::Check(){
	this->type = Type::floatType;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}
void BoolConstant::PrintChildren(int indentLevel) { 
    printf("%s", value ? "true" : "false");
}

void BoolConstant::Check(){
	this->type = Type::boolType;
}

//Semantic check for emptyExpr
void EmptyExpr::Check(){
	this->type = Type::voidType;
}

VarExpr::VarExpr(yyltype loc, Identifier *ident) : Expr(loc) {
    Assert(ident != NULL);
    this->id = ident;
}

//Semantic check for VarExpr
void VarExpr::Check() {
	Identifier* id = this->GetIdentifier();
	Symbol* sym = NULL;
	for ( int i = symbolTable->GetTables()->size()-1; i >=0; i--){
		sym = symbolTable->GetTables()->at(i)->find(id->GetName());
		if ( sym != NULL )
			break;
	}

	if ( sym == NULL ){
		this->type = Type::errorType;
		ReportError::IdentifierNotDeclared(id, LookingForVariable);
	}
	else{
		VarDecl* vardecl = dynamic_cast<VarDecl*>(sym->decl);
		this->type = vardecl->GetType();
	}
}

void VarExpr::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
}

//Semantic check for relational expr
void RelationalExpr::Check(){
	this->type = Type::boolType;
	left->Check();

	if ( left->type == Type::errorType )
                right->type = Type::errorType;
        else
                right->Check();
	
	if ( left->type != Type::errorType && right->type != Type::errorType ){
		if ( left->type != right->type ){
			ReportError::IncompatibleOperands(op, left->type, right->type);
			this->type = Type::errorType;
		}	
	}
	else {
		this->type = Type::errorType;
	}
	
}

//Semantic Check for Arithmetic expr
void ArithmeticExpr::Check(){
	if (left != NULL){
        	left->Check();

		if ( left->type == Type::errorType )
                	right->type = Type::errorType;
        	else
                	right->Check();

	        if ( left->type != Type::errorType && right->type != Type::errorType ){
        	        if ( left->type != right->type ){
                	        ReportError::IncompatibleOperands(op, left->type, right->type);
				this->type = Type::errorType;
                	}
			else{
				char* opTok = op->GetOpTokStr();
				if ( strcmp(opTok,(char*)"==") == 0 || 
				     strcmp(opTok,(char*)"!=") == 0 || 
				     strcmp(opTok, (char*)"&&") == 0 || 
				     strcmp(opTok,(char*)"||") == 0 )
					this->type = Type::boolType;
				else
					this->type = left->type;
			}
        	}
		else {
			this->type = Type::errorType;	
		}
	}

	else { //Unary Expr
		right->Check();
		if ( right->type == Type::errorType )
			this->type = Type::errorType;
		else
			this->type = right->type;
	}
}

//Semantic Check for postfix expr
void PostfixExpr::Check(){
	left->Check();
	if ( left->type == Type::intType || left->type == Type::floatType )
		this->type = left->type;
	else{
		ReportError::IncompatibleOperand(op, left->type);
		this->type = Type::errorType;
	}
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}

void Operator::PrintChildren(int indentLevel) {
    printf("%s",tokenString);
}

bool Operator::IsOp(const char *op) const {
    return strcmp(tokenString, op) == 0;
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o) 
  : Expr(Join(l->GetLocation(), o->GetLocation())) {
    Assert(l != NULL && o != NULL);
    (left=l)->SetParent(this);
    (op=o)->SetParent(this);
}

void CompoundExpr::PrintChildren(int indentLevel) {
   if (left) left->Print(indentLevel+1);
   op->Print(indentLevel+1);
   if (right) right->Print(indentLevel+1);
}
   
ConditionalExpr::ConditionalExpr(Expr *c, Expr *t, Expr *f)
  : Expr(Join(c->GetLocation(), f->GetLocation())) {
    Assert(c != NULL && t != NULL && f != NULL);
    (cond=c)->SetParent(this);
    (trueExpr=t)->SetParent(this);
    (falseExpr=f)->SetParent(this);
}

void ConditionalExpr::PrintChildren(int indentLevel) {
    cond->Print(indentLevel+1, "(cond) ");
    trueExpr->Print(indentLevel+1, "(true) ");
    falseExpr->Print(indentLevel+1, "(false) ");
}
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

void ArrayAccess::PrintChildren(int indentLevel) {
    base->Print(indentLevel+1);
    subscript->Print(indentLevel+1, "(subscript) ");
}
     
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}


void FieldAccess::PrintChildren(int indentLevel) {
    if (base) base->Print(indentLevel+1);
    field->Print(indentLevel+1);
}

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}

void Call::PrintChildren(int indentLevel) {
   if (base) base->Print(indentLevel+1);
   if (field) field->Print(indentLevel+1);
   if (actuals) actuals->PrintAll(indentLevel+1, "(actuals) ");
}

