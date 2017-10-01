/* 

	MIT license.

	Copyright 2017 Aaron Flin

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to
	deal in the Software without restriction, including without limitation the
	rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
	sell copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
	IN THE SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <inttypes.h>
#include <string.h>
#include "doubletostr.h"

/* this was left in for referenct */
/* 5 bits to char for negative numbers
char negchars[32] = {
* 0,    -1,   -2,   -3,   -4,   -5,   -6,   -7,   -8,   -9,  -10,  -11,  -12,  -13,  -14,  -15, *
   0x4f, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a, 0x49, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x40, 
* -16,  -17,  -18,  -19,  -20,  -21,  -22,  -23,  -24,  -25,  -26,  -27,  -28,  -29,  -30,  -31, *
   0x3f, 0x3e, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30
};
   5 bits to char for positive numbers
char poschars[32]= {
*   0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    10,   11,   12,   13,   14,   15 *
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 
*   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,   *
     0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f
};
*/

double str32todouble (char *s) {

	const double d=9007199254740992.0;// 2^53
	double r;
	int64_t m=0;
	char s2[16];		
	int i,j,e=0;

#ifdef DOUBLETOSTR_ENFORCELENGTH
	if (strlen(s) < 14) {
		return (NAN);
	}
#endif
	if (s[0]=='.' || s[0] == '-') {
		//printf("it was %s ",s);
		memcpy(s2,s,14);
		s=s2;
		s[0]-=3;
		for (i=1;i<14;i++) {
			if (s[i] > 96)s[i]-= 11;
			else if (s[i] > 63 ) s[i]-= 5;
		}
		//printf("and is now %s\n",s);
	}

	for (i=1;i<14;i++) {
		if (s[i]<0x30 || s[i]> 0x6f) return (NAN);
	}
	if ( s[0]=='+' ) {/* positive number */
		for ( i=0; i< 11; i++) {
			j=13-i;
			m+= (int64_t) ( ( (int64_t)s[j]-80 ) << (i*5) );
		}
	} else if ( s[0]=='*' ) { /* negative number */
		for ( i=0; i< 11; i++) {
			j=13-i;
			/* printf("out #%d: %d\n", i, (79-(int)s[j])); */
			m+= (int64_t) ( ( (int64_t)79 - (int64_t)s[j] ) << (i*5) );
		}
		m*=-1;
	} else {
		/*fprintf(stderr,"String not in proper format\n"); */
		return(NAN);
	}
	
	r=(double)m;
	
	/* decode exponent */
	
	if ( s[1] > 0x4f ) { /* positive exponent */
		e+=(int) (((int)s[1] - 80 ) << 5);
		e+=(int) (((int)s[2] - 80 ) );
	} else { /* negative exponent */
		e+=(int) ((79-(int)s[1]) << 5);
		e+=(int) ((79-(int)s[2]) );
		e*=-1;
	}
	/* exponent value was negated during encode if number is negative (to preserve ascii ordering) */
	if(s[0]=='*')
		e*=-1;
	/*adjust bias */
	e++;

	/* printf("e=%d , r/d = %f\n",e,r/d); */
	
	/* our flag for nan.  This number has an out of range mantissa 
	   for doubles, so no conflicts with valid double */
	if (e==-1022 && r/d > 1.0) return(NAN); 
	
	/*printf("e=%d , r=%f \n" , e ,r ); */

	/* pow(2,1024) is infinity, so use one less and x2 on mantissa to keep everything in range */
	return ( pow(2.0,(double) e-1) * (2*r/d) );
}

void doubletostr32(double x, char **str, int safe) {
	const double d=9007199254740992.0;// 2^53
	double r;
	int e,i,j;
	unsigned int ue;
	char e1, e2;
	char pos,neg,sign;
	int64_t m;
	char mat32[12];

	if(safe) {
		pos='.';
		neg='-';
	} else {
		pos='+';
		neg='*';
	}
	sign=pos;

	r=frexp(x,&e);
	/*printf("got %f , %d\n",r,e);*/

	r=r*d;
	m= (int64_t) r ;

	/* our mantissa is 55 bits, so we can use extra large values as flags */
	if (isinf(r)) {
		/* checking for isinf(r)==-1 doesn't work on mac */
		e=1024;
		if (r<0.0) m=(int64_t) (-1.1*d);
		else m=(int64_t) (1.1*d);
	} else if (isnan(r)) {
		e=-1022;
		m=(int64_t) (1.1*d);
	}
	/* printf("e=%d, m=%lld\n",e,m); */
	/* bias by -1 since range is -1021 to 1024 */
	if (e<=0) {
		if(m==0) {
			e=-1021; /* preserve our sort order - 2^-1022 *0 is still 0 */
		}
		ue = (unsigned int) (-1 * (e-1) );
		if (m<0) {
			e1 = (char) ( 80+ ((int)(ue & 0x1f)) );
			e2 = (char) ( 80+ ((int)(ue >> 5 ) & 0x1f)) ;
		} else {
			e1=(char) (79- ( (int)(ue & 0x1f)));
			e2=(char) (79- ((int) (ue >> 5) & 0x1f));
		}
	} else {
		ue = (unsigned int) e-1;
		if (m<0) {
			e1=(char) (79- ( (int)(ue & 0x1f)));
			e2=(char) (79- ((int) (ue >> 5) & 0x1f));
		} else {
			e1 = (char) ( 80+ ((int)(ue & 0x1f)) );
			e2 = (char) ( 80+ ((int)(ue >> 5 ) & 0x1f)) ;
		}
	}


	/*printf("got %f, %lld , %d absbias(%d) (%dx32 + %d) \n",r,m,e, ue, e2, e1); */

	/* pow(2,1024) is infinity, so use one less and x2 on mantissa to keep everything in range */
	/* printf("e=%d , r=%f \n" , e ,r ); */
	/*
	r=pow(2,e-1) * (2*r/d);
	printf ("does \n%.400f\n  =\n%.400f\n", r, x);
	*/
	
	if (m<0) sign=neg;

	mat32[11]='\0';
	if (m < 0 ) { /* negative */
		m*=-1;
		for (i=0;i<11;i++) {
			j=10-i;
			mat32[j]= (char) (79- ((int) (m >> (i*5)) & 0x1f));
		}
	} else {
		for (i=0;i<11;i++) {
			j=10-i;
			mat32[j]= (char) ( 80+ ((int)(m >> (i*5) ) & 0x1f)) ;
		}
	}

	//sprintf(*str,"%c%c%c%s (2^%d)*%.0f/(2^53)",sign,e2,e1,mat32,e,r);
	sprintf(*str,"%c%c%c%s",sign,e2,e1,mat32);
	/* eliminate any potential special chars (eg ;<>`\=?[]) by shifting upwards - results in [a-zA-Z0-9@:.-] range of chars */
	if(safe) {
		char *s=*str;
		//printf("it was %s ", s);
		fflush(stdout);
		i=0;
		while(s[i]!='\0') {
			if (s[i] > 85)s[i]+= 11;
			else if (s[i] > 58 ) s[i]+= 5;
			i++;
		}
		//printf("and is now %s\n",s);
	}
}

#ifdef DBLTEST
void usage(){
	printf("Convert a double sized float to an ascii sortable string\nUsage:\n  dtos [-s|-d|-r|-h] [string|float]\n     -s safe:     encode using a-z|A-Z|0-9|:|@|.|- chars only (url, html & shell safe)\n     -d decode:   decode an encoded string to double\n     -r reencode: reencode and print conversion\n     -h help:     This help message\n\n  stod            same as dtos -d (decode by defaut)\n"); 
	exit(0);
}

void strip0s(char *s) {
	int i=strlen(s)-1;
	while (s[i]=='0' && i>0) s--;
	if (s[i]=='.') i++; /* leave last 0 after decimal point */
	s[i+1]='\0';
}

int main(int argc, char **argv) {

	double res,x=NAN;

	char b[64]="";
	char *buf=&b[0];
	char *end,*a= argv[0];
	char *val=(char*)NULL;
	int safe=0,decode=0,recode=0;
	int i=1,px=0;
	char floatbuf[512];
	
	if (argc<2) usage();

	a+=strlen(a)-4;

	if ( !strncmp("stod",a,4) ) decode=1;
	
	while (argc > 1) {
		if (!strcmp("DBL_MAX",argv[i])) 
			x=DBL_MAX;
		else if (!strcmp("-DBL_MAX",argv[i]))
			x=-DBL_MAX; 
		else if (!strcmp("DBL_MIN",argv[i])) 
			x=DBL_MIN;
		else if (!strcmp("-DBL_MIN",argv[i]))
			x=-DBL_MIN;
		else if (!strcmp("-INF",argv[i]))
			x=(double)-INFINITY; 
		else if (!strcmp("INF",argv[i])) 
			x=(double)INFINITY;
		else if (!strcmp("NAN",argv[i]))
			x=NAN;
		else if (!strcmp("-s",argv[i]))
			safe=1;
		else if (!strcmp("-h",argv[i]))
			usage();
		else if (!strcmp("-?",argv[i]))
			usage();
		else if (!strcmp("--help",argv[i]))
			usage();
		else if (!strcmp("-d",argv[i]))
			decode=1;
		else if (!strcmp("-r",argv[i]))
			recode=1;
		else {
			val=argv[i];
			px=1;
		} 
		
		argc--;
		i++;
	}
		
	if (decode) {
		if (val==(char*)NULL) {
			fprintf(stderr,"provide an encoded double to decode\n");
			exit(1);
		}
		res=str32todouble(val);
		snprintf(floatbuf,512,"%.512f",res);
		strip0s(floatbuf);
		if (recode) {
			doubletostr32(res, &buf,safe);
			printf("%s=%s\n",floatbuf,buf);
		} else {
			printf("%s\n", floatbuf);
		}
	} else { 
		if (px) {
			x=strtod(val,&end);
			if (x==0&&val==end) x=NAN;
		}
		doubletostr32(x, &buf,safe);
		
		if (recode) {
			res=str32todouble(buf);
			snprintf(floatbuf,512,"%.512f",res);
			strip0s(floatbuf);
			printf("%s=%s\n",buf,floatbuf);
		} else {
			printf("%s\n",buf);
		}
	}

}
#endif
