/* Lint class. Version 1.5

Copyright VF (C) 2004
implemented by Vagelis Fortunas Mytilini Greece
email:  vagelisfortunas@lycos.com December 2004

You are free to use this code as you wish.
In no event will the author be hold liable for any damages arising from the use of this software.

There is no zero divisor check, so you must be careful whith divisions

No complex operations allowed such r= a*x+b*y+c%z..... or  f=(a+b)*(c-d)/(...
because this class can't keep intermediate temporary results for any complex
operation e.g.(b*y)
You can use this type of complex operation:
a=((b*c+3+w+r+t)*h/r/r/2+3)%f

MAXLEN is the  allocated (malloc) number of binary dword digits per large integer.

BUFLEN is the buffer size in bytes allocated in stack for display only purpose.

Look at header file for more details about defined functions
This class is not thread safe.

*/

//############################################################################################

#pragma warning(disable:4100) // Disable this warning because argumens passed via registers
#include <stdio.h>//fopen
#include <stdlib.h>// malloc free
#include <string.h>//memcpy
#include <ctype.h>//isalpha, isdigit
#include<io.h> //_close _open
#include<FCNTL.H> //_O_BINARY
#include<SYS\STAT.H> //_S_IWRITE
#include "LargeIntegers.h"
// variables for improving speed (but costs for memory)
Lint LITmp1,LITmp,LITmp2;;//
Lint gcd,gcd1;//
Lint divy;//div
Lint divtmp;//div
Lint ybnt;//div
Lint gen;//for general purpose
Lint cres,cres1,restmp;// temporary results for complex operations
Lint r2,q3,b2km;//for Barret modular reduction
Lint mdxp,mdxp1;//for modular exponentiation
Lint sqr;//for squaring
LINT y,r;//for Miller-Rabbin test
unsigned int radix=10;
extern const unsigned int primes[];//look at primes.cpp
//############################################################################################
unsigned int sth,stl;//stamp counter of proccessor
unsigned int tth,ttl;//time counter (0,1 micro sec interval)
char sc[25];
//############################################################################################
__declspec(naked) void Bc(void)
	{//this func must called before each operation to store the counts in sth:stl	
		__asm
		{
			push eax
			push edx
			rdtsc// main action
			mov sth,edx
			mov stl,eax
			pop edx
			pop eax
			ret
		}
	}

//############################################################################################
__declspec(naked) void Ac(void)
	{//this func must called after operation to calc the counts difference stored in sth:stl	
		__asm
		{
			push eax
			push edx
			rdtsc// main action
			sub eax,stl//calculate the difference between this rdtsc and pevious Bc()
			sbb edx,sth
			//minus 92
			sub eax,92//(92 counts is the offset between the Bc and Ac func)
			sbb edx,0
			push 10
			push offset sc//loads the sc buffer with result
			push edx
			push eax
			call _ui64toa//convert the difference to string and stores it to sc
			add esp,16
			pop edx
			pop eax
			ret
		}
	}
//############################################################################################
__declspec(naked) void Bt(void)
	{//this func must called before each operation to count. the time stored in tth:ttl	
		__asm
		{
			//representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
			push eax
			push edx
gt0:		mov edx,ds:[2147352600]//read the timer from this address
			mov eax,ds:[2147352596]//read the timer from this address 
			cmp edx,ds:[2147352604]//read the timer from this address 
			jne gt0
			mov ttl,eax 
			mov tth,edx 
			pop edx
			pop eax
			ret
		}
	}

//############################################################################################
__declspec(naked) void At(void)
	{//this func must called after  operation to count the time stored in tth:ttl	
		__asm
		{
			push eax
			push edx			
gt0:		mov edx,ds:[2147352600]//read the timer from this address
			mov eax,ds:[2147352596]//read the timer from this address 
			cmp edx,ds:[2147352604]//read the timer from this address 
			jne gt0
			sub eax,ttl
			sbb edx,tth
			push ebx
			mov ebx,10
			div ebx//convert to microseconds
			pop ebx
			mov edx,0
			push 10
			push offset sc
			push edx
			push eax
			call _ui64toa//loads the sc buffer with result
			add esp,16
			pop edx
			pop eax
			ret
		}
	}

void Lint::ExactBits(int bitscount)
{
unsigned int LeftBits = bitscount%32;
if (LeftBits==0) return; // we have 32 bit multiple

unsigned int Mask = (1<<LeftBits)-1;
Data[GetLength()-1] &= Mask;  // keep only necesary bits
/*
	unsigned int HighMask;
	if (LeftBits!=0)
	HighMask = (1<<(LeftBits-1));
	else
	HighMask = 1<<31;
	v[RealDwCount-1] |= HighMask;  // set highest bit
*/
}

//############################################################################################
void SetRadix(unsigned int r=10)// set radix
{
	radix=r;
	if (radix>36)radix=36;
	if (radix<=1)radix=10;	
}
//############################################################################################
int GetRandomBits(void* buffer,unsigned int bits)
{		
	int file;
	char filename[]="\\~VFtmp0001";
	unsigned int tm;
	tm=bits/16;
	if(bits%16>0)tm++;//round bits (if necessary) to next 16 bits value
	
	if((file=_open(filename,_O_TRUNC |_O_BINARY | _O_CREAT | _O_WRONLY,_S_IWRITE))!=-1)//open  file for write
	{	   
		char buf[16519];//not equal to any cluster size 

		for (unsigned int a=0;a<tm;a++)
		{	
			_write(file,buf,sizeof(buf));// actually this operation writes data to buffer
										//  NOT to disk


			Bc();			//Here is the main action(Counts needed for writing
			_commit(file);	//the data to the disc surface.)
			Ac();			//this value needs, in any case, debiasing.
					
			
			//    WARNING *  WARNING.   This, must be subject for many more tests.
			//  I do not recommend to use it without extended tests

			stl=stl & 0x3ffff;	//this kind of debiasing tested only on my PC
			stl=stl>>2;			//This debiasing keeps 16 bits after second bit
			//////////////////////////////////////////////////////////////////////////

			
			__asm
			{	//write the data to buffer (16 bits slices) 
				mov eax,stl
				mov ebx,a
				mov ecx,buffer
				mov [ecx+ebx*2],ax// 16 bits
			}			
		}
		_close(file);
		remove(filename);
		return 0;
	}
	return 1;
	
}
//############################################################################################
Lint::Lint(void)
{
	IntLen=1;
	this->DataBase=(PBU)malloc(MAXLEN*sizeof(BU));
	this->Data=this->DataBase;//Data points to the begin of integer
	sign=0;
	this->Data[0]=0;//without sign
}
//############################################################################################
Lint::~Lint(void)
{
	WipeOut();
	free(DataBase);	
}
//############################################################################################
Lint::Lint(BU l)//
{
	DataBase=(PBU)malloc(MAXLEN*sizeof(BU));
	Data=this->DataBase;//Data points to the begin of integer
	sign=0;
	IntLen=1;
	Data[0]=l;
}
//############################################################################################
Lint::Lint(int l)
{
	DataBase=(PBU)malloc(MAXLEN*sizeof(int));
	__asm
	{
		mov ecx,this
		mov edx,l
		mov eax,edx
		and eax,80000000h//we want only the sign
		jz nxt49
		neg edx
nxt49:	shr eax,31//put it to lsb
		mov [ecx]Lint.sign,eax//copy sign
		mov [ecx]Lint.IntLen,1//copy curlen
		mov eax,[ecx]Lint.DataBase
		mov [ecx]Lint.Data,eax
		mov [eax],edx//*this=l;
	}
}
//############################################################################################
Lint::Lint(Lint& source)//??0Lint@@QAE@PAV0@@Z
{
	DataBase=(PBU)malloc(MAXLEN*sizeof(BU));
	__asm
	{//copy the source to this
		mov ebx,this
		mov eax,source
		mov ecx,[eax]source.IntLen
		mov [ebx]this.IntLen,ecx//copy curlen first
		mov edx,[eax]source.sign
		mov [ebx]this.sign,edx//copy sign
		mov edx,[eax]source.Data
		sub edx,[eax]source.DataBase
		add edx,[ebx]this.DataBase
		mov [ebx]this.Data,edx//copy DataIndex
		mov esi,[eax]source.DataBase
		mov edi,[ebx]this.DataBase
		cld//  cld is for inc and std is for dec
		rep movsd//*this=source;
	}
}

//############################################################################################
Lint::Lint(const char* str)
{
	DataBase=(PBU)malloc(MAXLEN*sizeof(BU));//only one byte
	Data=DataBase;//Data points to the begin of integer
	Data[0]=0;
	sign=0;
	IntLen=1;
	bool tmpsign=false;//temporary sign
	unsigned char* index=(unsigned char*)str;
	if (*str == '-'){tmpsign=true;index++;}//negative number
me:	if (*index=='0'){index++;goto me;}//cut all leading zeroes
	while((isdigit(*index)&&(*index<(radix+'0')))||(isalpha(*index)&&(radix>10)&&(*index<(radix+'A'))))
	{
		if (radix<=10)LITmp1=*index-'0';
		else
		{
			if (*index>'9')
				LITmp1=*index-'A'+10;
			else LITmp1=*index-'0';
		}

		Mul2(radix,&gen);
		Add1(&gen, &LITmp1, this);
		++index;
	}
	NormalizeLength();
	if (tmpsign &&(Data[0]!=0 ||IntLen>1 )) sign=1;//negative number
}
//############################################################################################
int Lint::FromBuffer(void* itemdata,BU items,BU itemsize)
{
	if (MAXLEN*sizeof(BU)< items*itemsize)return 1;//buffer is biger than LINT    
	BU tmplen;
	tmplen=items*itemsize;//length of buffer in bytes(8bit)
	if (!tmplen) return 2;//invalid size or items
	IntLen= tmplen/sizeof(BU);
	if (tmplen%sizeof(BU)>0) IntLen++;//increase
	Data[IntLen-1]=0; //make the last dword zero.
	if(!memcpy(Data,itemdata,tmplen)) return 3;//cant copy	
	sign=0;//sign is always positive
	NormalizeLength();//cut leading zeroes
	return 0;
}
//############################################################################################
int Lint::ToBuffer(void* buffer,PBU bufsize)
{
	*bufsize=IntLen*sizeof(BU);//length of buffer in bytes(8bit)
	if(!memcpy(buffer,Data,*bufsize))
	{
		*bufsize=0;
		return 1;//cant copy
	}
	return 0;
}
//############################################################################################
int Lint::FromFile(char* filename)
{		
	FILE *stream;
	int nr_of_error=0;
	BU filelen;
	if((stream=fopen(filename,"rb"))!=0)
	{				
		setvbuf(stream,0,_IONBF,0);//allow no buffering
		fseek(stream,0,SEEK_END);//for read size only
		filelen=(BU)ftell(stream);		
		if (MAXLEN*sizeof(BU)< filelen){nr_of_error=1;goto er;}//file is biger than LINT can take   
		if (!filelen){nr_of_error=2;goto er;}//empty file
		IntLen= filelen/sizeof(BU);
		if (filelen%sizeof(BU)>0) IntLen++;//increase
		Data[IntLen-1]=0; //make the last dword zero.		
		fseek(stream,0,SEEK_SET);
		if (fread(Data,1,filelen,stream)!=filelen)nr_of_error=3;
er:		fclose(stream);
		return nr_of_error;
	}
	return 4;// file does not e
}
//############################################################################################
int Lint::ToFile(char* filename)
{		
	FILE *stream;
	int nr_of_error=0;
	if((stream=fopen(filename,"wb"))!=0)
	{				
		setvbuf(stream,0,_IONBF,0);//allow no buffering(not no HD cashing)
		if(fwrite(Data,sizeof(BU),IntLen,stream)!=IntLen)nr_of_error=1;//cant write
		fclose(stream);
		return nr_of_error;
	}
	return 2;//cant open file
}
//############################################################################################
Lint& Lint::operator >> (BU pp){Sr(pp);return *this;}
//############################################################################################
Lint& Lint::operator << (BU pp){Sl(pp);	return *this;}
//############################################################################################
bool Lint::operator ==(Lint& cc){return(CompareS(&cc)==0);}
//############################################################################################
bool Lint::operator >=(Lint& cc){BU zz;zz=CompareS(&cc);return((zz==0)||(zz==1));}
//############################################################################################
bool Lint::operator <=(Lint& cc){BU zz;zz=CompareS(&cc);return((zz==0)||(zz==2));}
//############################################################################################
bool Lint::operator >(Lint& cc){return(CompareS(&cc)==1);}
//############################################################################################
bool Lint::operator <(Lint& cc){return(CompareS(&cc)==2);}
//############################################################################################
bool Lint::operator !=(Lint& cc){return(CompareS(&cc)!=0);}
//############################################################################################
bool Lint::operator ==(BU cc){gen=cc;return(CompareS(&gen)==0);}
//############################################################################################
bool Lint::operator >=(BU cc){BU zz;gen=cc;zz=CompareS(&gen);return((zz==0)||(zz==1));}
//############################################################################################
bool Lint::operator <=(BU cc){BU zz;gen=cc;zz=CompareS(&gen);return((zz==0)||(zz==2));}
//############################################################################################
bool Lint::operator >(BU cc){gen=cc;return(CompareS(&gen)==1);}
//############################################################################################
bool Lint::operator <(BU cc){gen=cc;return(CompareS(&gen)==2);}
//############################################################################################
bool Lint::operator !=(BU cc){gen=cc;return(CompareS(&gen)!=0);}
//############################################################################################
bool Lint::operator ==(int cc){gen=cc;return(CompareS(&gen)==0);}
//############################################################################################
bool Lint::operator >=(int cc){BU zz;gen=cc;zz=CompareS(&gen);return((zz==0)||(zz==1));}
//############################################################################################
bool Lint::operator <=(int cc){BU zz;gen=cc;zz=CompareS(&gen);return((zz==0)||(zz==2));}
//############################################################################################
bool Lint::operator >(int cc){gen=cc;return(CompareS(&gen)==1);}
//############################################################################################
bool Lint::operator <(int cc){gen=cc;return(CompareS(&gen)==2);}
//############################################################################################
bool Lint::operator !=(int cc){gen=cc;return(CompareS(&gen)!=0);}
//############################################################################################
Lint& Lint::operator + (Lint& pp)
{
	Add1(this,&pp,&restmp);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator - (Lint& ss)
{
	Sub1(this,&ss,&restmp);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator * (Lint& mm)
{
	Mul1(&mm,&restmp);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator / (Lint& dd)
{
	Div1(&dd,&restmp,&gen);
	cres=restmp;
	return cres;

}
//############################################################################################
Lint& Lint::operator % (Lint& mm)
{
	Div1(&mm,&gen,&restmp);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator + (BU pp)
{
	gen=pp;
	Add1(this,&gen,&restmp);
	cres=restmp;
	return cres;

}
//############################################################################################
Lint& Lint::operator - (BU ss)
{
	gen=ss;
	Sub1(this,&gen,&restmp);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator * (BU mm)
{
	Mul2(mm,&restmp);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator / (BU dd)
{
	Div2(dd,&restmp,&gen);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator % (BU mm)
{
	Div2(mm,&gen,&restmp);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator + (int pp)
{
	gen=pp;
	Add1(this,&gen,&restmp);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator - (int ss)
{
	gen=ss;
	Sub1(this,&gen,&restmp);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator * (int mm)
{
	gen=mm;
	Mul1(&gen,&restmp);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator / (int dd)
{
	LITmp=dd;
	Div1(&LITmp,&restmp,&gen);
	cres=restmp;
	return cres;
}
//############################################################################################
Lint& Lint::operator % (int mm)
{
	gen=mm;
	Div1(&gen,&gen,&restmp);
	cres=restmp;
	return cres;
}
//############################################################################################
__declspec(naked)void Lint::operator = (int it)//signed
{
	__asm
	{
		mov eax,edx
		and eax,80000000h//we want only the sign
		jz nxt39
		neg edx
nxt39:	shr eax,31//put it to lsb
		mov [ecx]Lint.sign,eax//copy sign
		mov [ecx]Lint.IntLen,1//copy curlen
		mov eax,[ecx]Lint.DataBase
		mov [ecx]Lint.Data,eax
		mov [eax],edx//*this=it;
		ret //
	}
}
//############################################################################################
__declspec(naked)void Lint::operator = (BU v)
{	
	__asm	
	{//*this=v;
		mov [ecx]this.IntLen,1//copy curlen
		mov [ecx]this.sign,0//copy sign
		mov eax,[ecx]this.DataBase
		mov [ecx]this.Data,eax
		mov [eax],edx//*this=v;
		ret //
	}
}
//############################################################################################
__declspec(naked)void Lint::operator = (Lint& source)
{
	__asm	
	{//*this=source;
		push ebx
		push esi
		push edi
		
		mov ebx,[edx]source.IntLen
		mov [ecx]Lint.IntLen,ebx//copy curlen
		mov eax,[edx]source.sign
		mov [ecx]Lint.sign,eax//copy sign
		mov eax,[edx]source.Data
		sub eax,[edx]source.DataBase
		add ebx,eax
		add eax,[ecx]Lint.DataBase
		mov [ecx]Lint.Data,eax//copy DataIndex
		mov esi,[edx]source.DataBase
		mov edi,[ecx]Lint.DataBase
		mov ecx,ebx
		cld//  cld is for inc and std is for dec
		rep movsd//*this=source;

		pop edi
		pop esi
		pop ebx
		ret //
	}	
}
//############################################################################################
void Lint::operator ++ (){restmp=1;Add2(&restmp);}
//############################################################################################
void Lint::operator -- (){restmp=1;Sub2(&restmp);}
//############################################################################################
void Lint::operator ++ (int){restmp=1;Add2(&restmp);}
//############################################################################################
void Lint::operator -- (int){restmp=1;Sub2(&restmp);}
//############################################################################################
void Lint::operator += (Lint& pp){Add2(&pp);}
//############################################################################################
void Lint::operator -= (Lint& ss){Sub2(&ss);}
//############################################################################################
void Lint::operator *= (Lint& mm)
{
	Mul1(&mm,&cres1);
	*this=cres1;
}
//############################################################################################
void Lint::operator /= (Lint& dd)
{
	Div1(&dd,&cres1,&gen);
	*this=cres1;
}
//############################################################################################
void Lint::operator %= (Lint& mm)
{
	Div1(&mm,&gen,&cres1);
	*this=cres1;
}
//############################################################################################
void Lint::operator += (BU pp){restmp=pp;Add2(&restmp);}
//############################################################################################
void Lint::operator -= (BU ss){restmp=ss;Sub2(&restmp);}
//############################################################################################
void Lint::operator *= (BU mm)
{
	Mul2(mm,&cres1);
	*this=cres1;
}
//############################################################################################
void Lint::operator /= (BU dd)
{
	Div2(dd,&cres1,&gen);
	*this=cres1;
}
//############################################################################################
void Lint::operator %= (BU mm)
{
	Div2(mm,&gen,&cres1);
	*this=cres1;
}
//############################################################################################
void Lint::operator += (int pp){restmp=pp;Add2(&restmp);}
//############################################################################################
void Lint::operator -= (int ss){restmp=ss;Sub2(&restmp);}
//############################################################################################
void Lint::operator *= (int mm)
{
	restmp=mm;
	Mul1(&restmp,&cres1);//warning signed unsigned
	*this=cres1;
}
//############################################################################################
void Lint::operator /= (int dd)
{
	restmp=dd;
	Div1(&restmp,&cres1,&gen);
	*this=cres1;
}
//############################################################################################
void Lint::operator %= (int mm)
{
	restmp=mm;
	Div1(&restmp,&gen,&cres1);
	*this=cres1;
}
//############################################################################################
int Lint::Exp(BU A1,BU exp,Lint* res)//Left-to-right binary exponentiation
{
	if(exp==0){*res=1;return 0;}
	if(A1==0){*res=1;return 0;}
	if(A1==1){*res=1;return 0;}
	//if(fact<0){res=1;return 0;}if below zero
	char bitt;// for testing bit
	*res=1;	
	for(int i=31;i>=0;--i)
	{
		res->Sqr1();		
		__asm
		{			
			mov	eax,i	
			bt exp,eax
			setc bitt
		}
		if(bitt)*res*=A1;
	}
	return 0;
}
//############################################################################################
__declspec(naked)int Lint::ToStr(char *s)//converts to string.
{
	__asm
	{	push ebp	
		push edi//save all registers
		push esi
		push ebx
		push edx
		push ecx
		
		////if this is zero s[0]='0';s[1]=0;return 0;
		mov esi,[ecx]Lint.Data//this
		mov esi,[esi]
		add esi,[ecx]Lint.IntLen
		cmp esi,1
		jne bgn
		mov [edx],48// '0'
		mov [edx+1],0// null
		jmp endff

bgn:	//div=this		
		cld//  cld is for inc and std is for dec
		mov esi,[ecx]Lint.Data//this
		mov edi,LITmp.Data//dvd
		mov ecx,[ecx]Lint.IntLen
		mov LITmp.IntLen,ecx			
		rep movsd//*this=source;			
		//quo=1;
		mov LITmp1.IntLen,1//set intlen
		mov LITmp1.sign,0//set sign
		mov eax,LITmp1.Data
		mov [eax],1//=1;
		//initialize pointers
		lea ecx,LITmp //div
		lea edi,LITmp1//quo
		lea esi,LITmp2//rem
		mov ebx,edx//s
		mov ebp,LITmp2.Data
		mov edx,radix
		
		//////////////// begin of loop //////////////////////////	
arxd1:	//while(quot>0>)
		mov eax,[edi]Lint.Data
		mov eax,[eax]
		add eax,[edi]Lint.IntLen
		cmp eax,2
		jc endts
		//Div2(dvd,radix,quo,rem)
		push esi//rem
		push edi//quo
		call Div2
		//str[strindex] = (char)rem.Data[0] + '0';
		mov al,byte ptr[ebp]//first byte of rem
		cmp al,10
		jc nxt1
		add al,7
nxt1:	add al,48//add the '0'
		mov byte ptr [ebx],al
		// div=quo
		xchg ecx,edi
		//++index;
		inc ebx
		jmp arxd1
endts:	////////////////end of loop //////////////////////////	
		dec ebx//not leading zero		

		//if this.sign>0 s[ebx] = '-';++strindex;
		mov ecx,[esp+0]//this
		mov ecx,[ecx]Lint.sign//this.sign
		jecxz nxt6
		mov byte ptr [ebx],'-'
		inc ebx//inc strindex
nxt6:	mov byte ptr [ebx],0//terminate with null
		push [esp+4] 
		call _strrev//I use this func because is very fast(difference in speed with a substitute, is negligible)
		add esp,4

endff:	mov eax,0//return 0;
		pop ecx
		pop edx//restore
		pop ebx
		pop esi
		pop edi
		pop ebp
		ret //		

		}
}
//############################################################################################
__declspec(naked)unsigned int Lint::GetLength(void){__asm mov eax,[ecx]Lint.IntLen	__asm ret}
//############################################################################################
__declspec(naked)unsigned int Lint::GetDigit(BU pos)
{	
	__asm	mov eax,[ecx]Lint.Data
	__asm	mov eax,[eax+edx*4]
	__asm	ret
}
//############################################################################################
__declspec(naked)void Lint::WipeOut(void)
{
	__asm
	{
		push ecx
		push edi
		mov edi,[ecx]Lint.DataBase
		mov [ecx]Lint.Data,edi
		mov [ecx]Lint.IntLen,0
		mov ecx,MAXLEN
		mov eax,0h
		rep stosd
		pop edi
		pop ecx
		ret
	}
}
//############################################################################################
void Lint::WipeOutAllGlobals(void)
{
	LITmp1.WipeOut();
	LITmp.WipeOut();
	LITmp2.WipeOut();
	gcd.WipeOut();
	gcd1.WipeOut();
	divy.WipeOut();
	divtmp.WipeOut();
	ybnt.WipeOut();
	gen.WipeOut();
	cres.WipeOut();
	cres1.WipeOut();
	restmp.WipeOut();
	r2.WipeOut();
	q3.WipeOut();
	b2km.WipeOut();
	mdxp.WipeOut();
	mdxp1.WipeOut();
	sqr.WipeOut();
	y.WipeOut();
	r.WipeOut();
}
//############################################################################################
__declspec(naked)bool Lint::IsOdd(void)
{
	__asm
	{
		mov eax,[ecx]Lint.Data
		bt dword ptr[eax],0
		mov eax,0
		rcl eax,1
		ret
	}
}
//############################################################################################
__declspec(naked)bool Lint::IsNeg(void){__asm mov eax,[ecx]Lint.sign __asm ret}
//############################################################################################
__declspec(naked)void Lint::MakeNeg(void)
{
	__asm mov [ecx]Lint.sign,1
	__asm ret
}
//############################################################################################
__declspec(naked)void Lint::MakePos(void)
{
	__asm mov [ecx]Lint.sign,0
	__asm ret
}
//############################################################################################
__declspec(naked)unsigned int Lint::CompareS(Lint* A1)
{	
	__asm
	{
		call CompareU
		cmp eax,1
		jne cmp2
		cmp dword ptr[ecx]this.sign,0
		je cmp20
		mov eax,2//this<A1
		ret
cmp20:	mov eax,1//this>A1
		ret

cmp2:	cmp eax,2
		jne cmp0
		cmp dword ptr[edx]A1.sign,0
		je cmp21
		mov eax,1//this>A1
		ret
cmp21:	mov eax,2//this<A1
		ret

cmp0:	//absolute values are equal 
		cmp dword ptr[ecx]this.sign,0
		jne cmpn1
		cmp dword ptr[edx]A1.sign,0
		jne cmpn1
		mov eax,0//both are positive and equal
		ret
cmpn1://one, or both are negatives
		cmp dword ptr[ecx]this.sign,0
		jne cmpn2
		mov eax,1// A1<this
		ret
cmpn2:	//'this' is negative
		cmp dword ptr[edx]A1.sign,0
		je cmpend
		mov eax,0//both are negative and equals
		ret
cmpend: //A1 is positive and 'this' is negative
		mov eax,2//this>A1
		ret
	}
}
//############################################################################################
__declspec(naked)unsigned int Lint::CompareU(Lint* A1)
{
	__asm
	{
		push edi//save registers
		push esi
		push ecx
		pushf
		
		mov esi,ecx//passed from caller
		// and edx passed from caller
		mov ecx,[edx]A1.IntLen
		cmp [esi]this.IntLen,ecx
		je nxtc//if are equals in size go to next comparison
		jnc an1//A1>A2
an2:	mov eax,2//A2 is greater
		jmp endcmp
an0:	mov	eax,0//equals
		jmp endcmp
an1:	mov eax,1//A1 is greater
		jmp endcmp
nxtc:	mov eax,[esi]this.Data
		lea esi,[eax+ecx*4-4]
		mov eax,[edx]A1.Data
		lea edi,[eax+ecx*4-4]
		std//set dir flag for decrement. std is for dec
		repe cmpsd//if equals continue loop//comparison dword by dword
		je an0//if are equals
		jnc an1//A1>A2
		jmp	an2//A1<A2
endcmp:	
		popf//restore dir flag because some stupid compilers think this is allways cld
		pop ecx//restore
		pop esi
		pop edi	
		ret //
	}
}
//############################################################################################
__declspec(naked)int Lint::ReallocData(BU newsize)
{
	__asm	
	{
		push edi//save registers
		push esi
		push ebx
		push ecx
		push edx	
		
		mov esi,edx//newsize
		mov ebx,ecx//passed from caller
		lea edx,[ebx]this.IntLen//curlen 
		mov ebx,[ebx]this.Data//data		
		cmp [edx],esi//curlen-newsize
		je fend//if (newsize==IntLen)return 0;
		jae eql// if newsize < curlen jump		
		mov ecx,esi
		sub ecx,[edx]//set counter to newsize-curlen
		mov edi,[edx]//index to curlen (next dword for allocation)
		mov eax,0		
mlp:	mov dword ptr[ebx+edi*4+0],eax//zeroize all the rest dwords
		inc edi
		loop mlp		
		mov [edx],esi//set the new curlen
		jmp fend
eql:	mov [edx],esi//set new size
fend:
		mov eax,0//return 0;
		pop edx//restore
		pop ecx
		pop ebx
		pop esi
		pop edi	
		ret //
	}	
}
//############################################################################################
__declspec(naked)int Lint::NormalizeLength(void)//cuts all the leading zero bytes
{	
	__asm
	{	//distinguishes all leading zeros
		push ebx

		mov eax,[ecx]Lint.Data//
lp30:	mov ebx,[ecx]Lint.IntLen// length 
		cmp dword ptr[eax+ebx*4-4],0//
		jne end30//need no normalizing
		cmp [ecx]Lint.IntLen,2//not normalize if first dword=0
		jnae end30//not normalize if first dword=0
		dec [ecx]Lint.IntLen//normalize length of this
		jmp lp30

end30:	mov eax,0//return 0;
		pop ebx
		ret //
	}	
}
//############################################################################################
__declspec(naked)int Lint::Mul1(Lint* A2,Lint* product)
{
	BU A2digit;//current digit for internal multiplication
	BU savecx;//save ecx
	BU saveesp;//for saving stack pointer esp	
	BU A1len;//length of A1
	PBU A2data;//pointer of A2->Data
	__asm
	{	
		push ebp
		mov ebp,esp
		sub esp,__LOCAL_SIZE//as  many bytes as local variables
		push edi//save registers
		push esi
		push ebx
		push ecx
		push edx	
		
		//////////// multiplication /////////	
		mov ebx,this
		mov edx,A2
		//if we have A2<=32bit we use Mul2 (faster)
		cmp [edx]Lint.IntLen,1
		jne mmul
		//multiply
		mov esi,product
		push esi
		mov edx,[edx]Lint.Data
		mov edx,[edx]//A2.Data[0]
		mov ecx,ebx//this
		call Mul2
		jmp end31

mmul:	mov saveesp,esp//save esp

		mov esp,product
		
		//product->ReallocDataZero(A2len+A1->IntLen)
		mov ecx,[edx]A2.IntLen
		add ecx,[ebx]Lint.IntLen//A1len+A2len
		mov [esp]product.IntLen,ecx
		mov edi,[esp]product.Data
        mov esi,edi
		mov eax,0
		cld//  cld is for inc and std is for dec
		rep stosd//fill product  data with zeros		
		mov edi,[ebx]Lint.Data
        mov eax,[ebx]Lint.IntLen
		mov A1len,eax
		mov eax,[edx]A2.Data
		mov A2data,eax
		mov eax,[eax]
		mov A2digit,eax		
		mov ecx,[edx]A2.IntLen
beg://mul/////////////////////mul	///////////////////////
		mov savecx,ecx//push ecx
		mov ecx,A1len
		mov	edx,0
		mov ebx,0//for index        
arx:	mov	eax,[edi+ebx*4]// mov eax, A1data
		mov	esp,edx// save temporarily edx
		mul	A2digit//multiply 
		add	eax,esp// from previous	mult
		adc	edx,0//and the carry to	edx		
		add [esi+ebx*4],eax//add the product to memory
		adc edx,0//and the carry to	edx
		inc ebx
		loop arx
		////////////////////////////////////////////////
		mov [esi+ebx*4],edx//write the remain msb to memory
		add esi,4
		add A2data,4
		mov eax,A2data
		mov eax,[eax]
		mov A2digit,eax//
		mov ecx,savecx//pop ecx
		loop beg

		mov esp,saveesp//restore esp

		//product->NormalizeLength();
		mov esi,product
		mov ebx,[esi]product.Data
nl1:	mov	ecx,[esi]product.IntLen//length	value	
		test dword ptr[ebx+ecx*4-4],4294967295
		jnz end31//need no normalizing		
		dec [esi]product.IntLen//normalize length of product
		test [esi]product.IntLen,4294967295//if curlen is above 0
		jnz nl1
		inc [esi]product.IntLen//if 0 then curlen=1
end31:
		// test sign
		mov ebx,this
		mov eax,0
		cmp eax,[ebx]Lint.sign// test sign
		rcl eax,1
		mov ebx,A2
		mov ecx,0
		cmp ecx,[ebx]Lint.sign// test sign
		rcl ecx,1		
		xor eax,ecx
//		cmp eax,ecx//theese all are the xor function (compatibility with 486)
//		jne xr1
//		mov eax,0
//		jmp xr2
//xr1:	mov eax,1/////////// minus if one is minus
//xr2:	mov [esi]product.sign,eax//set sign		
		mov [esi]product.sign,eax//set sign	

		//if product==0 then sign must be plus
		//zero must have allways positive sign
		mov	ecx,[esi]product.IntLen//length	value
		mov eax,[esi]product.Data
		mov eax,[eax] //
		add eax,[esi]product.IntLen//if len==1 and data[0]==0
		cmp eax,1
		jne end002
		mov dword ptr[esi]product.sign,0//set positive sign	
end002:	
		mov eax,0//return 0;
		pop edx//restore
		pop ecx
		pop ebx
		pop esi
		pop edi
		mov esp,ebp
		pop ebp
		ret //	
	}
}

//############################################################################################
__declspec(naked)int Lint::Mul2(BU A2,Lint* product)
{
	__asm
	{	
//////////// before modify anything look at warnings ///////////////////////////////////
		push ebp
		push edi//save registers
		push esi
		push ebx
		push ecx
		push edx

		mov ebp,edx
		mov esi,[ecx]Lint.Data
		mov edi,[esp+28]////product  warning -------warning---------warning -------warning
		mov eax,[ecx]Lint.sign
		mov [edi]product.sign,eax//copy sign to product
		mov dword ptr[edi]product.IntLen,1
		mov	ecx, [ecx]Lint.IntLen
		add [edi]product.IntLen,ecx//prod.curlen= a1.curlen+1 
		mov edi,[edi]product.Data
		mov edx,0
		////////////mul ///////////////////////
arx:	mov eax,[esi]		
		mov ebx,edx// save temporarily edx
		mul	ebp//multiply 
		add eax,ebx// from previous mult
		adc edx,0//and the carry to edx
		mov [edi],eax//save the result 		
		add esi,4//increase pointers
		add edi,4
		loop arx
		mov [edi],edx
//////////// before modify anything look at warnings ///////////////////////////////////
		//prod->NormalizeLength();
		mov edi,[esp+28]////product  warning -------warning---------warning -------warning
		mov esi,[edi]product.Data//data	pointer
lpn1:	mov	ecx,[edi]product.IntLen//length	value	
		test dword ptr[esi+ecx*4-4],4294967295
		jnz end18//need no normalizing		
		dec [edi]product.IntLen//normalize length of prod
		test [edi]product.IntLen,4294967295//if curlen is above 0
		jnz lpn1
		inc [edi]product.IntLen//if 0 then curlen=1
end18:
		//zero product must have pos sign
		mov ecx,edi
		call ZeroSign//product		

		mov eax,0//return 0;
		pop edx//restore
		pop ecx
		pop ebx
		pop esi
		pop edi
		pop ebp
		ret 4//pop the last 4 bytes (1 argument)
	}

}

//############################################################################################
__declspec(naked)int Lint::Mul3(Lint* A2,Lint* product,BU digits)
{
	BU A2digit;//current digit for internal multiplication
	BU savecx;//save ecx
	BU saveesp;//for saving stack pointer esp	
	BU A1len;//length of A1
	PBU A2data;//pointer of A2->Data
	PBU digaddress;//for partial multiplication (product address )
	__asm
	{	
		push ebp
		mov ebp,esp
		sub esp,__LOCAL_SIZE//as  many bytes as local variables
		push edi//save registers
		push esi
		push ebx
		push ecx
		push edx	
		mov saveesp,esp//save esp

		//////////// multiplication /////////	
		mov ebx,this
		mov edx,A2
		mov esp,product		
		//product->ReallocDataZero(A2len+A1->IntLen)
		mov ecx,[edx]A2.IntLen
		add ecx,[ebx]Lint.IntLen//A1len+A2len
		//for partial mult
		cmp ecx,digits//for partial mult
		jc nxt02
		mov ecx,digits
nxt02:	mov [esp]product.IntLen,ecx
		mov edi,[esp]product.Data
        mov esi,edi
		mov eax,0
		cld//  cld is for inc and std is for dec
		rep stosd//fill product  data with zeros		
		mov edi,[ebx]Lint.Data
        //mov A1data,edi		
		mov eax,[ebx]Lint.IntLen
		mov A1len,eax
		mov eax,[edx]A2.Data
		mov A2data,eax
		mov eax,[eax]
		mov A2digit,eax		
		mov ecx,[edx]A2.IntLen
		//for patrial multiplication
		mov eax,digits
		dec eax//points to the first digit for value 1
		lea eax,[esi+eax*4]
		mov digaddress,eax
beg://mul/////////////////////mul	///////////////////////
		mov savecx,ecx//push ecx
		mov ecx,A1len
		mov	edx,0
		mov ebx,0//for index        
arx:	mov	eax,[edi+ebx*4]// mov eax, A1data
		mov	esp,edx// save temporarily edx
		mul	A2digit//multiply 
		add	eax,esp// from previous	mult
		adc	edx,0//and the carry to	edx		
		add [esi+ebx*4],eax//add the product to memory
		adc edx,0//and the carry to	edx
		inc ebx
		//for patrial multiplication
		lea eax,[esi+ebx*4-4]
		cmp eax,digaddress//for patrial multiplication(its not implemented right, but it is more fast)
		loopnz arx
		////////////////////////////////////////////////
		mov [esi+ebx*4],edx//write the remain msb to memory
		add esi,4
		add A2data,4
		mov eax,A2data
		mov eax,[eax]
		mov A2digit,eax//
		mov ecx,savecx//pop ecx
		//for patrial multiplication
		lea eax,[esi-4]
		cmp eax,digaddress
		loopnz beg

		//product->NormalizeLength();
		mov esi,product
		mov ebx,[esi]product.Data
nl1:	mov	ecx,[esi]product.IntLen//length	value	
		test dword ptr[ebx+ecx*4-4],4294967295
		jnz end31//need no normalizing		
		dec [esi]product.IntLen//normalize length of product
		test [esi]product.IntLen,4294967295//if curlen is above 0
		jnz nl1
		inc [esi]product.IntLen//if 0 then curlen=1
end31:
		// test sign
		mov ebx,this
		mov eax,0
		cmp eax,[ebx]Lint.sign// test sign
		rcl eax,1
		mov ebx,A2
		mov ecx,0
		cmp ecx,[ebx]Lint.sign// test sign
		rcl ecx,1		
		xor eax,ecx
//		cmp eax,ecx//theese all are the xor function (compatibility with 486)
//		jne xr1
//		mov eax,0
//		jmp xr2
//xr1:	mov eax,1/////////// minus if one is minus
//xr2:	mov [esi]product.sign,eax//set sign		
		mov [esi]product.sign,eax//set sign	

		//if product==0 then sign must be plus
		//zero must have allways positive sign
		mov	ecx,[esi]product.IntLen//length	value
		mov eax,[esi]product.Data
		mov eax,[eax] //
		add eax,[esi]product.IntLen//if len==1 and data[0]==0
		cmp eax,1
		jne end002
		mov dword ptr[esi]product.sign,0//set positive sign	
end002:	mov esp,saveesp//restore esp
		mov eax,0//return 0;
		pop edx//restore
		pop ecx
		pop ebx
		pop esi
		pop edi
		mov esp,ebp
		pop ebp
		ret //	
	}
}

//############################################################################################
__declspec(naked)void Lint::ZeroSign(void)//makes the sign positive if integer is zero
{
	__asm
	{	//zero must have allways positive sign
		mov eax,[ecx]this.Data
		mov eax,[eax] //
		add eax,[ecx]this.IntLen//if len==1 and data[0]==0
		cmp eax,1
		jne end000
		mov dword ptr[ecx]this.sign,0//set positive sign	
end000:	ret //		
	}
}
//############################################################################################
__declspec(naked)int Lint::Div1(Lint* A2,Lint* quot,Lint* rem)
{
	BU thissign,A2sign;//for temp save signs
	BU saveesp;// for saving esp purpose
	BU i;
	BU mp;//for normalizing purpose
	BU n;
	BU t;
	BU yt;//for storage yt
	BU yt1;//for storage y(t-1)
	PBU qit1;//for storage q(i-t-1)
	BU xi;//for storage Xi
	BU xi1;//for storage Xi-1
	BU xi2;//for storage Xi-2

	__asm
       { 
		push ebp
		mov ebp,esp
		sub esp,__LOCAL_SIZE//as  many bytes as local variables
		push edi//save registers
		push esi
		push ebx
		pushf//2 bytes
		push edx//dont touch it
		push ecx//dont touch it
		
		
		cld//  cld is for inc and std is for dec		
		//look first if this<A2 or equals
		call CompareU
		cmp eax,2
		jne eol
		//A2>this  -> rem=this quot=0
		//rem=this
		mov ebx,rem
		mov edx,[ecx]Lint.sign
		mov [ebx]rem.sign,edx//copy sign
		mov esi,[ecx]Lint.Data
		mov edi,[ebx]rem.Data
		mov ecx,[ecx]Lint.IntLen
		mov [ebx]rem.IntLen,ecx//copy curlen first
		rep movsd//*this=source;		
		//qouot=0
		mov ebx,quot
		mov dword ptr[ebx]quot.IntLen,1//copy curlen first
		mov dword ptr[ebx]quot.sign,0//copy sign
		mov eax,[ebx]quot.Data
		mov dword ptr[eax],0
		jmp endz
eol:	
		//make the signs both positive
		mov ebx,[ecx]Lint.sign
		mov thissign,ebx//save sign
		mov ebx,[edx]Lint.sign
		mov A2sign,ebx//save sign
		mov [ecx]Lint.sign,0//make positive
		mov [edx]Lint.sign,0//make positive			
		
		//if this>A2 
		cmp eax,1
		je bac//if this>A2 goto main algorythm
		//else if |this=A2| quot=1 (check sign) and rem=0
		//quot=1 without sign. Must checked after this
		mov ebx,quot
		mov dword ptr[ebx]quot.IntLen,1//copy curlen first
		mov eax,[ebx]quot.Data
		mov dword ptr[eax],1
		//rem=0
		mov ebx,rem
		mov dword ptr[ebx]rem.IntLen,1//copy curlen first
		mov eax,[ebx]rem.Data
		mov dword ptr[eax],0
		jmp end17 //go for sign checking
	////////// end looking first if this<A2 or equals///////

		
bac:	mov ecx,[edx]Lint.IntLen//for later use A2
		//if len=1 call the div2 function and return
		cmp ecx,2
		jae nodiv2
		//call the div2 function and return	
		push rem//push args in reverse order
		push quot
		mov eax,[edx]Lint.Data
		mov edx,[eax]
		mov ecx,[esp+8]//this
		call Div2
		jmp end17//to fix the sign
		
		//divy=*A2;
nodiv2:	mov divy.IntLen,ecx//copy curlen first
		mov eax,[edx]Lint.sign
		mov divy.sign,eax
		mov esi,[edx]Lint.Data
		mov edi,divy.Data
		rep movsd//divy=*A2;
		
		//rem=*this;
		mov esi,[esp]//this, else ecx
		mov edi,rem
		mov ecx,[esi]Lint.sign
		mov [edi]Lint.sign,ecx//copy sign
		mov ecx,[esi]Lint.IntLen
		mov [edi]Lint.IntLen,ecx//copy curlen first
		mov esi,[esi]Lint.Data
		mov edi,[edi]Lint.Data
		rep movsd//rem=*this;		

	   //normalize integers x*=mp divy*=mp	and set the values of n and t					
		mov mp,0
		mov esi,divy.Data//data	pointer
		mov	ebx, divy.IntLen//length value
		bt dword ptr[esi+ebx*4-4],31
		jc end111//compare to 1^32 / 2
		//////// find first the normalization(mp) factor/////
		bsr eax,dword ptr[esi+ebx*4-4]//scan for the position of the msb
		mov edx,31
		sub edx,eax//find how many positions must be shift left
		mov mp,edx//for later use

		//MULTIPLY x (rem)
		mov ecx,rem
		call Sl
		//multiply divy
		lea ecx,divy
		call Sl

end111:	
		//set the values of n and t
		mov edi,rem
		mov eax,[edi]Lint.IntLen
		dec eax
		mov n,eax//n= x.IntLen-1
		mov eax,divy.IntLen
		dec eax
		mov t,eax//t= divy.IntLen-1;

		//quot.ReallocDataZero(n-t+1);
		mov ecx,n
		sub ecx,t
		add ecx,1//n-t+1 digits		
		mov ebx,quot
		lea edx,[ebx]quot.IntLen
		mov ebx,[ebx]quot.Data
		mov [edx],ecx//
lp12:	mov dword ptr[ebx+ecx*4-4],0
		loop lp12

		//while(x >=ybnt) x= x-ybnt
		//ybnt=divy; and shift left n-t steps simultaneusly //ybnt=divy*b^(n-t)
		mov edi,divy.Data
		mov esi,ybnt.DataBase
		mov ybnt.Data,esi//initialize just in case
		mov ebx,n
		sub ebx,t
		lea edx,[esi+ebx*4]//shift left n-t steps
		mov ecx,divy.IntLen//
		mov ybnt.IntLen,ecx
		add ybnt.IntLen,ebx//curlen= divy.IntLen+n-t
cpy0:	mov eax,[edi+ecx*4-4]
		mov[edx+ecx*4-4],eax// ybnt=divy;
		loop cpy0
		//zeroize rest dwords of ybnt from n-t-1 to 0
		mov ecx,ebx		
		jecxz whl1//if n-t==0
set0:	mov dword ptr[esi+ecx*4-4],0
		loop set0
whl1:		
		//while ybnt<=x quot->Data[n-t]+=1;x-=ybnt
		lea edx,ybnt
		mov ecx,rem
whl12:	call CompareU
		cmp eax,2
		jnc blb1//if ybnt>x goto the next block of code
		//else quot->Data[n-t]+=1;x-=ybnt;repeat until ybnt>x
		mov ebx,n
		sub ebx,t
		mov eax,quot
		mov eax,[eax]quot.Data
		inc dword ptr[eax+ebx*4]// quot->Data[n-t]+=1		
		//x-=ybnt;		
		call Sub2
		jmp whl12
blb1:
	
///////////////////////////begin of big loop//////for (i=n;i>=t+1;i--)
		mov	ecx,divy.Data
		mov ebx,t		
		mov eax,dword ptr[ecx+ebx*4]// divy.Data[t]
		mov yt,eax
		mov eax,dword ptr[ecx+ebx*4-4]// divy.Data[t-1]
		mov yt1,eax
		mov eax,n
		mov i,eax
		inc ebx
		cmp eax,ebx
		jnae endbglp
biglpp:	//for (i=n;i>=t+1;i--)		
		
		mov	edi,i
		//move to variables the xi,xi1,xi2,*qit1 (to increase speed)
		mov ecx,edi
		sub ecx,t
		mov eax,quot
		mov eax,[eax]quot.Data
		lea esi,[eax+ecx*4-4]//find *qit1 (q(i-t-1))
		mov qit1,esi
		mov ecx,rem
		mov	ebx,[ecx]Lint.Data
		mov ecx,[ecx]Lint.IntLen
		mov eax,[ebx+edi*4]
		mov	xi,eax
		mov eax,[ebx+edi*4-4]
		mov	xi1,eax
		mov eax,[ebx+edi*4-8]
		mov	xi2,eax
		// if i...>= curlen then xi...=0
		cmp edi,ecx
		jb jb1
		mov xi,0//x.Data[i]=0
		dec ecx
		cmp edi,ecx
		jb jb1
		mov xi1,0//x.Data[i-1]=0
		dec ecx
		cmp edi,ecx
		jb jb1
		mov xi2,0//x.Data[i-2]=0		
jb1:
		//if (x.Data[i]==divy.Data[t]) then quot->Data[i-t-1]=4294967295 else	load quot->Data[i-t-1] with (xib+xi-1)/yt
		mov edx,xi
		cmp	edx,yt//compare with divy.Data[t]
		jne	neq
		mov	dword ptr[esi],4294967295//load	quot.Data[i-t-1] with ffffffff
		jmp	nxt		
neq:	//perform xi:xi-1 /	yt
		mov	eax,xi1//load	x.Data[i-1]
		div	dword ptr[yt]//perform xi-1 /yt
		mov	dword ptr[esi],eax//load	quot.Data[i-t-1] with result
nxt:			
		/////////////////////////while (qt1*(ytb+yt1)>xib2+xi1b+xi2) q(i-t-1) -- ///////////
		///// now multiply (yt:yt-1) by q(i-t-1) and save it to ltmp
		mov	eax,yt1//take first dword yt-1
		mul	dword ptr[esi]//multiply 
		mov	ebx,eax//save	the	result 
		mov	ecx,edx// save temporarily edx
		mov	eax,yt//take second dword yt		
		mul	dword ptr[esi]//multiply 
		add	eax,ecx// from previous	mult
		adc	edx,0//and the carry to	edx
		//if ltmp>rtmp set carry flag
		mov	ecx,xi2//xi-2
		sub	ecx,ebx
		mov	ecx,xi1//xi-1
		sbb	ecx,eax
		mov	ecx,xi//xi
		sbb	ecx,edx
		jnc	endw
		//q(i-t-1)= q(i-t-1)-1
		dec	dword ptr[esi]//q(i-t-1)= q(i-t-1)-1
		jmp	nxt
endw:	//////////end of while (qt1*(ytb+yt1)>xib2+xi1b+xi2) q(i-t-1) -- ///////////
		
		//x=x- quot.Data[i-t-1]*yb^(i-t-1)///////////////////////////
		mov	esi,divtmp.Data
		mov	ebx,ybnt.Data
		mov eax,n
		sub	eax,i
		add eax,1
		lea	ebx,[ebx+eax*4]//shift left	i-t-1 steps divy.Data
		mov ecx,ybnt.IntLen
		sub ecx,eax
		mov divtmp.IntLen,ecx//new IntLen for divtmp
		inc divtmp.IntLen//plus one for mult
		mov	edx,0
		mov edi,qit1
		mov saveesp,esp
		mov esp,ebx// ybnt.Data		
		////////////mul	///////////////////////
arxm:	mov	eax,[esp]		
		mov	ebx,edx// save temporarily edx
		mul dword ptr[edi]//multiply 
		add	eax,ebx// from previous	mult
		adc	edx,0//and the carry to	edx
		mov	[esi],eax//save	the	result 
		add	esi,4//increase	pointer
		add esp,4
		loop arxm
		mov [esi],edx//save	the	result 
		mov esp,saveesp
		//divtmp.NormalizeLength();
		mov	esi,divtmp.Data//data	pointer
nrm:	mov	ecx,divtmp.IntLen//length	value	
		test dword ptr[esi+ecx*4-4],4294967295
		jnz	end15//need	no normalizing		
		sub	divtmp.IntLen,1//normalize length of divtmp		
		jnz	nrm//if len is above 0
		inc	divtmp.IntLen//if 0 then curlen=1
end15:	//x-=divtmp
		lea edx,divtmp
		mov ecx,rem//pass "this" pointer
		call Sub2
		////////////end of x=x- quot.Data[i-t-1]*yb^(i-t-1)///////////////////////////
				
		//if(x>0)jump to	the	end25////////////////////////////////////
		test [ecx]Lint.sign,4294967295
		jz end25
		////////////x=x+ yb^(i-t-1)///////////////////////////
		mov	ebx,ybnt.Data
		mov eax,n
		sub	eax,i
		add eax,1
		lea	ebx,[ebx+eax*4]//shift left	i-t-1 steps divy.Data
		mov esi,ybnt.Data//save temporarily
		mov edi,ybnt.IntLen//save temporarily
		mov ybnt.Data,ebx
		sub ybnt.IntLen,eax
		//x+=ybnt;
		lea edx,ybnt
		mov ecx,rem
		call Add2
		mov ybnt.IntLen,edi//restore
		mov ybnt.Data,esi//restore
		//quot.Data[i-t-1]--;
		mov	eax,qit1
		dec	dword ptr[eax]//quot.Data[i-t-1]--;
end25:	////////////end of x=x+ quot.Data[i-t-1]*yb^(i-t-1)//////////////
		// end if(x>0)jump to the end25////////////////////////////////////		
		
		dec dword ptr[i]//for (i=n;i>=t+1;i--) end loop
		mov eax,i
		mov ebx,t
		cmp ebx,eax
		jnae biglpp
///////////////////////////end of big loop//////for (i=n;i>=t+1;i--)
endbglp:			
		//shift right rem, mp bits
		mov ecx,rem
		mov edx,mp
		call Sr
		
	//quot->NormalizeLength();
		mov esi,quot
		mov ebx,[esi]quot.Data
clp17:	mov	eax,[esi]quot.IntLen//length	value	
		mov eax,[ebx+eax*4-4]
		test eax,4294967295
		jnz end17//need no normalizing		
		lea eax,[esi]quot.IntLen
		dec dword ptr[eax]//normalize length of divy
		test dword ptr[eax],4294967295//if curlen is above 0
		jnz clp17
		inc dword ptr[eax]//if 0 then curlen=1
end17:

		////rem.sign=x.sign  quot sign depends of rem and A2
		//// test sign and restore original signs
		mov ecx,rem
		mov eax,thissign
		mov [ecx]Lint.sign,eax//rem.sign=this.sign
		mov ecx,[esp]//this
		mov [ecx]Lint.sign,eax//restore this.sign
		mov ecx,[esp+4]//A2
		mov eax,A2sign
		mov [ecx]Lint.sign,eax//restore A2.sign
		mov esi,quot
		
		mov eax,0
		cmp eax,thissign// test sign (this)
		rcl eax,1
		mov ecx,0
		cmp ecx,A2sign// test sign
		rcl ecx,1
		//cmp eax,ecx//theese all are the xor function (compatibility with 486)
//		jne xr1
//		mov eax,0
//		jmp xr2
//xr1:	mov eax,1/////////// minus if one is minus
//xr2:	mov [esi]quot.sign,eax//set sign
		xor eax,ecx
		mov [esi]quot.sign,eax//set sign
		
		//zero must have allways positive sign
		mov ecx,quot//quot
		call ZeroSign
		mov ecx,rem
		call ZeroSign//rem		

endz:	mov eax,0//return 0;		
		pop ecx
		pop edx//restore
		popf
		pop ebx
		pop esi
		pop edi	
		mov esp,ebp
		pop ebp
		ret 8
	}
}
//############################################################################################
__declspec(naked)int Lint::Div2(BU A2,Lint* quot,Lint* rem)//
{	
	__asm
    {
		push edi//save all registers
		push esi
		push ebx
		push ecx
		push edx
		
		mov ebx,[esp+24]//quot
		mov edx,[esp+28]//rem
		//fix the signs
		mov eax,[ecx]Lint.sign//this.sign
		mov [ebx]quot.sign,eax//quot.sign=this.sign
		mov [edx]rem.sign,eax//rem.sign=quot.sign		
		//initialize vars and pointers
		mov esi,[ebx]quot.Data
		mov edi,[ecx]Lint.Data
		mov ecx,[ecx]Lint.IntLen
		mov [ebx]quot.IntLen,ecx//set new curlen//reallocdatazero
		mov edx,0//
		//division
ddd:	mov eax,[edi+ecx*4-4]
		div dword ptr[esp]//edx or A2
		mov [esi+ecx*4-4],eax
		loop ddd		
		//rem =edx
		mov ecx, [esp+28]//rem
		mov dword ptr[ecx]rem.IntLen,1//set rem curlen=1( realloc)
		mov eax,[ecx]rem.Data
		mov [eax],edx//rem =edx
		//normalize quot
		mov eax,[ebx]quot.IntLen// quot length 
		test dword ptr[esi+eax*4-4],4294967295//
		jnz endff//need no normalizing
		sub [ebx]quot.IntLen,1//normalize length of prod
		jnz endff//if len is above 0
		inc [ebx]quot.IntLen//if 0 then curlen=1		
		
endff:	mov eax,0//return 0;
		pop edx//restore
		pop ecx
		pop ebx
		pop esi
		pop edi
		ret 8//		
	}	
}
//############################################################################################
__declspec(naked)int Lint::Add1(Lint*  A1, Lint* A2, Lint* sum)
{	
	PBU	mindata;
	PBU	maxdata;
	BU minlen;//minimum	length
	BU maxlen;//maximum	length
	
	__asm
	{
		push ebp
		mov ebp,esp
		sub esp,__LOCAL_SIZE//as  many bytes as local variables
		push edi//save registers
		push esi
		push ebx
		push ecx
		push edx
	}
	
	if (A1->sign==0)
	{
		if (A2->sign>0)
		{
			A2->sign=0;
			Sub1(A1,A2,sum);
			A2->sign=1;//restore
			goto endadd;
		}
	}
	else//A1.sign>0
	{
		if (A2->sign==0)
		{
			A1->sign=0;
			Sub1(A2,A1,sum);
			A1->sign=1;//restore
			goto endadd;
		}
		else 
		{
			A2->sign=0;
			A1->sign=0;
			Add1(A1,A2,sum);
			sum->sign=1;//negative
			A2->sign=1;//restore
			A1->sign=1;//restore			
			goto endadd;
		}
	}

	if (A1->IntLen>A2->IntLen)
	{
		minlen=A2->IntLen;
		maxlen=A1->IntLen;
		mindata=A2->Data;
		maxdata=A1->Data;		
	}
	else 
	{
		minlen=A1->IntLen;
		maxlen=A2->IntLen;
		mindata=A1->Data;
		maxdata=A2->Data;
	}
	__asm
	{	
		//sum->ReallocDataZero(maxlen+1)			
		mov ebx,sum
		mov ecx,maxlen
		add ecx,1//is the length of the sum
		mov edi,[ebx]sum.Data
		mov [ebx]sum.IntLen,ecx////set new curlen
		mov eax,0// fill memory and return 0;
		cld//  cld is for inc and std is for dec
		rep stosd//fill data with zeros
		//here begins the addition
		mov	edx,[ebx]sum.Data// from now up to the end edx points to sum.Data	
		mov	ecx, minlen	
		mov	esi, mindata
		mov	ebx,maxdata
		mov edi,0//for index purpose
		clc//clear carry
		////////////add	min	integer	first///////////////////////
arx:	mov	eax,[esi+edi*4]//mov eax,mindata
		adc	eax,[ebx+edi*4]//add
		mov	[edx+edi*4],eax//mov sum,eax
		inc edi//increase index
		loop arx
		///////////////proccess	the	rest bytes////////////////////
		mov	ecx, maxlen
		pushf//save	carry etc
		sub	ecx, minlen			
		popf//load carry		
		jecxz lastact
		jnc restwoc	// if not carry just copy the rest bytes(for fastener code)
		
rest:	mov	eax,[ebx+edi*4]//mov ebx,maxdata
		adc	eax,0//add
		mov	[edx+edi*4],eax//mov sum,eax			
		inc edi
		loop rest
		jmp lastact
//////////////////////////////////////////////////////////////
restwoc:mov eax,[ebx+edi*4]
		mov [edx+edi*4],eax
		inc edi
		loop restwoc
		clc
//////////////////////////////////////////////////////////////////
lastact:mov eax,0//last action  add carry(if exists) to sum
		adc	eax,0//add only carry
		mov [edx+edi*4],eax// save carry to the msb of sumdata
		// here ends the addition
	}		
	sum->NormalizeLength();
endadd:
	__asm
	{
		mov eax,0
		pop edx//restore
		pop ecx
		pop ebx
		pop esi
		pop edi	
		mov esp,ebp
		pop ebp
		ret //
	}
}
//############################################################################################
__declspec(naked)int Lint::Add2(Lint* A2)
{	
	__asm
	{
		push ebp//save registers
		push edi
		push esi
		push ebx
		push ecx
		push edx
	
		mov ebp,ecx
		mov edi,edx

		mov ebx,[edi]Lint.sign
		mov ecx,[ebp]Lint.sign
		jecxz cnt03//if this>0 jump
		mov ecx,ebx
		jecxz cnt04
		//both negatives
		mov [ebp]Lint.sign,0//make positive temporarily
		mov [edi]Lint.sign,0//make positive temporarily
		mov ecx,ebp
		call Add2
		mov [edi]Lint.sign,1//restore
		mov [ebp]Lint.sign,1//make it negative
		jmp end39

cnt04:	//this negative and A2 positive
		mov [ebp]Lint.sign,0//make positive temporarily
		mov ecx,ebp
		call Sub2
		not [ebp]Lint.sign //reverse the sign
		and [ebp]Lint.sign,1
		jmp end39

cnt03:	mov ecx,ebx
		jecxz cnt05
		//this positive and A2 negative
		mov [edi]Lint.sign,0//make positive temporarily
		mov ecx,ebp
		call Sub2
		mov [edi]Lint.sign,1//restore
		jmp end39
cnt05:	//both positives
	

		//set the len difference and realloc data
		mov ecx,[edi]Lint.IntLen
		mov edx,[ebp]Lint.IntLen
		sub edx,[edi]Lint.IntLen//save to edx the diference
		jnc cnt06//if this len<A2len patch this with zeroes
		mov ecx,ebp
		mov edx,[edi]Lint.IntLen
		call ReallocData
		mov ecx,edx
		mov edx,0
		
cnt06:	//reallocate one dword more (addition)
		mov eax,[ebp]Lint.Data
		mov ebx,[ebp]Lint.IntLen
		mov dword ptr[eax+ebx*4],0//zeroize dword
		inc [ebp]Lint.IntLen//new IntLen
	
		//here begins the addition
		mov	esi,[ebp]Lint.Data
		mov	edi,[edi]Lint.Data
		mov ebx,0//for index purpose
		clc//clear carry
		////////////add	min	integer	first///////////////////////
arx:	mov	eax,[edi+ebx*4]//mov eax,A2data
		adc	[esi+ebx*4],eax//add
		inc ebx//increase index
		loop arx
		///////////////proccess	the	rest bytes////////////////////
		mov ecx,edx
		jecxz lastact//if minlen==maxlen jump to lastact		
		//propagate the carry to the next dwords
rest:	adc	dword ptr[esi+ebx*4],0//add
		jnc lastact//if we have not a carry we dont need to continue the execution
		inc ebx//increase index
		loop rest
//////////////////////////////////////////////////////////////////
lastact://add carry(if exists) to sum
		adc	[esi+ebx*4],0//add only carry
		// here ends the addition

		//this->NormalizeLength();
		mov edx,[ebp]Lint.Data//
nrml1:	mov ecx,[ebp]Lint.IntLen
		test dword ptr[edx+ecx*4-4],4294967295//
		jnz end39//need no normalizing
		cmp dword ptr[ebp]Lint.IntLen,2//not normalize if first dword=0
		jnae end39//not normalize if first dword=0
		dec dword ptr[ebp]Lint.IntLen//normalize length of this
		jmp nrml1
end39:
		mov eax,0
		pop edx//restore
		pop ecx
		pop ebx
		pop esi
		pop edi	
		pop ebp
		ret //
	}
}
//############################################################################################
__declspec(naked)int Lint::Sub1(Lint* A1, Lint* A2, Lint*  result)
{
	BU eql;
	BU minlen;
	BU maxlen;

	__asm
	{
		push ebp
		mov ebp,esp
		sub esp,__LOCAL_SIZE//as  many bytes as local variables
		push edi//save registers
		push esi
		push ebx
		push ecx
		push edx

	}
	
	if (A1->sign==0)
	{
		if (A2->sign>0)
		{
			A2->sign=0;
			Add1(A1,A2,result);
			result->sign=0;//positive
			A2->sign=1;//restore
			goto endsub;
		}
	}
	else//A1.sign>0
	{
		if (A2->sign==0)
		{
			A1->sign=0;
			Add1(A1,A2,result);
			result->sign=1;//negative
			A1->sign=1;//restore
			goto endsub;
		}
		else 
		{
			A2->sign=0;
			A1->sign=0;
			Sub1(A2,A1,result);
			A2->sign=1;//restore
			A1->sign=1;//restore			
			goto endsub;
		}
	}

	eql=A1->CompareU(A2);//absolute values comparison
	if (eql==2)//|A2>A1|
	{
		Sub1(A2,A1,result);//we want a2 biger than a1
		if(result->sign==0) result->sign=1;else result->sign=0;
		goto endsub;
	}
	if (eql==0)//|A2=A1|
		(*result)=0;
	__asm
	{		
		mov esi,A1
		mov edi,A2
		mov eax,[edi]A2.IntLen
		mov minlen,eax//minlen=A2->IntLen;//minimum	length
		mov ecx,[esi]A1.IntLen
		mov maxlen,ecx//maxlen=A1->IntLen;//maximum	length		
		//result->ReallocDataZero(maxlen)			
		mov ebx,result
		lea edx,[ebx]result.IntLen
		mov edi,[ebx]result.Data
		mov [edx],ecx//
		mov eax,0// fill memory and return 0;
		cld
		rep stosd
		
		mov	ecx, minlen	
		clc//clear carry
		mov edi,0//index
		mov ebx,A2		
		mov	ebx,[ebx]A2.Data//mov	ebx,mindata
		mov esi,A1		
		mov	esi,[esi]A1.Data//mov	esi,maxdata
		mov edx,result		
		mov	edx,[edx]result.Data
		////////////subb	min	integer	first///////////////////////
arx:	mov eax,[esi+edi*4]//mov eax,maxdata
		sbb	eax,[ebx+edi*4]//sub
		mov	[edx+edi*4],eax//mov sum,eax
		inc edi
		loop arx
		///////////////proccess	the	rest bytes////////////////////
		mov	ecx,maxlen
		pushf//save	carry etc
		sub	ecx, minlen			
		popf//load carry
		jecxz lastact

		jnc restwoc	// if not carry just copy the rest bytes(for fastener code)
				
rest:	mov	eax,[esi+edi*4]//mov ebx,maxdata
		sbb	eax,0//sub
		mov	[edx+edi*4],eax//mov sum,eax
		inc edi
		loop rest
		jmp lastact
/////////////////////////////////////////////////////////////
restwoc:	
		mov	eax,[esi+edi*4]//mov ebx,maxdata
		mov [edx+edi*4],eax
		inc edi
		loop restwoc
		clc
/////////////////////////////////////////////////////////////////
lastact://last action  add carry(if exists) to sum
		mov edi,result
		lea ebx,[edi]result.sign
		setc [ebx]//set or not cf
		//result->NormalizeLength();
		mov esi,[edi]result.Data//
nrml1:	mov ecx,[edi]result.IntLen
		test dword ptr[esi+ecx*4-4],4294967295//
		jnz end39//need no normalizing
		cmp dword ptr[edi]result.IntLen,2//not normalize if first dword=0
		jnae end39//not normalize if first dword=0
		dec dword ptr[edi]result.IntLen//normalize length of this
		jmp nrml1
end39:	
	}	


endsub:
	__asm
	{
		mov eax,0
		pop edx//restore
		pop ecx
		pop ebx
		pop esi
		pop edi	
		mov esp,ebp
		pop ebp
		ret //
	}
}
//############################################################################################
__declspec(naked)int Lint::Sub2(Lint* A2)
{

	
	__asm
	{
		push ebp
		push edi//save registers
		push esi
		push ebx
		push ecx
		push edx

	
		mov ebp,ecx
		mov edi,edx

		mov ebx,[edi]Lint.sign
		mov ecx,[ebp]Lint.sign
		jecxz cnt03//if this>0 jump
		mov ecx,ebx
		jecxz cnt04
		//both negatives
		mov [ebp]Lint.sign,0//make positive temporarily
		mov [edi]Lint.sign,0//make positive temporarily
		mov ecx,ebp
		call Sub2
		mov [edi]Lint.sign,1//make it negative
		not [ebp]Lint.sign //reverse the sign
		and [ebp]Lint.sign,1
		jmp end39

cnt04:	//this negative and A2 positive
		mov [ebp]Lint.sign,0//make positive temporarily
		//mov edx,edi
		mov ecx,ebp
		call Add2
		mov [ebp]Lint.sign,1//make it negative
		jmp end39

cnt03:	mov ecx,ebx
		jecxz cnt05
		//this positive and A2 negative
		mov [edi]Lint.sign,0//make positive temporarily
		mov ecx,ebp
		call Add2
		mov [edi]Lint.sign,1//make it negative
		jmp end39
cnt05:	//both positives

		mov ecx,[edi]Lint.IntLen
		mov edx,[ebp]Lint.IntLen
		sub edx,[edi]Lint.IntLen//save to edx the diference
		jnc cnt//if this len<A2len patch this with zeroes
		mov ecx,ebp
		mov edx,[edi]Lint.IntLen
		call ReallocData
		mov ecx,edx
		mov edx,0

cnt:	mov	edi,[edi]A2.Data//mov	ebx,mindata
		mov	esi,[ebp]Lint.Data//mov	esi,maxdata
		mov ebx,0//index
		clc//clear carry
		////////////subb	min	integer	first///////////////////////
arx:	mov eax,[edi+ebx*4]//mov eax,mindata
		sbb	[esi+ebx*4],eax//sub
		inc ebx
		loop arx
		///////////////proccess	the	rest bytes////////////////////
		mov ecx,edx
		jecxz lastact//if difference=0 goto lastact				
rest:	sbb	dword ptr[esi+ebx*4],0//sub
		jnc lastact//if we have not a carry we dont need to continue the execution
		inc ebx
		loop rest		
/////////////////////////////////////////////////////////////////
lastact://last action  add carry(if exists) to sum
		mov [ebp]Lint.sign,0
		jnc cnt1

		//negate the integer (two's complement)
		mov [ebp]Lint.sign,1//set sign
		mov ecx,[ebp]Lint.IntLen
		mov edx,[ebp]Lint.Data
		not dword ptr[edx+0]//negate first byte
		add dword ptr[edx+0],1
		mov ebx,1
		dec ecx
		jecxz cnt1//if len=1 then we do not need to negate
acmp1:	not dword ptr[edx+ebx*4]//
		adc dword ptr[edx+ebx*4],0//propagate the carry
		inc ebx
		loop acmp1

cnt1:	//this->NormalizeLength();
		mov edx,[ebp]Lint.Data//
nrml1:	mov ecx,[ebp]Lint.IntLen
		test dword ptr[edx+ecx*4-4],4294967295//
		jnz end39//need no normalizing
		cmp dword ptr[ebp]Lint.IntLen,2//not normalize if first dword=0
		jnae end39//not normalize if first dword=0
		dec dword ptr[ebp]Lint.IntLen//normalize length of this
		jmp nrml1
end39:	
	
		mov eax,0
		pop edx//restore
		pop ecx
		pop ebx
		pop esi
		pop edi	
		//mov esp,ebp
		pop ebp
		ret //
	}
}
//############################################################################################
__declspec(naked)int Lint::Sr(BU t)//shift right 
{
	__asm
	{
		push edi//save registers
		push esi
		push ecx

		cmp edx,1
		jne zzz//if shift more than one bit

		mov eax,ecx
		mov edi,[eax]this.Data
		mov ecx,[eax]this.IntLen
		lea esi,[edi+ecx*4-4]//store for later use
		clc//clear carry flag
sr1:	rcr dword ptr[edi+ecx*4-4],1//rotate through cary
		loop sr1
		//normalize length(if we have leading zero)
		mov ecx,[esi]
		cmp ecx,0
		jne srend
		dec dword ptr[eax]this.IntLen//normalize
		jnz srend
		inc dword ptr[eax]this.IntLen//min curlen is 1
		jmp srend



//shift >1 bit right. slower than one bit shift		
zzz:	cmp edx,0
		je srend//if 0 times shift then return
		push edx
		xchg edx,ecx//both passed from caller		
		mov edi,[edx]Lint.Data
		mov eax,[edx]Lint.IntLen
		mov dword ptr[edi+eax*4],0
		mov esi,1
sr2:	mov eax,[edi+esi*4]
		shrd [edi+esi*4-4],eax,cl
		inc esi
		cmp [edx]Lint.IntLen,esi
		jnc sr2		
		//if msb is zero
		test dword ptr[edi+esi*4-8],4294967295
		jnz es3		
		dec [edx]this.IntLen//nomalize
		test [edx]this.IntLen,4294967295
		jnz es3
		inc [edx]this.IntLen
es3:	pop edx

srend:	
		mov eax,0
		pop ecx//restore
		pop esi
		pop edi	
		ret //
	}
}
//############################################################################################
__declspec(naked)int Lint::Sl(BU t)//shift left t bits //x=x*2
{
	__asm
	{
		push edi//save registers
		push esi
		push ecx

		cmp edx,1
		jne zzz//if shift more than one bits

		//shift one bit left. much faster than more bits shift		
		mov eax,ecx//passed from caller
		mov edi,[eax]this.Data
		mov ecx,[eax]this.IntLen
		clc//clear carry flag
		mov esi,0
sl1:	rcl dword ptr[edi+esi*4],1//rotate through cary
		inc esi
		loop sl1
		jnc endsl
		mov dword ptr[edi+esi*4],1//if we have cary
		inc [eax]this.IntLen
endsl:	jmp es1

		//shift >1 bit left. slower than one bit shift		
zzz:	cmp edx,0
		je es1//if 0 times shift then return
		push edx
		xchg edx,ecx//both passed from caller		
		mov edi,[edx]this.Data
		mov esi,[edx]this.IntLen
		mov dword ptr[edi+esi*4],0
sl2:	mov eax,[edi+esi*4-4]
		shld [edi+esi*4],eax,cl
		sub esi,1
		jnz sl2
		mov eax,0//shift the last dword
		shld [edi+esi*4],eax,cl
		//if msb is zero
		mov esi,[edx]this.IntLen
		mov eax,[edi+esi*4]
		cmp eax,0
		jz es2
		inc [edx]this.IntLen
es2:	pop edx
		
es1:	mov eax,0
		pop ecx//restore
		pop esi
		pop edi	
		ret //
	}
}
//############################################################################################
__declspec(naked)int Lint::Sqr1(void)
{
	BU A2digit;//current digit for internal multiplication
	BU savecx;//save ecx
	BU saveesp;//for saving stack pointer esp	
	BU A1len;//length of A1
	PBU A2data;//pointer of A2->Data
	Lint* li;
	__asm
	{	
		push ebp
		mov ebp,esp
		sub esp,__LOCAL_SIZE//as  many bytes as local variables
		push ecx
		push edx
		push edi//save registers
		push esi
		push ebx
		mov saveesp,esp//save esp
		mov li,ecx//passed from caller

		//////////// multiplication /////////	
		mov ebx,li
		//product->ReallocData(A2len+A1->IntLen)
		mov ecx,[ebx]li.IntLen
		shl ecx,1//A1len*2
		mov sqr.IntLen,ecx
		mov edi,sqr.Data
        mov esi,edi
		mov eax,0
		cld//  cld is for inc and std is for dec
		rep stosd//fill product  data with zeros		
		
		mov edi,[ebx]li.Data
		mov A2data,edi
		mov eax,[edi]
		mov A2digit,eax			
		mov ecx,[ebx]li.IntLen
		mov A1len,ecx		

beg://mul/////////////////////mul	///////////////////////
		mov savecx,ecx//push ecx
		mov ecx,A1len
		mov	edx,0
		mov ebx,0//for index        
arx:	mov	eax,[edi+ebx*4]// mov eax, A1data
		mov	esp,edx// save temporarily edx
		mul	A2digit//multiply 
		add	eax,esp// from previous	mult
		adc	edx,0//and the carry to	edx		
		add [esi+ebx*4],eax//add the product to memory
		adc edx,0//and the carry to	edx
		inc ebx
		loop arx
		////////////////////////////////////////////////
		mov [esi+ebx*4],edx//write the remain msb to memory
		add esi,4		
		add A2data,4
		mov eax,A2data
		mov eax,[eax]
		mov A2digit,eax//
		mov ecx,savecx//pop ecx
		loop beg

		//product->NormalizeLength();
		mov edx,li
		mov ebx,sqr.Data
		mov	ecx,sqr.IntLen//length	value
		mov [edx]li.IntLen,ecx//copy to"this"
		test dword ptr[ebx+ecx*4-4],4294967295
		jnz end33//need no normalizing		
		dec [edx]li.IntLen//normalize length of product
		test [edx]li.IntLen,4294967295//if curlen is above 0
		jnz end33
		inc [edx]li.IntLen//if 0 then curlen=1
end33:

		mov ecx,[edx]li.IntLen//
		mov esi,sqr.Data
		mov edi,[edx]li.Data
		cld//  cld is for inc and std is for dec
		rep movsd//divy=*A2;

		mov esp,saveesp//restore esp
		mov eax,0//return 0;
		pop ebx//restore
		pop esi
		pop edi
		pop edx
		pop ecx
		mov esp,ebp
		pop ebp
		ret //	
	}
}

//############################################################################################
int Lint::Gcd(Lint* A1,Lint* result)//binary gcd algorythm
{
	*result=*A1;//copy the integer
	gcd=*this;//copy the integer
	//gen2=1;
	BU sl=0;//how many bits are both shifted right 
	
	__asm
	{

		lea ebx,gcd
		mov esi,result
		lea edi,gcd1

		//while both integers are even
sa7:	mov eax,[ebx]Lint.Data
		mov eax,[eax+0]
		and eax,1//last bit
		mov edx,[esi]Lint.Data
		mov edx,[edx+0]
		and edx,1//last bit
		add eax,edx
		test eax,3
		jnz sa6
		mov ecx,ebx
		mov edx,1
		call Sr
		mov ecx,esi
		call Sr
		inc sl
		jmp sa7

		//while(*A1!=0)///////////////////////////////////////////////
sa6:	mov eax,[ebx]Lint.Data
		mov eax,[eax+0]
		add eax,[ebx]Lint.IntLen
		cmp eax,1
		je sa5
		//while A1 is even : A1->shift right
		mov ecx,ebx
		mov edx,1
sa0:	mov eax,[ecx]Lint.Data
		test [eax],1
		jnz sa1//if A1 is not even jump
		call Sr
		jmp sa0
		//while A2 is even : A2->shift right
sa1:	mov ecx,esi
sa2:	mov eax,[ecx]Lint.Data
		test [eax],1
		jnz sa3//if A2 is not even jump
		call Sr
		jmp sa2
sa3:	// A3 = A2 - A1
		push edi
		push esi
		push ebx
		push this
		call Sub1
		add esp,16
		//A3=|A3|/2
		mov ecx,edi
		mov edx,1
		call Sr
		mov eax,[ecx]Lint.sign//copy sign before make zero
		mov [ecx]Lint.sign,0

		test eax,4294967295
		jnz sa4// if not A1>=A2 jump
		xchg ebx,edi
		jmp sa6
sa4:	xchg esi,edi

		jmp sa6
		// end of while(*A1!=0)///////////////////////////////////////////////

sa5:	//if(result==esi) result->;
		mov eax,result
		cmp eax,esi
		jne sa8
		mov edx,sl
		mov ecx,result
		call Sl//shift result sl bits left		
		jmp sa9
sa8:	//else *result=esi .Sl(sl)
		mov edx,sl
		mov ecx,esi
		call Sl//shift result sl bits left			
		//copy (Lint)esi to result
		mov edx,esi
		mov ecx,result
		mov ebx,[edx]Lint.IntLen
		mov [ecx]Lint.IntLen,ebx//copy curlen
		mov eax,[edx]Lint.sign
		mov [ecx]Lint.sign,eax//copy sign
		mov esi,[edx]Lint.Data
		mov edi,[ecx]Lint.Data
		mov ecx,ebx
		cld//  cld is for inc and std is for dec
		rep movsd//*this=source;
		

sa9:
	}

	return 0;
}
//############################################################################################
__declspec(naked)int Lint::Egcd(Lint* A1,Lint* result)//efklidis gcd algorythm
{
	 //while(A2!=0){gcd3=this%A2;this=A2;A2=gcd3;}	
	__asm
	{	push ebp
		mov ebp,esp
		push edi//save registers
		push esi
		push ecx
		push edx
		push ebx
		pushf

		//result=this
		cld//  cld is for inc and std is for dec
		mov eax,this
		mov ebx,result
		mov ecx,[eax]this.IntLen
		mov [ebx]result.IntLen,ecx//copy Intlen first
		mov edx,[eax]this.sign
		mov [ebx]result.sign,edx//copy sign
		mov esi,[eax]this.Data
		mov edi,[ebx]result.Data
		rep movsd//*this=source;
		//gcd2=A1
		mov eax,A1
		mov ecx,[eax]A1.IntLen
		mov gcd.IntLen,ecx//copy Intlen first
		mov edx,[eax]A1.sign
		mov gcd.sign,edx//copy sign
		mov esi,[eax]A1.Data
		mov edi,gcd.Data
		rep movsd//*this=source;

		mov ecx,ebx//result
		lea edx,gcd
		lea esi,gcd1
		///////////// loop ///////////////////
gcdlp1:	//comp A1 with 0 and jump if equal
		mov eax,[edx]Lint.Data
		mov eax,[eax+0]
		add eax,[edx]Lint.IntLen
		cmp eax,1
		je endegcd
		//division
		push esi
		push offset gen
		call Div1//division	
		xchg ecx,edx
		xchg edx,esi
		jmp gcdlp1
//////////////////// end loop //////////////////
endegcd:			
		//result=t1
		cmp ecx,ebx
		je nc0//no copy if result=t1
		//copy t1 to result
		mov eax,ecx
		mov ecx,[eax]Lint.IntLen
		mov [ebx]result.IntLen,ecx//copy Intlen first
		mov edx,[eax]Lint.sign
		mov [ebx]result.sign,edx//copy sign
		mov esi,[eax]Lint.Data
		mov edi,[ebx]result.Data
		rep movsd//*this=source;
nc0:	
		mov eax,0
		popf
		pop ebx
		pop edx	
		pop ecx//restore
		pop esi
		pop edi	
		mov esp,ebp
		pop ebp
		ret //
	}
}

//############################################################################################
int Lint::ExpMod(Lint* A2/*exp*/,Lint* modl, Lint* result)
{
	Lint* tr;
	Lint* tr1;
	Lint* tmp;
	tr=result;
	tr1=&mdxp1;
	
	modl->BarrettReductionPrecalc();
	
	mdxp=(*this)%(*modl);
	//__asm lea eax,mdxp
	(*result) = 1;

	for(BU i=0;i<A2->IntLen;i++)
	{
		for(unsigned j=0;j<32;j++)
		{
			if ((A2->Data[i]>>j)&1)
			{
				tr->Mul1(&mdxp,tr1);
				tr1->BarrettReduction(modl);				
				tmp=tr;	tr=tr1;	tr1=tmp;	
				
			}
			mdxp.Sqr1();
			mdxp.BarrettReduction(modl);

		}
	}
	if (tr!=result) *result=*tr;
	
	return 0;
}
//############################################################################################
__declspec(naked)int Lint::ExpMod1( Lint* A2/*exp*/,Lint* modl, Lint* result)
{
	__asm
	{	
		push ebp
		push edi//save registers
		push esi
		push ebx
		push ecx
		push edx

		//mdxp1=*this;//
		mov eax,[ecx]Lint.sign
		mov mdxp1.sign,eax
		mov esi,[ecx]Lint.Data
		mov edi,mdxp1.Data
		mov ecx,[ecx]Lint.IntLen
		mov mdxp1.IntLen,ecx
		rep movsd//copy Data
		//modl->BarrettReductionPrecalc();
		mov ecx,[esp+28]//modl
		call BarrettReductionPrecalc
		//pointer initialization
		lea ebp,mdxp	// set the t1
		mov ebx,[esp+32]//result// set the tmpresult
		mov eax,[ebx]Lint.Data
		mov dword ptr[eax],1//*tmpresult=1
		mov [ebx]Lint.IntLen,1
		mov [ebx]Lint.sign,0
		//for(int i=A2->IntLen-1;i>=0;--i)
		mov ecx,[esp]//A2
		mov esi,[ecx]Lint.IntLen
lppi:	sub esi,1
		jc elp
		//for(int j=31;j>=0;--j)
		mov edi,32
lppj:	sub edi,1
		jc lppi//exit from loop and continue the other
		//tmpres->Sqr1();
		mov ecx,ebx
		call Sqr1
		//tmpres->BarrettReduction(modl);
		mov edx,[esp+28]//modl
		call BarrettReduction
		//if ((A2->Data[i]>>j)&1)
		mov ecx,[esp]//A2
		mov eax,[ecx]Lint.Data
		mov ecx,edi
		mov eax,[eax+esi*4]
		shr eax,cl
		shr eax,1
		jnc lppj//continue loop					
		//tmpres->Mul1(&mdxp1,t1);
		push ebp
		push offset mdxp1
		push ebx
		call Mul1
		add esp,8//actualy is not 8 but 12
		//t1->BarrettReduction(modl);				
		pop ecx//t1
		call BarrettReduction
		//tmp=tmpres;tmpres=t1;t1=tmp;
		xchg ebp,ebx
		// end if ((A2->Data[i]>>j)&1)
		jmp lppj
		// end for(int j=31;j>=0;--j)
		// end for(int i=A2->IntLen-1;i>=0;--i)
elp:
		//if (tmpres!=result)*result=*tmpres;
		cmp ebx,[esp+32]//result
		je end
		//*result=*tmpres;
		mov edx,[esp+32]//result
		mov eax,[ebx]Lint.sign
		mov [edx]Lint.sign,eax
		mov ecx,[ebx]Lint.IntLen
		mov [edx]Lint.IntLen,ecx
		mov esi,[ebx]Lint.Data
		mov edi,[edx]Lint.Data
		rep movsd//copy Data
end:
		mov eax,0//return 0;
		pop edx	
		pop ecx//restore
		pop ebx
		pop esi
		pop edi	
		pop ebp
		ret 8//
		}	
}
//############################################################################################
__declspec(naked)int Lint::BarrettReductionPrecalc(void)
{
	__asm
	{
		push edi//save registers
		push esi
		push ecx
		//calculate the b^2k/m
		mov ecx,[ecx]Lint.IntLen//this
		sal ecx,1//2*k
		add ecx,1//because b=2^32 (2 dwords)
		mov edi,q3.Data
		mov q3.IntLen,ecx//
		mov eax,0// fill memory and return 0;
		cld//  cld is for inc and std is for dec
		rep stosd//fill data with zeros
		mov dword ptr[edi-4],1//first dword=1 
		push offset gen// rem
		push offset b2km//quot
		mov edx,[esp+8]//this
		lea ecx,q3//divident
		call Div1	

		mov eax,0//return 0;
		pop ecx//restore
		pop esi
		pop edi	
		ret 

	}
}

//############################################################################################
__declspec(naked)int Lint::BarrettReduction(Lint* A2/*mod*/)
{
	__asm
	{	push edi//save registers
		push ebx
		push edx//dont touch it	
		
		//calculate k
		mov ebx,[edx]A2.IntLen
		mov edi,[ecx]Lint.IntLen
		cmp edi,ebx//if k>=this.len check over
        jc nosub1//
		//this and A2 have both the same length or this >A2
		cmp ebx,edi//distinguish
		jc cnt
		//this and A2 have both the same length
		call CompareU
		cmp eax,2
		je nosub1
		cmp eax,1
		je cnt
		//this and A2 are equal set the mod to zero
		mov [ecx]Lint.IntLen,1
		mov eax,[ecx]Lint.Data
		mov dword ptr[eax],0
		jmp nosub1
cnt:	//find x/b^k-1
		dec ebx
		mov edx,[ecx]Lint.Data
		lea edx,[edx+ebx*4]
		mov [ecx]Lint.Data,edx//data must return to Base after this
		sub [ecx]Lint.IntLen,ebx//
		//x/b^k-1 * b2km
		push offset q3//product
		push offset b2km//multiplicant
		push ecx
		call Mul1
		add esp,12//		
		//find q3=x/b^k+1
		add ebx,2
		mov edx,q3.Data
		lea edx,[edx+ebx*4]
		mov q3.Data,edx//data must return to DataBase
		sub q3.IntLen,ebx//j
		jnz cnt1
		mov q3.IntLen,1

		
cnt1:	//find x mod b^k+1
		mov edx,[ecx]Lint.DataBase
		mov [ecx]Lint.Data,edx//restore from prev state
		cmp edi,ebx
		jnc nxt1
		mov [ecx]Lint.IntLen,edi//
		jmp nxt2
nxt1:	mov [ecx]Lint.IntLen,ebx//
		//normalize just in case the x mod b^k+1 == 0
		//call NormalizeLength

nxt2:	
		//q3*A2 and find r2 mod b^k+1
		push ebx//digits for partial mul
		push offset r2//product
		push offset q3//multiplicant
		push [esp+12]//A2
		call Mul3
		add esp,16//
		mov edx,q3.DataBase
		mov q3.Data,edx//restore from prev state
		//this-r2 (r1-r2)and save it to result
		lea edx,r2
		call Sub2		
		//if result<0 then add to result b^k+1
		mov eax,[ecx]Lint.sign
		cmp eax,0
		je noadd1// if positive  jump
		//add to result b^k+1
		xchg ecx,ebx
		inc ecx//because b=2^32 (2 dwords)
		mov edi,q3.Data
		mov q3.IntLen,ecx//
		mov eax,0// fill memory and return 0;
		cld//  cld is for inc and std is for dec
		rep stosd//fill data with zeros
		mov dword ptr[edi-4],1//first dword=1 
		//Add
		lea edx,q3
		mov ecx,ebx//this
		call Add2
noadd1:	//while result>=A2 result-=A2
		mov edx,[esp]
		call CompareU
		cmp eax,2
		je nosub1
		//result=result-a2
		call Sub2//result-=A2
		jmp noadd1
nosub1:
		mov eax,0//return 0;
		pop edx//restore
		pop ebx
		pop edi
		ret //

	}
}

//############################################################################################
__declspec(naked)int Lint::MillerRabinTest(BU t)
{
	BU s,j;	
	__asm
	{
		push ebp
		mov ebp,esp
		sub esp,8
		push ebx
		push esi
		push edi
		push ecx
		push edx
		
		
		mov edi,this
		mov ecx,[edi]Lint.IntLen
		mov esi,[edi]Lint.Data


		//calc  s and r such this-1=r*2^s	
		//make the this-1
		btr dword ptr[esi],0
		jc cnt1//if is odd continue
		bts dword ptr[esi],0
		jmp end
		//find how many lsbytes are zero and put it to ebx(rest bits are put to edx)
cnt1:	mov ebx,-1
		//mov edx,32
arf1:	inc ebx
		bsf edx,[esi+ebx*4]
		loopz arf1
		//shift this to the right ebx*32+edx bits and put it to r
		mov ecx,[edi]Lint.IntLen//this len
		sub ecx,ebx//new len
		mov r.IntLen,ecx//apply it to r
		lea esi,[esi+ebx*4]//new base address
		mov edi,r.Data
		cld
		rep movsd//copy to r
		//shift right the rest bits
		lea ecx,r
		call Sr
		shl ebx,5//ebx*32
		add ebx,edx
		mov s,ebx//put the result to s

end:	lea ecx,y
		mov edx,this
		mov ebx,[edx]Lint.Data//this.Data
		mov esi,[ecx]Lint.Data//y.Data

		//for(BU i=1;i<=t;i++)
		mov edi,0//i
frl0:	inc edi
		cmp t,edi
		jc endw13			

		//y=primes[i];
		mov eax,[edi*4+primes-4]
		mov [esi],eax
		mov [ecx]Lint.IntLen,1
		
		or dword ptr[ebx],1//make n
		//special case if this=prime[i]  candidate<= maxprime 
		cmp [edx]Lint.IntLen,1//if 32bit long
		jne cntn//continue
		cmp eax,[ebx]
		jne cntn//continue
		jmp endw13//return prime (true)

		//y.ExpMod(&r,this,&y);
cntn:	push ecx//y
		push edx//this
		lea edx,r
		call ExpMod1
		mov edx,this//restore 
		mov ecx,this
		call BarrettReductionPrecalc//for later use
		lea ecx,y//restore
		and dword ptr[ebx],4294967294//make n-1

		//if((y!=1)&&(y!= *this-1))
		cmp dword ptr[esi],1
		jne wl0
		cmp [ecx]Lint.IntLen,1
		je endwl2				
wl0:	call CompareU
		cmp eax,0
		je frl0//continue loop
				
		mov j,1

		//while ((j=<s-1)&&(y!= *this-1))
wl1:	mov eax,s
		dec eax
		cmp eax,j
		jc endwl1
		call CompareU
		cmp eax,0
		je endwl1

		//y.Sqr1()
		call Sqr1
		or dword ptr[ebx],1//make n
		//BarrettReduction(this)
		call BarrettReduction
		and dword ptr[ebx],4294967294//make n-1

		//if y==1 return 'composite'
		cmp dword ptr[esi],1 
		jne nxt2
		cmp [edx]Lint.IntLen,1 
		je r1//return 'composite'
nxt2:	inc j
		jmp wl1
endwl1:	//end of while ((j<s)&&(y!= *this))
	
		//if (y!= *this)return 'composite'
		call CompareU
		cmp eax,0
		jne r1//return 'composite'			
endwl2:	////end of if((y!=1)&&(y!= *this))

		jmp frl0
endw13:	// end for(BU i=1;i<=t;i++) loop
		
		or dword ptr[ebx],1//make n
		mov eax,0
		jmp endf
r1:		or dword ptr[ebx],1//make n
		mov eax,1

endf:	pop edx//restore
		pop ecx
		pop edi
		pop esi
		pop ebx
		mov esp,ebp
		pop ebp
		ret //

	}
	
//////////////////////////////////////////////////////////////
			//for(BU i=1;i<=t;i++)
			//y=primes[i];
			//Data[0]=Data[0]|1;//make n
			//y.ExpMod(&r,this,&y);
			//BarrettReductionPrecalc();
			//Data[0]=Data[0]&4294967294;//make n-1			
			//if(((y.Data[0]!=1)||(y.IntLen>1))&&(y!= *this))
			//{	
			//
			//j=1;
			//while ((j<s)&&(y!= *this))
			//{	
			//	y.Sqr1();
			//	Data[0]=Data[0]|1;//make n
			//	y.BarrettReduction(this);
			//	//y%=*this;
			//	Data[0]=Data[0]&4294967294;//make n-1
			//	if ((y.Data[0]==1)&& (y.IntLen==1))goto r1;
			//	j++;
			//}
			//if (y!= *this)goto r1;
			//}
			//}
}
//############################################################################################
__declspec(naked)int Lint::DivTrial(BU maxprim)//
{	
	__asm
    {
		push ebp
		push edi//save all registers
		push esi
		push edx
		push ebx
		push ecx

		//initialize vars and pointers
		lea esi,primes
		mov edi,[ecx]Lint.Data
		push [ecx]Lint.IntLen//save the len
		mov ebx,0//index
		mov ebp,edx
		//if 'this' is even do not continue (faster)
		mov eax,2//return value
		test dword ptr[edi],1//first dword
		jz end
		cmp [esp],1//if the candidate is 32 bit
		jne lp
		
		//trial division 32 bit
		cmp ebp,[edi]//if maxprim>=candidate then set maxprim to candidate-1
		jc lp1
		mov ebp,[edi] //set maxprim to candidate-1
		dec ebp
		//div loop
lp1:	mov edx,0//		
		mov eax,[edi]
		div dword ptr[esi+ebx*4]//
		test edx,edx
		jz end1
		inc ebx//increase index
		cmp dword ptr[esi+ebx*4],ebp// maxprime
		jc lp1
        jmp end0
		////end trial division 32 bit

		//trial division
lp:		mov ecx,[esp]//Lint.IntLen
		mov edx,0//		
ddd:	mov eax,[edi+ecx*4-4]
		div dword ptr[esi+ebx*4]//
		loop ddd	
		test edx,edx
		jz end1
		inc ebx//increase index
		cmp dword ptr[esi+ebx*4],ebp// maxprime
		jc lp
end0:   mov eax,0
		jmp end
end1:	mov eax,[esi+ebx*4]//prime, which is a factor of 'this'

end:	pop ecx//pops the len (dummy)
		pop ecx
		pop ebx
		pop edx//restore
		pop esi
		pop edi
		pop ebp
		ret	
	}	
}
//############################################################################################
int Lint::FirstPrimeAbove(BU t)
{
	/*
	unsigned int ai = primes[primesSize-1];
	int BitsSize = IntLen*32;  //this->GetSize();
	int min = 0;
	if (BitsSize<primesSize)
	min = BitsSize;
	else
	min = primesSize;
*/
	if(!IsOdd()){*this+=1;goto rp2;}
rpt:*this+=2;
rp2:if(DivTrial(1000000)==0)
		if(MillerRabinTest(t)==0)return 0;	
	goto rpt;
}

bool Lint::IsProbablePrime()
{
if (!IsOdd()) return false;
if (DivTrial(1000000)!=0) return false;  // we have trivial divizor

if (MillerRabinTest(5)==1) return false;	// Miller return 1 if composite
if (MillerRabinTest(20)==1) return false;	// Miller return 1 if composite

return true;
}


//############################################################################################
int Lint::ExpMod3( Lint* A2/*exp*/,Lint* modl, Lint* result)
{
	Lint *tmpres,*t1,*tmp;
	tmpres=result;
	t1=&mdxp;
	mdxp1=*this;//
	//modl->BarrettReductionPrecalc();
	*tmpres = 1;
	for	(int i=A2->IntLen-1;i>=0;--i)
	{
		for( int j=31;j>=0;--j)
		{			
			tmpres->Sqr1();
			*tmpres%=*modl;
			
			//tmpres->BarrettReduction(modl);
			
			if ((A2->Data[i]>>j)&1)
			{				
				tmpres->Mul1(&mdxp1,t1);
				*t1%=*modl;
				//t1->BarrettReduction(modl);
				tmp=tmpres;tmpres=t1;t1=tmp;
			}

		}
	if (tmpres!=result) *result=*tmpres;
	}
	return 0;
}
//############################################################################################
int Lint::MakeRandom(BU bits)
{		
	if(bits/sizeof(BU)>MAXLEN)return 1;//false request
	WipeOut();//destroy all contents before action
	
	unsigned int rb;//requested bits
	rb=bits/32;
	if(bits%32>0)rb++;//round bits (if necessary) to next 32 bits value
	rb*=32;

	if(GetRandomBits(Data,rb))return 2;
	IntLen=rb/32;
	return 0;
	
}

//############################################################################################
int Lint::InvMod(Lint* modl,Lint* result)
{
	Lint *y,*a,*b,*r,*q,*x,*tmp;
	
	//assign integers to pointers 
	//we can use some global Lints for calculations 
	//but first, we must be sure that these Lints are not conflict
	//each other after an internal operation.
	a=&gcd1;
	b=&gcd;
	r=&LITmp;
	q=&LITmp1;
	y=&LITmp2;
	x=result;
	
	//initialize pointers
	*a=*modl;//save modl
	*b=*this;//save this	
	*y=1;
	*x=0;

	//find inverse modulus and gcd
	while (*b>0)
	{
		a->Div1(b,q,r);
		tmp=a;//work only with pointers to increase speed
		a=b;
		b=r;
		r=tmp;

		*q = *x - *q * *y;
		tmp=x;//work only with pointers to increase speed
		x=y;
		y=q;
		q=tmp;
	}
	
	//assign the 'x' to the 'result'
	if (x!=result) *result=*x;//if it is not the same	
	
	if (*a==1)//if gcd ==1
	{
		if(*result<0)*result+=*modl;		
		return 0;
	}
	else
	{
		*result=0;
		return 1;//no inverse mod
	}
}
//############################################################################################
