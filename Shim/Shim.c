#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>
#include <Protocol/LoadedImage.h>

#include <Library/MemoryAllocationLib.h>
#include <Guid/FileInfo.h>
#include <Library/DebugLib.h>

#define VERBOSE
#undef REALLYVERBOSE
#undef RECURSE_DIRECTORIES

EFI_STATUS
EFIAPI
ProcessFilesInDir (
IN EFI_FILE_HANDLE Dir,
IN EFI_DEVICE_PATH *DirDp
);

EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
    )
{
    volatile UINTN debug_status = 0xCAFEBABE00000000;
    EFI_STATUS Status = EFI_SUCCESS;

#ifdef VERBOSE
    SystemTable->ConOut->SetMode(SystemTable->ConOut, 0);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hacked!\r\n");
#endif
    
	EFI_HANDLE AgentHandle;
	UINTN NumHandles;
	EFI_HANDLE *Handles;
	UINTN Index;
	VOID *Context;

	AgentHandle = ImageHandle;
	
	// gets all handles with simple file system installed
	Status = gBS->LocateHandleBuffer ( ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &NumHandles, &Handles);
	if (EFI_ERROR (Status)) {
		return Status;
	}
	
	// loop through all handles we just got
	for (Index = 0; Index < NumHandles; Index++) {
		EFI_FILE_HANDLE Root;
		EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs;
		EFI_DEVICE_PATH *Dp;
	
		// get simple file system protocol instance
		// from current handle
		Status = gBS->OpenProtocol ( Handles[Index], &gEfiSimpleFileSystemProtocolGuid, (void **)&Fs, NULL, AgentHandle, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (EFI_ERROR (Status)) {
			continue;
		}
		
		// get device path instance from current handle
		Status = gBS->OpenProtocol ( Handles[Index], &gEfiDevicePathProtocolGuid, (void **)&Dp, NULL, AgentHandle, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (EFI_ERROR (Status)) {
			continue;
		}
		
		// open root dir from current simple file system
		Status = Fs->OpenVolume (Fs, &Root);
		if (EFI_ERROR (Status)) {
			continue;
		}
		
		// recursively process files in root dir
		Context = NULL;
		Status = ProcessFilesInDir (Root, Dp);
		Root->Close (Root);
		if (EFI_ERROR (Status)) {
			continue;
		}
	
	}
	
#ifdef REALLYVERBOSE
	gST->ConOut->OutputString(gST->ConOut, L"Done successfully\n");
#endif

    debug_status = 0xCAFEBABE44444444;
    while ( 1 ) { }

    return Status;
}


#define MAX_FILE_INFO_SIZE 1024

EFI_STATUS
EFIAPI
ProcessFilesInDir (
IN EFI_FILE_HANDLE Dir,
IN EFI_DEVICE_PATH *DirDp
)
{
	EFI_STATUS Status;
	EFI_FILE_INFO *FileInfo;
	CHAR16 *FileName;
	UINTN FileInfoSize;
	EFI_DEVICE_PATH *Dp;
#ifdef REALLYVERBOSE
	CHAR16 Buf[1024];
#endif
	
	// big enough to hold EFI_FILE_INFO struct and the whole file path
	FileInfo = AllocatePool (MAX_FILE_INFO_SIZE);
	if (FileInfo == NULL) {
		return EFI_OUT_OF_RESOURCES;
	}
	
	for (;;) {
		// get the next file's info. there's an internal position
		// that gets incremented when you read from a directory
		// so that subsequent reads gets the next file's info
		FileInfoSize = MAX_FILE_INFO_SIZE;
		Status = Dir->Read (Dir, &FileInfoSize, (VOID *) FileInfo);
		if (EFI_ERROR (Status) || FileInfoSize == 0) {
			FreePool (FileInfo);
			return Status;
		}
		
		FileName = FileInfo->FileName;
		
		// skip files named . or ..
		if (StrCmp (FileName, L".") == 0 || StrCmp (FileName, L"..") == 0) {
			continue;
		}
		
		// so we have absolute device path to child file/dir
		Dp = FileDevicePath (DirDp, FileName);
		if (Dp == NULL) {
			FreePool (FileInfo);
			return EFI_OUT_OF_RESOURCES;
		}
		
#ifdef REALLYVERBOSE
		// Do whatever processing on the file
		UnicodeSPrint(Buf, sizeof(Buf), L"Path = %s FileName = %s, FileSize = %d\n", ConvertDevicePathToText(DirDp, TRUE, TRUE), FileInfo->FileName, FileInfo->FileSize);
		gST->ConOut->OutputString(gST->ConOut, Buf);
#endif

		if (!StrCmp(FileInfo->FileName, L"Payload.efi")) {
			EFI_FILE_HANDLE PayloadHandle;
			void *PayloadBuf;
			UINTN PayloadSize;
			EFI_HANDLE AppImageHandle = NULL;
			UINTN ExitDataSize;
			volatile UINTN debug_status = 0xCAFEBABE00000000;
			EFI_STATUS second_status;

#ifdef VERBOSE
			gST->ConOut->OutputString(gST->ConOut, L"Found payload!\r\n");
#endif
			PayloadSize = FileInfo->FileSize;

			PayloadBuf = AllocatePool (PayloadSize);
			if (PayloadBuf == NULL) {
				return EFI_OUT_OF_RESOURCES;
			}

			Status = Dir->Open (Dir, &PayloadHandle, FileInfo->FileName, EFI_FILE_MODE_READ, 0);
			if (Status != EFI_SUCCESS) {
				FreePool (PayloadBuf);
				return Status;
			}
			PayloadHandle->SetPosition (PayloadHandle, 0);
			Status = PayloadHandle->Read (PayloadHandle, &PayloadSize, (VOID *) PayloadBuf);

			Status = gBS->LoadImage(FALSE, gImageHandle, NULL, PayloadBuf, PayloadSize, &AppImageHandle);
    			if (Status != EFI_SUCCESS) {
#ifdef VERBOSE
//    			    UnicodeSPrint(Buf, sizeof(Buf), L"LoadImage Failed: %lx\r\n", Status);
 //   			    gST->ConOut->OutputString(gST->ConOut, Buf);
  			    gST->ConOut->OutputString(gST->ConOut, L"LoadImage failed!\n");
#endif
			    second_status = Status;
    			    debug_status = 0xCAFEBABE11111111;
    			    while ( 1 ) { }
    			}
#ifdef VERBOSE
    			gST->ConOut->OutputString(gST->ConOut, L"LoadImage OK!\n");
#endif

    			Status = gBS->StartImage(AppImageHandle, &ExitDataSize, (CHAR16**) NULL);
    			if (Status != EFI_SUCCESS) {
#ifdef VERBOSE
    			    gST->ConOut->OutputString(gST->ConOut, L"StartImage Failed!\n");
#endif
    			    debug_status = 0xCAFEBABE33333333;
    			    while ( 1 ) { }
    			}
#ifdef VERBOSE
    			gST->ConOut->OutputString(gST->ConOut, L"StartImage OK!\n");
#endif
			
			Dir->Close (PayloadHandle);
			if (Status != EFI_SUCCESS) {
				FreePool (PayloadBuf);
				return Status;
			}
			
		}
		
#ifdef RECURSE_DIRECTORIES
		if (FileInfo->Attribute & EFI_FILE_DIRECTORY) {
			//
			// recurse
			//
			
			EFI_FILE_HANDLE NewDir;
			
			Status = Dir->Open (Dir, &NewDir, FileName, EFI_FILE_MODE_READ, 0);
			if (Status != EFI_SUCCESS) {
				FreePool (FileInfo);
				FreePool (Dp);
				return Status;
			}
			NewDir->SetPosition (NewDir, 0);
			
			Status = ProcessFilesInDir (NewDir, Dp);
			Dir->Close (NewDir);
			if (Status != EFI_SUCCESS) {
				FreePool (FileInfo);
				FreePool (Dp);
				return Status;
			}
		}
#endif
		
		FreePool (Dp);
	}
}
