// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#ifndef XMOS_USER_APP_H
#define XMOS_USER_APP_H 

int xmos_init(void);
int xmos_check_and_handle_interrupt(void);
int connect(int fd);
int disconnect(int handle);
char* run_command(int num_args, const char** cmd);
#endif
