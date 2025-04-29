//
// name: Kaylie Pham
// RedID: 828129478
//
// name: Aditya Bhagat
// RedID: 828612974

#include <iostream>
#include <unistd.h>

#include "vaddr_tracereader.h"
#include "page_table.h"
#include "log_helpers.h"
#include "wsclock.h"


int main(int argc, char* argv[]) {

    int frameNum = 0;               // stores physical frame number, which starts at 0 (and will be sequentially incremented)
    p2AddrTr mtrace;                // structure is typedefed in vaddr_tracereader
    unsigned int vAddr = 0;             // unsigned 32-bit integer type

    int clock_hand_position = 0;            // position of lock hand starting at 0

    int maxNumMemAccesses = -1;             // only process first N memory accesses, default is all
    int numPhysicalFrames = 999999;         // number of available physical frames, default is 999999
    int ageRecentLastAccess = 10;           // age of last access considered recent for page replacement, default is 10
    const char* logMode = "summary";        // log mode, specifying what to be printed, default is "summary"


    // command line argument parsing
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:f:a:l:")) != -1) {
        switch (opt) {
            case 'n':
                maxNumMemAccesses = atoi(optarg);
                if (maxNumMemAccesses < 1) {
                  std::cerr << "Number of memory accesses must be a number, greater than 0" << std::endl;
                  return 1;    // exit program, error occurred
                  }
            break;
            case 'f':
                numPhysicalFrames = atoi(optarg);
                if (numPhysicalFrames < 1) {
                  std::cerr << "Number of available frames must be a number, greater than 0" << std::endl;
                  return 1;    // exit program, error occurred
                }
            break;
            case 'a':
                ageRecentLastAccess = atoi(optarg);
                if (ageRecentLastAccess < 1) {
                  std::cerr << "Age of last access considered recent must be a number, greater than 0" << std::endl;
                    return 1;
                }
            break;
            case 'l':
                 logMode = optarg;
            break;
            default:
                return 1;
        }
    }

    int idx = optind;           // parameter, indexes next element to be processed

    const char* mtrace_file_name = argv[idx];
    idx++;                      // increment index to get next argument -- file
    const char* readswrites_file_name = argv[idx];
    idx++;                      // increment index to get next argument -- num bits for level(s)

    // opening trace and read/writes file
    FILE* mtrace_file = fopen(mtrace_file_name, "r");
    FILE* readswrites_file = fopen(readswrites_file_name, "r");

    // checking if file opens correctly
    if (!mtrace_file) {
        std::cerr << "Unable to open <<trace.tr>>" << std::endl;
        return 1;          // exit program, error occurred
    }
    if (!readswrites_file) {
        std::cerr << "Unable to open <<readswrites.tr>>" << std::endl;
        return 1;           // exit program, error occurred
    }

    // store number of bits to be used for each level
    int totalNumBitsAllLevels = 0;    // store total number of bits for all levels
    int levelCount = argc - idx;
    int bitsPerLevel[levelCount];    // create array to store bits for each level
    if (idx < argc) {
        for (int i = 0; i < levelCount; i++) {
          bitsPerLevel[i] = atoi(argv[idx + i]);            // converting and storing bits per level
            if (bitsPerLevel[i] < 1) {
                std::cerr << "Level " << argv[idx + i] << " page table must be at least 1 bit" << std::endl;
                return 1;
            }
          totalNumBitsAllLevels += bitsPerLevel[i];
        }

        // checking if too many bits used
       if (totalNumBitsAllLevels > 28) {
         std::cerr << "Too many bits used in page tables" << std::endl;
         return 1;          // exit program, error occurred
       }
    }

    // creating pagetable and WSClock entry arrays
    PageTable* page_table = create_pagetable(bitsPerLevel, levelCount, logMode);
    auto** WSClock_entries = new WSClockEntry*[numPhysicalFrames];      // array of pointers for WSClock entries
    for (int i = 0; i < numPhysicalFrames; i++) {
        WSClock_entries[i] = nullptr;
    }

    // main memory access loop
    // if maximum memory accesses if not specfied, continue reading available addresses OR
              // if maximum memory accesses is specified, read until max is reached
                // here maxNumMemAccesses is set to -1 incase there's no limit and the main keeps going
    while ((maxNumMemAccesses == -1 || page_table->numOfAddresses < maxNumMemAccesses) && NextAddress(mtrace_file, &mtrace)) {

        vAddr = mtrace.addr;        // reading & storing virtual address

        // extracting VPNs for each level
        unsigned int vpns[levelCount];
        for (int i = 0; i < levelCount; ++i) {
            vpns[i] = getVPNFromVirtualAddress(vAddr, page_table->bitMaskAry[i], page_table->shiftAry[i]);
        }

        // checking if access is a write or read
        bool isWrite = (fgetc(readswrites_file) == '1');

        // checking if the address has existing mapping already

        unsigned int fullVPN = vAddr >> page_table->numOfBitsOffset; // the full VPN for logging
        unsigned int offset = vAddr & ((1U << page_table->numOfBitsOffset) - 1);    // variable storing offset; 0 is a placeholder
        unsigned int physical_address = 0;     // variable storing physical address; 0 is a placeholder

        Map* map = findVpn2PfnMapping(page_table, vAddr);

        // if page table hit
        if (map != nullptr) {
            page_table->pageTableHits++;
            WSClock_entries[map->frameNum]->lastUsedTime = page_table->numOfAddresses;        // updating last used time
            if (isWrite) {
                WSClock_entries[map->frameNum]->dirty = isWrite;            // if the page is written, flag it as dirty
            }

            // logging based on depending mode
            if (strcmp(logMode, "vpn2pfn_pr") == 0){
                log_mapping(fullVPN, map->frameNum, -1, true);     // logging map hit
            }
            else if (strcmp(logMode, "vpns_pfn") == 0){
                log_vpns_pfn(page_table->levelCount, vpns, map->frameNum);
            }
            else if (strcmp(logMode, "offset") == 0) {
                print_num_inHex(offset);
            }
            else if (strcmp(logMode, "va2pa") == 0) {
                physical_address = (map->frameNum << page_table->numOfBitsOffset) + offset;
                log_va2pa(vAddr, physical_address);
            }
        }
        // if page table miss, add to page table and WSClock vector
        else {
            if (page_table->numOfFramesAllocated < numPhysicalFrames) {         // checking if a free frame is available
                insertVpn2PfnMapping(page_table, vAddr, frameNum);              // if yes, add new mapping
                WSClock_entries[frameNum] = create_WSClock_entry(vAddr, fullVPN, frameNum, page_table->numOfAddresses, isWrite);

                if (strcmp(logMode, "vpn2pfn_pr") == 0){
                    log_mapping(fullVPN, frameNum, -1, false);     // logging new map
                }
                else if (strcmp(logMode, "vpns_pfn") == 0){
                    log_vpns_pfn(page_table->levelCount, vpns, frameNum);
                }
                else if (strcmp(logMode, "offset") == 0) {
                    print_num_inHex(offset);
                }
                else if (strcmp(logMode, "va2pa") == 0) {
                    physical_address = (frameNum << page_table->numOfBitsOffset) + offset;
                    log_va2pa(vAddr, physical_address);
                }
                frameNum++;                 // incrementing frameNum
                page_table->numOfFramesAllocated++;
            }
            // performing page replacement
            else {

                clock_hand_position = page_replacement(WSClock_entries, clock_hand_position, ageRecentLastAccess, page_table->numOfAddresses, fullVPN, vAddr, page_table, isWrite);

                page_table->numOfPageReplaced++;

                Map* updated = findVpn2PfnMapping(page_table, vAddr);

                if (updated != nullptr) {
                    if (strcmp(logMode, "vpns_pfn") == 0) {
                        log_vpns_pfn(page_table->levelCount, vpns, updated->frameNum);
                    }
                    else if (strcmp(logMode, "offset") == 0) {
                        print_num_inHex(offset);
                    }
                    else if (strcmp(logMode, "va2pa") == 0) {
                        physical_address = (updated->frameNum << page_table->numOfBitsOffset) + offset;
                        log_va2pa(vAddr, physical_address);
                    }
                }

            }
        }
        page_table->numOfAddresses++;       // updating total number of address for pagetable
    }

    // print logs given the log mode from command line
    if (strcmp(logMode, "bitmasks") == 0){
        log_bitmasks(page_table->levelCount, page_table->bitMaskAry);
    }

    else if (strcmp(logMode, "summary") == 0) {
        log_summary(page_table->pageSize, page_table->numOfPageReplaced, page_table->pageTableHits,
            page_table->numOfAddresses, page_table->numOfFramesAllocated, page_table->pgTableEntries);
    }

    // close files before exiting
    fclose(mtrace_file);            // closing trace file
    fclose(readswrites_file);       // closing read/write file
    delete[] WSClock_entries;         // deallocating memory for WSClock entries

    return 0; // exit program, as it successfully complete

}
