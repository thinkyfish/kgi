/* ----------------------------------------------------------------------------
**	3Dlabs Permedia register definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1997-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: PERMEDIA.h,v $
*/
#ifndef	_chipset_3Dlabs_PERMEDIA_h
#define	_chipset_3Dlabs_PERMEDIA_h

/*	PCI config region
**
**	Please use the pci.h definitions for PCICFG register names.
*/
#ifndef	PCI_VENDOR_ID_3Dlabs
#define	PCI_VENDOR_ID_3Dlabs		0x3d3d
#endif
#ifndef	PCI_DEVICE_ID_PERMEDIA
#define	PCI_DEVICE_ID_PERMEDIA		0x0004
#endif
#ifndef	PCI_DEVICE_ID_PERMEDIA2
#define	PCI_DEVICE_ID_PERMEDIA2		0x0007
#endif
#ifndef	PCI_DEVICE_ID_PERMEDIA2v
#define	PCI_DEVICE_ID_PERMEDIA2v	0x0009
#endif

#ifndef	PCI_VENDOR_ID_TI
#define	PCI_VENDOR_ID_TI	0x104C
#endif
#ifndef	PCI_DEVICE_ID_TVP4010
#define	PCI_DEVICE_ID_TVP4010	0x3D04
#endif
#ifndef	PCI_DEVICE_ID_TVP4020
#define	PCI_DEVICE_ID_TVP4020	0x3D07
#endif
#ifndef	PCI_DEVICE_ID_TVP4020A
#define	PCI_DEVICE_ID_TVP4020A	0x3D11
#endif

/*
**	GC control region	(Base0)
*/
#define	PGC_Base0_LittleEndianBase	0x00000
#define	PGC_Base0_BigEndianBase		0x10000
#define	PGC_Base0_Size			0x20000

/*
**	Control Status
*/
#define	PGC_ControlStatusBase	0x0000
#define	PGC_ControlStatusSize	0x1000

#define	PGC_CS_ResetStatus	0x000
#	define	PGC_CS000_SoftwareReset		0x80000000

#define	PGC_CS_IntEnable	0x008
#	define	PGC_CS008_DisableInterrupts	0x00000000
#	define	PGC_CS008_DMAIntEnable		0x00000001
#	define	PGC_CS008_SyncIntEnable		0x00000002
#	define	PGC_CS008_Reserved		0x00000004
#	define	PGC_CS008_ErrorIntEnable	0x00000008
#	define	PGC_CS008_VRetraceIntEnable	0x00000010
#	define	PGC_CS008_ScanlineIntEnable	0x00000020
#	ifdef	__3Dlabs_Permedia2
#		define	PGC_CS008_TextureInvalidIntEnable	0x00000040
#		define	PGC_CS008_BypassDMAIntEnable		0x00000080
#		define	PGC_CS008_VSBIntEnable			0x00000100
#		define	PGC_CS008_VSAIntEnable			0x00000200
#		define	PGC_CS008_VideoStreamSerialIntEnable	0x00000400
#		define	PGC_CS008_VideoDDCIntEnable		0x00000800
#		define	PGC_CS008_VideoStreamExternalIntEnable	0x00001000
#	endif

#define	PGC_CS_IntFlags		0x010
#	define	PGC_CS010_DMAFlag		0x00000001
#	define	PGC_CS010_SyncFlag		0x00000002
#	define	PGC_CS010_Reserved		0x00000004
#	define	PGC_CS010_ErrorFlag		0x00000008
#	define	PGC_CS010_VRetraceFlag		0x00000010
#	define	PGC_CS010_ScanlineFlag		0x00000020
#	ifdef	__3Dlabs_Permedia2
#		define	PGC_CS010_TextureInvalidFlag		0x00000040
#		define	PGC_CS010_BypassDMAIntFlag		0x00000080
#		define	PGC_CS010_VSBFlag			0x00000100
#		define	PGC_CS010_VSAFlag			0x00000200
#		define	PGC_CS010_VideoStreamSerialFlag		0x00000400
#		define	PGC_CS010_VideoDDCFlag			0x00000800
#		define	PGC_CS010_VideoStreamExternalFlag	0x00001000
#	endif
#	define	PGC_CS010_SVGAFlag			0x80000000

#define	PGC_CS_InFIFOSpace	0x018
#	define	PGC_MAX_InFIFOSpace	32

#define	PGC_CS_OutFIFOSpace	0x020
#	define PGC_MAX_OutFIFOSpace	8

#define	PGC_CS_DMAAddress	0x028

#define	PGC_CS_DMACount		0x030
#	define	PGC_MAX_DMACount	0xFFFF

#define	PGC_CS_ErrorFlags	0x038
#	define	PGC_CS038_InFIFOError		0x00000001
#	define	PGC_CS038_OutFIFOError		0x00000002
#	define	PGC_CS038_MessageError		0x00000004
#	define	PGC_CS038_DMAError		0x00000008
#	ifdef	__3Dlabs_Permedia2
#		define	PGC_CS038_VideoFifoUnderflowError	0x00000010
#		define	PGC_CS038_VSBUnderflowError		0x00000020
#		define	PGC_CS038_VSAUnderflowError		0x00000040
#		define	PGC_CS038_MasterError			0x00000080
#		define	PGC_CS038_OutDMAError			0x00000100
#		define	PGC_CS038_InDMAOverwriteError		0x00000200
#		define	PGC_CS038_OutDMAOverwriteError		0x00000400
#		define	PGC_CS038_VSAInvalidInterlaceError	0x00000800
#		define	PGC_CS038_VSBInvalidInterlaceError	0x00001000
#	endif

#define	PGC_CS_VClkCtl		0x040
#	define	PGC_CS040_VClkCtl0		0x00000001
#	define	PGC_CS040_VClkCtl1		0x00000002
#	define	PGC_CS040_VClkMask		0x00000003
#	define	PGC_CS040_RecoveryTimeMask	0x000003FC
#	define	PGC_CS040_AuxIOLittleEndian	0x00000000	/* P1 only */
#	define	PGC_CS040_AuxIOBigEndian	0x00000400	/* P1 only */
#	ifdef	__3Dlabs_Permedia2
#		ifndef	__3Dlabs_Permedia
#			undef	PGC_CS040_AuxIOBigEndian
#			undef	PGC_CS040_AuxIOLittleEndian
#		endif
#	endif

#define	PGC_CS_TestRegister	0x048
	/* never write!	*/

#define	PGC_CS_ApertureOne	0x050
#	define	PGC_CS050_ByteCtrlStandard	0x00000000
#	define	PGC_CS050_ByteCtrlByteSwapped	0x00000001
#	define	PGC_CS050_ByteCtrlHalfWordSwap	0x00000002
#	define	PGC_CS050_ByteCtrlMask		0x00000003

#	define	PGC_CS050_EnablePacked16Bit	0x00000008Q

#	define	PGC_CS050_ReadBufferA		0x00000000
#	define	PGC_CS050_ReadBufferB		0x00000010
#	define	PGC_CS050_ReadBufferMask	0x00000010

#	define	PGC_CS050_WriteBufferA		0x00000000
#	define	PGC_CS050_WriteBufferB		0x00000020
#	define	PGC_CS050_WriteBufferMask	0x00000020

#	define	PGC_CS050_EnableDoubleWrites	0x00000040

#	define	PGC_CS050_ReadModeRegister	0x00000000
#	define	PGC_CS050_ReadModeMemory	0x00000080
#	define	PGC_CS050_ReadModeMask		0x00000080

#	define	PGC_CS050_ControllerMem		0x00000000
#	define	PGC_CS050_ControllerVGA		0x00000100
#	define	PGC_CS050_ControllerMask	0x00000100

#	define	PGC_CS050_AccessMemOrVGA	0x00000000
#	define	PGC_CS050_AccessROM		0x00000200
#	define	PGC_CS050_AccessMask		0x00000200

#define	PGC_CS_ApertureTwo	0x058
#	define	PGC_CS058_ByteCtrlStandard	0x00000000
#	define	PGC_CS058_ByteCtrlByteSwapped	0x00000001
#	define	PGC_CS058_ByteCtrlHalfWordSwap	0x00000002
#	define	PGC_CS058_ByteCtrlMask		0x00000003

#	define	PGC_CS058_EnablePacked16Bit	0x00000008

#	define	PGC_CS058_ReadBufferA		0x00000000
#	define	PGC_CS058_ReadBufferB		0x00000010
#	define	PGC_CS058_ReadBufferMask	0x00000010

#	define	PGC_CS058_WriteBufferA		0x00000000
#	define	PGC_CS058_WriteBufferB		0x00000020
#	define	PGC_CS058_WriteBufferMask	0x00000020

#	define	PGC_CS058_EnableDoubleWrites	0x00000040
#	define	PGC_CS058_ReadModeMemory	0x00000080
#	define	PGC_CS058_ControllerVGA		0x00000100
#	define	PGC_CS058_AccessROM		0x00000200

#define	PGC_CS_DMAControl	0x060
#	define	PGC_CS060_DMALittleEndian	0x00000000
#	define	PGC_CS060_DMABigEndian		0x00000001
#	define	PGC_CS060_DMAEndianMask		0x00000001
#	ifdef	__3Dlabs_Permedia2
#		define	PGC_CS060_DMAUsingAGP		0x00000002
#		define	PGC_CS060_DMADataThrottle	0x00000004
#		define	PGC_CS060_LongReadDisable	0x00000008
#		define	PGC_CS060_OutDMALittleEndian	0x00000000
#		define	PGC_CS060_OutDMABigEndian	0x00000010
#		define	PGC_CS060_AGPDataThrottle	0x00000020
#		define	PGC_CS060_AGPHighPriority	0x00000040
#		define	PGC_CS060_TexExecStandard	0x00000000
#		define	PGC_CS060_TexExecBit31ReadBuffer 0x0000080
#		define	PGC_CS060_TexExecHalfWordSwap	0x00000100
#		define	PGC_CS060_TexExecMask		0x00000180
#	endif

#define	PGC_CS_FIFODiscon	0x068
#	define	PGC_CS068_InFIFODisconnect	0x00000001
#	define	PGC_CS068_OutFIFODisconnect	0x00000002
#	ifdef	__3Dlabs_Permedia2
#		define	PGC_CS068_GraphicsProcessorActive	0x80000000
#	endif

#define	PGC_CS_ChipConfig	0x070
#	define	PGC_CS070_BaseClassZero		0x00000001
#	define	PGC_CS070_VGAEnable		0x00000002
#	define	PGC_CS070_VGAFixed		0x00000004
#	define	PGC_CS070_AuxEnable		0x00000008	/* P1 only */
#	define	PGC_CS070_AuxIOEnable		0x00000010	/* P1 only */
#	define	PGC_CS070_PCIRetryDisable	0x00000020
#	define	PGC_CS070_DeltaEnable		0x00000040	/* P1 only */
#	define	PGC_CS070_ShortReset		0x00000080
#	ifdef	__3Dlabs_Permedia2
#		define	PGC_CS070_SBACapable	0x00000100
#		define	PGC_CS070_AGPCapable	0x00000200
#		define	PGC_CS070_SClkSelectMask 0x0000C00
#		define	PGC_CS070_SClkPClk	0x00000000
#		define	PGC_CS070_SClkPClk_2	0x00000400
#		define	PGC_CS070_SClkMClk	0x00000800
#		define	PGC_CS070_SClkMClk_2	0x00000C00
#		define	PGC_CS070_SubsystemROM	0x00001000
#		ifndef	__3Dlabs_Permedia
#			undef	PGC_CS070_AuxEnable
#			undef	PGC_CS070_AuxIOEnable
#			undef	PGC_CS070_DeltaEnable
#		endif
#	endif
#	define	PGC_CS070_MClkPeriodMask	0xF0000000
#	define	PGC_CS070_MClkPeriodShift	28

#ifdef	__3Dlabs_Permedia2
#define	PGC_CS_OutDMAAddress		0x080
#define	PGC_CS_OutDMACount		0x088
#define	PGC_CS_AGPTexBaseAddress	0x090
#define	PGC_CS_ByDMAAddress		0x0A0
#define	PGC_CS_ByDMAStride		0x0B8
#define	PGC_CS_ByDMAMemAddr		0x0C0
#define	PGC_CS_ByDMASize		0x0C8
#define	PGC_CS_ByDMAByteMask		0x0D0
#define	PGC_CS_ByDMAControl		0x0D8
#	define	PGC_CS0D8_ModeMask	0x00000003
#	define	PGC_CS0D8_ModeOff	0x00000000
#	define	PGC_CS0D8_ModeDMA	0x00000001
#	define	PGC_CS0D8_ModeAperture1	0x00000002
#	define	PGC_CS0D8_ModeAperture2	0x00000003
#	define	PGC_CS0D8_RestartGP	0x00000004

#	define	PGC_CS0D8_DataMask	0x00000038
#	define	PGC_CS0D8_Data8bpp	0x00000000
#	define	PGC_CS0D8_Data16bpp	0x00000008
#	define	PGC_CS0D8_Data32bpp	0x00000010
#	define	PGC_CS0D8_Data4bpp	0x00000018
#	define	PGC_CS0D8_DataY		0x00000020
#	define	PGC_CS0D8_DataU		0x00000028
#	define	PGC_CS0D8_DataV		0x00000030
#	define	PGC_CS0D8_DataReserved	0x00000038

#	define	PGC_CS0D8_PatchMask	0x000000C0
#	define	PGC_CS0D8_PatchDisable	0x00000000
#	define	PGC_CS0D8_Patch8x8	0x00000040
#	define	PGC_CS0D8_Patch32x32	0x00000080
#	define	PGC_CS0D8_PatchReserved	0x000000C0

#	define	PGC_CS0D8_PP0Shift	8
#	define	PGC_CS0D8_PP0Mask	0x00000700
#	define	PGC_CS0D8_PP1Shift	11
#	define	PGC_CS0D8_PP1Mask	0x00003800
#	define	PGC_CS0D8_PP2Shift	14
#	define	PGC_CS0D8_PP2Mask	0x0001C000
#	define	PGC_CS0D8_OffsetXShift	17
#	define	PGC_CS0D8_OffsetXMask	0x003E0000
#	define	PGC_CS0D8_OffsetYShift	22
#	define	PGC_CS0D8_OffsetYMask	0x07C00000

#	define	PGC_CS0D8_ByteSwapMask	0x18000000
#	define	PGC_CS0D8_ByteSwapOff	0x00000000
#	define	PGC_CS0D8_ByteSwapByte	0x08000000
#	define	PGC_CS0D8_ByteSwapWord	0x10000000

#	define	PGC_CS0D8_UseAGPMaster	0x20000000

#define	PGC_CS_ByDMAComplete	0x0E8
#	define	PGC_CS0E8_TextureValid		0x00000000
#	define	PGC_CS0E8_TextureInvalid	0x00000001

#endif


/*
**	Memory Control
*/
#define	PGC_MemoryControlBase	0x1000
#define	PGC_MemoryControlSize	0x1000

#define	PGC_MC_Reboot		0x000

#define	PGC_MC_RomControl	0x040
#	define	PGC_MC040_ROMPulseWidthMask	0x0000000F
#	define	PGC_MC040_SGRAM			0x00000000
#	define	PGC_MC040_SDRAM			0x00000010
#	define	PGC_MC040_SxRAMMask		0x00000010
#	ifdef	__3Dlabs_Permedia2
#		ifndef	__3Dlabs_Permedia
#			undef	PGC_M040_ROMPulseWidthMask
#		endif
#	endif

#define	PGC_MC_BootAddress	0x080
#	define	PGC_MC080_BootAddressMask	0x000000FF

#define	PGC_MC_MemConfig	0x0C0
#	define	PGC_MAX_tRP			7
#	define	PGC_MC0C0_tRPMask		0x00000007
#	define	PGC_MC0C0_tRPShift		0

#	define	PGC_MAX_tRC			15
#	define	PGC_MC0C0_tRCMask		0x00000078
#	define	PGC_MC0C0_tRCShift		3

#	define	PGC_MAX_tRCDMax			7
#	define	PGC_MC0C0_tRCDMask		0x00000380
#	define	PGC_MC0C0_tRCDShift		7

#	define	PGC_MC0C0_RowCharge		0x00000400

#	define	PGC_MC0C0_BoardConfigMask	0x00001800
#	define	PGC_MC0C0_BoardConfigShift	11

#	define	PGC_MAX_tRMIN			7
#	define	PGC_MC0C0_tRMINMask		0x0000E000
#	define	PGC_MC0C0_tRMINShift		13

#	define	PGC_MC0C0_CASLatency3		0x00010000
#	define	PGC_MC0C0_DeadCycleEnable	0x00020000

#	define	PGC_MAX_BankDelay		7
#	define	PGC_MC0C0_BankDelayMask		0x001C0000
#	define	PGC_MC0C0_BankDelayShift	18

#	define	PGC_MAX_RefreshCountMax		255
#	define	PGC_MC0C0_RefreshCountMask	0x1FE00000
#	define	PGC_MC0C0_RefreshCountShift	21
#	ifdef	__3Dlabs_Permedia2
#		define	PGC2_MC0C0_RefreshCountMax	128
#		define	PGC2_MC0C0_RefreshCountMask	0x1FC00000
#		define	PGC2_MC0C0_RefreshCountShift	22
#		define	PGC2_MC0C0_BlockWrite1		0x00200000
#		ifndef	__3Dlabs_Permedia
#			undef	PGC_MC0C0_BoardConfigMask
#			undef	PGC_MC0C0_BoardConfigShift
#		endif
#	endif

#	define	PGC_MAX_NumberBanksMax		3
#	define	PGC_MC0C0_NumberBanksMask	0x60000000
#	define	PGC_MC0C0_NumberBanksShift	29

#	define	PGC_MC0C0_BurstCycle		0x80000000

#define	PGC_MC_BypassWriteMask		0x100
#define	PGC_MC_FramebufferWriteMask	0x140
#define	PGC_MC_Count			0x180

/*
**	GP FIFO Access
*/
#define	PGC_GPFIFOAccessBase	0x2000
#define	PGC_GPFIFOAccessSize	0x1000

/*
**	Video Control registers
*/
#define	PGC_VideoControlBase	0x3000
#define	PGC_VideoControlSize	0x1000

#define	PGC_VC_ScreenBase	0x000
#	define	PGC_MAX_ScreenBase		0x000FFFFF

#define	PGC_VC_ScreenStride	0x008
#	define	PGC_MAX_ScreenStride		0x000FFFFF

#define	PGC_VC_HTotal		0x010
#	define	PGC_MAX_HTotal			0x000007FF

#define	PGC_VC_HgEnd		0x018
#	define	PGC_MAX_HgEnd			0x000007FF

#define	PGC_VC_HbEnd		0x020
#	define	PGC_MAX_HbEnd			0x000007FF

#define	PGC_VC_HsStart		0x028
#	define	PGC_MAX_HsStart			0x000007FF

#define	PGC_VC_HsEnd		0x030
#	define	PGC_MAX_HsEnd			0x000007FF

#define	PGC_VC_VTotal		0x038
#	define	PGC_MAX_VTotal			0x000007FF

#define	PGC_VC_VbEnd		0x040
#	define	PGC_MAX_VbEnd			0x000007FF

#define	PGC_VC_VsStart		0x048
#	define	PGC_MAX_VsStart			0x000007FF

#define	PGC_VC_VsEnd		0x050
#	define	PGC_MAX_VsEnd			0x000007FF

#define	PGC_VC_VideoControl	0x058
#	define	PGC_VC058_EnableGPVideo		0x00000001
#	ifdef	__3Dlabs_Permedia2
#		define	PGC_VC058_BlankMask		0x00000002
#		define	PGC_VC058_BlankActiveHigh	0x00000000
#		define	PGC_VC058_BlankActiveLow	0x00000002
#	endif
#	define	PGC_VC058_LineDouble		0x00000004

#	define	PGC_VC058_HSyncActive		0x00000008
#	define	PGC_VC058_HSyncLow		0x00000010
#	define	PGC_VC058_HSyncHigh		0x00000000

#	define	PGC_VC058_VSyncActive		0x00000020
#	define	PGC_VC058_VSyncLow		0x00000040
#	define	PGC_VC058_VSyncHigh		0x00000000

#	define	PGC_VC058_BypassPending		0x00000080
#	define	PGC_VC058_GPPending		0x00000100
#	ifdef	__3Dlabs_Permedia2
#		define	PGC_VC058_BufferSwapMask	0x00000600
#		define	PGC_VC058_BufferSyncOnFBlank	0x00000000
#		define	PGC_VC058_BufferFreeRunning	0x00000200
#		define	PGC_VC058_BufferFrameRateLimit	0x00000600
#		define	PGC_VC058_StereoEnable		0x00000800
#		define	PGC_VC058_RightEyeActiveMask	0x00001000
#		define	PGC_VC058_RightEyeActiveHigh	0x00000000
#		define	PGC_VC058_RightEyeActiveLow	0x00001000
#		define	PGC_VC058_RightFrame		0x00002000
#		define	PGC_VC058_BypassPendignRight	0x00004000
#		define	PGC_VC058_GPPendingRight	0x00008000
#		define	PGC_VC058_Data64Enable		0x00010000
#	endif
	
#define	PGC_VC_InterruptLine	0x060
#	define	PGC_MAX_InterruptLine		0x000007FF

#define	PGC_VC_DDCData		0x068
#	define	PGC_VC068_DataInMask		0x00000001
#	ifdef	__3Dlabs_Permedia2
#		define	PGC_VC068_ClockInMask		0x00000002
#		define	PGC_VC068_DataOutTristate	0x00000004
#		define	PGC_VC068_ClockOutTristate	0x00000008
#		define	PGC_VC068_LatchedData0		0x00000000
#		define	PGC_VC068_LatchedData1		0x00000010
#		define	PGC_VC068_DataValid		0x00000020
#		define	PGC_VC068_Start			0x00000040
#		define	PGC_VC068_Stop			0x00000080
#		define	PGC_VC068_Wait			0x00000100
#		define	PGC_VC068_UseMonitorID		0x00000200
#		define	PGC_VC068_MonitorIDInMask	0x00000C00
#		define	PGC_VC068_MonitorIDOutMask	0x00003000
#	endif

#define	PGC_VC_LineCount	0x070
#	define	PGC_VC070_LineCountMask		0x000007FF

#ifdef	__3Dlabs_Permedia2
#define	PGC_VC_FifoControl	0x078
#	define	PGC_VC078_LowTresholdShift	0
#	define	PGC_VC078_LowTresholdMask	0x0000001F
#	define	PGC_VC078_HighTresholdShift	8
#	define	PGC_VC078_HighTresholdMask	0x00001F00
#	define	PGC_VC078_Underflow		0x00100000

#define	PGC_VC_ScreenBaseRight	0x080
#endif

/*
**	RAMDAC
*/
#define	PGC_DacBase		0x4000
#define	PGC_DacSize		0x1000
#define	PGC_MAX_DacRegisters	16

/*
**	Auxiliary (Permedia1) / VideoStreams (Permedia2)
*/
#define	PGC_AuxiliaryBase	0x5000
#define	PGC_AuxiliarySize	0x1000

#ifdef	__3Dlabs_Permedia2

#endif

/*
**	MMIO VGA 
*/
#define	PGC_VGABase		0x6000
#define	PGC_VGASize		0x1000

#include "chipset/IBM/VGA.h"

#define	PGC_SEQ_VGAControlReg	0x05
#	define	PGC_SR05_EnableHostMemoryAccess	0x01
#	define	PGC_SR05_EnableHostDACAccess	0x02
#	define	PGC_SR05_IRQEnableVGA		0x04
#	define	PGC_SR05_VGAEnableDisplay	0x08

#	define	PGC_SR05_DACAddr2		0x10
#	define	PGC_SR05_DACAddr3		0x20
#	define	PGC_SR05_DACAddrMask		0x30
#	define	PGC_SR05_DACAddrShift		4

#	define	PGC_SR05_EnableVTG		0x40
#	define	PGC_SR05_Reserved		0x80

#define	PGC_GRC_Mode640Reg	0x09
#	define	PGC_GRC09_Mode640Enable		0x80

#	define	PGC_GRC09_BankAMask		0x07
#	define	PGC_GRC09_BankAMax		7
#	define	PGC_GRC09_BankAShift		0

#	define	PGC_GRC09_BankBMask		0x38
#	define	PGC_GRC09_BankBMax		7
#	define	PGC_GRC09_BankBShift		3

#	define	PGC_GRC09_Reserved		0x40

/*
**	Reserved
*/
#define	PGC_ReservedBase	0x7000
#define	PGC_ReservedSize	0x1000

/*
** 	GP Register Access
*/
#define PGC_GPRegisterBase	0x8000
#define PGC_GPRegisterSize	0x8000

#define	PGC_GP_StartXDom		0x0000
#define	PGC_GP_dXDom			0x0001
#define	PGC_GP_StartXSub		0x0002
#define	PGC_GP_dXSub			0x0003
#define	PGC_GP_StartY			0x0004
#define	PGC_GP_dY			0x0005
#define	PGC_GP_Count			0x0006
#define	PGC_GP_Render			0x0007
#define	PGC_GP_ContinueNewLine		0x0008
#define	PGC_GP_ContinueNewDom		0x0009
#define	PGC_GP_ContinueNewSub		0x000A
#define	PGC_GP_Continue			0x000B
#define	PGC_GP_BitMaskPattern		0x000D

#define	PGC_GP_RasterizerMode		0x0014
#define	PGC_GP_YLimits			0x0015
#define	PGC_GP_WaitForCompletion	0x0017
#define	PGC_GP_XLimits			0x0019
#ifdef __3Dlabs_Permedia2
#	define	PGC_GP_RectangleOrigin	0x001A
#	define	PGC_GP_RectangleSize	0x001B
#endif
#define	PGC_GP_PackedDataLimits		0x002A

#define	PGC_GP_ScissorMode		0x0030
#define	PGC_GP_ScissorMinXY		0x0031
#define	PGC_GP_ScissorMaxXY		0x0032
#define	PGC_GP_ScreenSize		0x0033
#define	PGC_GP_AreaStippleMode		0x0034
#define	PGC_GP_WindowOrigin		0x0039

#define	PGC_GP_AreaStipplePattern0	0x0040
#define	PGC_GP_AreaStipplePattern1	0x0041
#define	PGC_GP_AreaStipplePattern2	0x0042
#define	PGC_GP_AreaStipplePattern3	0x0043
#define	PGC_GP_AreaStipplePattern4	0x0044
#define	PGC_GP_AreaStipplePattern5	0x0045
#define	PGC_GP_AreaStipplePattern6	0x0046
#define	PGC_GP_AreaStipplePattern7	0x0047

#define	PGC_GP_TextureAddressMode	0x0070
#define	PGC_GP_SStart			0x0071
#define	PGC_GP_dSdx			0x0072
#define	PGC_GP_dSdyDom			0x0073
#define	PGC_GP_TStart			0x0074
#define	PGC_GP_dTdx			0x0075
#define	PGC_GP_dTdyDom			0x0076
#define	PGC_GP_QStart			0x0077
#define	PGC_GP_dQdx			0x0078
#define	PGC_GP_dQdyDom			0x0079

#ifdef	__3Dlabs_Permedia2
#	define	PGC_GP_TexelLUTIndex	0x0098
#	define	PGC_GP_TexelLUTData	0x0099
#	define	PGC_GP_TexelLUTAddress	0x009A
#	define	PGC_GP_TexelLUTTransfer	0x009B
#endif

#define	PGC_GP_TextureBaseAddress	0x00B0
#define	PGC_GP_TextureMapFormat		0x00B1
#define	PGC_GP_TextureDataFormat	0x00B2

#define	PGC_GP_Texel0			0x00C0
#define	PGC_GP_TextureReadMode		0x00CE
#define	PGC_GP_TexelLUTMode		0x00CF

#define	PGC_GP_TextureColorMode		0x00D0
#define	PGC_GP_FogMode			0x00D2
#define	PGC_GP_FogColor			0x00D3
#define	PGC_GP_FStart			0x00D4
#define	PGC_GP_dFdx			0x00D5
#define	PGC_GP_dFdyDom			0x00D6
#define	PGC_GP_KsStart			0x00D9
#define	PGC_GP_dKsdx			0x00DA
#define	PGC_GP_dKsdyDom			0x00DB
#define	PGC_GP_KdStart			0x00DC
#define	PGC_GP_Kddx			0x00DD
#define	PGC_GP_dKddyDom			0x00DE

#define	PGC_GP_RStart			0x00F0
#define	PGC_GP_dRdx			0x00F1
#define	PGC_GP_dRdyDom			0x00F2
#define	PGC_GP_GStart			0x00F3
#define	PGC_GP_dGdx			0x00F4
#define	PGC_GP_dGdyDom			0x00F5
#define	PGC_GP_BStart			0x00F6
#define	PGC_GP_dBdx			0x00F7
#define	PGC_GP_dBdyDom			0x00F8
#define	PGC_GP_AStart			0x00F9
#define	PGC_GP_ColorDDAMode		0x00FC
#define	PGC_GP_ConstantColor		0x00FD
#define	PGC_GP_Color			0x00FE

#define	PGC_GP_AlphaBlendMode		0x0102
#define	PGC_GP_DitherMode		0x0103
#define	PGC_GP_FBSoftwareWriteMask	0x0104
#define	PGC_GP_LogicalOpMode		0x0105
#define	PGC_GP_FBWriteData		0x0106
#define	PGC_GP_LBReadMode		0x0110
#define	PGC_GP_LBReadFormat		0x0111
#define	PGC_GP_LBSourceOffset		0x0112
#define	PGC_GP_LBData			0x0113
#define	PGC_GP_LBStencil		0x0115
#define	PGC_GP_LBDepth			0x0116
#define	PGC_GP_LBWindowBase		0x0117
#define	PGC_GP_LBWriteMode		0x0118
#define	PGC_GP_LBWriteFormat		0x0119
#define	PGC_GP_TextureData		0x011D
#define	PGC_GP_TextureDownloadOffset	0x011E

#define	PGC_GP_Window			0x0130
#define	PGC_GP_StencilMode		0x0131
#define	PGC_GP_StencilData		0x0132
#define	PGC_GP_Stencil			0x0133
#define	PGC_GP_DepthMode		0x0134
#define	PGC_GP_Depth			0x0135
#define	PGC_GP_ZStartU			0x0136
#define	PGC_GP_ZStartL			0x0137
#define	PGC_GP_dZdxU			0x0138
#define	PGC_GP_dZdxL			0x0139
#define	PGC_GP_dZdyDomU			0x013A
#define	PGC_GP_dZdyDomL			0x013B

#define	PGC_GP_FBReadMode		0x0150
#define	PGC_GP_FBSourceOffset		0x0151
#define	PGC_GP_FBPixelOffset		0x0152
#define	PGC_GP_FBColor			0x0153
#define	PGC_GP_FBData			0x0154
#define	PGC_GP_FBSourceData		0x0155
#define	PGC_GP_FBWindowBase		0x0156
#define	PGC_GP_FBWriteMode		0x0157
#define	PGC_GP_FBHardwareWriteMask	0x0158
#define	PGC_GP_FBBlockColor		0x0159
#define	PGC_GP_FBReadPixel		0x015A

#define	PGC_GP_FilterMode		0x0180
#define	PGC_GP_StatisticMode		0x0181
#define	PGC_GP_MinRegion		0x0182
#define	PGC_GP_MaxRegion		0x0183
#define	PGC_GP_ResetPickResult		0x0184
#define	PGC_GP_MinHitRegion		0x0185
#define	PGC_GP_MaxHitRegion		0x0186
#define	PGC_GP_PickResult		0x0187
#define	PGC_GP_Sync			0x0188
#ifdef	__3Dlabs_Permedia2
#	define	PGC_GP_FBBlockColorU	0x018D
#	define	PGC_GP_FBBlockColorL	0x018E
#endif
#define	PGC_GP_SuspendUntilFrameBlank	0x018F

#ifdef	__3Dlabs_Permedia2
#	define	PGC_GP_FBSourceBase	0x01B0
#	define	PGC_GP_FBSourceDelta	0x01B1
#	define	PGC_GP_Config		0x01B2
#endif

#define	PGC_GP_TexelLUT0		0x01D0
#define	PGC_GP_TexelLUT1		0x01D1
#define	PGC_GP_TexelLUT2		0x01D2
#define	PGC_GP_TexelLUT3		0x01D3
#define	PGC_GP_TexelLUT4		0x01D4
#define	PGC_GP_TexelLUT5		0x01D5
#define	PGC_GP_TexelLUT6		0x01D6
#define	PGC_GP_TexelLUT7		0x01D7
#define	PGC_GP_TexelLUT8		0x01D8
#define	PGC_GP_TexelLUT9		0x01D9
#define	PGC_GP_TexelLUTA		0x01DA
#define	PGC_GP_TexelLUTB		0x01DB
#define	PGC_GP_TexelLUTC		0x01DC
#define	PGC_GP_TexelLUTD		0x01DD
#define	PGC_GP_TexelLUTE		0x01DE
#define	PGC_GP_TexelLUTF		0x01DF

#define	PGC_GP_YUVMode			0x01E0
#define	PGC_GP_ChromaUpperBound		0x01E1
#define	PGC_GP_ChromaLowerBound		0x01E2
#ifdef	__3Dlabs_Permedia2
#	define	PGC_GP_AlphaMapUpperBound	0x01E3
#	define	PGC_GP_AlphaMapLowerBound	0x01E4
#	define	PGC_GP_TextureID		0x01EE
#	define	PGC_GP_TexelLUTID		0x01EF
#endif

#ifdef	__3Dlabs_Permedia2

#	define	PGC_GP_V0Fixed_s	0x0200
#	define	PGC_GP_V0Fixed_t	0x0201
#	define	PGC_GP_V0Fixed_q	0x0202
#	define	PGC_GP_V0Fixed_Ks	0x0203
#	define	PGC_GP_V0Fixed_Kd	0x0204
#	define	PGC_GP_V0Fixed_red	0x0205
#	define	PGC_GP_V0Fixed_green	0x0206
#	define	PGC_GP_V0Fixed_blue	0x0207
#	define	PGC_GP_V0Fixed_alpha	0x0208
#	define	PGC_GP_V0Fixed_fog	0x0209
#	define	PGC_GP_V0Fixed_x	0x020A
#	define	PGC_GP_V0Fixed_y	0x020B
#	define	PGC_GP_V0Fixed_z	0x020C
#	define	PGC_GP_V0Fixed_color	0x020E

#	define	PGC_GP_V1Fixed_s	0x0210
#	define	PGC_GP_V1Fixed_t	0x0211
#	define	PGC_GP_V1Fixed_q	0x0212
#	define	PGC_GP_V1Fixed_Ks	0x0213
#	define	PGC_GP_V1Fixed_Kd	0x0214
#	define	PGC_GP_V1Fixed_red	0x0215
#	define	PGC_GP_V1Fixed_green	0x0216
#	define	PGC_GP_V1Fixed_blue	0x0217
#	define	PGC_GP_V1Fixed_alpha	0x0218
#	define	PGC_GP_V1Fixed_fog	0x0219
#	define	PGC_GP_V1Fixed_x	0x021A
#	define	PGC_GP_V1Fixed_y	0x021B
#	define	PGC_GP_V1Fixed_z	0x021C
#	define	PGC_GP_V1Fixed_color	0x021E

#	define	PGC_GP_V2Fixed_s	0x0220
#	define	PGC_GP_V2Fixed_t	0x0221
#	define	PGC_GP_V2Fixed_q	0x0222
#	define	PGC_GP_V2Fixed_Ks	0x0223
#	define	PGC_GP_V2Fixed_Kd	0x0224
#	define	PGC_GP_V2Fixed_red	0x0225
#	define	PGC_GP_V2Fixed_green	0x0226
#	define	PGC_GP_V2Fixed_blue	0x0227
#	define	PGC_GP_V2Fixed_alpha	0x0228
#	define	PGC_GP_V2Fixed_fog	0x0229
#	define	PGC_GP_V2Fixed_x	0x022A
#	define	PGC_GP_V2Fixed_y	0x022B
#	define	PGC_GP_V2Fixed_z	0x022C
#	define	PGC_GP_V2Fixed_color	0x022E

#	define	PGC_GP_V0Float_s	0x0230
#	define	PGC_GP_V0Float_t	0x0231
#	define	PGC_GP_V0Float_q	0x0232
#	define	PGC_GP_V0Float_Ks	0x0233
#	define	PGC_GP_V0Float_Kd	0x0234
#	define	PGC_GP_V0Float_red	0x0235
#	define	PGC_GP_V0Float_green	0x0236
#	define	PGC_GP_V0Float_blue	0x0237
#	define	PGC_GP_V0Float_alpha	0x0223
#	define	PGC_GP_V0Float_fog	0x0239
#	define	PGC_GP_V0Float_x	0x023A
#	define	PGC_GP_V0Float_y	0x023B
#	define	PGC_GP_V0Float_z	0x023C
#	define	PGC_GP_V0Float_color	0x023E

#	define	PGC_GP_V1Float_s	0x0240
#	define	PGC_GP_V1Float_t	0x0241
#	define	PGC_GP_V1Float_q	0x0242
#	define	PGC_GP_V1Float_Ks	0x0243
#	define	PGC_GP_V1Float_Kd	0x0244
#	define	PGC_GP_V1Float_red	0x0245
#	define	PGC_GP_V1Float_green	0x0246
#	define	PGC_GP_V1Float_blue	0x0247
#	define	PGC_GP_V1Float_alpha	0x0243
#	define	PGC_GP_V1Float_fog	0x0249
#	define	PGC_GP_V1Float_x	0x024A
#	define	PGC_GP_V1Float_y	0x024B
#	define	PGC_GP_V1Float_z	0x024C
#	define	PGC_GP_V1Float_color	0x023E

#	define	PGC_GP_V2Float_s	0x0250
#	define	PGC_GP_V2Float_t	0x0251
#	define	PGC_GP_V2Float_q	0x0252
#	define	PGC_GP_V2Float_Ks	0x0253
#	define	PGC_GP_V2Float_Kd	0x0254
#	define	PGC_GP_V2Float_red	0x0255
#	define	PGC_GP_V2Float_green	0x0256
#	define	PGC_GP_V2Float_blue	0x0257
#	define	PGC_GP_V2Float_alpha	0x0223
#	define	PGC_GP_V2Float_fog	0x0259
#	define	PGC_GP_V2Float_x	0x025A
#	define	PGC_GP_V2Float_y	0x025B
#	define	PGC_GP_V2Float_z	0x025C
#	define	PGC_GP_V2Float_color	0x023E

#	define	PGC_GP_DeltaMode	0x0260
#	define	PGC_GP_DrawTriangle	0x0261
#	define	PGC_GP_RepeatTriangle	0x0262
#	define	PGC_GP_DrawLine01	0x0263
#	define	PGC_GP_DrawLine10	0x0264
#	define	PGC_GP_RepeatLine	0x0265

#endif

/*
**	Bypass access to memory	(Base1 and Base2)
*/
#define	PGC_Base1_Size			0x00800000
#define	PGC_Base2_Size			0x00800000

/*
**	Auxiliary bus		(Base3)			Only on Permedia1
*/
#define	PGC_Base3_Size			0x00000010

/*
**	Delta Aperture		(Base4)			Only on Permedia1
*/
#define	PGC_Base4_Size			0x00040000

/*
**	Expansion ROM		(ROM0)
*/
#define	PGC_ROM_Size			0x10000

#endif	/* #ifdef _chipset_3Dlabs_PERMEDIA_h	*/
