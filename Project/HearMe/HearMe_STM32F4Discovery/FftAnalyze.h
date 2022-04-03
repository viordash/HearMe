#pragma once

void InitFft(uint32_t dataSize);
void FftAnalyze(float32_t *input, float32_t *magnitude, uint32_t magnitudeSize);
bool GetBinBorders(float32_t *magnitude, uint32_t magnitudeSize, uint32_t binIndex, uint32_t *startIndex, uint32_t *endIndex);
