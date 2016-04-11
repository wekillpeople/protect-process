#include <ntifs.h>
#include <string.h>
#include <fltKernel.h>
#include <ioctls.h>
#include <anarchy.h>

#define NT_DEVICE_NAME      L"\\Device\\ProtectProcess"
#define DOS_DEVICE_NAME     L"\\DosDevices\\ProtectProcess"


PFLT_FILTER FilterHandle;

FLT_OPERATION_REGISTRATION Callbacks[] =
{
	{ IRP_MJ_CREATE,
	0,
	PRE_MJ_CREATE,
	NULL },
	{ IRP_MJ_CLOSE,
	0,
	NULL,
	NULL },
	{ IRP_MJ_DEVICE_CONTROL,
	0,
	PRE_MJ_DEVICE_CONTROL,
	NULL },
	{
		IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
		0,
		PRE_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
		NULL
	},
	{ IRP_MJ_OPERATION_END }
};

FLT_REGISTRATION FilterRegistration =
{
	sizeof(FLT_REGISTRATION),
	FLT_REGISTRATION_VERSION,
	0,
	NULL,
	Callbacks,
	FltUnload,
	InstanceSetup,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL

};

NTSTATUS InstanceSetup(__in PCFLT_RELATED_OBJECTS FltObjects, __in FLT_INSTANCE_SETUP_FLAGS Flags, __in DEVICE_TYPE VolumeDeviceType, __in FLT_FILESYSTEM_TYPE VolumeFilesystemType)
{
	return STATUS_SUCCESS;
}

NTSTATUS FltUnload(FLT_FILTER_UNLOAD_FLAGS Flags)
{
	FltUnregisterFilter(FilterHandle);

	return STATUS_SUCCESS;
}

FLT_PREOP_CALLBACK_STATUS PRE_MJ_CREATE(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID *CompletionContext)
{
	NTSTATUS status;
	PFLT_FILE_NAME_INFORMATION nameInfo = NULL;


	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &nameInfo);


	if (!NT_SUCCESS(status))
	{
		status = FltGetFileNameInformationUnsafe(FltObjects->FileObject, FltObjects->Instance, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &nameInfo);
	}

	if (nameInfo)
	{
		// block access
		if (wcsstr(nameInfo->Name.Buffer, L"your_file.exe") != NULL)
		{
			Data->IoStatus.Status = STATUS_NO_SUCH_FILE;
			Data->IoStatus.Information = 0;
			return FLT_PREOP_COMPLETE;
		}

		FltReleaseFileNameInformation(nameInfo);
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS PRE_MJ_DEVICE_CONTROL(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID *CompletionContext)
{
	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS PRE_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID *CompletionContext)
{
	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	ntStatus = FltRegisterFilter(DriverObject, &FilterRegistration, &FilterHandle);

	ASSERT(NT_SUCCESS(ntStatus));

	if (NT_SUCCESS(ntStatus))
	{
		ntStatus = FltStartFiltering(FilterHandle);

		if (!NT_SUCCESS(ntStatus))
		{
			FltUnregisterFilter(FilterHandle);
		}
	}

	return ntStatus;
}

VOID DrvUnload(IN PDRIVER_OBJECT DriverObject)
{
	PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;
	FltUnregisterFilter(FilterHandle);
}