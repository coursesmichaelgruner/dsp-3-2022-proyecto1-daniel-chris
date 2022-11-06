/*
 * Copyright (C) 2010
 * Pablo Alvarado
 *
 * This file is part of the EL-5805 Digital Signal Processing Lecture at
 * the Costa Rica Institute of Technology
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the authors nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** 
 * \file   tarea04.c
 *         Contains the processing function
 * \author Pablo Alvarado
 * \author Student
 * \date   August 9th, 2010
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/**
 * You may need some global values 
 */
#define f2p(f,Fs) ((f)*2.0*M_PI/(Fs))
static float lowShift;
static float highShift;
static float lowPhase;
static float highPhase;

static float d[7][3];
static float coef[7];
#define DTMF_FRAME 990
#define THRESHHOLD 0.1
static int keyIndex;
static char key;
static char prevKey;
static int gFs;

void gInit(unsigned int Fs, float *d, float *c);
void gFilter (float *d, float in, float coeff);
void gFreqDetect(int *keyIndex, float *d, float *c, float threshhold);
void checkKey(char* key, int keyIndex);
void dtmfFreq(const float *data, float *d, float *c, int frameSize);


/**
 * This method is called before the real processing starts.
 * You may use it to initialize whatever you need to.
 */
void init(const unsigned int Fs) {
  // Decoder init
  gInit(Fs, &d[0][0], coef);

  // Encoder init
  lowShift=f2p(697.0, Fs);
  highShift=f2p(1209.0,Fs);
  lowPhase=0;
  highPhase=0;

  key='\0';
  prevKey='\0';
}

/**
 * Process a buffer of data.
 *
 * @param Fs sampling rate
 * @param channels number of channels per frame
 * @param frames   number of frames per buffer
 * @param buffer   input and output buffer
 *
 * @return zero if error, or 1 otherwise.
 */
int process(const unsigned int Fs,
            const int nframes,
            const float* in,
            float* out) {
  /*
   * PUT YOUR CODE IN HERE
   */

  /* This line just copies the data from input to output. REMOVE IT! */
  //memcpy(out, in, sizeof(float)*nframes);

  int n;

  // Encode
  for (n=0; n<nframes; n++) {
    out[n]=0.25*(cos(lowPhase)+cos(highPhase));
    lowPhase+=lowShift;
    if (lowPhase>2.0*M_PI) lowPhase-=2.0*M_PI;
    highPhase+=highShift;
    if (highPhase>2.0*M_PI) highPhase-=2.0*M_PI;
  }

  // Decode
  dtmfFreq(in, &d[0][0], coef, nframes);
  keyIndex=0;
  gFreqDetect(&keyIndex, &d[0][0], coef, THRESHHOLD);
  checkKey(&key, keyIndex);
  if (key) printf("Key: %c\n",key);

  /* Debug stuff */
  /*
  printf("In: %.5f, Out: %.5f\n",*in,*out);
  fflush(stdout);
  */
  return 0; // everything is ok 
}

void gInit(unsigned int Fs, float *d, float *c) {
  for (int i=0;i<7;i++) {
    for (int j=0;j<3;j++) {
      *d++ = 0;
    }
  }

  c[0]=2*cos(2*M_PI*697/Fs);
  c[1]=2*cos(2*M_PI*770/Fs);
  c[2]=2*cos(2*M_PI*852/Fs);
  c[3]=2*cos(2*M_PI*941/Fs);
  c[4]=2*cos(2*M_PI*1209/Fs);
  c[5]=2*cos(2*M_PI*1336/Fs);
  c[6]=2*cos(2*M_PI*1477/Fs);

  gFs=Fs;
}

void gFilter (float* d, float in, float coeff) {
  d[0] = in + coeff * d[1] - d[2];
  d[2] = d[1];
  d[1] = d[0];
}

float computeOutput(float *d, float coeff) {
  float out;

  out = d[2] * d[2] + d[1] * d[1] - coeff * d[1] * d[2];

  d[1] = 0;
  d[2] = 0;

  return out;
}

void gFreqDetect(int *keyIndex, float *d, float *c, float threshhold) {
  int i,j;

  for (j=1, i=0; i<7; i++) {
    float o=computeOutput(d, c[i]);
//printf("%f\t",o);
    if (o>threshhold) {
      *keyIndex |= j;
    } else {
      *keyIndex &= ~j;
    }
    j <<= 1;
    d += 3;
  }
//printf("\n");
}

enum {
  KEY_1 = 0x10|0x01,
  KEY_2 = 0x20|0x01,
  KEY_3 = 0x40|0x01,
  KEY_4 = 0x10|0x02,
  KEY_5 = 0x20|0x02,
  KEY_6 = 0x40|0x02,
  KEY_7 = 0x10|0x04,
  KEY_8 = 0x20|0x04,
  KEY_9 = 0x40|0x04,
  KEY_STAR = 0x10|0x08,
  KEY_0 = 0x20|0x08,
  KEY_POUND = 0x40|0x08,
};

void checkKey(char* key, int keyIndex) {
  switch(keyIndex) {
    case KEY_1: *key='1'; break;
    case KEY_2: *key='2'; break;
    case KEY_3: *key='3'; break;
    case KEY_4: *key='4'; break;
    case KEY_5: *key='5'; break;
    case KEY_6: *key='6'; break;
    case KEY_7: *key='7'; break;
    case KEY_8: *key='8'; break;
    case KEY_9: *key='9'; break;
    case KEY_0: *key='0'; break;
    case KEY_STAR: *key='*'; break;
    case KEY_POUND: *key='#'; break;
    default: *key='\0';
  }
}

void dtmfFreq(const float *data, float *d, float *c, int frameSize) {
  int i,j;
  float x;
  float* dd;

  for (j=0;j<frameSize;j++) {
    x=*data++;
    dd=d;
    for (i=0;i<7;i++) {
      gFilter(dd,x,c[i]);
      dd+=3;
    }
  }
}
