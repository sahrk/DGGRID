/*******************************************************************************
    Copyright (C) 2021 Kevin Sahr

    This file is part of DGGRID.

    DGGRID is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DGGRID is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
//
// DgRandom.C: DgRandom class implementation. Includes Marsaglia's "mother"
//             function and notes.
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgRandom.h>

static double Mother (unsigned long *pSeed);

////////////////////////////////////////////////////////////////////////////////
DgRandom::~DgRandom(void)
{
}

////////////////////////////////////////////////////////////////////////////////
unsigned long int
DgRandMother::nextRand (void)
{
   unsigned long int seed = status();

   Mother(&seed);

   return seed;

} // unsigned long int DgRandMother::nextRand

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// below is George Marsaglia's code and comments
//
////////////////////////////////////////////////////////////////////////////////

#include <string.h>

static short mother1[10];
static short mother2[10];
static short mStart=1;

#define m16Long 65536L				/* 2^16 */
#define m16Mask 0xFFFF          /* mask for lower 16 bits */
#define m15Mask 0x7FFF			/* mask for lower 15 bits */
#define m31Mask 0x7FFFFFFF     /* mask for 31 bits */
//#define m32Double  4294967295.0  /* 2^32-1 */

/* Mother **************************************************************
|	George Marsaglia's The mother of all random number generators
|		producing uniformly distributed pseudo random 32 bit values
with
|		period about 2^250.
|	The text of Marsaglia's posting is appended at the end of the function.
|
|	The arrays mother1 and mother2 store carry values in their
|		first element, and random 16 bit numbers in elements 1 to 8.
|		These random numbers are moved to elements 2 to 9 and a new
|		carry and number are generated and placed in elements 0 and 1.
|	The arrays mother1 and mother2 are filled with random 16 bit values
|		on first call of Mother by another generator.  mStart is the
switch.
|
|	Returns:
|	A 32 bit random number is obtained by combining the output of the
|		two generators and returned in *pSeed.  It is also scaled by
|		2^32-1 and returned as a double between 0 and 1
|
|	SEED:
|	The inital value of *pSeed may be any long value
|
|	Bob Wheeler 8/8/94
*/

static double Mother(unsigned long *pSeed)
{
	unsigned long  number, number1, number2;
	short n, *p;
	unsigned short sNumber;

		/* Initialize motheri with 9 random values the first time */
	if (mStart) {
		sNumber= *pSeed&m16Mask;   /* The low 16 bits */
		number= *pSeed&m31Mask;   /* Only want 31 bits */

		p=mother1;
		for (n=18;n--;) {
			number=30903*sNumber+(number>>16);   /* One line
multiply-with-cary */
			*p++=sNumber=number&m16Mask;
			if (n==9)
				p=mother2;
		}
		/* make cary 15 bits */
		mother1[0]&=m15Mask;
		mother2[0]&=m15Mask;
		mStart=0;
	}

		/* Move elements 1 to 8 to 2 to 9 */
	memmove(mother1+2,mother1+1,8*sizeof(short));
	memmove(mother2+2,mother2+1,8*sizeof(short));

		/* Put the carry values in numberi */
	number1=mother1[0];
	number2=mother2[0];

		/* Form the linear combinations */
	
number1+=1941*mother1[2]+1860*mother1[3]+1812*mother1[4]+1776*mother1[5]+
				
1492*mother1[6]+1215*mother1[7]+1066*mother1[8]+12013*mother1[9];
	
number2+=1111*mother2[2]+2222*mother2[3]+3333*mother2[4]+4444*mother2[5]+
				
5555*mother2[6]+6666*mother2[7]+7777*mother2[8]+9272*mother2[9];

		/* Save the high bits of numberi as the new carry */
	mother1[0]=number1/m16Long;
	mother2[0]=number2/m16Long;
		/* Put the low bits of numberi into motheri[1] */
	mother1[1]=m16Mask&number1;
	mother2[1]=m16Mask&number2;

		/* Combine the two 16 bit random numbers into one 32 bit */
	*pSeed=(((long)mother1[1])<<16)+(long)mother2[1];

		/* Return a double value between 0 and 1 */
	return ((double)*pSeed)/m32Double;
}

/*

*********************
Marsaglia's comments

		 Yet another RNG
Random number generators are frequently posted on
the network; my colleagues and I posted ULTRA in
1992 and, from the number of requests for releases
to use it in software packages, it seems to be
widely used.

I have long been interested in RNG's and several
of my early ones are used as system generators or
in statistical packages.

So why another one?  And why here?

Because I want to describe a generator, or
rather, a class of generators, so promising
I am inclined to call it

	The Mother of All Random Number Generators

and because the generator seems promising enough
to justify shortcutting the many months, even
years, before new developments are widely
known through publication in a journal.

This new class leads to simple, fast programs that
produce sequences with very long periods.  They
use multiplication, which experience has shown
does a better job of mixing bits than do +,- or
exclusive-or, and they do it with easily-
implemented arithmetic modulo a power of 2, unlike
arithmetic modulo a prime.  The latter, while
satisfactory, is difficult to implement.  But the
arithmetic here modulo 2^16 or 2^32 does not suffer
the flaws of ordinary congruential generators for
those moduli: trailing bits too regular.  On the
contrary, all bits of the integers produced by
this new method, whether leading or trailing, have
passed extensive tests of randomness.

Here is an idea of how it works, using, say, integers
of six decimal digits from which we return random 3-
digit integers.  Start with n=123456, the seed.

Then form a new n=672*456+123=306555 and return 555.
Then form a new n=672*555+306=373266 and return 266.
Then form a new n=672*266+373=179125 and return 125,

and so on.  Got it?  This is a multiply-with-carry
sequence x(n)=672*x(n-1)+ carry mod b=1000, where
the carry is the number of b's dropped in the
modular reduction. The resulting sequence of 3-
digit x's has period 335,999.  Try it.

No big deal, but that's just an example to give
the idea. Now consider the sequence of 16-bit
integers produced by the two C statements:

k=30903*(k&65535)+(k>>16); return(k&65535);

Notice that it is doing just what we did in the
example: multiply the bottom half (by 30903,
carefully chosen), add the top half and return the
new bottom.

That will produce a sequence of 16-bit integers
with period > 2^29, and if we concatenate two
such:
	  k=30903*(k&65535)+(k>>16);
	  j=18000*(j&65535)+(j>>16);
	  return((k<<16)+j);
we get a sequence of more than 2^59 32-bit integers
before cycling.

The following segment in a (properly initialized)
C procedure will generate more than 2^118
32-bit random integers from six random seed values
i,j,k,l,m,n:
		  k=30903*(k&65535)+(k>>16);
		  j=18000*(j&65535)+(j>>16);
		  i=29013*(i&65535)+(i>>16);
		  l=30345*(l&65535)+(l>>16);
		  m=30903*(m&65535)+(m>>16);
		  n=31083*(n&65535)+(n>>16);
		  return((k+i+m)>>16)+j+l+n);

And it will do it much faster than any of several
widely used generators designed to use 16-bit
integer arithmetic, such as that of Wichman-Hill
that combines congruential sequences for three
15-bit primes (Applied Statistics, v31, p188-190,
1982), period about 2^42.

I call these multiply-with-carry generators. Here
is an extravagant 16-bit example that is easily
implemented in C or Fortran. It does such a
thorough job of mixing the bits of the previous
eight values that it is difficult to imagine a
test of randomness it could not pass:

x[n]=12013x[n-8]+1066x[n-7]+1215x[n-6]+1492x[n-5]+1776x[n-4]
 +1812x[n-3]+1860x[n-2]+1941x[n-1]+carry mod 2^16.

The linear combination occupies at most 31 bits of
a 32-bit integer. The bottom 16 is the output, the
top 15 the next carry. It is probably best to
implement with 8 case segments. It takes 8
microseconds on my PC. Of course it just provides
16-bit random integers, but awfully good ones. For
32 bits you would have to combine it with another,
such as

x[n]=9272x[n-8]+7777x[n-7]+6666x[n-6]+5555x[n-5]+4444x[n-4]
	 +3333x[n-3]+2222x[n-2]+1111x[n-1]+carry mod 2^16.

Concatenating those two gives a sequence of 32-bit
random integers (from 16 random 16-bit seeds),
period about 2^250. It is so awesome it may merit
the Mother of All RNG's title.

The coefficients in those two linear combinations
suggest that it is easy to get long-period
sequences, and that is true.  The result is due to
Cemal Kac, who extended the theory we gave for
add-with-carry sequences: Choose a base b and give
r seed values x[1],...,x[r] and an initial 'carry'
c. Then the multiply-with-carry sequence

 x[n]=a1*x[n-1]+a2*x[n-2]+...+ar*x[n-r]+carry mod b,

where the new carry is the number of b's dropped
in the modular reduction, will have period the
order of b in the group of residues relatively
prime to m=ar*b^r+...+a1b^1-1.  Furthermore, the
x's are, in reverse order, the digits in the
expansion of k/m to the base b, for some 0<k<m.

In practice b=2^16 or b=2^32 allows the new
integer and the new carry to be the bottom and top
half of a 32- or 64-bit linear combination of  16-
or 32-bit integers.  And it is easy to find
suitable m's if you have a primality test:  just
search through candidate coefficients until you
get an m that is a safeprime---both m and (m-1)/2
are prime.  Then the period of the multiply-with-
carry sequence will be the prime (m-1)/2. (It
can't be m-1 because b=2^16 or 2^32 is a square.)

Here is an interesting simple MWC generator with
period> 2^92, for 32-bit arithmetic:

x[n]=1111111464*(x[n-1]+x[n-2]) + carry mod 2^32.

Suppose you have functions, say top() and bot(),
that give the top and bottom halves of a 64-bit
result.  Then, with initial 32-bit x, y and carry
c,  simple statements such as
	  y=bot(1111111464*(x+y)+c)
	  x=y
	  c=top(y)
will, repeated, give over 2^92 random 32-bit y's.

Not many machines have 64 bit integers yet.  But
most assemblers for modern CPU's permit access to
the top and bottom halves of a 64-bit product.

I don't know how to readily access the top half of
a 64-bit product in C.  Can anyone suggest how it
might be done? (in integer arithmetic)

George Marsaglia geo@stat.fsu.edu

*/

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

