extern "C"{
    #include "libc/system.h"
    #include "memory/memory.h"
    #include "common.h"
    #include "interrupts.h"
    #include "input.h"
    #include "song/song.h"
}

// Existing global operator new overloads
void* operator new(size_t size) {
    return malloc(size);
}

void* operator new[](size_t size) {
    return malloc(size);
}

// Existing global operator delete overloads
void operator delete(void* ptr) noexcept {
    free(ptr);
}

void operator delete[](void* ptr) noexcept {
    free(ptr);
}

// Add sized-deallocation functions
void operator delete(void* ptr, size_t size) noexcept {
    (void)size; // Size parameter is unused, added to match required signature
    free(ptr);
}

void operator delete[](void* ptr, size_t size) noexcept {
    (void)size; // Size parameter is unused, added to match required signature
    free(ptr);
}


SongPlayer* create_song_player() {
    auto* player = new SongPlayer();
    player->play_song = play_song_impl;
    return player;
}

extern "C" int kernel_main(void);
int kernel_main(){

    // Set up interrupt handlers
    register_interrupt_handler(3, [](registers_t* regs, void* context) {
        printf("Interrupt 3 - Working\n");
    }, NULL);

    register_interrupt_handler(4, [](registers_t* regs, void* context) {
        printf("Interrupt 4 - Working\n");
    }, NULL);

    
    register_interrupt_handler(14, [](registers_t* regs, void* context) {

        uint32_t faulting_address;
        __asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));

        int32_t present = !(regs->err_code & 0x1);
        int32_t rw = regs->err_code & 0x2;
        int32_t us = regs->err_code & 0x4;
        int32_t reserved = regs->err_code & 0x8;
        int32_t id = regs->err_code & 0x10;


        printf("Page fault! (");
        if (present)
            printf("present");
        if (rw)
            printf("read-only");
        if (us)
            printf("user-mode");
        if (reserved)
            printf("reserved");
        printf(")\n\n");
        panic("Page fault");

    }, NULL);

    // Trigger interrupts to test handlers
    asm volatile ("int $0x3");
    asm volatile ("int $0x4");

    // Enable interrupts
    asm volatile("sti");

    // Register IRQ handler for keyboard (IRQ1)
    register_irq_handler(IRQ1, [](registers_t*, void*) {
        // Read from keyboard
        unsigned char scan_code = inb(0x60);
        char f = scancode_to_ascii(&scan_code);

        printf("%c", f);

        // Disable
        asm volatile("cli");
    }, NULL);


    Song* songs[] = {
        new Song({interstaller_theme, sizeof(interstaller_theme) / sizeof(Note)}),
    };
    uint32_t n_songs = sizeof(songs) / sizeof(Song*);

    // Create a song player and play each song
    SongPlayer* player = create_song_player();
    for(uint32_t i = 0; i < n_songs; i++) {
        printf("Playing Interstaller Theme Music...\n");
        player->play_song(songs[i]);
        printf("By Hans Zimmer...\n");
    }

    // Main loop
    printf("          _                        _                   \n");
    printf("    _ __ (_) __   ____ _ _ __   __| |_ __ ___ _ __ ___ \n");
    printf("   | '_ \\| | \\ \\ / / _` | '_ \\ / _` | '__/ _ \\ '__/ _ \\\n");
    printf("   | | | | |  \\ V / (_| | | | | (_| | | |  __/ | |  __/\n");
    printf("   |_| |_|_|   \\_/ \\__,_|_| |_|\\__,_|_|  \\___|_|  \\___|\n");
    printf("                                                       \n");

          
    while(true) {
        // Kernel main tasks
    }

    // This part will not be reached
    printf("Done!\n");
    return 0;
}