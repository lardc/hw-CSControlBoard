// -----------------------------------------
// IQ Math utils
// ----------------------------------------

// Header
#include "IQmathUtils.h"


// Functions
//
#ifdef BOOT_FROM_FLASH_IQ16log
	#pragma CODE_SECTION(_IQ16log, "ramfuncs");
#endif
_iq16 _IQ16log(_iq16 x)
{
	Int32S t, y;

	y=0xa65af;
	if(x<0x00008000) x<<=16,              y-=0xb1721;
	if(x<0x00800000) x<<= 8,              y-=0x58b91;
	if(x<0x08000000) x<<= 4,              y-=0x2c5c8;
	if(x<0x20000000) x<<= 2,              y-=0x162e4;
	if(x<0x40000000) x<<= 1,              y-=0x0b172;
	t=x+(x>>1); if((t&0x80000000)==0) x=t,y-=0x067cd;
	t=x+(x>>2); if((t&0x80000000)==0) x=t,y-=0x03920;
	t=x+(x>>3); if((t&0x80000000)==0) x=t,y-=0x01e27;
	t=x+(x>>4); if((t&0x80000000)==0) x=t,y-=0x00f85;
	t=x+(x>>5); if((t&0x80000000)==0) x=t,y-=0x007e1;
	t=x+(x>>6); if((t&0x80000000)==0) x=t,y-=0x003f8;
	t=x+(x>>7); if((t&0x80000000)==0) x=t,y-=0x001fe;
	x=0x80000000-x;
	y-=x>>15;

	return y;
}
// ----------------------------------------

_iq FloatToIQ(Int32U Float)
{
	Int16U exponent;
	_iq result;

	// extract fraction
#if (GLOBAL_Q >= 23)
	result = (Float & 0x7fffff) << (GLOBAL_Q - 23);
#else
	result = (Float & 0x7fffff) >> (23 - GLOBAL_Q);
#endif

	// extract exponent
	exponent = (Float >> 23) & 0xFF;

	// unsigned result
	if (exponent >= 127)
		result = (result + _IQ(1)) << (exponent - 127);
	else
		result = (result + _IQ(1)) >> (127 - exponent);

	// sign
	if (Float >> 31) result = _IQmpy(result, _IQ(-1));

	return result;
}
// ----------------------------------------

Int32U IQToFloat(_iq iq)
{
	Int16S exponent;
	Int32U tmp;
	_iq iq_copy = iq;

	if (iq == 0)
	{
		return 0;
	}
	else
	{
		// invert to positive
		iq = (iq < 0) ? -iq : iq;

		// extract exponent
		tmp = iq >> GLOBAL_Q;
		exponent = 0;
		if (iq < _IQ(1))
		{
			while (iq < _IQ(1))
			{
				iq <<= 1;
				exponent--;
			}
		}
		else
		{
			while (tmp > 1)
			{
				tmp >>= 1;
				exponent++;
			}
		}

		// shift fraction
#if (GLOBAL_Q >= 23)
		iq >>= (GLOBAL_Q + exponent) - 23;
#else
		iq <<= 23 - (GLOBAL_Q + exponent);
#endif
		iq &= 0x7fffff;

		// write sign
		if (iq_copy < 0) iq |= 1l << 31;
		return (Int32U)(iq | (((Int32U)(127 + exponent)) << 23));
	}
}
// ----------------------------------------

// No more
