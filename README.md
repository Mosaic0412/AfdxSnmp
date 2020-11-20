# AfdxSnmp

# Overview

This is a project. It can help you realize the function of sending snmp frames in the snmp protocol through the ARINC664 board and get a reply.

If you're never used arinc664 card before,or you're trying to figure out how to use vcpkg, check out my Getting Started section section for how to start using arinc664 card to create a project.



# Table of Contents

- [Overview](#2)
- Table of Contents
- Getting Started
  - Quick Start : Windows
  - Windows Developer Tools

# Getting Started

First , follow the quick start for windows,depending on what you're using.

### Quick Start : Windows

Prerequisites:

- Windows 7 or newer
- [Visual Studio](https://visualstudio.microsoft.com/zh-hans/) 2013 or newer

#### Install the board driver

1. First, you need to install the arinc664 driver. The driver is in the /driver folder. Open the folder and you will see a file named setup.exe, which is the driver installation package.
2. According to your situation, select the installation location, confirm that there is no problem, click next, and the driver will be installed automatically.
3. After the driver installation process is over, you will see the check box to *update the system driver* (selected by default), click Finish.
4. Automatically jump out of the *Device Driver Installation Wizard*, to continue, click next.
5. After the two installations are over, open [*C:\Program Files\AIM GmbH\Arinc 664 Windows BSP 19.2.1*](file:///C:/Program Files/AIM GmbH/Arinc 664 Windows BSP 19.2.1) (if installed by default), and you will see the driver file directory you installed.

