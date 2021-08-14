
#include "LargeIntegers.h"
#include <Windows.h>
#include <tchar.h>

/* usually, the resource editor creates this file to us: */
#include "resource.h"
#include <iostream>

using namespace std;



char Buffer[BUFLEN];//allocate space for display large integers
std::ostream & operator << (std::ostream & Str,LINT& rl)
{
	rl.ToStr(Buffer);
	Str << Buffer;
	return Str;
}
 void Test(void);



int  __cdecl main(void)
{	



	char buf[1024];
	
	Test();


	unsigned int bits;	
	cout <<"give number of bits and i shall generate you an RSA key pair (rounded to next 32bits) :" ;
	cin >> bits;
	cout<<endl<<endl;

	LINT pbkmodulus,pbkexponent;
	LINT prkexponent;


	bits/=2;//because the bits value is for modulus not for primes
	LINT tmpkey1;
	LINT tmpkey2;
	//generate p and q
rg:	tmpkey1.MakeRandom(bits);
ag:	tmpkey1.FirstPrimeAbove(5);	
	if((tmpkey1.MillerRabinTest(20))!=0) goto ag;//make 'sure' this is a 'prime'

	tmpkey2.MakeRandom(bits);
ag1:tmpkey2.FirstPrimeAbove(5);	
	if((tmpkey2.MillerRabinTest(20))!=0) goto ag1;//make 'sure' this is a 'prime'

	if (((PLINT)(&(tmpkey1-tmpkey2)))->GetLength()<(bits/32)) goto rg;
		//if the difference is quiet small find some other keys

	//now we have the keys and calculate modulus 
	pbkmodulus= tmpkey1 * tmpkey2;
		
	//calculate (p-1)*(q-1)
	tmpkey1--;
	tmpkey2--;
	tmpkey1*=tmpkey2;//(p-1)*(q-1)
	
	//generate exponent for encryption. I decide to use a random 32 bit prime eg 1073741827;
rg1:pbkexponent.MakeRandom(32);
	pbkexponent.FirstPrimeAbove(50);	
	//gcd test must return one
	pbkexponent.Egcd(&tmpkey1,&tmpkey2);
	if (tmpkey2!=1) goto rg1;//if result of gcd !1 then regenerate another number

	//generate exponent for decryption.
	int rslt;
	rslt=pbkexponent.InvMod(&tmpkey1,&prkexponent);
	if (rslt!=0) goto rg1;//if there is not exist repeat proccess

	tmpkey1.WipeOut();//clear
	tmpkey2.WipeOut();//clear
	LINT::WipeOutAllGlobals();//clear all variables used for key generation
	// usually for security reasons

////////////////////end of key generation ////////////////////////////

	SetRadix(16);
	cout<<"public exponent:  "<<endl<<pbkexponent<<endl<<endl;
	cout<<"private exponent: "<<endl<<prkexponent<<endl<<endl;
	cout<<"modulus:          "<<endl<<pbkmodulus<<endl<<endl;
//	/////////////////////////////////////////////////////////////////////////////////
//	Bc();
//	x.Egcd(&y,&m);
//	Ac();
//	cout << "gcd of x and y using Efklidis algorithm is: m="<<m<< endl<< endl; 
//	cout <<"remainder x%m is "<<x%m<<"  and y%m is "<<y%m<< endl<< endl;	
//	////////////////////////////////////////////////////////////////////////////////
//	cout <<"gcd efklid counter diff= "<< sc<<endl<<endl<< endl;
//	///////////////////////////////////////////////////////////////////////////////
//	Bc();
//	x.Gcd(&y,&m);
//	Ac();
//	cout << "gcd of x and y using binary algorithm is: m="<<m<< endl<< endl; 
//	cout <<"remainder x%m is "<<x%m<<"  and y%m is "<<y%m<< endl<< endl;	
//	////////////////////////////////////////////////////////////////////////////////
//	cout <<"gcdbin counter diff= "<< sc<<endl<< endl<<endl;
//	////////////////////////////////////////////////////////////////////////////////
//	cout << "The operation ((x*y+1)*3-3)/3/y-x gives result equal to  " <<((x*y+1)*3-3)/3/y-x<< endl<<endl;
//	////////////////////////////////////////////////////////////////////////////////
//	m=x%y;	
//	cout << "lets x%y=m. Then x/y*y+m-x="<<x/y*y+m-x<< endl<< endl; 
//	////////////////////////////////////////////////////////////////////////////////
//	Bt();
//	x.Exp(2,100000,&r);
//	At();
//	cout << "The 2 exp 100000 is: r = " << r<< endl<< endl;
//	////////////////////////////////////////////////////////////////////////////////
//	cout << endl<<" exp counter diff= "<< sc<<" microseconds."<<endl<< endl;
//	////////////////////////////////////////////////////////////////////////////
	cout << "give me a number and i shall return you the factorial :";//
	cin >> buf;
	SetRadix(10);
	LINT xn(buf);	
	unsigned int i;
	LINT m(1);
	for(i=1;xn>=i;i++) m *= i;
	cout << endl<< endl<< "And the factorial is: " <<m<< endl<< endl;

	return 0;
}

void MyBarrettTest()
{
	LINT x;
	x = LINT("1234567894564556475864568456845684897895645645645645645645564758645684568456"\
		"84897895645645645645645645564758645684568456848978956456345645645645645564758645"\
		"68456845684897895645645645645645645564758645684568456848978956456456456456456455"\
		"64758645684568456848978956456345645645645645564758645684568456848978956456456456"\
		"45645645564758645684568456848978956456456456456456455647586456845684568489789564"\
		"56345645645645645564758645684568456848978956456456456456456455647586456845684568"\
		"48978956456456456456456455647586456845684568489789564563456456456456455647586456"\
		"84568456848978956456456456456456455647586456845684568489789564564564564564564556"\
		"47586456845684568489789564563456456456456455647586456845684568489789564564564564"\
		"56456455647586456845684568489789564564564564564564556475864568456845684897895645"\
		"63456456456456455647586456845684568489789564564564564564564556475864568456845684"\
		"89789564564564564564564556475864568456845684897895645634564564564564556475864568"\
		"45684568489789564564564564564564556475864568456845684897895645645645645645645564"\
		"75864568456845684897895645634564564564564556475864568456845684897895645645645645"\
		"64564556475864568456845684897895645645645645645645564758645684568456848978956456"\
		"34564564564564556475864568456845684897895645645645645645645564758645684568456848"\
		"97895645645645645645645564758645684568456848978956456345645645645645564758645684"\
		"56845684897895645645645645645645564758645684568456848978956456456456456456455647"\
		"5864568456845684897895645632000000000000000000000");
	
	LINT y1("1234567894564556475864568456845684897895645645645645645645564758645684568456"\
		"848978956456456456456456455647586456845684568489789564563456456456456455647586456"\
		"845684568489789564564564564564564556475864568456845684897895645645645645645645564"\
		"758645684568456848978956456345645645645645564758645684568456848978956456456456456"\
		"456455647586456845684568489789564564564564564564556475864568456845684897895645634"\
		"564564564564556475864568456845684897895645645645645645645564758645684568456848978"\
		"956456456456456456455647586456845684568489789564563456456456456455647586456845684"\
		"568489789564564564564564564556475864568456845684897895645645645645645645564758645"\
		"684568456848978956456345645645645645564758645684568456848978956456456456456456455"\
		"647582222222222985985222259259292598171512928591781478146456845684561");
	
	LINT z,t;
	LARGE_INTEGER Frequency;
bool QueryOk = QueryPerformanceFrequency(&Frequency);
LARGE_INTEGER li_start;
QueryPerformanceCounter(&li_start);

	t=x*y1;

LARGE_INTEGER li_end;
QueryPerformanceCounter(&li_end);
LONGLONG diff = li_end.QuadPart-li_start.QuadPart;
  // To get duration in milliseconds
  double dftDuration = (double) diff * 1000.0 / (double) Frequency.QuadPart;
  char buf2[100];
  int len = sprintf( buf2, "%ld\n", diff);
  SetDlgItemText(NULL,IDC_EDIT4,buf2);

MessageBox(NULL, buf2,"Error!!", MB_ABORTRETRYIGNORE |MB_ICONEXCLAMATION);


	z=x;
	y1.BarrettReductionPrecalc();//only for calc b2km

	QueryPerformanceCounter(&li_start);
	z.BarrettReduction(&y1);
QueryPerformanceCounter(&li_end);
diff = li_end.QuadPart-li_start.QuadPart;
  // To get duration in milliseconds
  dftDuration = (double) diff * 1000.0 / (double) Frequency.QuadPart;
  len = sprintf( buf2, "%ld\n", diff);
  SetDlgItemText(NULL,IDC_EDIT4,buf2);

MessageBox(NULL, buf2,"Error!!", MB_ABORTRETRYIGNORE |MB_ICONEXCLAMATION);


	if(z!=t)
		cout <<"fail at BarrettReduction(PLINT)= "<<z<<endl<< endl<<endl;	

}

void Test(void)
{
	SetRadix(10);
	LINT x("1234567894564556475864568456845684897895645645645645645645564758645684568456"\
		"84897895645645645645645645564758645684568456848978956456345645645645645564758645"\
		"68456845684897895645645645645645645564758645684568456848978956456456456456456455"\
		"64758645684568456848978956456345645645645645564758645684568456848978956456456456"\
		"45645645564758645684568456848978956456456456456456455647586456845684568489789564"\
		"56345645645645645564758645684568456848978956456456456456456455647586456845684568"\
		"48978956456456456456456455647586456845684568489789564563456456456456455647586456"\
		"84568456848978956456456456456456455647586456845684568489789564564564564564564556"\
		"47586456845684568489789564563456456456456455647586456845684568489789564564564564"\
		"56456455647586456845684568489789564564564564564564556475864568456845684897895645"\
		"63456456456456455647586456845684568489789564564564564564564556475864568456845684"\
		"89789564564564564564564556475864568456845684897895645634564564564564556475864568"\
		"45684568489789564564564564564564556475864568456845684897895645645645645645645564"\
		"75864568456845684897895645634564564564564556475864568456845684897895645645645645"\
		"64564556475864568456845684897895645645645645645645564758645684568456848978956456"\
		"34564564564564556475864568456845684897895645645645645645645564758645684568456848"\
		"97895645645645645645645564758645684568456848978956456345645645645645564758645684"\
		"56845684897895645645645645645645564758645684568456848978956456456456456456455647"\
		"5864568456845684897895645632000000000000000000000");
	
	LINT y1("1234567894564556475864568456845684897895645645645645645645564758645684568456"\
		"848978956456456456456456455647586456845684568489789564563456456456456455647586456"\
		"845684568489789564564564564564564556475864568456845684897895645645645645645645564"\
		"758645684568456848978956456345645645645645564758645684568456848978956456456456456"\
		"456455647586456845684568489789564564564564564564556475864568456845684897895645634"\
		"564564564564556475864568456845684897895645645645645645645564758645684568456848978"\
		"956456456456456456455647586456845684568489789564563456456456456455647586456845684"\
		"568489789564564564564564564556475864568456845684897895645645645645645645564758645"\
		"684568456848978956456345645645645645564758645684568456848978956456456456456456455"\
		"647582222222222985985222259259292598171512928591781478146456845684561");
	
	LINT y("10000000000000000000000000000000000"\
		"0000000000000000000000000000000000000000000001");
	
	LINT threeexpthousand("132207081948080663689045525975214436596542203275214816766492036"\
		"8226828597346704899540778313850608061963909777696872582355950954582100618911865342"\
		"7252579536740276202251983208038780147742289648412743904001175886180411289478156230"\
		"9443806156617305408667449050617812548034440554705439703889581746536825491613622083"\
		"0268563778582290228416398307887896918556404084898937609373242171846359938695516765"\
		"0189405881090604260896714388641028143503856487471658320106143661321731027689028552"\
		"20001");
	LINT xexpy1mody("6109774572987381843249160877325107255729964161407253739629164828048"\
		"9677842585060");	
	LINT z,t;
	//cout << endl<< MSG<<endl<< endl;
	if((z/y)!=0)
	cout << "fail at 0/y=   "<<z/y<< endl<< endl;
	if((z%y)!=0)	
	cout << "fail at 0%y=   "<<z%y<< endl<< endl;
	if((y/y)!=1)	
	cout << "fail at y/y=   "<<y/y<< endl<< endl;
	if((z%y)!=0)	
	cout << "fail at y%y=   "<<y%y<< endl<< endl;
	if((y/x)!=0)	
	cout << "fail at y/x=   "<<y/x<< endl<< endl;
	if((y%x)!=y)		
	cout << "fail at y%x=   "<<y%x<< endl<< endl;
	t=x/y;
	t*=y;
	t+=(x%y);
	t-=x;
	if(t!=0)
	cout << "fail at x/y=   "<<x/y<< endl<< endl;
	t=y+1;
	t-=1;
	if(t!=y)	
	cout << "fail at y+1=   "<<y+1<< endl<< endl;
	t=y-1;
	t+=1;
	if(t!=y)	
	cout << "fail at y-1=   "<<y-1<< endl<< endl;
	t=y;
	t.Sl(1);
	if(t!=(y*2))	
	cout << "fail at y*2=   "<<y*2<< endl<< endl;
	t=y;
	t.Sr(1);
	if(t!=(y/2))	
	cout << "fail at y/2=   "<<y/2<< endl<< endl;
	t=y;
	t*=y;
	if((y*y)!=t)	
	cout << "fail at y^2=   "<<y*y<< endl<< endl;
	t=y;
	if(!t.IsNeg()) t.MakeNeg(); else t.MakePos();
	t/=3;
	if((y/-3)!=t)	
	cout << "fail at y/-3=   "<<y/-3<< endl<< endl;
	t=y;
	if(!t.IsNeg()) t.MakeNeg(); else t.MakePos();
	t%=3;
	if((y%-3)==t)	
	cout << "fail at y%-3=   "<<y%-3<< endl<< endl;
	///////////////////////////////////////////////////////////////////////////////
	x.Egcd(&y,&t);
	if((x%t)!=0 || (y%t)!=0)
	{
		cout << "fail at gcd of x and y using Efklidis algorithm : m="<<t<< endl<< endl; 
		cout <<"remainder x%t is "<<x%t<<"  and y%t is "<<y%t<< endl<< endl;	
	}
	///////////////////////////////////////////////////////////////////////////////
	x.Gcd(&y,&t);
	if((x%t)!=0 || (y%t)!=0)
	{
		cout << "fail at gcd of x and y using binary algorithm : m="<<t<< endl<< endl; 
		cout <<"remainder x%t is "<<x%t<<"  and y%t is "<<y%t<< endl<< endl;	
	}
	///////////////////////////////////////////////////////////////////////////////
	x.ExpMod(&y1,&y,&t);

	if(t!=xexpy1mody)
		cout << endl<< endl<<"fail at ExpMod : x^y1 mod y= "<< t << endl<< endl;		
	///////////////////////////////////////////////////////////////////////////////
	//x.ExpMod1(&y1,&y,&t);false because x.GetLength > (2 * y.GetLength)
	if(t!=xexpy1mody)	
		cout << endl<< endl<<"fail at ExpMod1 : x^y1 mod1 y= "<< t << endl<< endl;
	///////////////////////////////////////////////////////////////////////////////
	x.Exp(3,1000,&t);
	if(t!=threeexpthousand)
		cout << "fail at Exp : 3exp1000 = " << t<< endl<< endl;
	////////////////////////////////////////////////////////////////////////////////
	t=x%y1;
	z=x;
	y1.BarrettReductionPrecalc();//only for calc b2km
	z.BarrettReduction(&y1);
	if(z!=t)
		cout <<"fail at BarrettReduction(PLINT)= "<<z<<endl<< endl<<endl;	

}

void IsPrimeTest(HWND hDlg)
{
LARGE_INTEGER Frequency;
bool QueryOk = QueryPerformanceFrequency(&Frequency);
LARGE_INTEGER li_start;
QueryPerformanceCounter(&li_start);

if (IsDlgButtonChecked(hDlg, IDC_CHECK1)==1)
SetRadix(16);  // hexadecimal
else
SetRadix(10);  // decimal

char nbuf[5024];
GetDlgItemText(hDlg, IDC_EDIT1, nbuf, sizeof(nbuf));
LINT numb(nbuf);
if (numb.IsProbablePrime())
SetDlgItemText(hDlg,IDC_EDIT2,"YES");
else
SetDlgItemText(hDlg,IDC_EDIT2,"NO");

LARGE_INTEGER li_end;
QueryPerformanceCounter(&li_end);
LONGLONG diff = li_end.QuadPart-li_start.QuadPart;
  // To get duration in milliseconds
  double dftDuration = (double) diff * 1000.0 / (double) Frequency.QuadPart;
  char buf2[100];
  int len = sprintf( buf2, "%f\n", dftDuration);
  SetDlgItemText(hDlg,IDC_EDIT4,buf2);
}

void GenerateNumber(HWND hDlg)
{
	char nbuf[5024];

	SetDlgItemText(hDlg,IDC_EDIT4,"--");
	BOOL bRet;
	unsigned int bits = GetDlgItemInt(hDlg,IDC_EDIT3, &bRet,FALSE);
	if (!bRet | bits==0) bits = 2048;   // 2048 bits

	LINT tmpkey1;
LARGE_INTEGER Frequency;
bool QueryOk = QueryPerformanceFrequency(&Frequency);
LARGE_INTEGER li_start;
QueryPerformanceCounter(&li_start);

tmpkey1.MakeRandom(bits);
if (IsDlgButtonChecked(hDlg, IDC_CHECK3)==0)
tmpkey1.ExactBits(bits);

if (IsDlgButtonChecked(hDlg, IDC_CHECK2)==1)
{  // if we choose prime number:
ag:	tmpkey1.FirstPrimeAbove(5);
	if((tmpkey1.MillerRabinTest(20))!=0) goto ag; //make 'sure' this is a 'prime'
}

if (IsDlgButtonChecked(hDlg, IDC_CHECK1)==1)
SetRadix(16);  // hexadecimal
else
SetRadix(10);  // decimal

tmpkey1.ToStr(nbuf);
SetDlgItemText(hDlg,IDC_EDIT1,nbuf);

LARGE_INTEGER li_end;
QueryPerformanceCounter(&li_end);
LONGLONG diff = li_end.QuadPart-li_start.QuadPart;
  // To get duration in milliseconds
  double dftDuration = (double) diff * 1000.0 / (double) Frequency.QuadPart;
  char buf2[100];
  int len = sprintf( buf2, "%d\n", (int)dftDuration);
  SetDlgItemText(hDlg,IDC_EDIT4,buf2);

//MessageBox(hDlg, buf2,"Error!!", MB_ABORTRETRYIGNORE |MB_ICONEXCLAMATION);

}
// https://forums.codeguru.com/showthread.php?500841-Set-my-window-position-at-screen-center
void SetCenter(HWND hWnd)
{
RECT rc;
GetWindowRect ( hWnd, &rc ) ;
int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right)/2;
int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom)/2;
SetWindowPos( hWnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
}

// https://stackoverflow.com/questions/671815/what-is-the-fastest-most-efficient-way-to-find-the-highest-set-bit-msb-in-an-i
int msb(unsigned int v) {
  static const int pos[32] = {0, 1, 28, 2, 29, 14, 24, 3,
    30, 22, 20, 15, 25, 17, 4, 8, 31, 27, 13, 23, 21, 19,
    16, 7, 26, 12, 18, 6, 11, 5, 10, 9};
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v = (v >> 1) + 1;
  return pos[(v * 0x077CB531UL) >> 27];
}

void GetBitsCount(HWND hDlg)
{
if (IsDlgButtonChecked(hDlg, IDC_CHECK1)==1)
SetRadix(16);  // hexadecimal
else
SetRadix(10);  // decimal

char nbuf[5024];
GetDlgItemText(hDlg, IDC_EDIT1, nbuf, sizeof(nbuf));
LINT numb(nbuf);
if (IsDlgButtonChecked(hDlg, IDC_CHECK3)==1)
{  // if round selected
int BitsCount = numb.GetLength()<<5;
SetDlgItemInt(hDlg, IDC_EDIT3, BitsCount, FALSE);
return;
}

int BitsCount = (numb.GetLength()-1)<<5;
unsigned int MSB = numb.GetDigit(numb.GetLength()-1);
unsigned int restOfBits = msb(MSB)+1;
BitsCount += restOfBits;
SetDlgItemInt(hDlg, IDC_EDIT3, BitsCount, FALSE);
}

// https://www.codeproject.com/Articles/227831/A-Dialog-Based-Win32-C-Program-Step-by-Step
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
  case WM_INITDIALOG:
	  SetCenter(hDlg);
	  CheckDlgButton(hDlg, IDC_CHECK1, BST_CHECKED);
	  CheckDlgButton(hDlg, IDC_CHECK2, BST_CHECKED);
	  CheckDlgButton(hDlg, IDC_CHECK3, BST_CHECKED);
	  break;

  case WM_COMMAND:
    switch(LOWORD(wParam))
    {
	case IDOK: case IDC_BUTTON3:
	GenerateNumber(hDlg);
	break;

	case IDC_BUTTON1:
	IsPrimeTest(hDlg);
	break;

	case IDC_BUTTON2:  // get bits count:
	GetBitsCount(hDlg);
	break;

    case IDCANCEL:
      SendMessage(hDlg, WM_CLOSE, 0, 0);
      return TRUE;
    }
    break;

  case WM_CLOSE:
    //if(MessageBox(hDlg, TEXT("Close the program?"), TEXT("Close"),
    //  MB_ICONQUESTION | MB_YESNO) == IDYES)
    //{
      DestroyWindow(hDlg);
    //}
    return TRUE;

  case WM_DESTROY:
    PostQuitMessage(0);
    return TRUE;
  }

  return FALSE;
}



int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE h0, LPTSTR lpCmdLine, int nCmdShow)
{
// MyBarrettTest();

  HWND hDlg;
  MSG msg;
  BOOL ret;

//  InitCommonControls();
  hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc, 0);
  ShowWindow(hDlg, nCmdShow);

  while((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
    if(ret == -1)
      return -1;

    if(!IsDialogMessage(hDlg, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return 0;
}

