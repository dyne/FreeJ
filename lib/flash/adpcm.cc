
#include "swf.h"

#ifdef RCSID
static char *rcsid = "$Id$";
#endif

// This file has been rearranged from the code posted
// on news:forums.macromedia.com by Jonathan Gay.
// Courtesy of Macromedia

//
// ADPCM tables
//

static const int indexTable2[2] = {
    -1, 2,
};

// Is this ok?
static const int indexTable3[4] = {
    -1, -1, 2, 4,
};

static const int indexTable4[8] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static const int indexTable5[16] = {
 -1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 4, 6, 8, 10, 13, 16,
};

static const int* indexTables[] = {
 indexTable2,
 indexTable3,
 indexTable4,
 indexTable5
};

static const int stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

long
Adpcm::GetBits(int n)
{
	if ( bitPos < n ) FillBuffer();

	assert(bitPos >= n);

	long v = ((unsigned long)bitBuf << (32-bitPos)) >> (32-n);
	bitPos -= n;

	return v;
}

long
Adpcm::GetSBits(int n)
{
	if ( bitPos < n ) FillBuffer();

	assert(bitPos >= n);

	long v = ((long)bitBuf << (32-bitPos)) >> (32-n);
	bitPos -= n;

	return v;
}

//
// The Decompressor
//

// Constructor
Adpcm::Adpcm(unsigned char *buffer, long isStereo)
{
	stereo = isStereo;
	src = buffer;

	nBits = 0; // flag that it is not inited
	nSamples = 0;

	bitPos = 0;
	bitBuf = 0;
}

void
Adpcm::FillBuffer()
{
	while ( bitPos <= 24 /*&& srcSize > 0*/ ) {
		bitBuf = (bitBuf<<8) | *src++;
		bitPos += 8;
	}
}

void
Adpcm::Decompress(short *dst, long n)
{
	if ( nBits == 0 ) {
		// Get the compression header
		nBits = (int)GetBits(2)+2;
	}

	const int* indexTable = indexTables[nBits-2];
	int k0 = 1 << (nBits-2);
	int signmask = 1 << (nBits-1);

	if ( !stereo ) {
		// Optimize for mono
		long		vp = valpred[0]; // maybe these can get into registers...
		int		ind = index[0];
		long		ns = nSamples;

		while ( n-- > 0 ) {
			ns++;

			if ( (ns & 0xFFF) == 1 ) {
				// Get a new block header
				*dst++ = (short)(vp = GetSBits(16));

				ind = (int)GetBits(6); // The first sample in a block does not have a delta
			} else {
				// Process a delta value
				int delta = (int)GetBits(nBits);

				// Compute difference and new predicted value
				// Computes 'vpdiff = (delta+0.5)*step/4'
				int step = stepsizeTable[ind];
				long vpdiff = 0;
				int k = k0;

				do {
					if ( delta & k )
					vpdiff += step;
					step >>= 1;
					k >>= 1;
				} while ( k );

				vpdiff += step; // add 0.5

				if ( delta & signmask ) // the sign bit
					vp -= vpdiff;
				else
					vp += vpdiff;

				// Find new index value
				ind += indexTable[delta&(~signmask)];

				if ( ind < 0 )
					ind = 0;
				else if ( ind > 88 )
					ind = 88;

				// clamp output value
				if ( vp != (short)vp )
					vp = vp < 0 ? -32768 : 32767;

				/* Step 7 - Output value */
				*dst++ = (short)vp;
			}
		}

		valpred[0] = vp;
		index[0] = ind;
		nSamples = ns;

	} else {
		int sn = stereo ? 2 : 1;

		// Stereo
		while ( n-- > 0 ) {

			nSamples++;

			if ( (nSamples & 0xFFF) == 1 ) {
				// Get a new block header
				for ( int i = 0; i < sn; i++ ) {

					*dst++ = (short)(valpred[i] = GetSBits(16));

					// The first sample in a block does not have a delta
					index[i] = (int)GetBits(6);
				}
			} else {
				// Process a delta value
				for ( int i = 0; i < sn; i++ ) {
					int delta = (int)GetBits(nBits);

					// Compute difference and new predicted value
					// Computes 'vpdiff = (delta+0.5)*step/4'

					int step = stepsizeTable[index[i]];
					long vpdiff = 0;
					int k = k0;

					do {
						if ( delta & k ) vpdiff += step;
						step >>= 1;
						k >>= 1;
					} while ( k );
					vpdiff += step; // add 0.5


					if ( delta & signmask ) // the sign bit
						valpred[i] -= vpdiff;
					else
						valpred[i] += vpdiff;

					// Find new index value
					index[i] += indexTable[delta&(~signmask)];

					if ( index[i] < 0 )
						index[i] = 0;
					else if ( index[i] > 88 )
						index[i] = 88;

					// clamp output value
					if ( valpred[i] != (short)valpred[i] )
						valpred[i] = valpred[i] < 0 ? -32768 : 32767;

					/* Step 7 - Output value */
					*dst++ = (short)valpred[i];
				}
			}
		}
	}
}
