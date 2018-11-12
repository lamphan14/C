/*
Lam Phan
CS 4348.501
Dr. Alagar
Project #2
use command: gcc -o vmm vmm.c
./vmm addresses.txt (your_output_name_file).txt BACKING_STORE.bin
the program will give the stats at the of the output file.
*/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

// Global definitions. 
#define PAGE_SIZE 256 // Page size, in bytes.
#define PAGE_ENTRIES 256 // Max page table entries.
#define PAGE_NUM_BITS 8 // Page number size, in bits.
#define FRAME_SIZE 256 // Frame size, in bytes.
#define FRAME_ENTRIES 256 // Number of frames in physical memory.
#define MEM_SIZE (FRAME_SIZE * FRAME_ENTRIES) // Mem size, in bytes.
#define TLB_ENTRIES 16 // Max TLB entries.

// Global variables. 
int virtual; // Virtual address.
int page_number; // Page number.
int offset; // Offset.
int physical; // Physical address.
int frame_number; // Frame number.
int value;
int page_table[PAGE_ENTRIES]; // Page table.
int tlb[TLB_ENTRIES][2]; // Translation look-aside buffer.
int TLB_Front = -1; // TLB front index, queue data structure.
int TLB_Back = -1; // TLB back index, queue data structure.
char memory[MEM_SIZE]; // Physical memory. Each char is 1 byte.
int mem_index = 0; // Points to beginning of first empty frame.

// Statistics variables. 
int fault_counter = 0; // Count the page faults.
int tlb_counter = 0; // TLB hit counter.
int address_counter = 0; // Counts addresses read from file.
float fault_rate; // Fault rate.
float tlb_rate; // TLB hit rate.

// Functions.
int Get_Physical(int virtual);
int Get_Offset(int virtual);
int Get_Page_Number(int virtual);
void Initialize_Page_Table(int n);
void Initialize_TLB(int n);
int Consult_Page_Table(int page_number);
int Consult_TLB(int page_number);
void Update_TLB(int page_number, int frame_number);
int Get_Frame();

int main(int argc, char *argv[]) {
    char* in_file; // Address file name.
    char* out_file; // Output file name.
    char* store_file; // Store file name.
    char* store_data; // Store file data.
    int store_fd; // Store file descriptor.
    char line[8]; // Temp string for holding each line in in_file.
    FILE* in_ptr; // Address file pointer.
    FILE* out_ptr; // Output file pointer.

    // Initialize page_table, set all elements to -1.
    Initialize_Page_Table(-1);
    Initialize_TLB(-1);

    // Get command line arguments.
    if (argc != 4) {
        printf("Enter input, output, and store file names!");

        exit(EXIT_FAILURE);
    }
    // Else, proceed execution.
    else {
        // Get the file names from argv[].
        in_file = argv[1];
        out_file = argv[2];
        store_file = argv[3];

        // Open the address file. 
        if ((in_ptr = fopen(in_file, "r")) == NULL) {
            // If fopen fails, print error and exit. 
            printf("Fail to open input file.\n");

            exit(EXIT_FAILURE);
        }

        // Open the output file. 
        if ((out_ptr = fopen(out_file, "a")) == NULL) {
            // If fopen fails, print error and exit. 
            printf("Fail to open output file.\n");
            exit(EXIT_FAILURE);
        }

        // Open the store file. 
        // Map the store file to memory. 
        // Initialize the file descriptor. 
        store_fd = open(store_file, O_RDONLY);
        store_data = mmap(0, MEM_SIZE, PROT_READ, MAP_SHARED, store_fd, 0);
        // Check that the mmap call succeeded. .
        if (store_data == MAP_FAILED) {
            close(store_fd);
            printf("Error mmapping the backing store file!");
            exit(EXIT_FAILURE);
        }

        // Loop through the input file one line at a time. .
        while (fgets(line, sizeof(line), in_ptr)) {
            // Read a single address from file, assign to virtual. .
            virtual = atoi(line);
            // Increment address counter. .
            address_counter++;

            // Get the page_number from the virtual address. .
            page_number = Get_Page_Number(virtual);
            // Get the offset from the virtual address. .
            offset = Get_Offset(virtual);

            // Use page_number to find frame_number in TLB, if it exists. .
            frame_number = Consult_TLB(page_number);

            // Check frame_number returned by Consult_TLB function. . 
            if (frame_number != -1) {
                // TLB lookup succeeded. .
                // No update to TLB required. .
                physical = frame_number + offset;

                // No store file access required... .
                // Fetch the value directly from memory. .
                value = memory[physical];
            }
            else {
                // TLB lookup failed. .
                // Look for frame_number in page table instead. .
                frame_number = Consult_Page_Table(page_number);

                // Check frame number from Consult_Page_Table. .
                if (frame_number != -1) {
                    // No page fault. .
                    physical = frame_number + offset;

                    // Update TLB. .
                    Update_TLB(page_number, frame_number);

                    // No store file access required... .
                    // Fetch the value directly from memory. .
                    value = memory[physical];
                }
                else {
                    // Page fault! .
                    // When a page fault occurs, you will read in a 256-byte
                     //page from the file BACKING_STORE.bin and store it in 
                     //an available page frame in the physical memory. .
                    
                    // Seek to the start of the page in store_ptr file. .
                    int page_address = page_number * PAGE_SIZE;

                    // Check if a free frame exists. . 
                    if (mem_index != -1) {
                        // Success, a free frame exists. .
                        // Store the page from store file into memory frame. .
                        memcpy(memory + mem_index, 
                               store_data + page_address, PAGE_SIZE);

                        // Calculate physical address of specific byte. .
                        frame_number = mem_index;
                        physical = frame_number + offset;
                        // Fetch value. .
                        value = memory[physical];

                        // Update page_table with correct frame number. .
                        page_table[page_number] = mem_index;
                        // Update TLB. .
                        Update_TLB(page_number, frame_number);

                        // Increment mem_index. .
                        if (mem_index < MEM_SIZE - FRAME_SIZE) {
                            mem_index += FRAME_SIZE;
                        }
                        else {
                            // Set mem_index to -1, indicating memory full. . 
                            mem_index = -1;
                        }
                    }
                    else {
                        // Failed, no free frame in memory exists. .
                        // Swap! .
                    }
                }
            }

            // Append the results to out_file. .
            fprintf(out_ptr, "Virtual address: %d ", virtual); 
            fprintf(out_ptr, "Physical address: %d ", physical);
            fprintf(out_ptr, "Value: %d\n", value);
        }

        // Calculate rates. .
        fault_rate = (float) fault_counter / (float) address_counter;
        tlb_rate = (float) tlb_counter / (float) address_counter;

        // Print the statistics to the end of the output file. .
        fprintf(out_ptr, "Number of Translated Addresses = %d\n", address_counter); 
        fprintf(out_ptr, "Page Faults = %d\n", fault_counter);
        fprintf(out_ptr, "Page Fault Rate = %.3f\n", fault_rate);
        fprintf(out_ptr, "TLB Hits = %d\n", tlb_counter);
        fprintf(out_ptr, "TLB Hit Rate = %.3f\n", tlb_rate);

        // Close all three files. .
        fclose(in_ptr);
        fclose(out_ptr);
        close(store_fd);
    }

    return EXIT_SUCCESS;
}

// Functions. .
// Calculate and return the physical address. .
int Get_Physical(int virtual) {
    // Calculate the physical address value. .
    physical = Get_Page_Number(virtual) + Get_Offset(virtual);

    return physical;
}

// Calculate and return the page number. .
int Get_Page_Number(int virtual) {
    // Shift virtual to the right by n bits. .
    return (virtual >> PAGE_NUM_BITS);
}

// Calculate and return the offset value. .
int Get_Offset(int virtual) {
    // Mask is decimal representation of 8 binary 1 digits.
     // It is actually 2^8-1. 
    int mask = 255;
    return virtual & mask;
}

// Sets all page_table elements to integer n. .
void Initialize_Page_Table(int n) {
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        page_table[i] = n;
    }
}

// Sets all TLB elements to integer n. .
void Initialize_TLB(int n) {
    for (int i = 0; i < TLB_ENTRIES; i++) {
        tlb[i][0] = -1;
        tlb[i][1] = -1;
    }
}

// Takes a page_number and checks for a corresponding frame number. .
int Consult_Page_Table(int page_number) {
    if (page_table[page_number] == -1) {
        fault_counter++;
    }

    return page_table[page_number];
}

// Takes a page_number and checks for a corresponding frame number. .
int Consult_TLB(int page_number) {
    // If page_number is found, return the corresponding frame number. .
    for (int i = 0; i < TLB_ENTRIES; i++) {
        if (tlb[i][0] == page_number) {
            // TLB hit! .
            tlb_counter++;

            return tlb[i][1];
        }
    }

    // If page_number doesn't exist in TLB, return -1. .
    // TLB miss! .
    return -1;
}

void Update_TLB(int page_number, int frame_number) {
    // Use FIFO policy. .
    if (TLB_Front == -1) {
        // Set front and back indices. .
        TLB_Front = 0;
        TLB_Back = 0;
        // Update TLB. .
        tlb[TLB_Back][0] = page_number;
        tlb[TLB_Back][1] = frame_number;
    }
    else {
        // Use circular array to implement queue. .
        TLB_Front = (TLB_Front + 1) % TLB_ENTRIES;
        TLB_Back = (TLB_Back + 1) % TLB_ENTRIES;
        // Insert new TLB entry in the back. .
        tlb[TLB_Back][0] = page_number;
        tlb[TLB_Back][1] = frame_number;
    }
    return;
}