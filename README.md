# EDK II UEFI Driver

This project contains PXE (Preboot Execution Environment) boot UNDI (Universal Network Driver Interface) driver code to build:

* IntelgbeUndiDxe.efi: Supporting Ethernet PHYs on NEX Platforms.

# How to Build for Windows

To build EFI executables, run the following commands to build:

```
  edksetup.bat rebuild   # Rebuild BaseTools (optional)
  edksetup.bat
  build -p IntelUndiPkg/IntelGigUndiPkg.dsc -t VS2019
```

# Windows Build Environment

* Microsoft Visual Studio 2019
* NASM
* Python 3.6.8

# How to load/unload UNDI Driver

Run the following commands from UEFI Shell to load/unload UNDI Driver. Please note that the driver loaded this way does not persist after power cycle:

```
  drivers                          # Display list of drivers
  unload <Driver ID>               # Eg: unload ED
  load <New IntelgbeUndiDxe.efi>   # Eg: load IntelgbeUndiDxe.efi
```

# How to permanently replace UNDI Driver in UEFI BIOS

1) Replace the existing UNDI driver file in the UEFI BIOS Source code and build the BIOS.
2) Flash the BIOS into the platform. The new UNDI driver will be loaded when the platform boots.

Note: For Intel Reference UEFI BIOS Source Code and steps to build UEFI BIOS, please contact your Independent BIOS Vendor (IBV).
