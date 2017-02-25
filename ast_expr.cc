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
		if ( right->type == Type::boolType ){
			ReportError::IncompatibleOperand(op, right->type);
			this->type = Type::errorType;
		}
		else {
			this->type = right->type;
		}
	}
}

//Semantic Check for postfix expr
void PostfixExpr::Check(){
	left->Check();
	if ( left->type == Type::boolType ){
		ReportError::IncompatibleOperand(op, left->type);
		this->type = Type::errorType;
	}
	else {
		this->type = left->type; 
	}
}

//Semantic Check for AssignExpr
void AssignExpr::Check(){
	left->Check();
	if ( left->type == Type::errorType )
		this->type = right->type = Type::errorType;
	else{
		right->Check();
		if ( right->type == Type::errorType )
			this->type = left->type = Type::errorType;
		else{
			if ( left->type != right->type ){
				ReportError::IncompatibleOperands(op,left->type,right->type);
				this->type = Type::errorType;			
			}
			else
				this->type = left->type;
		}
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

//Semantic Check for Array Access
void ArrayAccess::Check(){
	base->Check();
	VarExpr* varExpr = dynamic_cast<VarExpr*>(base);
	if ( varExpr == NULL ){
		// this must be variable expr. FloatConstant,IntConstant,BoolConstant is unacceptable;
		this->type = Type::errorType;
	}
	else {
		ArrayType* arrType = dynamic_cast<ArrayType*>(base->type);
		if ( arrType == NULL ){
			ReportError::NotAnArray(varExpr->GetIdentifier());
			this->type = Type::errorType;
		}
		else if ( base->type == Type::errorType )
			this->type = Type::errorType;
		else{
			subscript->Check();
			if ( subscript->type != Type::errorType )
				this->type = arrType->GetElemType();
			else
				this->type = Type::errorType;
		}
	}
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

//Semantic Check for FieldAccess expr:
void FieldAccess::Check(){
	if ( base != NULL )
		base->Check();
	
	if ( base->type != Type::vec2Type && base->type !=Type::vec3Type && base->type != Type::vec4Type ){
		ReportError::InaccessibleSwizzle(field, base);
		this->type = Type::errorType;
		return;
	}

	string swiz = string(field->GetName());
	for ( int i = 0; i < swiz.size(); i++ ){
		if ( swiz[i] != 'x' && swiz[i] != 'y' && swiz[i] != 'z' && swiz[i]!= 'w' ){
			ReportError::InvalidSwizzle(field, base);
			this->type = Type::errorType;
			return;
		}
	}
	
	if ( base->type == Type::vec2Type ){
		for ( int i = 0; i < swiz.size(); i++ ){
                	if ( swiz[i] == 'z' ||  swiz[i] == 'w' ){
				ReportError::SwizzleOutOfBound(field, base);
                        	this->type = Type::errorType;
                        	return;
			}
                }
        }
	
	else if ( base->type == Type::vec3Type) {
	 	for ( int i = 0; i < swiz.size(); i++ ){
                	if ( swiz[i] == 'w' ){
                        	ReportError::SwizzleOutOfBound(field, base);
                        	this->type = Type::errorType;
                        	return;
                	}
		}

	}
	
	if (swiz.size() > 4) {
		ReportError::OversizedVector(field, base);
		this->type = Type::errorType;
		return;
	}
	

	switch ( swiz.size() ){
		case 4: this->type = Type::vec4Type; break;
		case 3: this->type = Type::vec3Type; break;
		case 2: this->type = Type::vec2Type; break;
		default: this->type = Type::floatType; break;
	}
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

//Semantic Check for Call expr
void Call::Check(){
	if ( base != NULL ){
		base->Check();
	}
	Decl* decl = NULL;
	// Check for function declaration in global scope
	for ( int i = symbolTable->GetTables()->size() - 1; i >= 0; i-- ){
		Symbol* fnSym = symbolTable->GetTables()->at(i)->find(field->GetName());
		if ( fnSym != NULL ){
			decl = fnSym->decl;
			break;
		}
	}
	
		
	if(decl == NULL) {
		ReportError::IdentifierNotDeclared(field, LookingForFunction);
		this->type = Type::errorType;	
	}
	else{
		FnDecl* fndecl = dynamic_cast<FnDecl*>(decl);	
		if ( fndecl == NULL ){
			ReportError::ReportError::NotAFunction(field);
			this->type = Type::errorType;
		}
		else{
			if(fndecl->GetFormals()->NumElements() > actuals->NumElements()) {
				ReportError::LessFormals(field, fndecl->GetFormals()->NumElements(), actuals->NumElements());
				this->type = Type::errorType;
			}
			else if(fndecl->GetFormals()->NumElements() < actuals->NumElements()) {
				ReportError::ExtraFormals(field, fndecl->GetFormals()->NumElements(), actuals->NumElements());
				this->type = Type::errorType;
			}
			else{
				this->type = fndecl->GetType();

				for(int i = 0; i < actuals->NumElements(); i++) {
					Expr* actual = actuals->Nth(i);
					actual->Check();
					
					if ( actual->type != Type::errorType ){
						Type* giventype = fndecl->GetFormals()->Nth(i)->GetType();
						if ( actual->type != giventype ){
							ReportError::FormalsTypeMismatch(field, i+1, giventype, actual->type);
							this->type = Type::errorType;
							break;
						}
					}
					else{
						this->type = Type::errorType;
					}
				}
			}		
		}
	}
}
