NXP SoCs - Overview
=====================
.. section-numbering::
    :suffix: .

The QorIQ family of ARM based SoCs that are supported on TF-A are:

1. LX2160ARDB:
        Platform Name:

        a. lx2160ardb (Board details can be fetched from the link: `lx2160ardb`_)


Table of Supported Boot-Modes by each platform:
-----------------------------------------------
+---+-----------------+-------+--------+-------+-------+-------+-------------+--------------+
|   |   Platform      |  SD   |  QSPI  |  NOR  | NAND  | eMMC  | Flexspi-NOR | Flexspi-NAND |
+===+=================+=======+========+=======+=======+=======+=============+==============+
| 1.| lx2160ardb      |  yes  |        |       |       |       |   yes       |              |
+---+-----------------+-------+--------+-------+-------+-------+-------------+--------------+

Boot Sequence
-------------
::

+                           Secure World        |     Normal World
+ EL0                                           |
+                                               |
+ EL1                           BL32(Tee OS)    |     kernel
+                                ^ |            |       ^
+                                | |            |       |
+ EL2                            | |            |     BL33(u-boot)
+                                | |            |      ^
+                                | v            |     /
+ EL3        BootROM --> BL2 --> BL31 ---------------/
+


How to build
=============

Code Locations
--------------

-  OP-TEE:
   `link <https://source.codeaurora.org/external/qoriq/qoriq-components/optee_os>`__

-  U-Boot:
   `link <https://source.codeaurora.org/external/qoriq/qoriq-components/u-boot>`__

-  RCW:
   `link <https://source.codeaurora.org/external/qoriq/qoriq-components/rcw>`__

Build Procedure
---------------

-  Fetch all the above repositories into local host.

-  Prepare AARCH64 toolchain and set the environment variable "CROSS_COMPILE".

   .. code:: shell

       export CROSS_COMPILE=.../bin/aarch64-linux-gnu-

-  Build RCW. Refer README from the respective cloned folder for more details.

-  Build u-boot and OPTee firstly, and get binary images: u-boot.bin and tee.bin.
   For u-boot you can use the <platform>_tfa_defconfig for build.

-  Below are the steps to build TF-A images for the supported platforms.

BUILD BL2:
~~~~~~~~~~

-To compile
   .. code:: shell

       make PLAT=<platform_name>ardb bl2 BOOT_MODE=<any_one_of_the_supported_boot_mode_by_the_platform> pbl RCW_PATH=<RCW_file_name_with_path>

BUILD FIP:
~~~~~~~~~~

-To compile without OPTEE and without Trusted Board Boot.
   .. code:: shell

	make PLAT=<platform_name> fip BOOT_MODE=<any_one_of_the_supported_boot_mode_by_the_platform> BL33=u-boot-dtb.bin

-To compile with OPTEE and without Trusted Board Boot.
   .. code:: shell

	make all PLAT=<platform_name> fip BOOT_MODE=<anyone_supported_boot_mode_by_the_platform> BL33=u-boot-dtb.bin SPD=opteed BL32=<tee.bin>


Deploy ATF Images
=================

Note: The size in the standard uboot commands for copy to nor, qspi, nand or sd
should be modified based on the binary size of the image to be copied.

-  Deploy ATF images on flexspi-Nor flash Alt Bank from U-Boot prompt.

   .. code:: shell

       => tftp 82000000  $path/bl2_flexspi_nor.pbl;
       => i2c mw 66 50 20;sf probe 0:0; sf erase 0 +$filesize; sf write 0x82000000 0x0 $filesize;

       => tftp 82000000  $path/fip.bin;
       => i2c mw 66 50 20;sf probe 0:0; sf erase 0x100000 +$filesize; sf write 0x82000000 0x100000 $filesize;

    Then change to Alt bank and boot up ATF

       => qixisreset altbank;


.. _lx2160ardb: https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/layerscape-communication-process/layerscape-lx2160a-multicore-communications-processor:LX2160A
