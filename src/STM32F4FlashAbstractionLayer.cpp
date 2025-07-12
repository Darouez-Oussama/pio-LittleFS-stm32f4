/* 
 **************************************************************************************************
 *
 * @file    : STM32F4FlashAbstractionLayer.cpp
 * @author  : Oussama Darouez
 * @version : 1.0
 * @date    : July 2025
 * @brief   : STM32F4 Flash Abstraction Layer Implementation for LittleFS
 * 
 **************************************************************************************************
 * 
 * @project  : stm32_littlefs
 * @board    : nucleo_f401re
 * @compiler : gcc-arm-none-eabi
 * 
 **************************************************************************************************
 *
 */

/*-----------------------------------------------------------------------------------------------*/
/* Includes                                                                                      */
/*-----------------------------------------------------------------------------------------------*/
#include <Arduino.h>
#include "STM32F4FlashAbstractionLayer.h"

/*-----------------------------------------------------------------------------------------------*/
/* Private Defines                                                                               */
/*-----------------------------------------------------------------------------------------------*/
#define FLASH_SECTOR_SIZE_BYTES   (128U * 1024U)  // 128 KB sectors for STM32F401RE
#define FLASH_TOTAL_SIZE_BYTES    (256U * 1024U)  // 256 KB for LittleFS
#define BLOCK_SIZE_BYTES          (1024U)         // LittleFS block size
#define LITTLE_FS_STARTIN_ADDRESS (0x08040000U)   // Sector 6 for STM32F401RE

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */

/*-----------------------------------------------------------------------------------------------*/
/* Global Variables                                                                              */
/*-----------------------------------------------------------------------------------------------*/
uint32_t ef_err_port_cnt = 0;
uint32_t on_ic_write_cnt = 0;
uint32_t on_ic_read_cnt = 0;

/*-----------------------------------------------------------------------------------------------*/
/* Public methods                                                                                */
/*-----------------------------------------------------------------------------------------------*/
/**************************************************************************************************
 * @brief      Constructor for the STM32F4 Flash Abstraction Layer
 * @return     Nothing
 ********************************************************************************************** */
STM32F4FlashAbstractionLayer::STM32F4FlashAbstractionLayer() {
}

/**************************************************************************************************
 * @brief      Destructor
 * @return     Nothing
 ********************************************************************************************** */
STM32F4FlashAbstractionLayer::~STM32F4FlashAbstractionLayer() {
}

/**************************************************************************************************
 * @brief      Erase a region of flash memory
 * @param      offset Starting offset to erase from (relative to flash base)
 * @param      size Number of bytes to erase
 * @return     Number of bytes erased if successful, negative error code otherwise
 ********************************************************************************************** */
int STM32F4FlashAbstractionLayer::erase(long offset, size_t size) {
  uint32_t FirstSector = 0, NbOfSectors = 0;
  uint32_t SECTORError = 0;
  FLASH_EraseInitTypeDef EraseInitStruct;
  long addr = LITTLE_FS_STARTIN_ADDRESS + offset;

  Serial.print("Erasing offset: 0x"); Serial.println(offset, HEX);
  Serial.print("Erase size: "); Serial.println(size);
  Serial.print("First sector: "); Serial.println(getSectorFromOffset(addr));
  Serial.print("Number of sectors: "); Serial.println(getSectorFromOffset(addr + size - 1) - getSectorFromOffset(addr) + 1);

  // Validate offset and size
  if (offset < 0 || offset >= FLASH_TOTAL_SIZE_BYTES || size == 0 || (offset + size) > FLASH_TOTAL_SIZE_BYTES) {
    Serial.println("Error: Invalid erase offset or size");
    ef_err_port_cnt++;
    return -1;
  }

  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

  FirstSector = getSectorFromOffset(addr);
  NbOfSectors = getSectorFromOffset(addr + size - 1) - FirstSector + 1;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = FirstSector;
  EraseInitStruct.NbSectors = NbOfSectors;

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
    Serial.print("Erase failed, HAL error: "); Serial.println(HAL_FLASH_GetError());
    ef_err_port_cnt++;
    HAL_FLASH_Lock();
    return -1;
  }

  HAL_FLASH_Lock();
  return size;
}

/**************************************************************************************************
 * @brief      Write data to flash memory
 * @param      offset Offset to write to (relative to flash base)
 * @param      buf Pointer to the data to write
 * @param      size Number of bytes to write
 * @return     Number of bytes written if successful, negative error code otherwise
 ********************************************************************************************** */
int STM32F4FlashAbstractionLayer::write(long offset, const uint8_t *buf, size_t size) {
  long addr = LITTLE_FS_STARTIN_ADDRESS + offset;

  Serial.print("Writing offset: 0x"); Serial.println(offset, HEX);
  Serial.print("Write size: "); Serial.println(size);

  // Validate offset and size
  if (offset < 0 || offset >= FLASH_TOTAL_SIZE_BYTES || size == 0 || (offset + size) > FLASH_TOTAL_SIZE_BYTES) {
    Serial.println("Error: Invalid write offset or size");
    ef_err_port_cnt++;
    return -1;
  }

  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

  for (size_t i = 0; i < size; i++) {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + i, buf[i]) != HAL_OK) {
      Serial.print("Write failed, HAL error: "); Serial.println(HAL_FLASH_GetError());
      ef_err_port_cnt++;
      HAL_FLASH_Lock();
      return -1;
    }
    // Verify written data
    if (*(uint8_t*)(addr + i) != buf[i]) {
      Serial.println("Write verification failed");
      ef_err_port_cnt++;
      HAL_FLASH_Lock();
      return -1;
    }
  }

  HAL_FLASH_Lock();
  on_ic_write_cnt++;
  return size;
}

/**************************************************************************************************
 * @brief      Read data from flash memory
 * @param      offset Offset to read from (relative to flash base)
 * @param      buf Pointer to buffer to store read data
 * @param      size Number of bytes to read
 * @return     Number of bytes read if successful, negative error code otherwise
 ********************************************************************************************** */
int STM32F4FlashAbstractionLayer::read(long offset, uint8_t *buf, size_t size) {
  size_t i;
  long addr = LITTLE_FS_STARTIN_ADDRESS + offset;

  Serial.print("Reading offset: 0x"); Serial.println(offset, HEX);
  Serial.print("Read size: "); Serial.println(size);

  // Validate offset and size
  if (!buf || size == 0) {
    Serial.println("Error: Invalid read buffer or size");
    ef_err_port_cnt++;
    return -1;
  }

  if (offset < 0 || offset >= FLASH_TOTAL_SIZE_BYTES || (offset + size) > FLASH_TOTAL_SIZE_BYTES) {
    Serial.println("Error: Invalid read offset or size");
    ef_err_port_cnt++;
    return -1;
  }

  for (i = 0; i < size; i++, buf++, addr++) {
    *buf = *(uint8_t *)addr;
  }
  on_ic_read_cnt++;
  return size;
}

/**************************************************************************************************
 * @brief      Commit all buffered write operations to flash memory
 * @return     0 if successful, negative error code otherwise
 ********************************************************************************************** */
int STM32F4FlashAbstractionLayer::sync() {
  return 0; // No buffering, direct writes
}

/**************************************************************************************************
 * @brief      Verify flash is erased
 * @param      addr Start address
 * @param      size Size to check
 * @return     True if erased (all 0xFF), false otherwise
 ********************************************************************************************** */
bool STM32F4FlashAbstractionLayer::verify_flash_erased(uint32_t addr, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (*(uint8_t*)(addr + i) != 0xFF) {
      Serial.print("Flash not erased at 0x"); Serial.println(addr + i, HEX);
      ef_err_port_cnt++;
      return false;
    }
  }
  return true;
}

/*-----------------------------------------------------------------------------------------------*/
/* Private methods                                                                               */
/*-----------------------------------------------------------------------------------------------*/
/**
 * @brief Get the flash sector from the given address
 * @param addr Absolute address in flash
 * @return Flash sector number
 */
uint32_t STM32F4FlashAbstractionLayer::getSectorFromOffset(uint32_t addr) {
  uint32_t sector = 0;

  Serial.print("Address: 0x"); Serial.println(addr, HEX);

  if ((addr < ADDR_FLASH_SECTOR_1) && (addr >= ADDR_FLASH_SECTOR_0)) {
    sector = FLASH_SECTOR_0;
  } else if ((addr < ADDR_FLASH_SECTOR_2) && (addr >= ADDR_FLASH_SECTOR_1)) {
    sector = FLASH_SECTOR_1;
  } else if ((addr < ADDR_FLASH_SECTOR_3) && (addr >= ADDR_FLASH_SECTOR_2)) {
    sector = FLASH_SECTOR_2;
  } else if ((addr < ADDR_FLASH_SECTOR_4) && (addr >= ADDR_FLASH_SECTOR_3)) {
    sector = FLASH_SECTOR_3;
  } else if ((addr < ADDR_FLASH_SECTOR_5) && (addr >= ADDR_FLASH_SECTOR_4)) {
    sector = FLASH_SECTOR_4;
  } else if ((addr < ADDR_FLASH_SECTOR_6) && (addr >= ADDR_FLASH_SECTOR_5)) {
    sector = FLASH_SECTOR_5;
  } else if ((addr < ADDR_FLASH_SECTOR_7) && (addr >= ADDR_FLASH_SECTOR_6)) {
    sector = FLASH_SECTOR_6;
  } else {
    sector = FLASH_SECTOR_7;
  }

  Serial.print("Sector: "); Serial.println(sector);
  return sector;
}