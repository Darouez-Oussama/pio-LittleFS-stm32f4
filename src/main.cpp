/* 
 **************************************************************************************************
 *
 * @file    : main.cpp
 * @author  : [Your Name]
 * @version : 1.0
 * @date    : July 2025
 * @brief   : Main program for LittleFS on STM32 with PlatformIO and Arduino
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
#include <lfs.h>
#include "FlashAbstractionLayerFactory.h"

/*-----------------------------------------------------------------------------------------------*/
/* Global Variables                                                                              */
/*-----------------------------------------------------------------------------------------------*/
IFlashAbstractionLayer *fal = FlashAbstractionLayerFactory::createFlashAbstractionLayer();
lfs_t lfs;
extern uint32_t ef_err_port_cnt;  // Error counter for flash operations
extern uint32_t on_ic_write_cnt;  // Counter for successful write operations
extern uint32_t on_ic_read_cnt;   // Counter for successful read operations

/*-----------------------------------------------------------------------------------------------*/
/* Private Functions                                                                             */
/*-----------------------------------------------------------------------------------------------*/
int erase_littlefs_region() {
    // Erase 256 KB (sectors 6–7)
    int err = fal->erase(0, 256 * 1024);
    if (err < 0) {
      Serial.println("Error: Failed to erase LittleFS region");
      return err;
    }
  
    // Verify the erased state
    if (! fal->verify_flash_erased(0x08040000, 256 * 1024)) {
      Serial.println("Error: Flash verification failed");
      return -1;
    }
  
    Serial.println("LittleFS region erased and verified");
    return 0;
  }
int erase(const struct lfs_config *c, lfs_block_t block) {
  long offset = block * c->block_size;
  int result = fal->erase(offset, c->block_size);
  return (result >= 0) ? 0 : -1; // STM32F4FlashAbstractionLayer::erase returns size or -1
}

int sync(const struct lfs_config *c) {
  return fal->sync();
}

int write(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
  long offset = (block * c->block_size) + off;
  int result = fal->write(offset, (const uint8_t*)buffer, size);
  return (result == size) ? 0 : -1;
}

int read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
  long offset = (block * c->block_size) + off;
  int result = fal->read(offset, (uint8_t*)buffer, size);
  return (result == size) ? 0 : -1;
}

/*-----------------------------------------------------------------------------------------------*/
/* Setup                                                                                         */
/*-----------------------------------------------------------------------------------------------*/
void setup() {
lfs_file_t file;
const struct lfs_config cfg = {
    .read = read,
    .prog = write,
    .erase = erase,
    .sync = sync,
    .read_size = 16,
    .prog_size = 1,
    .block_size = 1024,
    .block_count = 256,
    .block_cycles = 500,
    .cache_size = 256,
    .lookahead_size = 16,
  };
  Serial.begin(9600);
  while (!Serial) {} // Wait for serial
  Serial.println("STM32F401RE LittleFS Demo");
  Serial.println("========================================");
  
  Serial.print("System clock: "); Serial.print(SystemCoreClock / 1000000); Serial.println(" MHz");
  Serial.print("Flash latency: "); Serial.println((FLASH->ACR & FLASH_ACR_LATENCY) >> FLASH_ACR_LATENCY_Pos);
  Serial.println("Note: Skipping write protection check as confirmed disabled in STM32CubeProgrammer");

  // Erase LittleFS region
  if (erase_littlefs_region() != 0) {
    Serial.println("Setup aborted due to erase failure");
    return;
  }
  
  // Mount filesystem
  int err = lfs_mount(&lfs, &cfg);
  if (err) {
    Serial.println("Formatting filesystem...");
    err = lfs_format(&lfs, &cfg);
    if (err) {
      Serial.print("Format failed, error: "); Serial.println(err);
      return;
    }
    err = lfs_mount(&lfs, &cfg);
    if (err) {
      Serial.print("Failed to mount filesystem, error: "); Serial.println(err);
      return;
    }
    Serial.println("Filesystem formatted and mounted successfully");
  } else {
    Serial.println("Filesystem mounted successfully");
  }
  
  // Boot count
  uint32_t boot_count = 0;
  err = lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
  if (err) {
    Serial.print("Failed to open boot_count file, error: "); Serial.println(err);
    lfs_unmount(&lfs);
    return;
  }
  for (int i = 0; i < 5; i++) {
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));
    Serial.print("Boot count: "); Serial.println(boot_count);
  }
  lfs_file_close(&lfs, &file);

  // Create directory
  err = lfs_mkdir(&lfs, "txts");
  if (err && err != LFS_ERR_EXIST) {
    Serial.print("Failed to create directory, error: "); Serial.println(err);
    lfs_unmount(&lfs);
    return;
  }
  Serial.println("Created directory 'txts'");

  // Create and write to file
  err = lfs_file_open(&lfs, &file, "txts/myfile.txt", LFS_O_RDWR | LFS_O_CREAT);
  if (err) {
    Serial.print("File open failed, error: "); Serial.println(err);
    lfs_unmount(&lfs);
    return;
  }
  const char *data = "This is a text file in the txts directory!";
  lfs_ssize_t bytes_written = lfs_file_write(&lfs, &file, data, strlen(data));
  if (bytes_written < 0) {
    Serial.print("Write failed, error: "); Serial.println(bytes_written);
  } else {
    Serial.print("Wrote "); Serial.print(bytes_written); Serial.println(" bytes to txts/myfile.txt");
  }
  lfs_file_close(&lfs, &file);

  // Read from file
  err = lfs_file_open(&lfs, &file, "txts/myfile.txt", LFS_O_RDONLY);
  if (err) {
    Serial.print("Failed to open txts/myfile.txt for reading, error: "); Serial.println(err);
    lfs_unmount(&lfs);
    return;
  }
  char buffer[64]; // Buffer to hold file contents (larger than the 41-byte string)
  lfs_ssize_t bytes_read = lfs_file_read(&lfs, &file, buffer, sizeof(buffer) - 1);
  if (bytes_read < 0) {
    Serial.print("Read failed, error: "); Serial.println(bytes_read);
    lfs_file_close(&lfs, &file);
    lfs_unmount(&lfs);
    return;
  }
  buffer[bytes_read] = '\0'; // Null-terminate the string
  Serial.print("Read "); Serial.print(bytes_read); Serial.println(" bytes from txts/myfile.txt");
  Serial.print("File contents: "); Serial.println(buffer);
  lfs_file_close(&lfs, &file);

  // Unmount filesystem
  err = lfs_unmount(&lfs);
  if (err) {
    Serial.print("Unmount failed, error: "); Serial.println(err);
    return;
  }
  Serial.print("Read operations: "); Serial.println(on_ic_read_cnt);
  Serial.print("Write operations: "); Serial.println(on_ic_write_cnt);
  Serial.print("Port errors: "); Serial.println(ef_err_port_cnt);
}

/*-----------------------------------------------------------------------------------------------*/
/* Loop                                                                                          */
/*-----------------------------------------------------------------------------------------------*/
void loop() {
  delay(1000);
}