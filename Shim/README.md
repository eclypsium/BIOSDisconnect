This executable is the second stage payload which was appended to the first
stage loader stub by the exploit generation script in order to generate the
hex string to place in the sha256 field.

Just a simple example of using UEFI Boot Serices to iterate over available
filesystems to find an executable named Payload.efi to load and run.

This could be done more easily by just attempting to open a file with the
name we're looking for in the root of each filesystem, but this example
also includes some optional code to recursively search all directories to
find the next payload stage.

To build this, setup a EDK II build environment using the instructions at:

https://github.com/tianocore/tianocore.github.io/wiki/Getting-Started-with-EDK-II

The Shim directory should be placed in MdeModulePkg/Applications and
MdeModulePkg/Application/Shim/Shim.inf should be added to the [Components]
section of MdeModulePkg/MdeModulePkg.dsc in order to include this in the
build process.

