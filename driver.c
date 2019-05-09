#include "imports.h"

const		WCHAR deviceName[] = L"\\Device\\BasicDriver";
const		WCHAR deviceSymbolicLink[] = L"\\DosDevices\\BasicDriver";

UNICODE_STRING unicodeDeviceNameBuffer, unicodeSymLinkBuffer;
PDEVICE_OBJECT g_pDeviceObject = 0;

void OnUnload(IN PDRIVER_OBJECT driverObject);
int InitUnicodeStrings();
int SetupIoDevice(PDRIVER_OBJECT pDriverObject);

NTSTATUS Io_Recieved(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Io_Unsupported(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Create_DeviceIo(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Close_DeviceIo(PDEVICE_OBJECT DeviceObject, PIRP Irp);


int InitUnicodeStrings() {

	RtlInitUnicodeString(&unicodeDeviceNameBuffer, deviceName);
	RtlInitUnicodeString(&unicodeSymLinkBuffer, deviceSymbolicLink);
	return 1;
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT driverObject, IN PUNICODE_STRING regPath)
{
	UNREFERENCED_PARAMETER(regPath);

	driverObject->DriverUnload = OnUnload;

	for (unsigned int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
		driverObject->MajorFunction[i] = Io_Unsupported;

	driverObject->MajorFunction[IRP_MJ_CREATE] = Create_DeviceIo;
	driverObject->MajorFunction[IRP_MJ_CLOSE] = Close_DeviceIo;
	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Io_Recieved;

	if (!InitUnicodeStrings())
		return STATUS_UNSUCCESSFUL; //report the driver could not be loaded.

	SetupIoDevice(driverObject);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BasicDriver]: My Driver loaded.\n");
	return STATUS_SUCCESS;
}

NTSTATUS Create_DeviceIo(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BasicDriver]: Handle to IO Device has been opened.\n");
	return STATUS_SUCCESS;
}

NTSTATUS Close_DeviceIo(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BasicDriver]: Handle to IO Device has been closed.\n");
	return STATUS_SUCCESS;
}

NTSTATUS Io_Recieved(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	return STATUS_SUCCESS;
}

NTSTATUS Io_Unsupported(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BasicDriver]: Unsupported Majour Function Requested. Returning STATUS_SUCCESS\n");
	return STATUS_SUCCESS;
}

int SetupIoDevice(PDRIVER_OBJECT pDriverObject) {

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BasicDriver]: Creating I\\O device under name: %ws\n", unicodeDeviceNameBuffer.Buffer);

	int ret = IoCreateDevice(pDriverObject, 0, &unicodeDeviceNameBuffer, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &g_pDeviceObject);

	g_pDeviceObject->Flags |= DO_BUFFERED_IO;
	g_pDeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

	if (ret != STATUS_SUCCESS) {

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BasicDriver]: IoCreateDevice failed.\n");
		return -1;
	}

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BasicDriver]: Device Created\n");

	ret = IoCreateSymbolicLink(&unicodeSymLinkBuffer, &unicodeDeviceNameBuffer);
	if (ret != STATUS_SUCCESS) {

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BasicDriver]: error creating symbolic link to %ws, named: %ws\n", unicodeDeviceNameBuffer.Buffer, deviceSymbolicLink);
		return -1;
	}

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BasicDriver]: Created symbolic link to %ws, named: %ws\n", unicodeDeviceNameBuffer.Buffer, unicodeSymLinkBuffer.Buffer);
	return 1;
}

VOID OnUnload(IN PDRIVER_OBJECT driverObject) {

	UNREFERENCED_PARAMETER(driverObject);
	if (g_pDeviceObject) {

		IoDeleteDevice(g_pDeviceObject);
		IoDeleteSymbolicLink(&unicodeSymLinkBuffer);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BasicDriver]: Deleted IO Device and symbolic link.\n");
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BasicDriver]: My Driver unloaded.\n");
}
