#ifndef _ADPCM_H_
#define _ADPCM_H_

class Adpcm {

	// Destination format - note we always decompress to 16 bit
	long		 stereo;
	int		 nBits;  // number of bits in each sample

	long		 valpred[2]; // Current state
	int		 index[2];

	long 		 nSamples; // number of samples decompressed so far

	// Parsing Info
	unsigned char 	*src;
	long		 bitBuf; // this should always contain at least 24 bits of data
	int		 bitPos;

	void FillBuffer();

	long GetBits(int n);

	long GetSBits(int n);

public:
	Adpcm(unsigned char *buffer, long isStereo);

	void Decompress(short * dst, long n); // return number of good samples
#ifdef DUMP
	void dump(BitStream *bs);
	void Compress(short *pcm, long n, int bits);
#endif
};

#endif /* _ADPCM_H_ */
