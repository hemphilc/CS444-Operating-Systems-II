typedef struct _DEVICE_EXTENSION {
  KEVENT evKill;
  PKTHREAD thread;
};

NTSTATUS StartThread(PDEVICE_EXTENSION pdx)
{
  NTSTATUS status;
  HANDLE hthread;
  KeInitializeEvent(&pdx->evKill, NotificationEvent, FALSE);
  status = PsCreateSystemThread(&hthread, THREAD_ALL_ACCESS,
    NULL, NULL, NULL, (PKSTART_ROUTINE) ThreadProc, pdx);
  if (!NT_SUCCESS(status))
    return status;
  ObReferenceObjectByHandle(hthread, THREAD_ALL_ACCESS, NULL,
    KernelMode, (PVOID*) &pdx->thread, NULL);
  ZwClose(hthread);
  return STATUS_SUCCESS;
}

VOID StopThread(PDEVICE_EXTENSION pdx)
{
  KeSetEvent(&pdx->evKill, 0, FALSE);
  KeWaitForSingleObject(pdx->thread, Executive, KernelMode, FALSE, NULL);
  ObDereferenceObject(pdx->thread);
}

VOID ThreadProc(PDEVICE_EXTENSION pdx)
{
  KeWaitForXxx(<at least pdx->evKill>);
  PsTerminateSystemThread(STATUS_SUCCESS);
}
