//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* public domain, Simple, Minimalistic GIF writer - http://jonolick.com
 *
 * Quick Notes:
 * 	Supports only 4 component input, alpha is currently ignored. (RGBX)
 *
 * Latest revisions:
 * 	1.00 (2015-11-03) initial release
 *
 * Basic usage:
 *	char *frame = new char[128*128*4]; // 4 component. RGBX format, where X is unused 
 *	jo_gif_t gif = jo_gif_start("foo.gif", 128, 128, 0, 32); 
 *	jo_gif_frame(&gif, frame, 4, false); // frame 1
 *	jo_gif_frame(&gif, frame, 4, false); // frame 2
 *	jo_gif_frame(&gif, frame, 4, false); // frame 3, ...
 *	jo_gif_end(&gif);
 * */

#ifndef JO_INCLUDE_GIF_H
#define JO_INCLUDE_GIF_H

// To get a header file for this, either cut and paste the header,
// or create jo_gif->h, #define JO_GIF_HEADER_FILE_ONLY, and
// then include jo_gif->cpp from it.

typedef struct {
	struct byte_stream_descriptor_t *fp;
	unsigned char palette[0x300];
	short width, height, repeat;
	int numColors, palSize;
	int frame;
} jo_gif_t;

// width/height	| the same for every frame
// repeat       | 0 = loop forever, 1 = loop once, etc...
// palSize		| must be power of 2 - 1. so, 255 not 256.
extern jo_gif_t *jo_gif_start(short width, short height, short repeat);

// gif			| the state (returned from jo_gif_start)
// rgba         | the pixels
// delayCsec    | amount of time in between frames (in centiseconds)
extern void jo_gif_frame(jo_gif_t *gif, unsigned char *rgba, short delayCsec);

// gif          | the state (returned from jo_gif_start)
extern void jo_gif_end(jo_gif_t *gif);

#endif

#ifndef JO_GIF_HEADER_FILE_ONLY

#if defined(_MSC_VER) && _MSC_VER >= 0x1400
#define _CRT_SECURE_NO_WARNINGS // suppress warnings about fopen()
#endif

// Based on NeuQuant algorithm
static void jo_gif_quantize(unsigned char *rgba, int rgbaSize, int sample, unsigned char *map, int numColors) {
	// defs for freq and bias
	const int intbiasshift = 16; /* bias for fractions */
	const int intbias = (((int) 1) << intbiasshift);
	const int gammashift = 10; /* gamma = 1024 */
	const int betashift = 10;
	const int beta = (intbias >> betashift); /* beta = 1/1024 */
	const int betagamma = (intbias << (gammashift - betashift));

	// defs for decreasing radius factor
	const int radiusbiasshift = 6; /* at 32.0 biased by 6 bits */
	const int radiusbias = (((int) 1) << radiusbiasshift);
	const int radiusdec = 30; /* factor of 1/30 each cycle */

	// defs for decreasing alpha factor
	const int alphabiasshift = 10; /* alpha starts at 1.0 */
	const int initalpha = (((int) 1) << alphabiasshift);

	// radbias and alpharadbias used for radpower calculation
	const int radbiasshift = 8;
	const int radbias = (((int) 1) << radbiasshift);
	const int alpharadbshift = (alphabiasshift + radbiasshift);
	const int alpharadbias = (((int) 1) << alpharadbshift);

	sample = sample < 1 ? 1 : sample > 30 ? 30 : sample;
	int network[256][3];
	int bias[256] = {}, freq[256];
	for(int i = 0; i < numColors; ++i) {
		// Put nurons evenly through the luminance spectrum.
		network[i][0] = network[i][1] = network[i][2] = (i << 12) / numColors;
		freq[i] = intbias / numColors; 
	}
	// Learn
	{
		const int primes[5] = {499, 491, 487, 503};
		int step = 4;
		for(int i = 0; i < 4; ++i) {
			if(rgbaSize > primes[i] * 4 && (rgbaSize % primes[i])) { // TODO/Error? primes[i]*4?
				step = primes[i] * 4;
			}
		}
		sample = step == 4 ? 1 : sample;

		int alphadec = 30 + ((sample - 1) / 3);
		int samplepixels = rgbaSize / (4 * sample);
		int delta = samplepixels / 100;
		int alpha = initalpha;
		delta = delta == 0 ? 1 : delta;

		int radius = (numColors >> 3) * radiusbias;
		int rad = radius >> radiusbiasshift;
		rad = rad <= 1 ? 0 : rad;
		int radSq = rad*rad;
		int radpower[32];
		for (int i = 0; i < rad; i++) {
			radpower[i] = alpha * (((radSq - i * i) * radbias) / radSq);
		}

		// Randomly walk through the pixels and relax neurons to the "optimal" target.
		for(int i = 0, pix = 0; i < samplepixels;) {
			int r = rgba[pix + 0] << 4;
			int g = rgba[pix + 1] << 4;
			int b = rgba[pix + 2] << 4;
			int j = -1;
			{
				// finds closest neuron (min dist) and updates freq 
				// finds best neuron (min dist-bias) and returns position 
				// for frequently chosen neurons, freq[k] is high and bias[k] is negative 
				// bias[k] = gamma*((1/numColors)-freq[k]) 

				int bestd = 0x7FFFFFFF, bestbiasd = 0x7FFFFFFF, bestpos = -1;
				for (int k = 0; k < numColors; k++) {
					int *n = network[k];
					int dist = abs(n[0] - r) + abs(n[1] - g) + abs(n[2] - b);
					if (dist < bestd) {
						bestd = dist;
						bestpos = k;
					}
					int biasdist = dist - ((bias[k]) >> (intbiasshift - 4));
					if (biasdist < bestbiasd) {
						bestbiasd = biasdist;
						j = k;
					}
					int betafreq = freq[k] >> betashift;
					freq[k] -= betafreq;
					bias[k] += betafreq << gammashift;
				}
				freq[bestpos] += beta;
				bias[bestpos] -= betagamma;
			}

			// Move neuron j towards biased (b,g,r) by factor alpha
			network[j][0] -= (network[j][0] - r) * alpha / initalpha;
			network[j][1] -= (network[j][1] - g) * alpha / initalpha;
			network[j][2] -= (network[j][2] - b) * alpha / initalpha;
			if (rad != 0) {
				// Move adjacent neurons by precomputed alpha*(1-((i-j)^2/[r]^2)) in radpower[|i-j|]
				int lo = j - rad;
				lo = lo < -1 ? -1 : lo;
				int hi = j + rad;
				hi = hi > numColors ? numColors : hi;
				for(int jj = j+1, m=1; jj < hi; ++jj) {
					int a = radpower[m++];
					network[jj][0] -= (network[jj][0] - r) * a / alpharadbias;
					network[jj][1] -= (network[jj][1] - g) * a / alpharadbias;
					network[jj][2] -= (network[jj][2] - b) * a / alpharadbias;
				}
				for(int k = j-1, m=1; k > lo; --k) {
					int a = radpower[m++];
					network[k][0] -= (network[k][0] - r) * a / alpharadbias;
					network[k][1] -= (network[k][1] - g) * a / alpharadbias;
					network[k][2] -= (network[k][2] - b) * a / alpharadbias;
				}
			}

			pix += step;
			pix = pix >= rgbaSize ? pix - rgbaSize : pix;

			// every 1% of the image, move less over the following iterations.
			if(++i % delta == 0) {
				alpha -= alpha / alphadec;
				radius -= radius / radiusdec;
				rad = radius >> radiusbiasshift;
				rad = rad <= 1 ? 0 : rad;
				radSq = rad*rad;
				for (j = 0; j < rad; j++) {
					radpower[j] = alpha * ((radSq - j * j) * radbias / radSq);
				}
			}
		}
	}
	// Unbias network to give byte values 0..255
	for (int i = 0; i < numColors; i++) {
		map[i*3+0] = network[i][0] >>= 4;
		map[i*3+1] = network[i][1] >>= 4;
		map[i*3+2] = network[i][2] >>= 4;
	}
}

typedef struct {
	struct byte_stream_descriptor_t *fp;
	int numBits;
	unsigned char buf[256];
	unsigned char idx;
	unsigned tmp;
	int outBits;
	int curBits;
} jo_gif_lzw_t;

static void jo_gif_lzw_write(jo_gif_lzw_t *s, int code) {
	s->outBits |= code << s->curBits;
	s->curBits += s->numBits;
	while(s->curBits >= 8) {
		s->buf[s->idx++] = s->outBits & 255;
		s->outBits >>= 8;
		s->curBits -= 8;
		if (s->idx >= 255) {
			add_byte_to_byte_stream(s->fp, s->idx);
			add_bytes_to_byte_stream(s->fp, s->buf, s->idx);
			s->idx = 0;
		}
	}
}

static void jo_gif_lzw_encode(unsigned char *in, int len, struct byte_stream_descriptor_t *fp) {
	jo_gif_lzw_t state = {fp, 9};
	int maxcode = 511;

	// Note: 30k stack space for dictionary =|
	const int hashSize = 5003;
	short codetab[hashSize];
	int hashTbl[hashSize];
	memset(hashTbl, 0xFF, sizeof(hashTbl));

	jo_gif_lzw_write(&state, 0x100);

	int free_ent = 0x102;
	int ent = *in++;
CONTINUE: 
	while (--len) {
		int c = *in++;
		int fcode = (c << 12) + ent;
		int key = (c << 4) ^ ent; // xor hashing
		while(hashTbl[key] >= 0) {
			if(hashTbl[key] == fcode) {
				ent = codetab[key];
				goto CONTINUE;
			}
			++key;
			key = key >= hashSize ? key - hashSize : key;
		}
		jo_gif_lzw_write(&state, ent);
		ent = c;
		if(free_ent < 4096) {
			if(free_ent > maxcode) {
				++state.numBits;
				if(state.numBits == 12) {
					maxcode = 4096;
				} else {
					maxcode = (1<<state.numBits)-1;
				}
			}
			codetab[key] = free_ent++;
			hashTbl[key] = fcode;
		} else {
			memset(hashTbl, 0xFF, sizeof(hashTbl));
			free_ent = 0x102;
			jo_gif_lzw_write(&state, 0x100);
			state.numBits = 9;
			maxcode = 511;
		}
	}
	jo_gif_lzw_write(&state, ent);
	jo_gif_lzw_write(&state, 0x101);
	jo_gif_lzw_write(&state, 0);
	if(state.idx) {
		add_byte_to_byte_stream(fp, state.idx);
		add_bytes_to_byte_stream(fp, state.buf, state.idx);
	}
}

static int jo_gif_clamp(int a, int b, int c) { return a < b ? b : a > c ? c : a; }

jo_gif_t *jo_gif_start(short width, short height, short repeat) {
	jo_gif_t *gif = (jo_gif_t *) (calloc(sizeof(jo_gif_t)));
	gif->width = width;
	gif->height = height;
	gif->repeat = repeat;
	gif->numColors = 255;
 gif->palSize = 0x7;

	gif->fp = create_byte_stream(BYTE_STREAM_1_MB_BLOCK);
	add_bytes_to_byte_stream(gif->fp, "GIF89a", 6);
	// Logical Screen Descriptor
	add_word_to_byte_stream(gif->fp, gif->width);
	add_word_to_byte_stream(gif->fp, gif->height);
	add_byte_to_byte_stream(gif->fp, 0xF0 | gif->palSize);
	add_bytes_to_byte_stream(gif->fp, "\x00\x00", 2); // bg color index (unused), aspect ratio
	return gif;
}

void jo_gif_frame(jo_gif_t *gif, unsigned char * rgba, short delayCsec) {
	short width = gif->width;
	short height = gif->height;
	int size = width * height;

 reset_timer();
 log("GIF: start");

	unsigned char localPalTbl[0x300];
	unsigned char *palette = gif->frame == 0 ? gif->palette : localPalTbl;
	if(gif->frame == 0) {
		jo_gif_quantize(rgba, size*4, 1, palette, gif->numColors);		
	}

 log("\n"); log_var(get_timer_value_in_microseconds());

	unsigned char *indexedPixels = (unsigned char *)malloc(size);
	{
		unsigned char *ditheredPixels = (unsigned char*)malloc(size*4);
		memcpy(ditheredPixels, rgba, size*4);
		for(int k = 0; k < size*4; k+=4) {
			int rgb[3] = { ditheredPixels[k+0], ditheredPixels[k+1], ditheredPixels[k+2] };
			int bestd = 0x7FFFFFFF, best = -1;
			// TODO: exhaustive search. do something better.
			for(int i = 0; i < gif->numColors; ++i) {
				int bb = palette[i*3+0]-rgb[0];
				int gg = palette[i*3+1]-rgb[1];
				int rr = palette[i*3+2]-rgb[2];
				int d = bb*bb + gg*gg + rr*rr;
				if(d < bestd) {
					bestd = d;
					best = i;
				}
			}
			indexedPixels[k/4] = best;
			int diff[3] = { ditheredPixels[k+0] - palette[indexedPixels[k/4]*3+0], ditheredPixels[k+1] - palette[indexedPixels[k/4]*3+1], ditheredPixels[k+2] - palette[indexedPixels[k/4]*3+2] };
			// Floyd-Steinberg Error Diffusion
			// TODO: Use something better -- http://caca.zoy.org/study/part3.html
			if(k+4 < size*4) { 
				ditheredPixels[k+4+0] = (unsigned char)jo_gif_clamp(ditheredPixels[k+4+0]+(diff[0]*7/16), 0, 255); 
				ditheredPixels[k+4+1] = (unsigned char)jo_gif_clamp(ditheredPixels[k+4+1]+(diff[1]*7/16), 0, 255); 
				ditheredPixels[k+4+2] = (unsigned char)jo_gif_clamp(ditheredPixels[k+4+2]+(diff[2]*7/16), 0, 255); 
			}
			if(k+width*4+4 < size*4) { 
				for(int i = 0; i < 3; ++i) {
					ditheredPixels[k-4+width*4+i] = (unsigned char)jo_gif_clamp(ditheredPixels[k-4+width*4+i]+(diff[i]*3/16), 0, 255); 
					ditheredPixels[k+width*4+i] = (unsigned char)jo_gif_clamp(ditheredPixels[k+width*4+i]+(diff[i]*5/16), 0, 255); 
					ditheredPixels[k+width*4+4+i] = (unsigned char)jo_gif_clamp(ditheredPixels[k+width*4+4+i]+(diff[i]*1/16), 0, 255); 
				}
			}
		}
		free(ditheredPixels);
	}
	if(gif->frame == 0) {
		// Global Color Table
		add_bytes_to_byte_stream(gif->fp, palette, 3*(1<<(gif->palSize+1)));
		if(gif->repeat >= 0) {
			// Netscape Extension
			add_bytes_to_byte_stream(gif->fp, "\x21\xff\x0bNETSCAPE2.0\x03\x01", 16);
			add_word_to_byte_stream(gif->fp, gif->repeat); // loop count (extra iterations, 0=repeat forever)
			add_byte_to_byte_stream(gif->fp, 0); // block terminator
		}
	}
	// Graphic Control Extension
	add_bytes_to_byte_stream(gif->fp, "\x21\xf9\x04\x00", 4);
	add_word_to_byte_stream(gif->fp, delayCsec); // delayCsec x 1/100 sec
	add_bytes_to_byte_stream(gif->fp, "\x00\x00", 2); // transparent color index (first byte), currently unused
	// Image Descriptor
	add_bytes_to_byte_stream(gif->fp, "\x2c\x00\x00\x00\x00", 5); // header, x,y
	add_word_to_byte_stream(gif->fp, width);
	add_word_to_byte_stream(gif->fp, height);
	if (gif->frame == 0) {
		add_byte_to_byte_stream(gif->fp, 0);
	}
 else {
		add_byte_to_byte_stream(gif->fp, 0x80 | gif->palSize);
		add_bytes_to_byte_stream(gif->fp, palette, 3*(1<<(gif->palSize+1)));
	}
	add_byte_to_byte_stream(gif->fp, 8); // block terminator
	jo_gif_lzw_encode(indexedPixels, size, gif->fp);
	add_byte_to_byte_stream(gif->fp, 0); // block terminator
	++gif->frame;
	free(indexedPixels);
}

void jo_gif_end(jo_gif_t *gif) {
	add_byte_to_byte_stream(gif->fp, 0x3b); // gif trailer
}
#endif