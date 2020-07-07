
Steps to blow fuses on NXP LS SoC:
=================================


- Enable POVDD
  -- Refer board manual for the steps to enable POVDD.
+---+-----------------+-----------+------------+-----------------+
|   |   Platform      |  Jumper   |  Switch    | LED to Verify   |
+===+=================+===========+============+=================+
| 1.| lx2160ardb      |  J9       |            |                 |
+---+-----------------+-----------+------------+-----------------+
| 2.| lx2160aqds      |  J9       |            |                 |
+---+-----------------+-----------+------------+-----------------+
| 3.| lx2162aqds      |  J35      | SW9[4] = 1 |    D15          |
+---+-----------------+-----------+------------+-----------------+

- SFP registers to be written to:

+---+----------------------------------+----------------------+----------------------+
|   |   Platform                       |   OTPMKR0..OTPMKR7   |   SRKHR0..SRKHR7     |
+===+==================================+======================+======================+
| 1.| lx2160ardb/lx2160aqds/lx2162aqds | 0x1e80234..0x1e80250 | 0x1e80254..0x1e80270 |
+---+----------------------------------+----------------------+----------------------+

- At U-Boot prompt, verify that SNVS register - HPSR, whether OTPMK was written, already:

+---+----------------------------------+-------------------------------------------+---------------+
|   |   Platform                       |           OTPMK_ZERO_BIT(=value)          | SNVS_HPSR_REG |
+===+==================================+===========================================+===============+
| 1.| lx2160ardb/lx2160aqds/lx2162aqds | 27 (= 1 means not blown, =0 means blown)  | 0x01E90014    |
+---+----------------------------------+-------------------------------------------+---------------+

From u-boot prompt:

  --  Check for the OTPMK.
   .. code:: shell

       => md $SNVS_HPSR_REG
       => 88000900

       In case it is read as 00000000, then read this register using jtag (in development mode only through CW tap).
                   +0       +4       +8       +C
       [0x01E90014] 88000900

       Note: OTPMK_ZERO_BIT is 1, indicating that the OTPMK is not blown.

  --  Check for the SRK Hash.
   .. code:: shell

       => md $SRKHR0 0x10
       01e80254: 00000000 00000000 00000000 00000000    ................
       01e80264: 00000000 00000000 00000000 00000000    ................

       Note: Zero means that SRK hash is not blown.

- If not blown, then from the U-Boot prompt, using following commands:
  --  Provision the OTPMK.

   .. code:: shell

       => mw.l $OTPMKR0  <OTMPKR_0_32Bit_val>
       => mw.l $OTPMKR1  <OTMPKR_1_32Bit_val>
       => mw.l $OTPMKR2  <OTMPKR_2_32Bit_val>
       => mw.l $OTPMKR3  <OTMPKR_3_32Bit_val>
       => mw.l $OTPMKR4  <OTMPKR_4_32Bit_val>
       => mw.l $OTPMKR5  <OTMPKR_5_32Bit_val>
       => mw.l $OTPMKR6  <OTMPKR_6_32Bit_val>
       => mw.l $OTPMKR7  <OTMPKR_7_32Bit_val>

  --  Provision the SRK Hash.

   .. code:: shell

       => mw.l $SRKHR0  <SRKHR_0_32Bit_val>
       => mw.l $SRKHR1  <SRKHR_1_32Bit_val>
       => mw.l $SRKHR2  <SRKHR_2_32Bit_val>
       => mw.l $SRKHR3  <SRKHR_3_32Bit_val>
       => mw.l $SRKHR4  <SRKHR_4_32Bit_val>
       => mw.l $SRKHR5  <SRKHR_5_32Bit_val>
       => mw.l $SRKHR6  <SRKHR_6_32Bit_val>
       => mw.l $SRKHR7  <SRKHR_7_32Bit_val>

       Note: SRK Hash should be carefully written keeping in mind the SFP Block Endianness.

- At U-Boot prompt, verify that SNVS registers for OTPMK are correctly written:

  --  Check for the OTPMK.
   .. code:: shell
       => md $SNVS_HPSR_REG
       => 80000900

       In case it is read as 00000000, then read this register using jtag (in development mode only through CW tap).
                   +0       +4       +8       +C
       [0x01E90014] 80000900


      Note: OTPMK_ZERO_BIT is zero, indicating that the OTPMK is blown.

   .. code:: shell

       => md $OTPMKR0 0x10
       01e80234: ffffffff ffffffff ffffffff ffffffff    ................
       01e80244: ffffffff ffffffff ffffffff ffffffff    ................

       Note: OTPMK will never be visible in plain.

  --  Check for the SRK Hash.
   .. code:: shell

       for example, if following SRK hash is written:
       SFP SRKHR0 = fdc2fed4
       SFP SRKHR1 = 317f569e
       SFP SRKHR2 = 1828425c
       SFP SRKHR3 = e87b5cfd
       SFP SRKHR4 = 34beab8f
       SFP SRKHR5 = df792a70
       SFP SRKHR6 = 2dff85e1
       SFP SRKHR7 = 32a29687, then following could be the value on dumping SRK hash.

       => md $SRKHR0 0x10
       01e80254: d4fec2fd 9e567f31 5c422818 fd5c7be8    ....1.V..(B\.{\.
       01e80264: 8fabbe34 702a79df e185ff2d 8796a232    4....y*p-...2...

       Note: SRK Hash is visible in plain based on the SFP Block Endianness.

- Caution: Donot proceed to the next step, until you are sure that OTPMK and SRKH are correctly blown from above steps.
  -- After the next step, there is no turning back.
  -- Fuses will be burnt, which cannot be undo.

- Write SFP_INGR[INST] with the PROGFB(0x2) instruction to blow the fuses.
  -- User need to save the SRK key pair and OTPMK Key forever, to continue using this board.
+---+----------------------------------+-------------------------------------------+-----------+
|   |   Platform                       | SFP_INGR_REG | SFP_WRITE_DATE_FRM_MIRROR_REG_TO_FUSE  |
+===+==================================+=======================================================+
| 1.| lx2160ardb/lx2160aqds/lx2162aqds | 0x01E80020   |    0x2                                 |
+---+----------------------------------+--------------+----------------------------------------+

   .. code:: shell

       => md $SFP_INGR_REG  $SFP_WRITE_DATE_FRM_MIRROR_REG_TO_FUSE

- On reset, if the SFP register were read from u-boot, it will show the following:
  --  Check for the OTPMK.
   .. code:: shell

       => md $SNVS_HPSR_REG
       => 80000900

       In case it is read as 00000000, then read this register using jtag (in development mode only through CW tap).
                   +0       +4       +8       +C
       [0x01E90014] 80000900

      Note: OTPMK_ZERO_BIT is zero, indicating that the OTPMK is blown.

   .. code:: shell

       => md $OTPMKR0 0x10
       01e80234: ffffffff ffffffff ffffffff ffffffff    ................
       01e80244: ffffffff ffffffff ffffffff ffffffff    ................

       Note: OTPMK will never be visible in plain.

  -- SRK Hash

   .. code:: shell

       => md $SRKHR0 0x10
       01e80254: d4fec2fd 9e567f31 5c422818 fd5c7be8    ....1.V..(B\.{\.
       01e80264: 8fabbe34 702a79df e185ff2d 8796a232    4....y*p-...2...

       Note: SRK Hash is visible in plain based on the SFP Block Endianness.

