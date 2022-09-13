#include<ntddk.h>
#include<wdf.h>
#include<ntstrsafe.h>

// Declare the entry callback function invoked by the system.
DRIVER_INITIALIZE DriverEntry;

// Declare the callback function for driver invoked by the system.
EVT_WDF_DRIVER_DEVICE_ADD Amey_KMDFEvtDeviceAdd;

// Our own function declaration which will be called from 'Amey_KMDFEvtDeviceAdd' callback.
void create_systemroot_file();


NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	// Print debug message.
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL,
		(" (*) Amey_KMDF -> DriverEntry(), RegistryPath : %s\n", (PCSTR)(*RegistryPath).Buffer)
		));

	// Create & initialize driver configuration object with callback function.
	WDF_DRIVER_CONFIG config;
	WDF_DRIVER_CONFIG_INIT(&config, Amey_KMDFEvtDeviceAdd);

	// Create framework driver object for the driver & return status.
	return WdfDriverCreate(
		DriverObject,
		RegistryPath,
		WDF_NO_OBJECT_ATTRIBUTES,
		&config,
		WDF_NO_HANDLE
	);
}

NTSTATUS Amey_KMDFEvtDeviceAdd(_In_ WDFDRIVER Driver, _Inout_ PWDFDEVICE_INIT DeviceInit) {
	// As the Driver object is unused, mark it as unreferenced.
	UNREFERENCED_PARAMETER(Driver);

	// Print debug message.
	KdPrintEx(
		(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, " (*) Amey_KMDF -> Amey_KMDFEvtDeviceAdd()\n")
	);

	// Call to our own function! :)
	create_systemroot_file();

	// Create device object.
	WDFDEVICE device;

	// Create framework device object & return its status.
	return WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &device);
}

void create_systemroot_file() {
	// Print debug message.
	KdPrintEx(
		(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, " (*) Amey_KMDF -> create_systemroot_file()\n")
	);

	// Create variables for file name & object attributes for file.
	UNICODE_STRING unicodeFileName;
	OBJECT_ATTRIBUTES objectAttribute;

	// Assign file name.
	RtlInitUnicodeString(
		&unicodeFileName,
		L"\\SystemRoot\\Amey_KMDF_File.txt"  // Expands to: "C:\Windows\Amey_KMDF_File.txt"
	);

	// Initialize file object attributes.
	InitializeObjectAttributes(
		&objectAttribute,
		&unicodeFileName,
		(OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
		NULL,
		NULL
	);

	HANDLE fileHandle;
	NTSTATUS status = STATUS_SUCCESS;
	IO_STATUS_BLOCK ioStatusBlock;

	// Process only if no interrupts are masked off in the driver's dispatch routine.
	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		return;

	// Create file using 'fileHandle', 'objectAttribute' & 'ioStatusBlock'.
	status = ZwCreateFile(
		&fileHandle,
		GENERIC_WRITE,
		&objectAttribute,
		&ioStatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OVERWRITE_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);

	// Create buffer for data & variable to store length in bytes.
	CHAR dataBuffer[50];
	size_t lengthBytes;

	// Allocate data into buffer.
	status = RtlStringCbPrintfA(
		dataBuffer,
		sizeof(dataBuffer),
		"File written by Amey_KMDF kernel driver!"
	);

	if (NT_SUCCESS(status))
	{
		// Compute size of data in bytes.
		status = RtlStringCbLengthA(dataBuffer, sizeof(dataBuffer), &lengthBytes);
		if (NT_SUCCESS(status))
		{
			// Write data to file.
			status = ZwWriteFile(
				fileHandle,
				NULL,
				NULL,
				NULL,
				&ioStatusBlock,
				dataBuffer,
				(ULONG)lengthBytes,
				NULL,
				NULL
			);
		}
	}
	// Close file after writing.
	ZwClose(fileHandle);
}
