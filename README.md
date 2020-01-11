# LxDK - Driver Development Kit for WSLinux (WSL1)

This is a driver development kit for WSL1. It enables the creation of drivers that interface with the WSL1 kernel component (`lxcore.sys`). These drivers present themselves to Linux programs as character devices with major number 10 and can be opened, read, written, etc.

The LxDK consists of the following:

- A driver loader (`lxldr.sys`) that is used to load and initialize drivers whenever a new `lxcore.sys` session is created.
- A set of header and library files that can be used to develop a driver that interfaces with `lxcore.sys` and presents itself as a Linux character device.
- A sample (`lxtstdrv.sys`) that can be used to demo/test the LxDK.

The LxDK is distributed as a Windows MSI that includes signed drivers.

**NOTES:**
- This release should be considered of **ALPHA** quality and you should therefore try it in a VM.
- This release has been tested on Windows 1809 (10.0.17763.379) and Windows 1909 (10.0.18363.418). It will likely not work with versions earlier than 1809, but it should work with in-between and later versions. Please report your findings via the GitHub Issues list.

## Documentation

There is currently no documentation although the header files and the `lxtstdrv.sys` sample should be enough to set a determined system programmer on the right path.

## How to test

Here are steps to test this on your own:

- Install the latest `lxdk-*.msi`.
    - Install both Core and Developer components.
    - This will install and start `lxldr.sys`. It will also copy the `lxtstdrv.sys` files, but will not register them as a driver.
- *OPTIONAL:* Verify that the `lxldr.sys` driver is running.
    ```
    sc query lxldr
    ```
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
- Enable kernel `DbgPrint` via the registry.
    ```
    Windows Registry Editor Version 5.00

    [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter]
    "DEFAULT"=dword:0000000f
    ```
- Install [DebugView from Sysinternals](https://docs.microsoft.com/en-us/sysinternals/downloads/debugview).
- Start DebugView as Administrator.
    - From the menu select Capture > Capture Kernel to view debug output.
- Make sure that WSL1 is not currently running and start a new `bash.exe` WSL1 session.
- *OPTIONAL:* Verify that the `lxtstdrv.sys` driver is now running as well.
    ```
    sc query lxtstdrv
    ```
- You should immediately see in DebugView a log from `lxtstdrv.sys`. If you do not it means that either there was a WSL1 session already running when the LxDK was installed or that you have not enabled `DbgPrint` in the registry.
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

**NOTES:**
- The `lxldr.sys` driver currently only loads drivers such as `lxtstdrv.sys` when a new WSL1 session is created (see `CreateInitialNamespace`). If there is already a WSL1 session it fails to load any drivers. To solve this problem simply reboot Win10. The `lxldr.sys` driver is registered as a `SERVICE_SYSTEM_START` driver, so it loads early during boot (before any WSL1 sessions) and this problem will not happen again after the first reboot.
- In the future this problem may be worked around by either improving the `lxldr.sys` driver or by simply having the installer demand a reboot after installation.

## History

By a rather strange coincidence the LxDK was originally developed a week or two prior to Microsoft's announcement that WSL1 was being superseded by WSL2. After Microsoft's announcement (early May 2019) my interest in this project diminished and it was abandoned for a while. In early January 2020 I finally decided to make a proper release of it.
