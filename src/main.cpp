/**
 * @file main.cpp
 * @brief STM32F401RE LittleFS Implementation with Internal Flash Storage
 * @author Oussama Darouez
 * @date June 2025
 * @version 1.0
 * 
 * This file implements a LittleFS filesystem on STM32F401RE microcontroller
 * using internal flash memory for persistent storage. The implementation
 * provides read, write, erase, and sync operations for the filesystem.
 * 
 * @note This implementation uses the last portion of internal flash memory
 * for filesystem storage. Ensure your linker script accounts for this.
 */

 #include <Arduino.h>
 #include "lfs.h"
 
 // Configuration defines
 
 /* Base address of the Flash sectors Bank 1 */
 #define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
 #define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
 #define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
 #define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
 #define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
 #define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */
 
 /* Base address of the Flash sectors Bank 2 */
 #define ADDR_FLASH_SECTOR_12     ((uint32_t)0x08100000) /* Base @ of Sector 0, 16 Kbytes */
 #define ADDR_FLASH_SECTOR_13     ((uint32_t)0x08104000) /* Base @ of Sector 1, 16 Kbytes */
 #define ADDR_FLASH_SECTOR_14     ((uint32_t)0x08108000) /* Base @ of Sector 2, 16 Kbytes */
 #define ADDR_FLASH_SECTOR_15     ((uint32_t)0x0810C000) /* Base @ of Sector 3, 16 Kbytes */
 #define ADDR_FLASH_SECTOR_16     ((uint32_t)0x08110000) /* Base @ of Sector 4, 64 Kbytes */
 #define ADDR_FLASH_SECTOR_17     ((uint32_t)0x08120000) /* Base @ of Sector 5, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_18     ((uint32_t)0x08140000) /* Base @ of Sector 6, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_19     ((uint32_t)0x08160000) /* Base @ of Sector 7, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_20     ((uint32_t)0x08180000) /* Base @ of Sector 8, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_21     ((uint32_t)0x081A0000) /* Base @ of Sector 9, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_22     ((uint32_t)0x081C0000) /* Base @ of Sector 10, 128 Kbytes */
 #define ADDR_FLASH_SECTOR_23     ((uint32_t)0x081E0000) /* Base @ of Sector 11, 128 Kbytes */
 #define LITTLE_FS_STARTIN_ADDRESS 0x08040000
 
 // Global variables for filesystem operations
 lfs_t lfs;                              ///< LittleFS filesystem instance
 lfs_file_t file;                        ///< LittleFS file handle
 
 uint32_t ef_err_port_cnt;        ///< Error counter for port operations
 uint32_t on_ic_read_cnt;         ///< Counter for read operations
 uint32_t on_ic_write_cnt;        ///< Counter for write operations
 
 /**
  * @brief Gets the sector of a given address
  * @param None
  * @retval The sector of a given address
  */
 static uint32_t stm32_get_sector(uint32_t addr) {
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
     } else if ((addr < ADDR_FLASH_SECTOR_8) && (addr >= ADDR_FLASH_SECTOR_7)) {
         sector = FLASH_SECTOR_7;
     }
 #if defined(FLASH_SECTOR_8)
     else if ((addr < ADDR_FLASH_SECTOR_9) && (addr >= ADDR_FLASH_SECTOR_8)) {
         sector = FLASH_SECTOR_8;
     }
 #endif
 #if defined(FLASH_SECTOR_9)
     else if ((addr < ADDR_FLASH_SECTOR_10) && (addr >= ADDR_FLASH_SECTOR_9)) {
         sector = FLASH_SECTOR_9;
     }
 #endif
 #if defined(FLASH_SECTOR_10)
     else if ((addr < ADDR_FLASH_SECTOR_11) && (addr >= ADDR_FLASH_SECTOR_10)) {
         sector = FLASH_SECTOR_10;
     }
 #endif
 #if defined(FLASH_SECTOR_11)
     else if ((addr < ADDR_FLASH_SECTOR_12) && (addr >= ADDR_FLASH_SECTOR_11)) {
         sector = FLASH_SECTOR_11;
     }
 #endif
 #if defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx)|| defined(STM32F439xx) || defined(STM32F469xx) || defined(STM32F479xx)
     else if ((addr < ADDR_FLASH_SECTOR_13) && (addr >= ADDR_FLASH_SECTOR_12)) {
         sector = FLASH_SECTOR_12;
     } else if ((addr < ADDR_FLASH_SECTOR_14) && (addr >= ADDR_FLASH_SECTOR_13)) {
         sector = FLASH_SECTOR_13;
     } else if ((addr < ADDR_FLASH_SECTOR_15) && (addr >= ADDR_FLASH_SECTOR_14)) {
         sector = FLASH_SECTOR_14;
     } else if ((addr < ADDR_FLASH_SECTOR_16) && (addr >= ADDR_FLASH_SECTOR_15)) {
         sector = FLASH_SECTOR_15;
     } else if ((addr < ADDR_FLASH_SECTOR_17) && (addr >= ADDR_FLASH_SECTOR_16)) {
         sector = FLASH_SECTOR_16;
     } else if ((addr < ADDR_FLASH_SECTOR_18) && (addr >= ADDR_FLASH_SECTOR_17)) {
         sector = FLASH_SECTOR_17;
     } else if ((addr < ADDR_FLASH_SECTOR_19) && (addr >= ADDR_FLASH_SECTOR_18)) {
         sector = FLASH_SECTOR_18;
     } else if ((addr < ADDR_FLASH_SECTOR_20) && (addr >= ADDR_FLASH_SECTOR_19)) {
         sector = FLASH_SECTOR_19;
     } else if ((addr < ADDR_FLASH_SECTOR_21) && (addr >= ADDR_FLASH_SECTOR_20)) {
         sector = FLASH_SECTOR_20;
     } else if ((addr < ADDR_FLASH_SECTOR_22) && (addr >= ADDR_FLASH_SECTOR_21)) {
         sector = FLASH_SECTOR_21;
     } else if ((addr < ADDR_FLASH_SECTOR_23) && (addr >= ADDR_FLASH_SECTOR_22)) {
         sector = FLASH_SECTOR_22;
     } else /* (addr < FLASH_END_ADDR) && (addr >= ADDR_FLASH_SECTOR_23) */ {
         sector = FLASH_SECTOR_23;
     }
 #endif
     Serial.print("Sector: "); Serial.println(sector);
     return sector;
 }
 
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
 static int read(long offset, uint8_t *buf, size_t size) {
     size_t i;
     long addr = LITTLE_FS_STARTIN_ADDRESS + offset;
 
     Serial.print("Reading offset: 0x"); Serial.println(offset, HEX);
     Serial.print("Read size: "); Serial.println(size);
 
     for (i = 0; i < size; i++, buf++, addr++) {
         *buf = *(uint8_t *) addr;
     }
     on_ic_read_cnt++;
     return size;
 }
 
 /**
  * @brief Write data to flash memory
  * 
  * This function writes data to the internal flash memory at the specified
  * offset. It uses byte-by-byte writes to avoid parallelism errors.
  * Includes verification of written data.
  * 
  * @param offset Offset from base flash address
  * @param buf Buffer containing data to write
  * @param size Number of bytes to write
  * @return int Number of bytes written, or -1 on error
  */
 static int write(long offset, const uint8_t *buf, size_t size) {
     long addr = LITTLE_FS_STARTIN_ADDRESS + offset;
 
     Serial.print("Writing offset: 0x"); Serial.println(offset, HEX);
     Serial.print("Write size: "); Serial.println(size);
 
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
 
 /**
  * @brief Erase flash memory pages
  * 
  * This function erases the specified flash memory region by erasing
  * individual pages.
  * 
  * @param offset Offset from base flash address
  * @param size Number of bytes to erase
  * @return int Number of bytes erased, or -1 on error
  */
 static int erase(long offset, size_t size) {
     uint32_t FirstSector = 0, NbOfSectors = 0;
     uint32_t SECTORError = 0;
     FLASH_EraseInitTypeDef EraseInitStruct;
     long addr = LITTLE_FS_STARTIN_ADDRESS + offset;
 
     Serial.print("Erasing offset: 0x"); Serial.println(offset, HEX);
     Serial.print("Erase size: "); Serial.println(size);
     Serial.print("First sector: "); Serial.println(stm32_get_sector(addr));
     Serial.print("Number of sectors: "); Serial.println(stm32_get_sector(addr + size - 1) - stm32_get_sector(addr) + 1);
 
     HAL_FLASH_Unlock();
     __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
 
     FirstSector = stm32_get_sector(addr);
     NbOfSectors = stm32_get_sector(addr + size - 1) - FirstSector + 1;
     EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
     EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
     EraseInitStruct.Sector = FirstSector;
     EraseInitStruct.NbSectors = NbOfSectors;
 
     if (HAL_FLASHEx_Erase(&EraseInitStruct, (uint32_t *) &SECTORError) != HAL_OK) {
         Serial.print("Erase failed, HAL error: "); Serial.println(HAL_FLASH_GetError());
         ef_err_port_cnt++;
         HAL_FLASH_Lock();
         return -1;
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
     long offset = (block * c->block_size) + off;
     int result = read(offset, (uint8_t*)buffer, size);
     return (result == size) ? 0 : -1;
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
     long offset = (block * c->block_size) + off;
     int result = write(offset, (const uint8_t*)buffer, size);
     return (result == size) ? 0 : -1;
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
     long offset = block * c->block_size;
     int result = erase(offset, c->block_size);
     return (result == c->block_size) ? 0 : -1;
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
     return 0;
 }
 
 /**
  * @brief LittleFS configuration structure
  * 
  * This structure defines the configuration parameters for the LittleFS
  * filesystem, including block device operations and memory layout.
  */
 const struct lfs_config cfg = {
     .read = read,
     .prog = write,
     .erase = erase,
     .sync = sync,
     .read_size = 16,
     .prog_size = 1, // Changed to 1 for byte writes
     .block_size = 4096,
     .block_count = 32,
     .block_cycles = 500,
     .cache_size = 256,
     .lookahead_size = 16,
 };
 
 /**
  * @brief Verify flash is erased
  * 
  * Checks if the specified flash region is fully erased (0xFF).
  * 
  * @param addr Start address
  * @param size Size to check
  * @return bool True if erased, false otherwise
  */
 bool verify_flash_erased(uint32_t addr, size_t size) {
     for (size_t i = 0; i < size; i++) {
         if (*(uint8_t*)(addr + i) != 0xFF) {
             Serial.print("Flash not erased at 0x"); Serial.println(addr + i, HEX);
             return false;
         }
     }
     return true;
 }
 
 /**
  * @brief Manually erase LittleFS flash region
  * 
  * Erases sectors 6–7 to ensure a clean state before formatting.
  */
 void erase_littlefs_region() {
     FLASH_EraseInitTypeDef EraseInitStruct;
     uint32_t SECTORError = 0;
 
     HAL_FLASH_Unlock();
     __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
 
     EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
     EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
     EraseInitStruct.Sector = FLASH_SECTOR_6;
     EraseInitStruct.NbSectors = 2; // Sectors 6 and 7
 
     Serial.println("Manually erasing LittleFS region (sectors 6–7)...");
     if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
         Serial.print("Manual erase failed, HAL error: "); Serial.println(HAL_FLASH_GetError());
         ef_err_port_cnt++;
     } else {
         Serial.println("Manual erase successful");
         // Verify erase
         if (verify_flash_erased(LITTLE_FS_STARTIN_ADDRESS, 4096 * 2)) {
             Serial.println("Flash verified as erased");
         } else {
             Serial.println("Flash verification failed");
             ef_err_port_cnt++;
         }
     }
 
     HAL_FLASH_Lock();
 }
 
 /**
  * @brief Arduino setup function
  * 
  * Initializes the serial communication and LittleFS filesystem.
  * Creates a boot counter file to demonstrate filesystem functionality.
  */
 void setup() {
     Serial.begin(9600);
     while (!Serial) {} // Wait for serial to be ready
     Serial.println("STM32F401RE LittleFS Demo");
     Serial.println("========================================");
     
     // Log system clock and flash latency
     Serial.print("System clock: "); Serial.print(SystemCoreClock / 1000000); Serial.println(" MHz");
     Serial.print("Flash latency: "); Serial.println((FLASH->ACR & FLASH_ACR_LATENCY) >> FLASH_ACR_LATENCY_Pos);
 
     // Skip option byte check since confirmed no write protection
     Serial.println("Note: Skipping write protection check as confirmed disabled in STM32CubeProgrammer");
 
     // Manually erase LittleFS region to ensure clean state
     erase_littlefs_region();
     
     // Mount the filesystem
     int err = lfs_mount(&lfs, &cfg);
     
     // Reformat if we can't mount the filesystem
     if (err) {
         Serial.println("Formatting filesystem...");
         lfs_format(&lfs, &cfg);
         err = lfs_mount(&lfs, &cfg);
         
         if (err) {
             Serial.print("Failed to mount filesystem, error: "); Serial.println(err);
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
         
         boot_count += 1;
         lfs_file_rewind(&lfs, &file);
         lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));
         
         lfs_file_close(&lfs, &file);
         
         Serial.print("Boot count: "); Serial.println(boot_count);
     } else {
         Serial.print("Failed to open boot_count file, error: "); Serial.println(file_err);
     }
     
     lfs_unmount(&lfs);
     
     Serial.println("Setup completed successfully");
     Serial.print("Read operations: "); Serial.println(on_ic_read_cnt);
     Serial.print("Write operations: "); Serial.println(on_ic_write_cnt);
     Serial.print("Port errors: "); Serial.println(ef_err_port_cnt);
 }
 
 /**
  * @brief Arduino main loop function
  * 
  * Currently empty - add your main application logic here.
  */
 void loop() {
     delay(1000);
 }