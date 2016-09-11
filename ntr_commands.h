// Copyright 2014 Normmatt
// Licensed under GPLv2 or any later version
// Taken from Decrypt9WIP by Kitlith (3 Sep 2016)
// Modified from #define to enum.

// Card commands
enum ntrcard_command {
    NTRCARD_CMD_DUMMY         = 0x9Fu,
    NTRCARD_CMD_HEADER_READ   = 0x00u,
    NTRCARD_CMD_HEADER_CHIPID = 0x90u,
    NTRCARD_CMD_ACTIVATE_BF   = 0x3Cu,  // Go into blowfish (KEY1) encryption mode
    NTRCARD_CMD_ACTIVATE_BF2  = 0x3Du,  // Go into blowfish (KEY1) encryption mode
    NTRCARD_CMD_ACTIVATE_SEC  = 0x40u,  // Go into hardware (KEY2) encryption mode
    NTRCARD_CMD_SECURE_CHIPID = 0x10u,
    NTRCARD_CMD_SECURE_READ   = 0x20u,
    NTRCARD_CMD_DISABLE_SEC   = 0x60u,  // Leave hardware (KEY2) encryption mode
    NTRCARD_CMD_DATA_MODE     = 0xA0u,
    NTRCARD_CMD_DATA_READ     = 0xB7u,
    NTRCARD_CMD_DATA_CHIPID   = 0xB8u
};
