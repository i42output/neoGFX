/****************************************************************************
*
* NAME: smbPitchShift.cpp
* VERSION: 1.2
* HOME URL: http://blogs.zynaptiq.com/bernsee
* KNOWN BUGS: none
*
* SYNOPSIS: Routine for doing pitch shifting while maintaining
* duration using the Short Time Fourier Transform.
*
* DESCRIPTION: The routine takes a pitchShift factor value which is between 0.5
* (one octave down) and 2. (one octave up). A value of exactly 1 does not change
* the pitch. numSampsToProcess tells the routine how many samples in indata[0...
* numSampsToProcess-1] should be pitch shifted and moved to outdata[0 ...
* numSampsToProcess-1]. The two buffers can be identical (ie. it can process the
* data in-place). fftFrameSize defines the FFT frame size used for the
* processing. Typical values are 1024, 2048 and 4096. It may be any value <=
* MAX_FRAME_LENGTH but it MUST be a power of 2. osamp is the STFT
* oversampling factor which also determines the overlap between adjacent STFT
* frames. It should at least be 4 for moderate scaling ratios. A value of 32 is
* recommended for best quality. sampleRate takes the sample rate for the signal 
* in unit Hz, ie. 44100 for 44.1 kHz audio. The data passed to the routine in 
* indata[] should be in the range [-1.0, 1.0), which is also the output range 
* for the data, make sure you scale the data accordingly (for 16bit signed integers
* you would have to divide (and multiply) by 32768). 
*
* COPYRIGHT 1999-2015 Stephan M. Bernsee <s.bernsee [AT] zynaptiq [DOT] com>
*
* 						The Wide Open License (WOL)
*
* Permission to use, copy, modify, distribute and sell this software and its
* documentation for any purpose is hereby granted without fee, provided that
* the above copyright notice and this license appear in all source copies. 
* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF
* ANY KIND. See http://www.dspguru.com/wol.htm for more information.
*
*****************************************************************************/ 

#include <string.h>
#include <math.h>
#include <stdio.h>

#include <array>

#include <neogfx/core/numerical.hpp>

#include <ffts.h>

#define MAX_FRAME_LENGTH 8192

struct smbContext
{
	std::array<float, MAX_FRAME_LENGTH> gInFIFO = {};
	std::array<float, MAX_FRAME_LENGTH> gOutFIFO = {};
	std::array<float, MAX_FRAME_LENGTH * 2> alignas(16) gFFTworksp = {};
	std::array<float, MAX_FRAME_LENGTH * 2> alignas(16) gFFTworksp2 = {};
	std::array<float, MAX_FRAME_LENGTH / 2 + 1> gLastPhase = {};
	std::array<float, MAX_FRAME_LENGTH / 2 + 1> gSumPhase = {};
	std::array<float, MAX_FRAME_LENGTH * 2> gOutputAccum = {};
	std::array<float, MAX_FRAME_LENGTH> gAnaFreq = {};
	std::array<float, MAX_FRAME_LENGTH> gAnaMagn = {};
	std::array<float, MAX_FRAME_LENGTH> gSynFreq = {};
	std::array<float, MAX_FRAME_LENGTH> gSynMagn = {};
	long gRover = false;
	ffts_plan_t* fftsToComplexPlan = nullptr;
	ffts_plan_t* fftsToRealPlan = nullptr;
};

smbContext* smbCreateContext(long fftFrameSize)
{
	auto newContext = new smbContext{};
	newContext->fftsToComplexPlan = ffts_init_1d(fftFrameSize, FFTS_FORWARD);
	newContext->fftsToRealPlan = ffts_init_1d(fftFrameSize, FFTS_BACKWARD);
	return newContext;
}

void smbDestroyContext(smbContext* context)
{
	ffts_free(context->fftsToComplexPlan);
	ffts_free(context->fftsToRealPlan);
	delete context;
}

// -----------------------------------------------------------------------------------------------------------------


void smbPitchShift(smbContext* context, float pitchShift, long numSampsToProcess, long fftFrameSize, long osamp, float sampleRate, float *indata, float *outdata)
/*
	Routine smbPitchShift(). See top of file for explanation
	Purpose: doing pitch shifting while maintaining duration using the Short
	Time Fourier Transform.
	Author: (c)1999-2015 Stephan M. Bernsee <s.bernsee [AT] zynaptiq [DOT] com>
*/
{
	std::array<float, MAX_FRAME_LENGTH>& gInFIFO = context->gInFIFO;
	std::array<float, MAX_FRAME_LENGTH>& gOutFIFO = context->gOutFIFO;
	std::array<float, MAX_FRAME_LENGTH * 2>& gFFTworksp = context->gFFTworksp;
	std::array<float, MAX_FRAME_LENGTH * 2>& gFFTworksp2 = context->gFFTworksp2;
	std::array<float, MAX_FRAME_LENGTH / 2 + 1>& gLastPhase = context->gLastPhase;
	std::array<float, MAX_FRAME_LENGTH / 2 + 1>& gSumPhase = context->gSumPhase;
	std::array<float, MAX_FRAME_LENGTH * 2>& gOutputAccum = context->gOutputAccum;
	std::array<float, MAX_FRAME_LENGTH>& gAnaFreq = context->gAnaFreq;
	std::array<float, MAX_FRAME_LENGTH>& gAnaMagn = context->gAnaMagn;
	std::array<float, MAX_FRAME_LENGTH>& gSynFreq = context->gSynFreq;
	std::array<float, MAX_FRAME_LENGTH>& gSynMagn = context->gSynMagn;
	long& gRover = context->gRover;
	
	float magn, phase, tmp, window, real, imag;
	float freqPerBin, expct;
	long i,k, qpd, index, inFifoLatency, stepSize, fftFrameSize2;

	/* set up some handy variables */
	fftFrameSize2 = fftFrameSize/2;
	stepSize = fftFrameSize/osamp;
	freqPerBin = sampleRate/(float)fftFrameSize;
	expct = 2.0f * neogfx::math::pi<float>() * (float)stepSize / (float)fftFrameSize;
	inFifoLatency = fftFrameSize-stepSize;
	if (gRover == false) gRover = inFifoLatency;

	/* main processing loop */
	for (i = 0; i < numSampsToProcess; i++){

		/* As long as we have not yet collected enough data just read in */
		gInFIFO[gRover] = indata[i];
		outdata[i] = gOutFIFO[gRover-inFifoLatency];
		gRover++;

		/* now we have enough data for processing */
		if (gRover >= fftFrameSize) {
			gRover = inFifoLatency;

			/* do windowing and re,im interleave */
			for (k = 0; k < fftFrameSize;k++) {
				window = -0.5f * cos(2.0f * neogfx::math::pi<float>() * (float)k / (float)fftFrameSize) +0.5f;
				gFFTworksp[2*k] = gInFIFO[k] * window;
				gFFTworksp[2*k+1] = 0.;
			}


			/* ***************** ANALYSIS ******************* */
			/* do transform */
			ffts_execute(context->fftsToComplexPlan, &gFFTworksp[0], &gFFTworksp2[0]);
			std::memcpy(&gFFTworksp[0], &gFFTworksp2[0], fftFrameSize * 2 * sizeof(float));

			/* this is the analysis step */
			for (k = 0; k <= fftFrameSize2; k++) {

				/* de-interlace FFT buffer */
				real = gFFTworksp[2*k];
				imag = gFFTworksp[2*k+1];

				/* compute magnitude and phase */
				magn = 2.0f * sqrt(real*real + imag*imag);
				phase = atan2(imag,real);

				/* compute phase difference */
				tmp = phase - gLastPhase[k];
				gLastPhase[k] = phase;

				/* subtract expected phase difference */
				tmp -= (float)k*expct;

				/* map delta phase into +/- Pi interval */
				qpd = static_cast<long>(tmp / neogfx::math::pi<float>());
				if (qpd >= 0) qpd += qpd&1;
				else qpd -= qpd&1;
				tmp -= neogfx::math::pi<float>() * (float)qpd;

				/* get deviation from bin frequency from the +/- Pi interval */
				tmp = osamp * tmp / (2.0f * neogfx::math::pi<float>());

				/* compute the k-th partials' true frequency */
				tmp = (float)k*freqPerBin + tmp*freqPerBin;

				/* store magnitude and true frequency in analysis arrays */
				gAnaMagn[k] = magn;
				gAnaFreq[k] = tmp;

			}

			/* ***************** PROCESSING ******************* */
			/* this does the actual pitch shifting */
			memset(&gSynMagn[0], 0, fftFrameSize*sizeof(float));
			memset(&gSynFreq[0], 0, fftFrameSize*sizeof(float));
			for (k = 0; k <= fftFrameSize2; k++) { 
				index = static_cast<long>(k*pitchShift);
				if (index <= fftFrameSize2) { 
					gSynMagn[index] += gAnaMagn[k]; 
					gSynFreq[index] = gAnaFreq[k] * pitchShift; 
				} 
			}
			
			/* ***************** SYNTHESIS ******************* */
			/* this is the synthesis step */
			for (k = 0; k <= fftFrameSize2; k++) {

				/* get magnitude and true frequency from synthesis arrays */
				magn = gSynMagn[k];
				tmp = gSynFreq[k];

				/* subtract bin mid frequency */
				tmp -= (float)k*freqPerBin;

				/* get bin deviation from freq deviation */
				tmp /= freqPerBin;

				/* take osamp into account */
				tmp = 2.0f * neogfx::math::pi<float>() * tmp / osamp;

				/* add the overlap phase advance back in */
				tmp += (float)k*expct;

				/* accumulate delta phase to get bin phase */
				gSumPhase[k] += tmp;
				phase = gSumPhase[k];

				/* get real and imag part and re-interleave */
				gFFTworksp[2*k] = magn*cos(phase);
				gFFTworksp[2*k+1] = magn*sin(phase);
			} 

			/* zero negative frequencies */
			for (k = fftFrameSize+2; k < 2*fftFrameSize; k++) gFFTworksp[k] = 0.0f;

			/* do inverse transform */
			ffts_execute(context->fftsToRealPlan, &gFFTworksp[0], &gFFTworksp2[0]);
			std::memcpy(&gFFTworksp[0], &gFFTworksp2[0], fftFrameSize * 2 * sizeof(float));

			/* do windowing and add to output accumulator */ 
			for(k=0; k < fftFrameSize; k++) {
				window = -0.5f * cos(2.0f * neogfx::math::pi<float>() * (float)k / (float)fftFrameSize) + 0.5f;
				gOutputAccum[k] += 2.0f * window * gFFTworksp[2*k] / (fftFrameSize2 * osamp);
			}
			for (k = 0; k < stepSize; k++) gOutFIFO[k] = gOutputAccum[k];

			/* shift accumulator */
			memmove(&gOutputAccum[0], &gOutputAccum[stepSize], fftFrameSize*sizeof(float));

			/* move input FIFO */
			for (k = 0; k < inFifoLatency; k++) gInFIFO[k] = gInFIFO[k+stepSize];
		}
	}
}


// -----------------------------------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------