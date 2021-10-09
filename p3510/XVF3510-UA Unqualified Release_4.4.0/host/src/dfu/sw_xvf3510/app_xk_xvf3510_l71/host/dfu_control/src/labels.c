// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include "dfu_commands.h"
#include "dfu_types.h"
#include "labels.h"

const char *command_str(enum dfu_command command)
{
  switch (command) {
    case DFU_CMD_DETACH:            return "DETACH";
    case DFU_CMD_BUS_RESET:         return "BUS_RESET";
    case DFU_CMD_DNLOAD:            return "DNLOAD";
    case DFU_CMD_CLRSTATUS:         return "CLRSTATUS";
    case DFU_CMD_GETSTATE:          return "GETSTATE";
    case DFU_CMD_GETSTATUS:         return "GETSTATUS";
    case DFU_CMD_REBOOT:            return "REBOOT";
    case DFU_CMD_GET_ERROR_INFO:    return "GET_ERROR_INFO";
    case DFU_CMD_OVERRIDE_SPISPEC:  return "OVERRIDE_SPISPEC";
    default: return "?";
  }
}

const char *state_str(enum dfu_state state)
{
  switch (state) {
    case APP_IDLE:                return "appIDLE";
    case APP_DETACH:              return "appDETACH";
    case DFU_IDLE:                return "dfuIDLE";
    case DFU_DNLOAD_SYNC:         return "dfuDNLOAD-SYNC";
    case DFU_DNBUSY:              return "dfuDNBUSY";
    case DFU_DNLOAD_IDLE:         return "dfuDNLOAD-IDLE";
    case DFU_MANIFEST_SYNC:       return "dfuMANIFEST-SYNC";
    case DFU_MANIFEST:            return "dfuMANIFEST";
    case DFU_MANIFEST_WAIT_RESET: return "dfuMANIFEST-WAIT-RESET";
    case DFU_UPLOAD_IDLE:         return "dfuUPLOAD-IDLE";
    case DFU_ERROR:               return "dfuERROR";
    default: return "?";
  }
}

const char *status_str(enum dfu_status status)
{
  switch (status) {
    case DFU_OK:           return "OK";
    case ERR_TARGET:       return "errTARGET";
    case ERR_FILE:         return "errFILE";
    case ERR_WRITE:        return "errWRITE";
    case ERR_ERASE:        return "errFILE";
    case ERR_CHECK_ERASED: return "errCHECK_ERASED";
    case ERR_PROG:         return "errPROG";
    case ERR_VERIFY:       return "errVERIFY";
    case ERR_ADDRESS:      return "errADDRESS";
    case ERR_NOTDONE:      return "errNOTDONE";
    case ERR_FIRMWARE:     return "errFIRMWARE";
    case ERR_VENDOR:       return "errVENDOR";
    case ERR_USBR:         return "errUSBR";
    case ERR_POR:          return "errPOR";
    case ERR_UNKNOWN:      return "errUNKNOWN";
    case ERR_STALLED_PKT:  return "errSTALLEDPKT";
    default: return "?";
  }
}
