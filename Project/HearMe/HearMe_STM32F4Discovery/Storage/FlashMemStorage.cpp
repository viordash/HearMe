#include "Board.h"
#include <memory.h>
#include "FlashMemStorage.h"

#define FLASH_VOLTAGE_RANGE FLASH_VOLTAGE_RANGE_3

__attribute__((section(".Storage0"))) uint8_t Storage0[16384];
__attribute__((section(".Storage1"))) uint8_t Storage1[16384];
__attribute__((section(".Storage2"))) uint8_t Storage2[16384];

#define FLASH_TIMEOUT_VALUE 50000U /* 50 s */

static bool WriteStorageRawData(uint32_t *dstData, uint32_t *srcData, uint32_t size32);
static int GetFlashMemSectorNumber(uint32_t address);

static bool FlashErase(uint32_t address, uint32_t wordsCount);
static bool FlashProgram(uint32_t dstData, uint32_t *srcData, uint32_t wordsCount);
static bool FLASH_ErasePage(uint32_t sector);
static bool FLASH_Program_Word(uint32_t address, uint32_t data);

void ReadAudioDigest0(PTAudioDigest *ppAudioDigest) {
	*ppAudioDigest = (PTAudioDigest)Storage0;
}
bool WriteAudioDigest0(PTAudioDigest pAudioDigest) {
	return WriteStorageRawData((uint32_t *)Storage0, (uint32_t *)pAudioDigest, sizeof(*pAudioDigest) / sizeof(uint32_t));
}

void ReadAudioDigest1(PTAudioDigest *ppAudioDigest) {
	*ppAudioDigest = (PTAudioDigest)Storage1;
}
bool WriteAudioDigest1(PTAudioDigest pAudioDigest) {
	return WriteStorageRawData((uint32_t *)Storage1, (uint32_t *)pAudioDigest, sizeof(*pAudioDigest) / sizeof(uint32_t));
}

void ReadAudioDigest2(PTAudioDigest *ppAudioDigest) {
	*ppAudioDigest = (PTAudioDigest)Storage2;
}
bool WriteAudioDigest2(PTAudioDigest pAudioDigest) {
	return WriteStorageRawData((uint32_t *)Storage2, (uint32_t *)pAudioDigest, sizeof(*pAudioDigest) / sizeof(uint32_t));
}

static bool WriteStorageRawData(uint32_t *dstData, uint32_t *srcData, uint32_t size32) {
	bool res = true;

	HAL_FLASH_Unlock();
	if (FLASH_ErasePage(GetFlashMemSectorNumber((uint32_t)dstData))) {
		for (size_t i = 0; i < size32; i++) {
			if (!FLASH_Program_Word((uint32_t)dstData, *srcData)) {
				res = false;
				break;
			}
			dstData++;
			srcData++;
		}
	}
	HAL_FLASH_Lock();
	return res;
}

static int GetFlashMemSectorNumber(uint32_t address) {
	if (address > FLASH_END) {
		return -1;
	}
	if (address >= (FLASH_END - (128 * 1 * 1024)) + 1) {
		return FLASH_SECTOR_11;
	}
	if (address >= (FLASH_END - (128 * 2 * 1024)) + 1) {
		return FLASH_SECTOR_10;
	}
	if (address >= (FLASH_END - (128 * 3 * 1024)) + 1) {
		return FLASH_SECTOR_9;
	}
	if (address >= (FLASH_END - (128 * 4 * 1024)) + 1) {
		return FLASH_SECTOR_8;
	}
	if (address >= (FLASH_END - (128 * 5 * 1024)) + 1) {
		return FLASH_SECTOR_7;
	}
	if (address >= (FLASH_END - (128 * 6 * 1024)) + 1) {
		return FLASH_SECTOR_6;
	}
	if (address >= (FLASH_END - (128 * 7 * 1024)) + 1) {
		return FLASH_SECTOR_5;
	}
	if (address >= (FLASH_END - ((64 * 1 * 1024) + (128 * 7 * 1024))) + 1) {
		return FLASH_SECTOR_4;
	}
	if (address >= (FLASH_END - ((16 * 1 * 1024) + (64 * 1 * 1024) + (128 * 7 * 1024))) + 1) {
		return FLASH_SECTOR_3;
	}
	if (address >= (FLASH_END - ((16 * 2 * 1024) + (64 * 1 * 1024) + (128 * 7 * 1024))) + 1) {
		return FLASH_SECTOR_2;
	}
	if (address >= (FLASH_END - ((16 * 3 * 1024) + (64 * 1 * 1024) + (128 * 7 * 1024))) + 1) {
		return FLASH_SECTOR_1;
	}
	if (address >= (FLASH_END - ((16 * 4 * 1024) + (64 * 1 * 1024) + (128 * 7 * 1024))) + 1) {
		return FLASH_SECTOR_0;
	}
	return -1;
}

static bool FLASH_ErasePage(uint32_t sector) {
	if (!IS_FLASH_SECTOR(sector)) {
		return false;
	}

	HAL_StatusTypeDef status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE); /* Wait for last operation to be completed */
	if (status != HAL_OK) {
		return false;
	}

	uint32_t tmp_psize = 0U;

	if (FLASH_VOLTAGE_RANGE == FLASH_VOLTAGE_RANGE_1) {
		tmp_psize = FLASH_PSIZE_BYTE;
	} else if (FLASH_VOLTAGE_RANGE == FLASH_VOLTAGE_RANGE_2) {
		tmp_psize = FLASH_PSIZE_HALF_WORD;
	} else if (FLASH_VOLTAGE_RANGE == FLASH_VOLTAGE_RANGE_3) {
		tmp_psize = FLASH_PSIZE_WORD;
	} else {
		tmp_psize = FLASH_PSIZE_DOUBLE_WORD;
	}

	CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE); /* If the previous operation is completed, proceed to erase the sector */
	FLASH->CR |= tmp_psize;
	CLEAR_BIT(FLASH->CR, FLASH_CR_SNB);
	FLASH->CR |= FLASH_CR_SER | (sector << POSITION_VAL(FLASH_CR_SNB));
	FLASH->CR |= FLASH_CR_STRT;

	status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE); /* Wait for last operation to be completed */

	CLEAR_BIT(FLASH->CR, (FLASH_CR_SER | FLASH_CR_SNB)); /* If the erase operation is completed, disable the SER and SNB Bits */

	return status == HAL_OK;
}

static bool FLASH_Program_Word(uint32_t address, uint32_t data) {

	assert_param(IS_FLASH_ADDRESS(Address)); /* Check the parameters */

	HAL_StatusTypeDef status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE); /* Wait for last operation to be completed */
	if (status != HAL_OK) {
		return false;
	}

	/* If the previous operation is completed, proceed to program the new data */
	CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
	FLASH->CR |= FLASH_PSIZE_WORD;
	FLASH->CR |= FLASH_CR_PG;

	*(__IO uint32_t *)address = data;

	status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE); /* Wait for last operation to be completed */

	FLASH->CR &= (~FLASH_CR_PG); /* If the program operation is completed, disable the PG Bit */
	return status == HAL_OK;
}

bool FlashErase(uint32_t address, uint32_t wordsCount) {
	bool res = true;
	uint32_t sectorToBeErase = GetFlashMemSectorNumber((uint32_t)address);
	uint32_t lastPageToBeErase = GetFlashMemSectorNumber((uint32_t)address + (wordsCount * sizeof(uint32_t) - 1));
	HAL_FLASH_Unlock();
	while (sectorToBeErase <= lastPageToBeErase) {
		if (!FLASH_ErasePage(sectorToBeErase)) {
			res = false;
			break;
		}
		sectorToBeErase++;
	}
	HAL_FLASH_Lock();
	return res;
}

bool FlashProgram(uint32_t dstData, uint32_t *srcData, uint32_t wordsCount) {
	bool res = true;
	HAL_FLASH_Unlock();
	for (size_t i = 0; i < wordsCount; i++) {
		if (!FLASH_Program_Word(dstData, *srcData)) {
			res = false;
			break;
		}
		dstData += sizeof(uint32_t);
		srcData++;
	}
	HAL_FLASH_Lock();
	return res;
}