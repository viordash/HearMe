#pragma once

void InitFft(uint32_t dataSize);
void FftAnalyze(float32_t *input, float32_t *magnitude, uint32_t magnitudeSize);
bool GetBinBorders(const float32_t *magnitude, uint32_t magnitudeSize, uint32_t binIndex, uint32_t *startIndex, uint32_t *endIndex);
void GetMaxMagnitude(const float32_t *pSrc, uint32_t blockSize, float32_t *pResult, uint32_t *pIndex);