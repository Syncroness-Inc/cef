import ctypes
from enum import Enum, auto


class errorCode(Enum):
    errorCode_OK                                                                = 0
    errorCode_LogFatalReturn                                                    = 1
    errorCode_PointerIsNullptr                                                  = 2
    errorCode_reserved2                                                         = 3
    errorCode_reserved3                                                         = 4
    errorCode_reserved4                                                         = 5
    errorCode_reserved5                                                         = 6
    errorCode_IllegalCommandState                                               = 7
    errorCode_CmdPingReceiveValuesDoNotMatchExpectedValues                      = 8
    errorCode_CmdBaseImportCefCommandOpCodeDoesNotMatchCommand                  = 9
    errorCode_CmdBaseImportCefCommandNumBytesInCefRequestDoesNotMatch           = 11
    errorCode_NumApplicationErrorCodes                                          = auto()


class commandOpCode(Enum):
    commandOpCodeNone           = 0
    commandOpCodePing           = 1

    maxCommandOpCodeNumber      = auto()
    commandOpCodeInvalid        = 0xFFFF


class cefCommandHeader(ctypes.Structure):
    _fields_ = [
        ('m_commandOpCode', ctypes.c_uint16),
        ('m_commandSequenceNumber', ctypes.c_uint16),
        ('m_commandErrorCode', ctypes.c_uint32),
        ('m_commandNumBytes', ctypes.c_uint32),
        ('m_padding1', ctypes.c_uint32)
    ]


CMD_PING_UINT8_REQUEST_EXPECTED_VALUE   = 0xA3
CMD_PING_UINT16_REQUEST_EXPECTED_VALUE  = 0x93A3
CMD_PING_UINT32_REQUEST_EXPECTED_VALUE  = 0x208461A3
CMD_PING_UINT64_REQUEST_EXPECTED_VALUE  = 0x936217995202A373


class cefCommandPingRequest(ctypes.Structure):
    _fields_ = [
        ('m_header', cefCommandHeader),

        ('m_uint8Value', ctypes.c_uint8),
        ('m_padding1', ctypes.c_uint8),
        ('m_uint16Value', ctypes.c_uint16),
        ('m_testValue', ctypes.c_uint32),
        ('m_uint32Value', ctypes.c_uint32),
        ('m_padding2', ctypes.c_uint32),
        ('m_offsetToAddToResponse', ctypes.c_uint64),
        ('m_uint64Value', ctypes.c_uint64)
    ]


class cefCommandPingResponse(ctypes.Structure):
    _fields_ = [
        ('m_header', cefCommandHeader),

        ('m_uint8Value', ctypes.c_uint8),
        ('m_padding1', ctypes.c_uint8),
        ('m_uint16Value', ctypes.c_uint16),
        ('m_testValue', ctypes.c_uint32),
        ('m_uint32Value', ctypes.c_uint32),
        ('m_padding2', ctypes.c_uint32),
        ('m_uint64Value', ctypes.c_uint64)
    ]