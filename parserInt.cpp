/* 
Implements Recursive-Descent Parser
*/

#include "parserInt.h"
#include "lex.h"
#include "val.h"
#include <iostream>
static int error_count = 0;
map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constance 
queue <Value> * ValQue; //declare a pointer variable to a queue of Value objects
queue <string> * varHold; 

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
			
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;
	}
}

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
    
}



//Program is: Prog = PROGRAM IDENT {Decl} {Stmt} END PROGRAM IDENT
bool Prog(istream& in, int& line)
{
	bool dl = false, sl = false;
	LexItem tok = Parser::GetNextToken(in, line);
	
	
	if (tok.GetToken() == PROGRAM) {
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == IDENT) {
			string progName = tok.GetLexeme();
			dl = Decl(in, line);
			if( !dl  )
			{
				ParseError(line, "Incorrect Declaration in Program");
				return false;
			}
			sl = Stmt(in, line);
			if( !sl  )
			{
				ParseError(line, "Incorrect Statement in program");
				return false;
			}	
			tok = Parser::GetNextToken(in, line);
			
			if (tok.GetToken() == END) {
				tok = Parser::GetNextToken(in, line);
				
				if (tok.GetToken() == PROGRAM) {
					tok = Parser::GetNextToken(in, line);
					
					if (tok.GetToken() == IDENT) {
						if(progName.compare(tok.GetLexeme()) != 0){
							ParseError(line, "Incorrect Program Name");
							return false;
						}
						return true;
					}
					else
					{
						ParseError(line, "Missing Program Name");
						return false;
					}	
				}
				else
				{
					ParseError(line, "Missing PROGRAM at the End");
					return false;
				}	
			}
			else
			{
				ParseError(line, "Missing END of Program");
				return false;
			}	
		}
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	
	return false;
}

//Decl = Type : VarList 
//Type = INTEGER | REAL | CHAR
bool Decl(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	
	LexItem t = Parser::GetNextToken(in, line);
	
	if(t == INTEGER || t == REAL || t == CHAR) {
		tok = t;
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == COLON) {
			status = IdList(in, line, t);
			//cout<< tok.GetLexeme() << " " << (status? 1: 0) << endl;
			if (status)
			{
				status = Decl(in, line);
				return status;
			}
		}
		else{
			ParseError(line, "Missing Colon");
			return false;
		}
	}
		
	Parser::PushBackToken(t);
	return true;
}

//Stmt is either a PrintStmt, ReadStmt, IfStmt, or an AssigStmt
//Stmt = AssigStmt | IfStmt | PrintStmt | ReadStmt
bool Stmt(istream& in, int& line) {
	bool status;
	
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {

	case PRINT:
		status = PrintStmt(in, line);
		
		if(status)
			status = Stmt(in, line);
		break;

	case IF:
		status = IfStmt(in, line);
		if(status)
			status = Stmt(in, line);
		break;

	case IDENT:
		Parser::PushBackToken(t);
        status = AssignStmt(in, line);
		if(status)
			status = Stmt(in, line);
		break;
		
	case READ:
		status = ReadStmt(in, line);
		
		if(status)
			status = Stmt(in, line);
		break;

	default:
		Parser::PushBackToken(t);
		return true;
	}

	return status;
}

//PrintStmt:= print, ExprList 
bool PrintStmt(istream& in, int& line) { 
    LexItem t;
    /*create an empty queue of Value objects.*/
    ValQue = new queue<Value>;
    if( (t=Parser::GetNextToken(in,line)) != COMA){
        ParseError(line,"Missing a Comma");
        return false;
    }

    bool ex=ExprList(in,line);
    
    if(!ex){
        ParseError(line,"Missing expression after print");
        while( !(*ValQue).empty() ){
            ValQue->pop();
        }
        delete ValQue;
        return false;
    }

    //Evaluate:print out the list of expressions' values
    while( !(*ValQue).empty() ){
        Value nextVal=(*ValQue).front();
        cout<<nextVal;ValQue->pop();
    }
    cout<<endl;
    return ex;
}//PrintStmt

//IfStmt:= if (Expr) then {Stmt} END IF
bool IfStmt(istream& in, int& line) {
	Value val1;
	bool ex=false ; 
	LexItem t;
	
	if( (t=Parser::GetNextToken(in, line)) != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	
	ex = LogicExpr(in, line, val1);
	if( !ex ) {
		ParseError(line, "Missing if statement Logic Expression");
		return false;
	}

	if((t=Parser::GetNextToken(in, line)) != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	
	if((t=Parser::GetNextToken(in, line)) != THEN ) {
		
		ParseError(line, "Missing THEN");
		return false;
	}

	if(val1.GetBool()){
		bool st = Stmt(in, line);
		if( !st ) {
			ParseError(line, "Missing statement for IF");
			return false;
		}
	}else{
		t=Parser::GetNextToken(in, line);
		while(t != END){
			t=Parser::GetNextToken(in, line);
		}
		Parser::PushBackToken(t);
	}
	
	
	if((t=Parser::GetNextToken(in, line)) != END ) {
		
		ParseError(line, "Missing END of IF");
		return false;
	}
	if((t=Parser::GetNextToken(in, line)) != IF ) {
		
		ParseError(line, "Missing IF at End of IF statement");
		return false;
	}
	
	return true;
}

bool ReadStmt(istream& in, int& line)
{
	
	LexItem t;
	
	if( (t=Parser::GetNextToken(in, line)) != COMA ) {
		
		ParseError(line, "Missing a Comma");
		return false;
	}
	
	bool ex = VarList(in, line);
	
	if( !ex ) {
		ParseError(line, "Missing Variable after Read Statement");
		return false;
	}
	
	

	return ex;
}
//IdList:= IDENT {,IDENT}
bool IdList(istream& in, int& line, LexItem type) {
	bool status = false;
	string identstr;
	
	LexItem tok = Parser::GetNextToken(in, line);
	if(tok == IDENT)
	{
		//set IDENT lexeme to the type tok value
		identstr = tok.GetLexeme();
		if (!(defVar.find(identstr)->second))
		{
			defVar[identstr] = true;
			SymTable[identstr] = type.GetToken();
		}	
		else
		{
			ParseError(line, "Variable Redefinition");
			return false;
		}
		
	}
	else
	{
		ParseError(line, "Missing Variable");
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	
	if (tok == COMA) {
		status = IdList(in, line, type);
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

//VarList
bool VarList(istream& in, int& line)
{
	bool status = false;
	string identstr;
	
	status = Var(in, line);
	
	if(!status)
	{
		ParseError(line, "Missing Variable");
		return false;
	}
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMA) {
		status = VarList(in, line);
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

//Var:= ident
bool Var(istream& in, int& line)
{
	//called only from the AssignStmt function
	string identstr;
	
	LexItem tok = Parser::GetNextToken(in, line);
	if (tok == IDENT){
		identstr = tok.GetLexeme();
		if (!(defVar.find(identstr)->second))
		{
			ParseError(line, "Undeclared Variable");
			return false;
			
		}
		(*varHold).push(identstr);
		return true;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
}

//AssignStmt:= Var = Expr
bool AssignStmt(istream& in, int& line) {
	Value val1;
	bool varstatus = false, status = false;
	LexItem t;
	varHold = new queue<string>;
	ValQue = new queue<Value>;
	varstatus = Var(in, line);
	
	
	if (varstatus){
		t = Parser::GetNextToken(in, line);
		
		if (t == ASSOP){
			status = Expr(in, line, val1);
			if(!status) {
				ParseError(line, "Missing Expression in Assignment Statment");
				return status;
			}
			TempsResults[(*varHold).front()] = val1;
			(*varHold).pop();
			(*ValQue).pop();
		}
		else if(t.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}
		else {
			ParseError(line, "Missing Assignment Operator =");
			return false;
		}
	}
	else {
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
		return false;
	}
	delete varHold;
	delete ValQue;
	return status;	
}

//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
	Value val1;
	bool status = false;

	status = Expr(in, line, val1);
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}
	if(val1.IsInt() || val1.IsReal()){
		(*ValQue).push(val1);
	}
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMA) {
		status = ExprList(in, line);
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

//Expr:= Term {(+|-) Term}
bool Expr(istream& in, int& line, Value & retVal) {
    Value val1, val2;
    bool t1 = Term(in, line, val1);
    LexItem tok;
    
    if( !t1 ) {
        return false;
    }
    
	retVal = val1;

    tok = Parser::GetNextToken(in, line);
    if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    while ( tok == PLUS || tok == MINUS ) {
        t1 = Term(in, line, val2);
        if( !t1 ) 
        {
            ParseError(line, "Missing operand after operator");
            return false;
        }
    //evaluate the expression for addition or subtraction
    //and update the retVal object
    //check if the operation of PLUS/MINUS is legal for the
    //type of operands
        if(retVal.GetType()==VCHAR || val2.GetType()==VCHAR){
            ParseError(line,"Run-Time Error-Illegal Mixed Type Operands");
            return false;
        }
        else{
            if(tok == PLUS){
                retVal = retVal + val2;
            }
            else if(tok == MINUS){
                retVal = retVal - val2;
            }
        }

        tok=Parser::GetNextToken(in,line);
        if(tok.GetToken() == ERR){
            ParseError(line,"Unrecognized Input Pattern");
            cout<< "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
    }
    Parser::PushBackToken(tok);
    return true;
}

//Term:= SFactor {(*|/) SFactor}
bool Term(istream& in, int& line, Value & retVal) {
	Value val1, val2;
	bool t1 = SFactor(in, line, val1);
	LexItem tok;
	

	if( !t1 ) {
		return false;
	}

	retVal = val1;
	tok	= Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
	}
	while ( tok == MULT || tok == DIV  )
	{
		t1 = SFactor(in, line, val2);
		
		if( !t1 ) {
			ParseError(line, "Missing operand after operator");
			return false;
		}


		if(retVal.GetType()==VCHAR || val2.GetType()==VCHAR){
            ParseError(line,"Run-Time Error-Illegal Mixed Type Operands");
            return false;
        }
        else{
            if(tok == MULT){
                retVal = retVal * val2;

            }
            else if(tok == DIV){
                if(val2.GetInt() == 0){
					ParseError(line,"Run-Time Error-Illegal Division by Zero");
            		return false;
				}else{
					retVal = retVal / val2;
				}
            }
        }
		
		tok	= Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
		
	}
	Parser::PushBackToken(tok);
	return true;
}

//SFactor = Sign Factor | Factor
bool SFactor(istream& in, int& line, Value & retVal)
{
	bool status;
	Value val1;
	int sign = 1; //1 is for positive sign, -1 for negative sign
    LexItem possSign = Parser::GetNextToken(in, line);

    if(possSign == MINUS){
        sign = -1;
    }else if(!(possSign == PLUS)){
        Parser::PushBackToken(possSign);
    }

	status = Factor(in, line, sign, val1);
	retVal = val1;
	return status;
}
//LogicExpr = Expr (== | <) Expr
bool LogicExpr(istream& in, int& line, Value & retVal)
{
	Value val1, val2;
	bool t1 = Expr(in, line, val1);
	LexItem tok;
	
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	if ( tok == LTHAN  || tok == EQUAL) 
	{
		t1 = Expr(in, line, val2);
		if( !t1 ) 
		{
			ParseError(line, "Missing expression after relational operator");
			return false;
		}

		if(val1.GetType() == val2.GetType() || (val1.IsInt() && val2.IsReal()) || (val1.IsReal() && val2.IsInt())){
			if(tok == LTHAN){
				retVal = (val1 < val2);
			}else{
				retVal = (val1 == val2);
			}
		}else{
			ParseError(line,"Run-Time Error-Illegal Mixed Type Operands");
            return false;
		}
		return true;
	}
	Parser::PushBackToken(tok);
	return true;
}

//Factor := ident | iconst | rconst | sconst | (Expr)
bool Factor(istream& in, int& line, int sign, Value & retVal) {
	
	LexItem tok = Parser::GetNextToken(in, line);
	

	if( tok == IDENT ) {
		//check if the var is defined 
		string lexeme = tok.GetLexeme();
		if (!(defVar.find(lexeme)->second) || (TempsResults.count(lexeme) == 0))
		{
			ParseError(line, "Undefined Variable");
			return false;	
		}
		if (SymTable.count(lexeme) == 0){
			ParseError(line, "Unintialized Variable");
			return false;	
		}else{
			
			switch (SymTable[lexeme])
			{
				case INTEGER:
					retVal = Value(sign*(TempsResults[lexeme].GetInt()));
					
					break;
				case REAL:
					retVal = Value(sign*(TempsResults[lexeme].GetReal()));
					break;
				case CHAR:
					retVal = Value(TempsResults[lexeme].GetChar());
					(*ValQue).push(retVal);
					break;
				default:
					break;
			}
		}

		return true;
	}
	else if( tok == ICONST ) {
		retVal = Value(sign*stoi(tok.GetLexeme()));
		(*ValQue).push(retVal);
		return true;
	}
	else if( tok == SCONST ) {
		retVal = Value(tok.GetLexeme());
		(*ValQue).push(retVal);
		return true;
	}
	else if( tok == RCONST ) {
		retVal = Value(sign*stof(tok.GetLexeme()));
		(*ValQue).push(retVal);
		return true;
	}
	else if( tok == LPAREN ) {
		Value val1;
		bool ex = Expr(in, line, val1);
		if( !ex ) {
			ParseError(line, "Missing expression after (");
			return false;
		}
		retVal = val1;
		if( Parser::GetNextToken(in, line) == RPAREN )
			return ex;

		ParseError(line, "Missing ) after expression");
		return false;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	ParseError(line, "Unrecognized input");
	return 0;
}



