/* vioinput_log.h — 简易内核文件日志 (调试用, 写 C:\vioinput-drv.log) */
#ifndef VIOINPUT_LOG_H
#define VIOINPUT_LOG_H

#include <ntddk.h>
#include <wdm.h>

#define VIOINPUT_LOG_PATH L"\\??\\C:\\vioinput-drv.log"

static ULONG vioinput_log_seq = 0;

static NTSTATUS
VIOInputLogWrite(const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    ULONG len;
    NTSTATUS status;
    HANDLE h;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK iosb;
    LARGE_INTEGER off;
    UNICODE_STRING path;

    va_start(ap, fmt);
    len = (ULONG)_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (len >= sizeof(buf))
        len = sizeof(buf) - 1;

    RtlInitUnicodeString(&path, VIOINPUT_LOG_PATH);
    InitializeObjectAttributes(&oa, &path, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    off.QuadPart = 0;

    status = ZwCreateFile(&h, GENERIC_WRITE, &oa, &iosb, NULL,
        FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);
    if (!NT_SUCCESS(status))
        return status;

    /* 写前缀 [seq] 然后写内容 */
    {
        char prefix[48];
        int plen = sprintf(prefix, "[%lu] ", ++vioinput_log_seq);
        if (plen > 0)
            ZwWriteFile(h, NULL, NULL, NULL, &iosb, prefix, (ULONG)plen, &off, NULL);
    }
    ZwWriteFile(h, NULL, NULL, NULL, &iosb, buf, len, &off, NULL);
    ZwClose(h);
    return STATUS_SUCCESS;
}

#define VIOINPUT_LOG(...) VIOInputLogWrite(__VA_ARGS__)

#endif
