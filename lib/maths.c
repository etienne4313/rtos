/*
   The following are all fast versions of specific divisions
Ref: http://www.hackersdelight.org/divcMore.pdf
 */
unsigned int divu10(unsigned int n)
{
	unsigned long q, r;
	q = (n >> 1) + (n >> 2);
	q = q + (q >> 4);
	q = q + (q >> 8);
	q = q + (q >> 16);
	q = q >> 3;
	r = n - (q * 10);
	return q + ((r + 6) >> 4);
}
