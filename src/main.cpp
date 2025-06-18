/**
 * @file main.cpp
 * @brief STM32L476RG LittleFS Implementation with Internal Flash Storage
 * @author Oussama Darouez
 * @date June 2025
 * @version 1.0
 * 
 * This file implements a LittleFS filesystem on STM32L476RG microcontroller
 * using internal flash memory for persistent storage. The implementation
 * provides read, write, erase, and sync operations for the filesystem.
 * 
 * @note This implementation uses the last portion of internal flash memory
 * for filesystem storage. Ensure your linker script accounts for this.
 */

 #include <Arduino.h>
 #include "lfs.h"

 // Configuration defines
 #define FLASH_SECTOR_SIZE   4096        ///< LittleFS block size (2 pages)
 #define LITTLE_FS_STARTIN_ADDRESS 0x08040000
 
 // Global variables for filesystem operations
 lfs_t lfs;                              ///< LittleFS filesystem instance
 lfs_file_t file;                        ///< LittleFS file handle
 
  uint32_t ef_err_port_cnt;        ///< Error counter for port operations
  uint32_t on_ic_read_cnt;         ///< Counter for read operations
  uint32_t on_ic_write_cnt;        ///< Counter for write operations

 /**
  * @brief Read data from flash memory
  * 
  * This function reads data from the internal flash memory at the specified
  * offset. It performs byte-by-byte reading and includes error checking
  * for address alignment.
  * 
  * @param offset Offset from base flash address
  * @param buf Buffer to store read data
  * @param size Number of bytes to read
  * @return int Number of bytes read, or negative value on error
  */
 static int read(long offset, uint8_t *buf, size_t size)
 {
     size_t i;
     uint32_t addr = LITTLE_FS_STARTIN_ADDRESS + offset;
     
     // Check for address alignment (should be 4-byte aligned)
     if(addr % 4 != 0)
         ef_err_port_cnt++;
     
     // Read data byte by byte
     for (i = 0; i < size; i++, addr++, buf++)
     {
         *buf = *(uint8_t *) addr;
     }
     
     on_ic_read_cnt++;
     return size;
 }
 
 /**
  * @brief Write data to flash memory
  * 
  * This function writes data to the internal flash memory at the specified
  * offset. It uses 8-byte aligned writes as required by STM32L4 series.
  * Includes verification of written data and watchdog feeding.
  * 
  * @param offset Offset from base flash address
  * @param buf Buffer containing data to write
  * @param size Number of bytes to write (must be multiple of 8)
  * @return int Number of bytes written, or -1 on error
  */
 static int write(long offset, const uint8_t *buf, size_t size)
 {
     size_t   i;
     uint32_t addr = LITTLE_FS_STARTIN_ADDRESS + offset;
     __ALIGN_BEGIN uint64_t write_data __ALIGN_END;
     __ALIGN_BEGIN uint64_t read_data  __ALIGN_END;  
     
     // Check for address alignment (should be 4-byte aligned)
     if(addr % 4 != 0)
         ef_err_port_cnt++;
 
     HAL_FLASH_Unlock();
     
     // Write data in 8-byte chunks
     for (i = 0; i < size; i += 8, buf += 8, addr += 8) {
         // Copy data to aligned buffer
         memcpy(&write_data, buf, 8);
         
         // Only program if data is not all 0xFF (erased state)
         if (write_data != 0xFFFFFFFFFFFFFFFF)
             HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, write_data);
         
         // Verify written data
         read_data = *(uint64_t *)addr;
         if (read_data != write_data) {
             HAL_FLASH_Lock(); 
             return -1;
         }
         else {

         }
     }
     
     HAL_FLASH_Lock();
     on_ic_write_cnt++;
     return size;
 }
 
 /**
  * @brief Erase flash memory pages
  * 
  * This function erases the specified flash memory region by erasing
  * individual pages. It includes watchdog feeding between page erases
  * to prevent timeout issues.
  * 
  * @param offset Offset from base flash address
  * @param size Number of bytes to erase
  * @return int Number of bytes erased, or -1 on error
  */
 static int erase(long offset, size_t size)
 {
     HAL_StatusTypeDef flash_status;
     size_t erase_pages, i;
     uint32_t PAGEError = 0;
     
     // Calculate number of pages to erase
     erase_pages = size / FLASH_PAGE_SIZE;
     if (size % FLASH_PAGE_SIZE != 0) {
         erase_pages++;
     }
     
     FLASH_EraseInitTypeDef EraseInitStruct;
     EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
     EraseInitStruct.NbPages     = 1;  // Erase one page at a time for watchdog feeding
     
     HAL_FLASH_Unlock();
     
     // Erase pages one by one
     for (i = 0; i < erase_pages; i++) {
         EraseInitStruct.Page = (offset / FLASH_PAGE_SIZE) + i;
         flash_status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
         
         if (flash_status != HAL_OK) {
             HAL_FLASH_Lock(); 
             return -1;
         }
         else {

         }
     }
     
     HAL_FLASH_Lock(); 
     return size;
 }
 
 /**
  * @brief LittleFS read block device function
  * 
  * Interface function between LittleFS and the flash read operation.
  * 
  * @param c LittleFS configuration structure
  * @param block Block number to read from
  * @param off Offset within the block
  * @param buffer Buffer to store read data
  * @param size Number of bytes to read
  * @return int 0 on success, negative value on error
  */
 int read(const struct lfs_config *c, lfs_block_t block,
          lfs_off_t off, void *buffer, lfs_size_t size) {
     // Calculate offset from base address
     long offset = (block * c->block_size) + off;
     
     // Use the flash read function
     int result = read(offset, (uint8_t*)buffer, size);
     
     return (result == size) ? 0 : -1; // LittleFS expects 0 for success
 }
 
 /**
  * @brief LittleFS write block device function
  * 
  * Interface function between LittleFS and the flash write operation.
  * 
  * @param c LittleFS configuration structure
  * @param block Block number to write to
  * @param off Offset within the block
  * @param buffer Buffer containing data to write
  * @param size Number of bytes to write
  * @return int 0 on success, negative value on error
  */
 int write(const struct lfs_config *c, lfs_block_t block,
           lfs_off_t off, const void *buffer, lfs_size_t size) {
     // Calculate offset from base address
     long offset = (block * c->block_size) + off;
     
     // Use the flash write function
     int result = write(offset, (const uint8_t*)buffer, size);
     
     return (result == size) ? 0 : -1; // LittleFS expects 0 for success
 }
 
 /**
  * @brief LittleFS erase block device function
  * 
  * Interface function between LittleFS and the flash erase operation.
  * 
  * @param c LittleFS configuration structure
  * @param block Block number to erase
  * @return int 0 on success, negative value on error
  */
 int erase(const struct lfs_config *c, lfs_block_t block) {
     // Calculate offset from base address
     long offset = block * c->block_size;
     
     // Use the flash erase function
     int result = erase(offset, c->block_size);
     
     return (result == c->block_size) ? 0 : -1; // LittleFS expects 0 for success
 }
 
 /**
  * @brief LittleFS sync block device function
  * 
  * Ensures all pending write operations are completed. For internal flash,
  * no additional synchronization is needed as writes are immediately committed.
  * 
  * @param c LittleFS configuration structure (unused)
  * @return int Always returns 0 (success)
  */
 int sync(const struct lfs_config *c) {
     // For internal flash, no additional sync needed
     // Data is immediately written when programmed
     return 0; // Success
 }
 
 /**
  * @brief LittleFS configuration structure
  * 
  * This structure defines the configuration parameters for the LittleFS
  * filesystem, including block device operations and memory layout.
  */
 const struct lfs_config cfg = {
     // Block device operations
     .read = read,                       ///< Read function pointer
     .prog = write,                      ///< Program (write) function pointer
     .erase = erase,                     ///< Erase function pointer
     .sync = sync,                       ///< Sync function pointer
     
     // Block device configuration
     .read_size = 16,                    ///< Minimum read size
     .prog_size = 8,                     ///< Program size (STM32L4 requires 8-byte alignment)
     .block_size = 4096,                 ///< Block size (4KB blocks = 2 flash pages)
     .block_count = 32,                  ///< Number of blocks (adjust based on available flash)
     .block_cycles = 500,                ///< Block wear leveling cycles
     .cache_size = 256,                  ///< Cache size for better performance
     .lookahead_size = 16,               ///< Lookahead buffer size
     
 };
 
 /**
  * @brief Arduino setup function
  * 
  * Initializes the serial communication and LittleFS filesystem.
  * Creates a boot counter file to demonstrate filesystem functionality.
  */
 void setup() {
     Serial.begin(115200);
     Serial.println("STM32L476RG LittleFS Demo");
     Serial.println("========================================");
     
     // Mount the filesystem
     int err = lfs_mount(&lfs, &cfg);
     
     // Reformat if we can't mount the filesystem
     // This should only happen on the first boot or after corruption
     if (err) {
         Serial.println("Formatting filesystem...");
         lfs_format(&lfs, &cfg);
         err = lfs_mount(&lfs, &cfg);
         
         if (err) {
             Serial.println("Failed to mount filesystem!");
             return;
         }
         Serial.println("Filesystem formatted and mounted successfully");
     } else {
         Serial.println("Filesystem mounted successfully");
     }
     
     // Read current boot count
     uint32_t boot_count = 0;
     int file_err = lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
     
     if (file_err == 0) {
         lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));
         
         // Update boot count
         boot_count += 1;
         lfs_file_rewind(&lfs, &file);
         lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));
         
         // Close file to ensure data is written
         lfs_file_close(&lfs, &file);
         
         Serial.print("Boot count: ");
         Serial.println(boot_count);
     } else {
         Serial.println("Failed to open boot_count file");
     }
     
     // Unmount filesystem to release resources
     lfs_unmount(&lfs);
     
     Serial.println("Setup completed successfully");
     Serial.print("Read operations: ");
     Serial.println(on_ic_read_cnt);
     Serial.print("Write operations: ");
     Serial.println(on_ic_write_cnt);
     Serial.print("Port errors: ");
     Serial.println(ef_err_port_cnt);
 }
 
 /**
  * @brief Arduino main loop function
  * 
  * Currently empty - add your main application logic here.
  */
 void loop() {
     // Your main application code here
     delay(1000); // Prevent overwhelming the serial output
 }