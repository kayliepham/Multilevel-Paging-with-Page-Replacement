//
// name: Kaylie Pham
// RedID: 828129478
//
// name: Aditya Bhagat
// RedID: 828612974

#include <vector>
#include <cstdint>

#include "wsclock.h"
#include "log_helpers.h"

// creates a new WSClockEntry that takes virtual address, VPN, frame number, last access time, and dirty bool
WSClockEntry* create_WSClock_entry(unsigned int virtAddress, unsigned int vpn, int frameNum, int lastAccessTime, bool dirty) {

    // create new WSClock entry and initialize attributes
    WSClockEntry* WSClock_entry = new WSClockEntry;         // allocating new entry
    WSClock_entry->vpn = vpn;                               // setting vpn
    WSClock_entry->virtAddress = virtAddress;
    WSClock_entry->frameNum = frameNum;                     // setting frame number
    WSClock_entry->lastUsedTime = lastAccessTime;           // setting last access time
    WSClock_entry->dirty = dirty;                           // setting dirty
    return WSClock_entry;                                   // returning the new entry
}

// performs page replacement using the WSClock algorithm
int page_replacement(WSClockEntry** entries, int clock_hand_position, int ageThreshold, int currentTime, unsigned int newVpn, unsigned int newVpnAddress, PageTable* PageTable, bool isWrite){

    bool victimFound = false;                               // initializing victimFound to false for tracking
    int numFrames = PageTable->numOfFramesAllocated;        // initializing numFrames with the total number of frames given
    int clock_hand_pos = clock_hand_position;                // store current clock hand position

    // initialize to 0 or nullptr as placeholders
    unsigned int evictedVPN = 0;
    unsigned int evictedAddress = 0;
    Map* victimMap = nullptr;
    WSClockEntry* entry = nullptr;
    int reusedFrame = 0;

    // starts at clock_hand_pos and looks for a victim Page
    while(!victimFound) {
        if (entries[clock_hand_pos] != nullptr) {
            entry = entries[clock_hand_pos];         // getting current frame's WSClock entry

            int ageOfLastAccessConsideredRecent = currentTime - (entry->lastUsedTime);          // getting the age of the page

            //checking if the age of the page is greater than threshold
            if (ageOfLastAccessConsideredRecent > ageThreshold) {

                // checking if the page is clean
                if (!entry->dirty){

                    evictedVPN = entry->vpn;
                    evictedAddress = entry->virtAddress;               // if the page is clean, evictedVPN: setting with vpn of the current evicted page
                    victimMap = findVpn2PfnMapping(PageTable, evictedAddress);         // the current page in the page table

                    if (victimMap != nullptr) {            // invalidating the current vpn
                        victimMap->valid = false;
                        victimMap->frameNum = -1;
                    }

                    // updating WSClock entry with new vpn information
                    reusedFrame = entry->frameNum;                      // using the same frame
                    entry->vpn = newVpn;                                //replacing the entry's vpn info with newVpn
                    entry->virtAddress = newVpnAddress;
                    entry->lastUsedTime = currentTime;                  // updating last used time
                    entry->dirty = isWrite;                               // new page is clean /// check and update if read or write

                    // inserting the new vpn mapping to pagetable
                    insertVpn2PfnMapping(PageTable, newVpnAddress, reusedFrame);
                    victimFound = true;            // setting victim as found

                    if (strcmp(PageTable->logMode, "vpn2pfn_pr") == 0){
                        log_mapping(newVpn, reusedFrame, evictedVPN, false);
                    }
                }
                // if the page is dirty
                else {
                    // clearing the dirty flag
                    entry->dirty = false;
                }
            }
            // incrementing clock_hand_position in circular manner if no victim found yet
            if (!victimFound) {
                clock_hand_pos = (clock_hand_pos + 1) % numFrames;
            }
        }
    }
    return (clock_hand_pos + 1) % numFrames;     // returning new position of the clock hand if victime found
}