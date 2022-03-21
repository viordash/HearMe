#pragma once

#include "I2SPdmAudioIn.h"

void ReadAudioDigest0(PTAudioFragmentAnalysis *ppAudioFragmentAnalysis);
bool WriteAudioDigest0(PTAudioFragmentAnalysis pAudioFragmentAnalysis);

void ReadAudioDigest1(PTAudioFragmentAnalysis *ppAudioFragmentAnalysis);
bool WriteAudioDigest1(PTAudioFragmentAnalysis pAudioFragmentAnalysis);

void ReadAudioDigest2(PTAudioFragmentAnalysis *ppAudioFragmentAnalysis);
bool WriteAudioDigest2(PTAudioFragmentAnalysis pAudioFragmentAnalysis);
