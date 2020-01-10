# LxDK - Driver Development Kit for WSLinux (WSL1)

This is a driver development kit for WSL1. It enables the creation of drivers that interface with the WSL1 kernel component (`lxcore.sys`). These drivers present themselves to Linux programs as character devices with major number 10 and can be opened, read, written, etc.

The LxDK consists of the following:

- A driver loader (`lxldr.sys`) that is used to load and initialize drivers whenever a new `lxcore.sys` session is created.
- A set of header and library files that can be used to develop a driver that interfaces with `lxcore.sys` and presents itself as a Linux character device.
- A sample (`lxtstdrv.sys`) that can be used to demo/test the LxDK.

The LxDK is distributed as a Windows MSI that includes signed drivers.

**Note that this release should be considered of ALPHA quality and you should therefore try it in a VM. Please also note that I have only tested this on Windows 10.0.17763.379. It will likely not work with earlier versions, and it may or may not work with later versions. Please report your findings via the GitHub Issues list.**

## Documentation

There is currently no documentation although the header files and the `lxtstdrv.sys` sample should be enough to set a determined system programmer on the right path.

## How to test

Here are steps to test this on your own:

- Install the latest `lxdk-*.msi`.
    - Install both Core and Developer components.
    - This will install and start `lxldr.sys`. It will also copy the `lxtstdrv.sys` files, but will not register them as a driver.
- Use the command line to register `lxtstdrv.sys`:
    - Change to the LxDK bin directory:
        ```
        cd \Program Files\LxDK\bin
        ```
    - Register `lxtstdrv.sys`:
        ```
        lxreg.bat lxtstdrv lxtstdrv.sys
        ```

    - Unregister `lxtstdrv.sys` (once done with testing):
        ```
        lxreg.bat -u lxtstdrv
        ```
- Install [DebugView from Sysinternals](https://docs.microsoft.com/en-us/sysinternals/downloads/debugview).
- Start DebugView as Administrator.
    - From the menu select Capture > Capture Kernel to view debug output.
    - NOTE: You may also need to enable `DbgPrint` in the registry.
        ```
        Windows Registry Editor Version 5.00

        [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter]
        "DEFAULT"=dword:0000000f
        ```
- Make sure that WSL1 is not currently running and start a new `bash.exe` WSL1 session.
- You should immediately see in DebugView a log from `lxtstdrv.sys`. The function `CreateInitialNamespace` is only called when a new WSL1 session is started.
    ```
    CreateInitialNamespace(Instance=XXXXXXXXXXXXXXXX) = 0
    ```
- From the `bash.exe` prompt issue the following commands:
    ```
    $ cd /dev
    $ sudo mknod lxtst c 10 375486
    $ sudo chmod a+rw lxtst

    # after trying each of the following commands observe the output in DebugView
    $ echo hello > lxtst
    $ cat lxtst
    ```

## History

By a rather strange coincidence the LxDK was originally developed a week or two prior to Microsoft's announcement that WSL1 was being superseded by WSL2. After Microsoft's announcement (early May 2019) my interest in this project diminished and it was abandoned for a while. In early January 2020 I finally decided to make a proper release of it.
