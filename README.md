# RoadscopePCBInspector

It can help to inspect Roadscope9 PCBA by UART communication.

## Usage environment

* Windows 10/11 64bit PC

## Prerequisites

* An USB UART cable to connect Roadscope9 RTOS

## How to inspect

1. Connect USB UART cable to you PC, and figure out which COM port is assigned.

2. Turn on the PCBA and click the TEST button.

3. The test result will be shown within 30 seconds.

----

# Tool Development

## Environment

* Qt
  * https://d13lb3tujbc8s0.cloudfront.net/onlineinstallers/qt-unified-windows-x64-4.6.0-online.exe
  * QT 6.5.2
  * Qt Creator 11.0.2
  * Compiler: MinGW 11.2.0 64-bit C/C++

## How to build

* Select the CMakeLists.txt file included in the project to open the Qt Creator project.

* Switch to Proejcts mode (Ctrl+5)

* Double-click "Desktop Qt 6.5.2 MinGW 64-bit" in the "Build & Run" list to activate it. When activaged, it will be displayed in bold font.

* (Optional) You can select "Build-->Run CMake" from the menu bar.
  * However, this step is usually performed automatically.

* Next, Click "Build-->Rebuild" from the menu bar to build your project.
  * The output files will be located in the new build directory created within the parent directory for this proeject.

## Deployment

The `RoadscopePcbInspector.exe` file won't execute directly as it can't find the linked DLL libraries.

The `windeployqt6.exe` tool assists in gathering the nessecary files to execution.

Here's what you need to do:
* Create an empty directory.
* Copy the output .exe file (`RoadscopePcbInsepector.exe`) to the directory you created.
* Open `PowerShell` in this directory.
* Run the following command to execute `windeployqt6.exe`:

```
> C:\Qt\6.5.2\mingw_64\bin\windeployqt6.exe .\RoadscopePcbInspector.exe --compiler-runtime
```
All the required files to run `RoadscopePcbInspector.exe` will be placed in the directory you created.

Note: `windeployqt6.exe` may not function propperly when output is built in Release mode. This appears to be a bug in the tool.
