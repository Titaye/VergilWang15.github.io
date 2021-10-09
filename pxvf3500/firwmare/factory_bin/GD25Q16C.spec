35,                     /* 1. libflash device ID */
256,                    /* 2. Page size */
8192,                   /* 3. Number of pages */
3,                      /* 4. Address size */
4,                      /* 5. Clock divider */
0x9F,                   /* 6. RDID cmd */
0,                      /* 7. RDID dummy bytes */
3,                      /* 8. RDID data size in bytes */
0xC84015,               /* 9. RDID manufacturer ID */
0x20,                   /* 10. SE cmd */
4096,                   /* 11. SE full sector erase */
0x06,                   /* 12. WREN cmd */
0x04,                   /* 13. WRDI cmd */
PROT_TYPE_NONE,         /* 14. Protection type */
{{0,0},{0x00,0x00}},    /* 15. SR protect and unprotect cmds */
0x02,                   /* 16. PP cmd */
0xEB,                   /* 17. READ cmd */
1,                      /* 18. READ dummy bytes */
SECTOR_LAYOUT_REGULAR,  /* 19. Sector layout */
{4096,{0,{0}}},         /* 20. Sector sizes */
0x05,                   /* 21. RDSR cmd */
0x01,                   /* 22. WRSR cmd */
0x01,                   /* 23. WIP bit mask */
