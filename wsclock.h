//
// name: Kaylie Pham
// RedID: 828129478
//
// name: Aditya Bhagat
// RedID: 828612974

#ifndef WSCLOCK_H
#define WSCLOCK_H

#include "page_table.h"
/**
 *
 * @Struct WSClockEntry
 * @brief represents an entry in the WSClock. Holds the vpn(virtual page number),
 * the virtual address, related frame number, last time of access, and the dirty bit
*/
struct WSClockEntry {
    unsigned int vpn;       // virtual page number (full)
    unsigned virtAddress;
    int frameNum;           // physical frame number
    int lastUsedTime;       // time this page was last accessed
    bool dirty;             // true if page has been written to
};

/**
 * @brief creates a new WSClock entry and initializes its values
 *
 * @param virtAddress - full virtual address of the page
 * @param vpn - virtual page number
 * @param frameNum - physical frame number associated with the page
 * @param lastAccessTime - timestamp of when the page was last accessed
 * @param dirty - boolean indicating whether the page has been written to
 * @return pointer to the newly created WSClockEntry
 */

WSClockEntry* create_WSClock_entry(unsigned int virtAddress, unsigned int vpn, int frameNum, int lastAccessTime, bool dirty);


/**
 * @func page_replacement
 * @brief applies the WSClock page replacement algorithm to find a victim page and update the memory and page table
 *
 * @param entries - array of WSClockEntry pointers
 * @param clock_hand_position - current position of the clock hand
 * @param ageThreshold - age threshold to check for an older page
 * @param currentTime - current time
 * @param newVpn - virtual page number of the new page
 * @param newVpnAddress - full virtual address for the new page
 * @param PageTable - gives access to page table to update mappings
 * @param isWrite - true for a write operations, otherwise false
 * @return new position of clock hand after page replacement
 */
int page_replacement(WSClockEntry** entries, int clock_hand_position, int ageThreshold, int currentTime,
                     unsigned int newVpn, unsigned int newVpnAddress, PageTable* PageTable, bool isWrite);

#endif