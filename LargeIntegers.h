/*
Lint class. Version 1.5

Copyright VF (C) 2004
implemented by Vagelis Fortunas Mytilini Greece
email:  vagelisfortunas@lycos.com
December 2004

*/
#ifndef LARGE_INTEGERS_VF_H
#define LARGE_INTEGERS_VF_H

#pragma once
#define MAXLEN 150000//number of binary dword digits per large integer (for allocation)
//you can change the MAXLEN to improve eficiency (improves eficiency only for memory).
//the max MAXLEN value must be up to about 16 millions (~ 64 Mb per Large Integer)
// all the large integers are dynamicaly allocated (not on the stack)
#define BUFLEN 450000//for displaying, string representation of LINTs, purposes

const int primesSize = 78499;

class Lint 
{
public:
	typedef	unsigned __int32 BU,*PBU;//Basic unit for calculations
	Lint(void);//constructor
	Lint(int);//constructor
	Lint(BU l);//constructor
	Lint(Lint&	source);//constructor
	Lint(const	char* str);//constructor from null terminating sting 
	virtual ~Lint(void);//destructor
	////// shift operators //////////////////
	Lint& __fastcall operator >> (BU pp);//not above 32 bits shift allowed
	Lint& __fastcall operator << (BU pp);//not above 32 bits shift allowed
	//////basic	arithmitic operators//////////
	Lint& __fastcall operator + (Lint&	pp);
	Lint& __fastcall operator - (Lint&	ss);
	Lint& __fastcall operator / (Lint&	dd);
	Lint& __fastcall operator * (Lint&	mm);
	Lint& __fastcall operator % (Lint&	mm);
	Lint& __fastcall operator + (BU pp);
	Lint& __fastcall operator - (BU ss);
	Lint& __fastcall operator / (BU dd);
	Lint& __fastcall operator * (BU mm);
	Lint& __fastcall operator % (BU mm);
	Lint& __fastcall operator + (int pp);
	Lint& __fastcall operator - (int ss);
	Lint& __fastcall operator / (int dd);
	Lint& __fastcall operator * (int mm);
	Lint& __fastcall operator % (int mm);
	////////inc & dec operators ////// thanks Tomas Nemec for advice
	void __fastcall operator ++ ();		//prefix
	void __fastcall operator -- ();		//prefix
	void __fastcall operator ++ (int);	//postfix
	void __fastcall operator -- (int);	//postfix
	////////assign operators //////	
	void __fastcall operator =	(Lint& LI);
	void __fastcall operator =	(BU	bu);
	void __fastcall operator =	(int it);
	void __fastcall operator += (Lint&	pp);
	void __fastcall operator -= (Lint&	ss);
	void __fastcall operator /= (Lint&	dd);
	void __fastcall operator *= (Lint&	mm);
	void __fastcall operator %= (Lint&	mm);
	void __fastcall operator += (BU pp);
	void __fastcall operator -= (BU ss);
	void __fastcall operator /= (BU dd);
	void __fastcall operator *= (BU mm);
	void __fastcall operator %= (BU mm);
	void __fastcall operator += (int pp);
	void __fastcall operator -= (int ss);
	void __fastcall operator /= (int dd);
	void __fastcall operator *= (int mm);
	void __fastcall operator %= (int mm);
	////////// comparison operators	////////////////////////
	bool __fastcall operator ==(Lint& cc);
	bool __fastcall operator >=(Lint& cc);
	bool __fastcall operator <=(Lint& cc);
	bool __fastcall operator >(Lint& cc);
	bool __fastcall operator <(Lint& cc);
	bool __fastcall operator !=(Lint& cc);
	bool __fastcall operator ==(BU	cc);
	bool __fastcall operator >=(BU	cc);
	bool __fastcall operator <=(BU	cc);
	bool __fastcall operator >(BU cc);
	bool __fastcall operator <(BU cc);
	bool __fastcall operator !=(BU	cc);
	bool __fastcall operator ==(int cc);
	bool __fastcall operator >=(int cc);
	bool __fastcall operator <=(int cc);
	bool __fastcall operator >(int	cc);
	bool __fastcall operator <(int	cc);
	bool __fastcall operator !=(int cc);
	//////////////////////	functions ///////////////////////////

	int FromFile(char* filename);//reads all raw binary data from an file and
	// saves it to LINT. filename must be the full path and filename string.

	int ToFile(char* filename);//writes all raw binary data from an Linteger to
	// a file. filename must be the full path and filename string.

	int __fastcall	FromBuffer(void* itemdata,BU items,BU itemsize);
	//	loads the LINT with	raw	binary data	from itemdata. 'itemdata' have size
	//	'itemsize' bytes. 'items' is the number	of items to	be loaded.
	//	'itemdata' points to the first item	of buffer (lsb). Returned sign is always positive.
	
	int __fastcall	ToBuffer(void* buffer,PBU bufsize);
	//	loads the buffer with	raw	binary data	from Lint. bufsize is the returned
	// buffer size(in bytes). Warning: to avoid buffer overruns, the size of
	// buffer must be greater or equal than IntLen*sizeof(BU).   (IntLen*4)
	// a returned value >0  or buffsize==0 means error 

	static void __fastcall WipeOutAllGlobals(void);//clears all the data from all global Lint variables
	// Sometimes for security reasons we must clean all variables.
	
	void __fastcall WipeOut(void);//clears	all	the	data from integer
	void __fastcall MakePos(void);//makes the Linteger	positive
	void __fastcall MakeNeg(void);//negate	the	Linteger
	int __fastcall	Div1(Lint* divisor,Lint* quot,Lint*	rem);//Large Integer Division(
	//the divident is the caller object)

	int __fastcall	Div2(BU	divisor,Lint* quot,Lint* rem);//division by	dword
	//(the divident	is the caller object)

	int __fastcall	DivTrial(unsigned int md);//divide 'this' by
	//first	primes up to md	and	returns	the	first prime	factor if composite
	//this is a	very fast function (linear to IntLen). Primes are stored in
	// variable	'primes'. All numbers below	md^2 tested with this function which
	//returned	0, are sure primes.

	int __cdecl Mul1(Lint*	A2,	Lint* product);//Large Integer multiplication
	//(traditional)(the	multiplicant is	the	caller object)

	int __fastcall	Mul2(BU	A2,	Lint* product);//Large Integer multiplication 
	//by dword(the multiplicant	is the caller object)

	int __cdecl Mul3(Lint*	A2,	Lint* product,BU digits);//	partial	Large
	//Integer multiplication (calculate	only last digits)

	int __cdecl Sub1(Lint*	A1,	Lint* A2, Lint*	result);//subtraction 
	int __fastcall	Sub2(Lint* A1);//subtraction this=this-A1 
	int __cdecl Add1(Lint*	A1,Lint* A2, Lint* Sum);//addition 
	int __fastcall	Add2(Lint* A1);//addition this=this+A1
	int __cdecl Exp(BU	fact,BU	exp,Lint* res);//exponentiation
	int __cdecl Egcd(Lint*	A1,Lint* result);//efklidis	gcd	algorythm inputs must be positives
	int __cdecl Gcd(Lint* A1,Lint*	result);//binary gcd algorythm inputs must be both positives
	
	void __fastcall ExactBits(int bitscount);
	
	int __fastcall	Sl(BU t=1);//shift left	t bits x=x*2^t
	int __fastcall	Sr(BU t=1);//shift right one bit x=x/2
	int __fastcall	Sqr1(void);//squares the integer
	int __fastcall	ToStr(char*	str);//converts	LINT tostring
	BU __fastcall GetLength(void);//returns the lenght in dword digits of Integer
	BU __fastcall GetDigit(BU pos);//returns the digit(dword) in specified position
	// Warning: no error check  pos must < MAXLEN. GetDigit(0)is the first 
	// digit. You must be carefull not pos>=GetLength() because you take false result.

	bool __fastcall IsOdd(void);//returns true if integer is odd else false
	bool __fastcall IsNeg(void);//returns true if integer is negative else false	

	unsigned int __fastcall CompareU(Lint*	A1);//compares absolute	values(unsigned)
	//returns 0	if equal, 1	if the first(this) operand is greater, and 2 if	second
	//operand is greater

	BU __fastcall CompareS(Lint* A1);//compares signed LargeIntegers
	//returns 0	if equal, 1	if the first(this) operand is greater, and 2 if	second 
	//operand is greater
	
	int __fastcall	ExpMod(Lint* A2/*exp*/,Lint* mod,Lint* result);// calc this^A2 mod mod 
	//and puts it to result (slower than ExpMod1)
	
	int __fastcall	ExpMod1( Lint* A2/*exp*/,Lint* mod,Lint* result);//	calc this^A2 mod mod 
	//and puts it to result. Warning: Length of	'this' must	not	above 2 * mod length
	// eg:	   if(this->GetLength >	(2 * mod.GetLength)) goto somecode;//but not to	ExpMod1
	
	int __fastcall	ExpMod3( Lint* A2/*exp*/,Lint* mod,Lint* result);//	calc this^A2 mod mod 

	int __fastcall	BarrettReduction(Lint* A2/*mod*/);//this=this mod A2
	//Warning:	this->IntLen<=2	* A2->IntLen (else we will have	problems) inputs must be positives
	
	int __fastcall	BarrettReductionPrecalc(void);//is for calculate the 32^2k/m for first time.
	//The function produces	(32^(2*k))/m , an stores it	to the static variable b2km.
	//(eg. x.BarrettReductionPrecalc() -> b2km=32^2k /x)  k	is the x.IntLen

	int __fastcall	InvMod(Lint* modl,Lint*	result);//calculate	this^-1	mod	modl (inverse mod)
	
	int __cdecl MillerRabinTest(BU	t);//returns 1 if candidate	'this' is composite.
	//Returns 0	if this" is	probably prime.	Increasing "t" factor by one, 
	//decreases	the	probability	by about 1/4 to	have false result when the function	returns	0
	// The probability,	the	'this' is composite, is	not	more (at worst case) than 0.25^t

	int	__fastcall FirstPrimeAbove(BU t);//makes 'this', the first probable	prime above	'this'
	// t is	the	factor described above (MillerRabinTest). 

	bool __fastcall IsProbablePrime(); // is the number prime ???

	int	__fastcall MakeRandom(BU bits);//this is a kind	of 'true random	number generator'
	// 'bits' is the number of random bits returned (rounded to dwords 32bit).
	// for further details look	at definition.
	// Regardless of the results (diehard or something else) this number generator must be 
	// subject for further study.

private:
	void __fastcall ZeroSign(void);//makes the sign positive if integer is zero
	int __fastcall ReallocData(BU newsize);//increases or decreases the length of integer
	int __fastcall NormalizeLength(void);//cuts all the leading zero bytes
	BU sign;//the sign 0=plus and other= minus
	BU IntLen;//actual length of integer (dwords)
	PBU Data;//points to the integers.data (can be ghange for shift right purposes)
	//to increase the speed. Use this pointer carefully. Data >= DataBase.
	PBU DataBase;//points to the first dword of the allocated memory for Data (change this, forbiden)
 	BU Resrv;//Reseved dword. This can be a marker to mark the Integer  e.g for deletion,
	//for saving, as unused,... etc.

};

void SetRadix(unsigned int r);//sets the radix representation for string functions
int GetRandomBits(void* buffer,unsigned int bits);

//global counter and timer functions. To count the speed of the functions

extern char sc[25];//this is the result after Ac or At operations

void  Bc(void);//call it just before the function you want to measure
void  Ac(void);//calc the difference(in clock pulses) between Bc() and Ac()
// and saves it as a string in global variable sc
// Theese functions are like QueryPerformanceCounter in windows kernel

void  Bt(void);//must be called prior to At()for measuring time difference
void  At(void);//calc the difference(in 100 ns) between Bt() and At()
// and saves it as a string in global variable sc
// Theese functions are equivalents to GetSystemTimeAsFileTime in windows kernel
//but without include windows headers and libraries


typedef Lint LINT,*PLINT;

#endif //LARGE_INTEGERS_VF_H
