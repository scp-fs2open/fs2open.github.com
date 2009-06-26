
/*
Bobboau - 
variable system: takes a string and constructs an expression includeing game variables in it

  to use an expression, make a pointer to a variable, then allocate (useing new, not malloc) it 
  to a new expression passing an input string containing a mathematical expression consisting 
  of constants defined variables and opperators. to get the value of the expression, call 
  value() and pass it a pointer to a variable_call_data struct with as much data as you can 
  give packed inside it value(&variable_call_data(pointer_to_an_object)) is probly going to 
  be the most common call

 syntax for expression is normal mathematical notation, ie 1+1
 each expression is made of a combineation of floating point constants, 
 floating point game variables, and binary and unary opperators
 a binary operator is on charicter opperator that has two opperands, 
 operand 1 is on the left side of it, and opperand 2 the right 
 1+3 for example has + as the opperator, 1 as opperand1 and 3 as opperand2
 parenthises () make a subexpression act like a single opperand, the statment 2*(3+2)
 for example has * as the opperator, 2 as opperant1, and (3+2) as opperand2, this is 
 actualy exactly how it will look in the expression tree.
 as the tree is built, it looks to see if it can remove nodes, if all operands 
 of an opperatorare constant that the opperator will be reomved and replaced with a constant
 equal to the evaluation of the opperator with it's constant opperands.

 different opperators have diferent priority levels.
 comparison opperators have the lowest prority value, they are evaluated last
 addition and subtraction are the next highest, they are performed after exponation 
 and multiplication, but before comparison
 multiplication level operators are next,
 and finaly exponent opperators the highest priority are performed first, before any multiplication
 if two opperators of the same priority are encountered then the one furthest to the 
 left is evaluated first
 the current list of available binary opperators, by priority level are:
 level 1:comparison (return a floating 0.0 for true or 1.0 for false)
 =,<,>
 level 2:addition
 +,-
 level 3:multiplication
 *,/,% (modulus treats operands as intigers, floating parts of both opperands are ignored and will be lost)
 level 4:exponation
 ^,#(b#x == b'th root of x),~(b~x == the log base b of x)
 remember highest priority is evaluated first

 unary opperators are performed before any binary opperator. a unary opperator is defined as a function with 
 one parameter, for example sin(x). a unary opperator can have as many charicters as desiered to define it
 and it's one opperand must always be on it's right side (though not nesisaraly within parenthises, though 
 it's generaly a good idea to follow this for clarity, there is no evaluation penalty for additional 
 parenthises).
 the current list of defined unary opperators is:

  (all trig use radian input)
 hypsin - hyperbolic trig
 hyptan
 hyptan
 arcsin - inverse trig
 arccos
 arctan
 sin - standard trig
 cos
 tan
 log - base 10
 ln - base E
 sqrt - square root
 deg - converts radian to degree
 rad - converts degree into radian
 ! - factorial
 abs - absolute value
 ipart - intiger part (drops floating point)
 fpart - floating point (drops everyting <= 1.0)
 sign - if(>0)returns 1 if(<0)returns -1 if(==0) returns 0
 not - logical not, if any value other than 0 it returns 0, if ==0 returns 1

  there are also some constants defined, when the expression string is first 
  read these are replaced with numarical approximations
  PI = 3.14159265358979323846264338327
  E = 2.718281828459045235360287471352

  game variables.
  the last important aspect of the variable system is the game variables, 
  these give accsess to variables within the game such as : 
  global variables (time, fov, ect)
  object variables (the currently relevent object, speed, hull streignth, engine output, ect)
  texture (number of frames, frames per second (both for animation))
  other stuff... this is still heavily under construction, and not many variables have been made yet
  all game variables rely on a data pack passed to it, not all evaluations will have all data 
  items relevent to them. for example texture variables will not be relevent to per ship evaluations
  ship variables (like shield strength) will not be relevent to per weapon evaluations. 
  in the instance that irrelevent variables are used in expressions the default return value for 
  them should , generaly, be 0.0f. however there may be instances in wich a diferent irrelevent 
  return would be mor logical, it is up to the person adding the new game variable to make sure
  they cover all the bases.

  to make a new game variable, in variables.cpp derive from the class game_variable (feel free to 
  make a diferent style constructor, like one that takes the name of an element within a linked 
  list, but do remember to add the apropriate instansiation code and if posable default values), 
  and define the value(variable_call_data*data) function with an appropriate function, keep in 
  mind that the default value for data is NULL, and the default value for all member pointers 
  within data is NULL, take care of these cases at all times. 
  
  once you have the class defined you need to add parseing code to get_object_variable, there is
  a big if/elseif cascade and it should be obvius what to do if you look at it for five minutes.
  there is the class level and the member leve cascades, all you need to do is copy and paste from
  a variable similar to what you want, changeing the names as apropriate.

  game variable syntax goes as such

  :scope.member[element].submember:

  the oppening and closeing collens are requiered to identify the string within as a single game 
  variable. scope is the object or area that the variable inhabits. member is the actual variable
  the same named variable in diferent scopes may do totaly diferent things. and a variable may 
  requier some information, such as an index to an element in an array, or the name of a item in 
  a linked list, or something else to identify a subset in some sort of list, this should go within 
  brakets, although at the time of this documentation no variable actualy uses this. finaly, member
  submembers, members of, members same syntax as member, if your reading this then you undoubtably 
  understand the thinking behind this and I won't insult your intelect by describeing it any 
  further basic access of mebers within a structure. though if this gets used for public documentation 
  someone probly will have to.

  if there is a syntax error in a game variable the default error behavior is to create a 
  constant of value 0.0f

*/


//#include "object/object.h"	//WMC - not really needed
#include "globalincs/pstypes.h"
#include <math.h>

#ifndef _VARIABLE_H__
#define _VARIABLE_H__


enum variable_type {VAR_BASE, VAR_CONSTANT, VAR_OPP, VAR_EXP, VAR_GAME};

//data structure for holding information about a texture
struct texture_variable_data{
	texture_variable_data():n_frames(1),fps(1){};
	texture_variable_data(texture_variable_data*t){
		if(t){
			n_frames = t->n_frames; 
			fps = t->fps;
			}else{
			n_frames = 1;
			fps = 1;
		}
	};
	int n_frames;
	int fps;
};

//a pointer passed when evaluateing an expression gives information to the 
//expression about wich object is the current object and wich texture is the 
//current texture, crap like that.
struct variable_call_data{
	variable_call_data(struct object*o=NULL,texture_variable_data*t=NULL):obj(o),texture(t){};
	void operator = (int){};
	object*obj;
	texture_variable_data texture;
};


//base class of all variables from wich all other variables 
//(includeing full expressions) are derived
class variable{
protected:
	variable_type type;//an identifier, needed so I can perform constant colapseing
public:
	variable(variable_type t):type(t){};//all ariables must define what type of variable they are

	virtual~variable(){};//virtual so derived destructors will get called when you expect them to be
	// all variable's wich child variables _*must*_ kill there childeren when they die

	virtual float value(variable_call_data*data=NULL) = 0;//this is the critical function of 
	//the varialbe system it will be diferent on each derived class, it returns the value of 
	//a variable, if it's a game variable, it's usualy calling a accessor function like 
	//timestamp() or returning something from the passed data

	variable_type get_type(){return type;};
	//returns what type of variable this is, used for constant colapseing, 
	//type should never change after instansiation
};



class expression:public variable{
	//this is a class made to mbuild and manage an expression tree

	variable *head;	//the head of the expression tree

	variable* parse_next(char*&);//part of the constructor, 
	//is recursively called to build the expression tree from a postfix expression
	//each call moves the string as it parses it, hence the reference

	variable* get_object_variable(char**);//this takes a pointer to a cstring
	//that contains game variable formatting
	/*game variable syntax goes like this
	:scope.member[element]:
	the colens denote the begining and end of a game variable
	scope is wich object or area the requiered variable is in, 
	for example global, or object, or texture
	member is the name of the specific variable
	and [element] is used for variables that are in the form of 
	arrays or lists. element is usualy a number, but sometimes a string or enumeration
	like parse_next it moves the input string as it parses it so upon return the calling 
	function's string is at the next item ready to be parsed
	*/
	int expression::wich_unary(char*c);
	//utility function for determineing wich if any unary opperator the input string denotes

	//the function prefix() used to be a member, but it requiered the string class and I 
	//didn't want to requier the include of that file for one function declairation
public:
	expression(char*);//constructor, takes an expression string and builds the tree
	~expression(){delete head;};//kills the head. the head in turn kills it's 
	//childeren wich them selves kill there childeren, 
	//culminateing in a cascade of infantacidal dealocation horror!

	float value(variable_call_data*data=NULL);
	//recursively evaluates all elements of the tree.
	//actualy, it doesn't call expression::value
	//so maybe recursive isn't quite the right word for it... hmmm...
};

bool is_value(char c);//utility, tells if a charicter is part of a number

class var_matrix{
	//a matrix of expressions
	//neat huh?
	//built to minimize evaluation calls
	//if no evaluations are requiered, it returns identity

	variable**mat;//will always be allocated to a 4*4 matrix of expressions
	int size[2];//the largest direction in a given dimention assigned a value
	//I was going to use this for something, but I forgot what... something 
	//about it auto-maticly being able to make variable vectors and/or N*M matriciese
public:
	void set_element(int i,int j, char*str){
		//set's an element in the matrix, if the value to be set 
		//is what it would be if the matrix were an identity at the 
		//specified element no allocation is performed
		//code in the get_value function will find a NULL there and return the apropriate value

		Assert(j<4);
		Assert(i<4);
		if(i+1 > size[0])size[0]=i+1;
		if(j+1 > size[1])size[1]=j+1;

		char *s = str;
		while(is_value(*s))
			s++;
		//if there is a number at the front of the expression go to the next item
		if(!*s){
			//if there is nothing after the first element in the expression see if it's 
			//an identity element if it is then we ignore it
			float f = (float)atof(str);
			if(i == j && f == 1.0f)return;
			if(i != j && f == 0.0f)return;
		}

		//not identity, well we have to allocate an expression

		//if there is no element pointer matrix yet, 
		//it would probly be a good idea to build that first though
		if(!mat){
			mat = new variable*[16]; 
			memset(mat,NULL,sizeof(expression*)*16);
		}

		//ok get the index of the element
		int idx = j*4+i;
		//if something lives there now kill it and take it's land
		if(mat[idx]){
			delete mat[idx];
			mat[idx] = NULL;
		}
		//ah, an empy home, well I'll just move right on in
		mat[idx] = new expression(str);
	}
	float get_element(int i,int j, object*obj=NULL){
		Assert(j<4);
		Assert(i<4);
		int idx = j*4+i;
		//grab the index
		//if there is a matrix, and there is an element in the matrix at the defined  location
		//return the value of the expression
		if(mat && mat[idx])return mat[idx]->value(&variable_call_data(obj));
		//otherwise retrun identity
		if(i==j)return 1.0f;
		return 0.0f;
	}
	int get_size(int d){return size[d%2];}//get how big the matrix is in a dimention

	var_matrix():mat(NULL){};//construct to identity

	~var_matrix(){
		//kill off each element within the matrix, them kill the list of elements it'self
		if(mat){
			for(int i = 0; i<16; i++)
				if(mat[i])delete mat[i];
			delete[]mat;
		}
	}
};

#endif
