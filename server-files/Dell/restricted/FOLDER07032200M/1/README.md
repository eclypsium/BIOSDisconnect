Replace the placeholder in this directory with whatever you want to download
and run as the third stage executable.

A good candidate to allow you to examine the system post-exploitation is 
the UEFI Shell available here:

https://github.com/tianocore/edk2/raw/UDK2018/ShellBinPkg/UefiShell/X64/Shell.efi

This file must be the largest file downloaded via the Csos json file as these
files are downloaded in order of size from smallest to largest and we want the
sha256 overflow to correspond to the last payload file we download.
