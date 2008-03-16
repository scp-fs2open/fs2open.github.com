
#include "variables/variables.h"
#include "io/timer.h"
#include "ship/ship.h"
#include "globalincs/pstypes.h"

#include <string>
#include <stack>
#include <math.h>

using namespace std;

//the only thing code outside the variable modual needs to know about are the 
//variable and expression classes, hence why all the derived classes are in here

class constant:public variable{
	//a constant variable, returns the same value every time value is called
	//aconstant's value should never be changed after instansiation
	float val;
public:
	constant(float in):val(in),variable(VAR_CONSTANT){};
	float value(variable_call_data*data=NULL){return val;};
};


/*start opperators*/

class operate:public variable{
protected:
	variable *operand1, *operand2;
public:
	virtual float value(variable_call_data*data=NULL) = 0;
	operate(variable*op1, variable*op2):variable(VAR_OPP){operand1 = op1; operand2 = op2;}
	virtual ~operate(){
		if(operand1)delete operand1; 
		if(operand2)delete operand2;
	}
};


class add:public operate{
public:
	add(variable*op1, variable*op2):operate(op1,op2){}
	float value(variable_call_data*data=NULL){return operand1->value(data) + operand2->value(data);}
};

class sub:public operate{
public:
	sub(variable*op1, variable*op2):operate(op1,op2){}
	float value(variable_call_data*data=NULL){return operand1->value(data) - operand2->value(data);};
};

class divide:public operate{
public:
	divide(variable*op1, variable*op2):operate(op1,op2){}
	float value(variable_call_data*data=NULL){return operand1->value(data) / operand2->value(data);};
};

class mult:public operate{
public:
	mult(variable*op1, variable*op2):operate(op1,op2){}
	float value(variable_call_data*data=NULL){return operand1->value(data) * operand2->value(data);};
};

class mod:public operate{
public:
	mod(variable*op1, variable*op2):operate(op1,op2){}
	float value(variable_call_data*data=NULL){return i2fl((int)operand1->value(data) % (int)operand2->value(data));};
};

class power:public operate{
public:
	power(variable*op1, variable*op2):operate(op1,op2){}
	float value(variable_call_data*data=NULL){return(float)pow(operand1->value(data), operand2->value(data));};
};

//b#x == the b'th rooth ofx
class root:public operate{
public:
	root(variable*op1, variable*op2):operate(op2,op1){}
	float value(variable_call_data*data=NULL){return(float)pow(operand1->value(data), 1.0f/operand2->value(data));};
};

//b~x == the log base b of x
class logb:public operate{
public:
	logb(variable*op1, variable*op2):operate(op1,op2){}
	float value(variable_call_data*data=NULL){return(float)log(operand2->value(data))/log(operand1->value(data));};
};


//comparison
class comp_equal:public operate{
public:
	comp_equal(variable*op1, variable*op2):operate(op1,op2){}
	float value(variable_call_data*data=NULL){if(operand1->value(data) == operand2->value(data))return 1.0;else return 0.0f;};
};

class comp_greater:public operate{
public:
	comp_greater(variable*op1, variable*op2):operate(op1,op2){}
	float value(variable_call_data*data=NULL){if(operand1->value(data) > operand2->value(data))return 1.0;else return 0.0f;};
};

class comp_less:public operate{
public:
	comp_less(variable*op1, variable*op2):operate(op1,op2){}
	float value(variable_call_data*data=NULL){if(operand1->value(data) < operand2->value(data))return 1.0;else return 0.0f;};
};

//+++++++++++++++++++++++++++++++++++++++++++++
//unary variables

class unary:public variable{
protected:
	variable *operand;
public:
	virtual float value(variable_call_data*data=NULL) = 0;
	unary(variable*op):variable(VAR_OPP){operand = op;}
	virtual~unary(){
		if(operand)delete operand;
	}
};

class sin_op:public unary{
public:
	sin_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return sin(operand->value(data));}
};

class cos_op:public unary{
public:
	cos_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return cos(operand->value(data));}
};

class tan_op:public unary{
public:
	tan_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return tan(operand->value(data));}
};

class hypsin_op:public unary{
public:
	hypsin_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return sinh(operand->value(data));}
};

class hypcos_op:public unary{
public:
	hypcos_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return cosh(operand->value(data));}
};

class hyptan_op:public unary{
public:
	hyptan_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return tanh(operand->value(data));}
};

class arcsin_op:public unary{
public:
	arcsin_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return asin(operand->value(data));}
};

class arccos_op:public unary{
public:
	arccos_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return acos(operand->value(data));}
};

class arctan_op:public unary{
public:
	arctan_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return atan(operand->value(data));}
};

class degree_op:public unary{
public:
	degree_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return (float)(operand->value(data)*3.14159265358979323846264338327/180.0f);}
};

class radian_op:public unary{
public:
	radian_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return (float)(operand->value(data)*180.0f/3.14159265358979323846264338327);}
};

class factorial_op:public unary{
public:
	factorial_op(variable*op):unary(op){}
	float evaluate(float f){if(f==0.0f)return 1;else return evaluate(f-1);};
	float value(variable_call_data*data=NULL){return evaluate(operand->value(data));}
};

class abs_op:public unary{
public:
	abs_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){float f = operand->value(data); if(f<0)return -f;else return f;}
};

class ipart_op:public unary{
public:
	ipart_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return operand->value(data);}
};

class fpart_op:public unary{
public:
	fpart_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){float f = operand->value(data); return f-(int)f;}
};

class sign_op:public unary{
public:
	sign_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){float f = operand->value(data); if(f==0)return 0;else if(f>0) return 1;else return -1;}
};

class log_op:public unary{
public:
	log_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return log10(operand->value(data));}
};

class ln_op:public unary{
public:
	ln_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return log(operand->value(data));}
};

class sqrt_op:public unary{
public:
	sqrt_op(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){return sqrt(operand->value(data));}
};

class not:public unary{
public:
	not(variable*op):unary(op){}
	float value(variable_call_data*data=NULL){if(operand->value(data)==0.0f)return 1.0f;else return 0.0f;}
};

string unaries[] = {"hypsin","hyptan","hyptan","arcsin","arccos","arctan","sin","cos","tan", "log","ln", "sqrt", "deg","rad","!", "abs", "ipart", "fpart", "sign", "not"};
#define N_UNARIES (sizeof(unaries)/sizeof(string))

/*end opperators*/

/*object variables*/

class game_variable:public variable{
public:
	game_variable():variable(VAR_GAME){}
};

class engine_output:public game_variable{
public:
	engine_output(){};
	float value(variable_call_data*data=NULL){
		if(data && data->obj)
		switch(data->obj->type){
		case OBJ_SHIP:
			return data->obj->phys_info.forward_thrust;
		case OBJ_WEAPON:
			return 1.0f;
		default: return 0.0f;
		}
		else return 0.0f;
	};
};

class shields:public game_variable{
public:
	shields(){};
	float value(variable_call_data*data=NULL){
		if(data && data->obj)
		switch(data->obj->type){
		case OBJ_SHIP:
			return get_shield_strength(data->obj);
		default: return 0.0f;
		}
		else return 0.0f;
	};
};

///////////////////////////////////////////////////////////
//global variables
class global_time:public game_variable{
public:
	global_time(){};
	float value(variable_call_data*data=NULL){
		return timestamp()/1000.0f;
	};
};

//texture variables
class n_frames_op:public game_variable{
public:
	n_frames_op(){};
	float value(variable_call_data*data=NULL){
		return i2fl(data->texture.n_frames);
	};
};

class fps_op:public game_variable{
public:
	fps_op(){};
	float value(variable_call_data*data=NULL){
		return i2fl(data->texture.fps);
	};
};

/*end object_variables*/

void postfix(string&str);

expression::expression(char* in_str):variable(VAR_EXP),head(NULL){
	//takes a cstring and bulds an expression tree from it
	string str = in_str;
	//first step involves converting the input string from infix notation to prefix notation
	postfix(str);
	char* is = new char[str.length()+1];
	char* os = is;
	strcpy(is, str.c_str());
	//second step is makeing a tree from the prefix expression
	//in infix the operator precededs the operands
	head = parse_next(is);
	delete[]os;
}

int expression::wich_unary(char*c){
	//this is a utility function for determineing which if any unary functions the input string denotes
	if(!c)return -1;
	string ch="";

	while(*c && (*c)!=','){
		ch+=*(c++);
	}

	for(int i = 0; i<N_UNARIES; i++){
		if(ch == unaries[i])return i;
	}
	return -1;
}

int prefix_unary(const char*c){
//this is used in the prefix converter, it takes a reversed cstring and tells if it is a unary function
	string str;
	int i;
	int k;

	string ch = "";
	//! is factorial, all other unary functions ar made from multable lower case charicters
	for(k=0; (c[k] >= 'a' && c[k] <= 'z') || c[k] == '!'; k++)
		ch += (c[k]);

	for(i = 0; i<N_UNARIES; i++){
		int  l = unaries[i].length();

		str = "";
		for(k=0; k<l; k++)
			str+= unaries[i][l-k-1];
		

		if(str == ch)return l;
	}
	return 0;
}

//this recursively builds the expression tree
variable* expression::parse_next(char*&in_str){
	//kill any noise
	while((*in_str == ' ' || *in_str == '	') && *in_str != char(0))in_str++;

	char op_char[] = "+-/*%^#~<>=";//charicters denoteing binary operators
	if(*in_str == ':'){
		in_str++;
		//it's a object variable
		return get_object_variable(&in_str);
	}else{
		//it's a constant, or an opporateor
		int un = wich_unary(in_str);//if it's a unary opperator lets get the value now

		if(strchr(op_char, *in_str) && *(in_str+1) == ','){//all items are followed by commas
			//it must be an operator
			char o = *in_str++;//grab the opperator char
			if(*in_str == ',')in_str++;//move the input string to the next item

			//grab the two operands recursively
			variable*a = parse_next(in_str);
			variable*b = parse_next(in_str);
			

			variable*c;

			switch(o){
			case '+':c =  new add(a,b);break;
			case '-':c =  new sub(a,b);break;
			case '*':c =  new mult(a,b);break;
			case '%':c =  new mod(a,b);break;
			case '/':c =  new divide(a,b);break;
			case '^':c =  new power(a,b);break;
			case '#':c =  new root(a,b);break;
			case '~':c =  new logb(a,b);break;
			case '=':c =  new comp_equal(a,b);break;
			case '>':c =  new comp_greater(a,b);break;
			case '<':c =  new comp_less(a,b);break;
			}

			if(a->get_type() == VAR_CONSTANT && b->get_type() == VAR_CONSTANT){
				//if both operands are constants then perform the operation get the constant result and set this as a constant
				variable*con = new constant(c->value());
				delete c;
				c = con;
			}
			
			return c;
			

		}else if(un != -1){
			//it must have found a unary string
//string unaries[] = {"hypsin","hyptan","hyptan","arcsin","arccos","arctan","sin","cos","tan", "log","ln", "sqrt", "deg","rad","!", "abs", "ipart", "fpart", "sign"};
			in_str+=unaries[un].length();
			if(*in_str == ',')in_str++;//move to the next item

			variable*a=parse_next(in_str);
			variable*c;
			//instasiate the variable as a unary while recersively grabing it's operand
			switch(un){
			case 0:c = new hypsin_op(a);break;
			case 1:c = new hypcos_op(a);break;
			case 2:c = new hyptan_op(a);break;
			case 3:c = new arcsin_op(a);break;
			case 4:c = new arccos_op(a);break;
			case 5:c = new arctan_op(a);break;
			case 6:c = new sin_op(a);break;
			case 7:c = new cos_op(a);break;
			case 8:c = new tan_op(a);break;
			case 9:c = new log_op(a);break;
			case 10:c = new ln_op(a);break;
			case 11:c = new sqrt_op(a);break;
			case 12:c = new radian_op(a);break;
			case 13:c = new degree_op(a);break;
			case 14:c = new factorial_op(a);break;
			case 15:c = new abs_op(a);break;
			case 16:c = new ipart_op(a);break;;
			case 17:c = new fpart_op(a);break;
			case 18:c = new sign_op(a);break;
			case 19:c = new not(a);break;
			}

			if(a->get_type() == VAR_CONSTANT){
				//if the operand is a constant then perform the operation get the constant result and set this as a constant
				variable*con = new constant(c->value());
				delete c;
				c = con;
			}
			
			return c;
		}else{
			//it must be a constant
			char *str = in_str;//grab the current position
			while(*(in_str) != ',' && *in_str != 0)in_str++;
			if(*in_str == ',')in_str++;//move the input string to the next item
			return  new constant(atof(str));//allocate as a constant, parseing the char string you grabed as a float
		}
	}
	
	return NULL;
}

variable* expression::get_object_variable(char**str){
	char object_id[] = "object";
	char global_id[] = "global";
	char texture_id[] = "texture";
	//others to be implemented
	//submodel-information related to the current subobject
	//player_object

	if(!strncmp(*str, object_id, strlen(object_id))){
		//object scope variables
		variable*ret;
		*str+=strlen(object_id);
		//now we should have {variable_name}
		(*str)++; //get rid of the leading {

		char engine_output_id[] = "engine_output";
		char shield_id[] = "shields";
		//the variable cascade
		if(!strncmp(*str,engine_output_id,strlen(engine_output_id))){
			*str+=strlen(engine_output_id);
			ret = new engine_output();
		}else
		if(!strncmp(*str,shield_id,strlen(shield_id))){
			*str+=strlen(shield_id);
			ret = new shields();
		}else
		{
			//unsuported object variable
			while(**str!='}')(*str)++;
			ret = new constant(0.0f);
		}

		(*str)++;
		if(**str == ',')(*str)++;//move the input string to the next item
		return ret;
	}
	if(!strncmp(*str, global_id, strlen(global_id))){
		//global scope variables
		variable*ret;
		*str+=strlen(global_id);
		//now we should have {variable_name}
		(*str)++; //get rid of the leading {

		char time_id[] = "time";

		if(!strncmp(*str,time_id,strlen(time_id))){
			*str+=strlen(time_id);
			ret = new global_time();
		}else
		{
			//unsuported object variable
			while(**str!='}')(*str)++;
			ret = new constant(0.0f);
		}

		(*str)++;
		if(**str == ',')(*str)++;//move the input string to the next item
		return ret;
	}
	if(!strncmp(*str, texture_id, strlen(texture_id))){
		//texture scope variables
		variable*ret;
		*str+=strlen(texture_id);
		//now we should have {variable_name}
		(*str)++; //get rid of the leading {

		char nframe_id[] = "n_frames";
		char fps_id[] = "fps";

		if(!strncmp(*str,nframe_id,strlen(nframe_id))){
			*str+=strlen(nframe_id);
			ret = new n_frames_op();
		}else
		if(!strncmp(*str,fps_id,strlen(fps_id))){
			*str+=strlen(fps_id);
			ret = new fps_op();
		}else
		{
			//unsuported object variable
			while(**str!='}')(*str)++;
			ret = new constant(0.0f);
		}

		(*str)++;
		if(**str == ',')(*str)++;//move the input string to the next item
		return ret;
	}

//unsuported variable
	while(**str!='}')(*str)++;
	(*str)++;
	if(**str == ',')(*str)++;//move the input string to the next item
	return new constant(0.0f);
}

inline int get_value(char c){
	//tells what the binar opperator value of the current charicter is
	switch(c){
	case '=':
	case '>':
	case '<':
		return 1;
	case '+':
	case '-':
		return 2;
	case '*':
	case '/':
	case '%':
		return 3;
	case '^':
	case '#':
	case '~':
		return 4;
	default:return -1;
	}
}

bool is_value(char c){
	//tells waether the current charicterr is part of a number
	if(c >= '0' && c <= '9')return true;
	if(c == '.')return true;
	return false;
}
void fix_implied_mult(string &str){
	//a preprocess stepp on the string to fix common syntax errors
	//and to implant defined constants

	//first replace constants
	while(str.find("PI") != string::npos){
		str.replace(str.find("PI"),2,"3.14159265358979323846264338327");
	}
	//I'm not sure how many digets a float can hanndle, but I'm sure it's less than these
	while(str.find("E") != string::npos){
		str.replace(str.find("E"),1,"2.718281828459045235360287471352");
	}


	int P = 0;//counter for parenthises
	for(uint i = 0; i< str.length(); i++){
		if(str[i] == '('){
			P++;
			if(i)
			if(is_value(str[i-1]) || str[i-1] == ')')
				str.insert(i,"*");
			//fix implied multiplication at the front of a parenthises
		}
		if(str[i-1] == ')'){
			P--;
			if(i)
			if(is_value(str[i]) || str[i] == '(')
				str.insert(i,"*");
			//fix implied multiplication at the end of a parenthises
		}
		if(str[i] == '-'){
			if((!i || (!is_value(str[i-1]) && str[i-1] != ')') )){
				str.insert(i,"(");
				int p = 0;
				for(int k = i+2; (get_value(str[k]) == -1 || p) && str[k]; k++){
					if(str[k] == '(')p++;
					if(str[k] == ')')p--;
				}
				str.insert(k,")");
				str.insert(i+2,"1*");
				i+=3;
				//fixes negitive numbers by changeing them to a block getting multiplied by -1
			}
		}
	}
	for(P;P > 1;P--)str+=')';//add any missing parenthises
}

void reverse(string&s){
	//reverses a string makeing it sutable for prefix conversion
	string out = "";
	int l = s.length();
	string temp = "";
	for(int i = 0; i<l; i++){
		for(i; i<l; i++){
			if(s[i] == ':'){
				//we have a game variable
				out.insert(0,s[i++]);
				while(s[i]!=':')
					out.insert(0,s[i++]);
				out.insert(0,s[i]);
				break;
			}
			temp += s[i];
			if(!is_value(s[i]) || !is_value(s[i+1]))break;
		}
		out.insert(0,temp);
		temp = "";
	}
	s = out;
}

int object_string(const char*str){
	//object variables hold this format
	//:object.name_of_variable:
	if(str[0] != ':')return 0;
	int i;
	for( i = 1; str[i] != ':' || str[i] == '.' || str[i] == '_' || (str[i]>='a' && str[i]<='z'); i ++);
	return i+1;
}

void postfix(string& str){
	//the prefix converter
	//uses a stack to create a prefix version of the input string
	string s = str;
	fix_implied_mult(s);
	stack<char> op_stack;

	string out;
	reverse(s);

	int i = 0;
	while(s[i]){
		if(is_value(s[i])){
			//if we have a number just spit it out to the out string followed by a comma
			//int sidx = i;	//WMC - not needed

			while(is_value(s[i]) || prefix_unary(s.c_str()+i))out+=s[i++];

			if(s[i] == '-' && (s[i+1] == 0 || s[i+1] == '(')){
				out+="-";
				i++;
			}

			out+=',';
		}else if(object_string(s.c_str()+i)){
			//object variables
			int SL = object_string(s.c_str()+i);
			for(int S = 0; S<SL; S++) out += s[i+S];
			out+=',';
			i+=SL;
		}else if(prefix_unary(s.c_str()+i)){
			//handel unaries, just spit them to the out string
			int sl = prefix_unary(s.c_str()+i);
			for(int S = 0; S<sl; S++)out+=s[i++];
			out+=',';
		}else if(s[i] == ')'){
			//if it's a closeing bracket push it
			op_stack.push(s[i++]);
		}else if(s[i] == '('){
			//if it's an open bracket pop everything untill you find a close
			while(op_stack.top() != ')'){
				out+=op_stack.top();
				op_stack.pop();
				out+=',';//every item sould be followed by a comma
			}
			op_stack.pop();//get rid of the ')'
			i++;
		}else{
			int v = get_value(s[i]);
			if(v){
				//it's an opperator
				if(op_stack.empty() || op_stack.top() == ')' || get_value(op_stack.top()) <= v){
					//if there is no stack yet
					//the top of the stack is a close parenthises
					//or the value of this opperator is not greater than the top of the stack
					//push this opperator on top of the stack
					op_stack.push(s[i++]);
				}else{
					//otherwise pop everything off the stack and onto the out string untill we find one that is greator
					//or we clear the stack
					while(!op_stack.empty() && get_value(op_stack.top()) > v){
						out+=op_stack.top();
						op_stack.pop();
						out+=',';
					}
					//then push the current operator onto the stack
					op_stack.push(s[i++]);
				}
			}
		}
	}
	//pop any remaining opperators onto the out string
	while(!op_stack.empty()){out+=op_stack.top(); op_stack.pop(); out+=',';}
	out.resize(out.length()-1);//get rid of the trailing comma
	reverse(out);//reverse the string again to put it into proper prefix notation

	str = out;//copy the out string to the in string
}

float expression::value(variable_call_data*data){
	//will recursively find the value of all nodes
	return head->value(data);
}
