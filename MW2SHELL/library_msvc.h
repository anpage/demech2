#ifdef 0
// C runtime library code in MW2SHELL.DLL: VC++ 4.1's LIBCMT.LIB
// (the multithreaded CRT, linked with the default /OPT:REF: only referenced functions), identified object by object
// against the library.
//
// Public functions are matched by their public symbol (SYMBOL). The debug and release CRT
// share those, while the names reccmp derives from CodeView differ between them.
// Static functions have no public symbol, and LIBCMT carries no CodeView records, so
// reccmp can't match them; they are annotated to mark their addresses as library code.
// Aliases at one address (__chkstk/__alloca_probe, __strcmpi/__stricmp) take the symbol
// reccmp keeps for that address.

// sprintf.obj

// LIBRARY: MW2SHELL 0x100492e0 SYMBOL
// _sprintf

// ftol.obj

// LIBRARY: MW2SHELL 0x10049348 SYMBOL
// __ftol

// fpinit.obj

// LIBRARY: MW2SHELL 0x10049370 SYMBOL
// __fpmath

// LIBRARY: MW2SHELL 0x10049390 SYMBOL
// __fpclear

// LIBRARY: MW2SHELL 0x100493a0 SYMBOL
// __cfltcvt_init

// qsort.obj

// LIBRARY: MW2SHELL 0x100493e0 SYMBOL
// _qsort

// LIBRARY: MW2SHELL 0x10049540 SYMBOL
// _shortsort

// LIBRARY: MW2SHELL 0x100495a0 SYMBOL
// _swap

// fclose.obj

// LIBRARY: MW2SHELL 0x100495d0 SYMBOL
// _fclose

// LIBRARY: MW2SHELL 0x10049610 SYMBOL
// __fclose_lk

// fread.obj

// LIBRARY: MW2SHELL 0x10049670 SYMBOL
// _fread

// LIBRARY: MW2SHELL 0x100496b0 SYMBOL
// __fread_lk

// fopen.obj

// LIBRARY: MW2SHELL 0x10049800 SYMBOL
// __fsopen

// LIBRARY: MW2SHELL 0x10049840 SYMBOL
// _fopen

// exsup.obj

// LIBRARY: MW2SHELL 0x10049858 SYMBOL
// __global_unwind2

// LIBRARY: MW2SHELL 0x10049878 SYMBOL
// __unwind_handler

// LIBRARY: MW2SHELL 0x1004989a SYMBOL
// __local_unwind2

// LIBRARY: MW2SHELL 0x100498f2 SYMBOL
// __NLG_Return2

// LIBRARY: MW2SHELL 0x10049902 SYMBOL
// __abnormal_termination

// LIBRARY: MW2SHELL 0x10049925 SYMBOL
// __NLG_Notify1

// LIBRARY: MW2SHELL 0x1004992e SYMBOL
// __NLG_Notify

// LIBRARY: MW2SHELL 0x10049941 SYMBOL
// __NLG_Dispatch

// trnsctrl.obj

// LIBRARY: MW2SHELL 0x10049950 SYMBOL
// ?_JumpToContinuation@@YGXPAXPAUEHRegistrationNode@@@Z

// LIBRARY: MW2SHELL 0x10049990 SYMBOL
// ?_CallMemberFunction0@@YGXPAX0@Z

// LIBRARY: MW2SHELL 0x100499a0 SYMBOL
// ?_CallMemberFunction1@@YGXPAX00@Z

// LIBRARY: MW2SHELL 0x100499b0 SYMBOL
// ?_CallMemberFunction2@@YGXPAX00H@Z

// LIBRARY: MW2SHELL 0x100499c0 SYMBOL
// ?_UnwindNestedFrames@@YGXPAUEHRegistrationNode@@PAUEHExceptionRecord@@@Z

// LIBRARY: MW2SHELL 0x10049a20 SYMBOL
// ___CxxFrameHandler

// LIBRARY: MW2SHELL 0x10049a60 SYMBOL
// ?_CallCatchBlock2@@YAPAXPAUEHRegistrationNode@@PBU_s_FuncInfo@@PAXHK@Z

// LIBRARY: MW2SHELL 0x10049ad0 SYMBOL
// ?CatchGuardHandler@@YA?AW4_EXCEPTION_DISPOSITION@@PAUEHExceptionRecord@@PAUCatchGuardRN@@PAX2@Z

// LIBRARY: MW2SHELL 0x10049b00 SYMBOL
// ?_CallSETranslator@@YAHPAUEHExceptionRecord@@PAUEHRegistrationNode@@PAX2PBU_s_FuncInfo@@H1@Z

// LIBRARY: MW2SHELL 0x10049be0 SYMBOL
// ?TranslatorGuardHandler@@YA?AW4_EXCEPTION_DISPOSITION@@PAUEHExceptionRecord@@PAUTranslatorGuardRN@@PAX2@Z

// delete.obj

// LIBRARY: MW2SHELL 0x10049c50 SYMBOL
// ??3@YAXPAX@Z

// new.obj

// LIBRARY: MW2SHELL 0x10049c60 SYMBOL
// ??2@YAPAXI@Z

// strncpy.obj

// LIBRARY: MW2SHELL 0x10049c70 SYMBOL
// _strncpy

// tolower.obj

// LIBRARY: MW2SHELL 0x10049d70 SYMBOL
// _tolower

// LIBRARY: MW2SHELL 0x10049de0 SYMBOL
// __tolower_lk

// free.obj

// LIBRARY: MW2SHELL 0x10049ed0 SYMBOL
// _free

// bsearch.obj

// LIBRARY: MW2SHELL 0x10049ef0 SYMBOL
// _bsearch

// fflush.obj

// LIBRARY: MW2SHELL 0x10049fa0 SYMBOL
// _fflush

// LIBRARY: MW2SHELL 0x10049fe0 SYMBOL
// __fflush_lk

// LIBRARY: MW2SHELL 0x1004a020 SYMBOL
// __flush

// LIBRARY: MW2SHELL 0x1004a090 SYMBOL
// __flushall

// LIBRARY: MW2SHELL 0x1004a0a0 SYMBOL
// _flsall

// _file.obj

// LIBRARY: MW2SHELL 0x1004a170 SYMBOL
// ___initstdio

// LIBRARY: MW2SHELL 0x1004a240 SYMBOL
// ___endstdio

// fprintf.obj

// LIBRARY: MW2SHELL 0x1004a260 SYMBOL
// _fprintf

// crt0dat.obj

// LIBRARY: MW2SHELL 0x1004a2b0 SYMBOL
// __cinit

// LIBRARY: MW2SHELL 0x1004a2e0 SYMBOL
// _exit

// LIBRARY: MW2SHELL 0x1004a300 SYMBOL
// __exit

// LIBRARY: MW2SHELL 0x1004a320 SYMBOL
// __cexit

// LIBRARY: MW2SHELL 0x1004a330 SYMBOL
// _doexit

// LIBRARY: MW2SHELL 0x1004a3f0 SYMBOL
// __lockexit

// LIBRARY: MW2SHELL 0x1004a400 SYMBOL
// __unlockexit

// LIBRARY: MW2SHELL 0x1004a410 SYMBOL
// __initterm

// fwrite.obj

// LIBRARY: MW2SHELL 0x1004a430 SYMBOL
// _fwrite

// LIBRARY: MW2SHELL 0x1004a470 SYMBOL
// __fwrite_lk

// strchr.obj

// LIBRARY: MW2SHELL 0x1004a600 SYMBOL
// _strchr

// LIBRARY: MW2SHELL 0x1004a606 SYMBOL
// ___from_strstr_to_strchr

// rmdir.obj

// LIBRARY: MW2SHELL 0x1004a6c0 SYMBOL
// __rmdir

// strstr.obj

// LIBRARY: MW2SHELL 0x1004a6f0 SYMBOL
// _strstr

// strtok.obj

// LIBRARY: MW2SHELL 0x1004a770 SYMBOL
// _strtok

// toupper.obj

// LIBRARY: MW2SHELL 0x1004a850 SYMBOL
// _toupper

// LIBRARY: MW2SHELL 0x1004a8c0 SYMBOL
// __toupper_lk

// calloc.obj

// LIBRARY: MW2SHELL 0x1004a9b0 SYMBOL
// _calloc

// chkstk.obj

// LIBRARY: MW2SHELL 0x1004aa10 SYMBOL
// __chkstk

// msize.obj

// LIBRARY: MW2SHELL 0x1004aa40 SYMBOL
// __msize

// malloc.obj

// LIBRARY: MW2SHELL 0x1004aa60 SYMBOL
// _malloc

// LIBRARY: MW2SHELL 0x1004aa80 SYMBOL
// __nh_malloc

// LIBRARY: MW2SHELL 0x1004aac0 SYMBOL
// __heap_alloc

// rand.obj

// LIBRARY: MW2SHELL 0x1004aae0 SYMBOL
// _srand

// LIBRARY: MW2SHELL 0x1004aaf0 SYMBOL
// _rand

// clock.obj

// LIBRARY: MW2SHELL 0x1004ab20 SYMBOL
// _clock

// LIBRARY: MW2SHELL 0x1004ab60 SYMBOL
// ___inittime

// vsnprint.obj

// LIBRARY: MW2SHELL 0x1004ab70 SYMBOL
// __vsnprintf

// memmove.obj

// LIBRARY: MW2SHELL 0x1004abe0 SYMBOL
// _memmove

// fputs.obj

// LIBRARY: MW2SHELL 0x1004ad30 SYMBOL
// _fputs

// strncmp.obj

// LIBRARY: MW2SHELL 0x1004ada0 SYMBOL
// _strncmp

// ungetc.obj

// LIBRARY: MW2SHELL 0x1004ade0 SYMBOL
// _ungetc

// LIBRARY: MW2SHELL 0x1004ae10 SYMBOL
// __ungetc_lk

// fgetc.obj

// LIBRARY: MW2SHELL 0x1004aea0 SYMBOL
// _fgetc

// realloc.obj

// LIBRARY: MW2SHELL 0x1004aee0 SYMBOL
// _realloc

// strcspn.obj

// LIBRARY: MW2SHELL 0x1004af50 SYMBOL
// _strcspn

// rename.obj

// LIBRARY: MW2SHELL 0x1004af90 SYMBOL
// _rename

// atox.obj

// LIBRARY: MW2SHELL 0x1004afd0 SYMBOL
// _atol

// LIBRARY: MW2SHELL 0x1004b080 SYMBOL
// _atoi

// 87ctran.obj

// LIBRARY: MW2SHELL 0x1004b090 SYMBOL
// _pow

// LIBRARY: MW2SHELL 0x1004b09a SYMBOL
// _log

// LIBRARY: MW2SHELL 0x1004b0a4 SYMBOL
// _log10

// LIBRARY: MW2SHELL 0x1004b0ab SYMBOL
// _exp

// LIBRARY: MW2SHELL 0x1004b0b2 SYMBOL
// __CIpow

// LIBRARY: MW2SHELL 0x1004b0bc SYMBOL
// __CIlog

// LIBRARY: MW2SHELL 0x1004b0c6 SYMBOL
// __CIlog10

// LIBRARY: MW2SHELL 0x1004b0cd SYMBOL
// __CIexp

// ftell.obj

// LIBRARY: MW2SHELL 0x1004b0e0 SYMBOL
// _ftell

// LIBRARY: MW2SHELL 0x1004b110 SYMBOL
// __ftell_lk

// fseek.obj

// LIBRARY: MW2SHELL 0x1004b2c0 SYMBOL
// _fseek

// LIBRARY: MW2SHELL 0x1004b300 SYMBOL
// __fseek_lk

// setvbuf.obj

// LIBRARY: MW2SHELL 0x1004b3a0 SYMBOL
// _setvbuf

// dllcrt0.obj

// LIBRARY: MW2SHELL 0x1004b470 SYMBOL
// __CRT_INIT@12

// LIBRARY: MW2SHELL 0x1004b5c0 SYMBOL
// __DllMainCRTStartup@12

// LIBRARY: MW2SHELL 0x1004b690 SYMBOL
// __amsg_exit

// _flsbuf.obj

// LIBRARY: MW2SHELL 0x1004b6d0 SYMBOL
// __flsbuf

// output.obj

// LIBRARY: MW2SHELL 0x1004b820 SYMBOL
// __output

// LIBRARY: MW2SHELL 0x1004c200 SYMBOL
// _write_char

// LIBRARY: MW2SHELL 0x1004c250 SYMBOL
// _write_multi_char

// LIBRARY: MW2SHELL 0x1004c290 SYMBOL
// _write_string

// cprintf.obj

// LIBRARY: MW2SHELL 0x1004c2d0 SYMBOL
// _get_int_arg

// LIBRARY: MW2SHELL 0x1004c2e0 SYMBOL
// _get_int64_arg

// LIBRARY: MW2SHELL 0x1004c300 SYMBOL
// _get_short_arg

// fp8.obj

// LIBRARY: MW2SHELL 0x1004c310 SYMBOL
// __setdefaultprecision

// testfdiv.obj

// LIBRARY: MW2SHELL 0x1004c330 SYMBOL
// __ms_p5_test_fdiv

// LIBRARY: MW2SHELL 0x1004c380 SYMBOL
// __ms_p5_mp_test_fdiv

// cvt.obj

// LIBRARY: MW2SHELL 0x1004c4d0 SYMBOL
// __forcdecpt

// LIBRARY: MW2SHELL 0x1004c540 SYMBOL
// __cropzeros

// LIBRARY: MW2SHELL 0x1004c5a0 SYMBOL
// __positive

// LIBRARY: MW2SHELL 0x1004c5c0 SYMBOL
// __fassign

// LIBRARY: MW2SHELL 0x1004c610 SYMBOL
// __cftoe

// LIBRARY: MW2SHELL 0x1004c690 SYMBOL
// __cftoe2

// LIBRARY: MW2SHELL 0x1004c780 SYMBOL
// __cftof

// LIBRARY: MW2SHELL 0x1004c7f0 SYMBOL
// __cftof2

// LIBRARY: MW2SHELL 0x1004c8b0 SYMBOL
// __cftog

// LIBRARY: MW2SHELL 0x1004c960 SYMBOL
// __cfltcvt

// LIBRARY: MW2SHELL 0x1004c9d0 SYMBOL
// __shift

// mlock.obj

// LIBRARY: MW2SHELL 0x1004ca00 SYMBOL
// __mtinitlocks

// LIBRARY: MW2SHELL 0x1004ca30 SYMBOL
// __mtdeletelocks

// LIBRARY: MW2SHELL 0x1004caa0 SYMBOL
// __lock

// LIBRARY: MW2SHELL 0x1004cb10 SYMBOL
// __unlock

// LIBRARY: MW2SHELL 0x1004cb30 SYMBOL
// __lock_file

// LIBRARY: MW2SHELL 0x1004cb70 SYMBOL
// __lock_file2

// LIBRARY: MW2SHELL 0x1004cba0 SYMBOL
// __unlock_file

// LIBRARY: MW2SHELL 0x1004cbe0 SYMBOL
// __unlock_file2

// close.obj

// LIBRARY: MW2SHELL 0x1004cc10 SYMBOL
// __close

// LIBRARY: MW2SHELL 0x1004cc80 SYMBOL
// __close_lk

// _freebuf.obj

// LIBRARY: MW2SHELL 0x1004cd10 SYMBOL
// __freebuf

// _filbuf.obj

// LIBRARY: MW2SHELL 0x1004cd50 SYMBOL
// __filbuf

// read.obj

// LIBRARY: MW2SHELL 0x1004ce50 SYMBOL
// __read

// LIBRARY: MW2SHELL 0x1004ced0 SYMBOL
// __read_lk

// _open.obj

// LIBRARY: MW2SHELL 0x1004d100 SYMBOL
// __openfile

// stream.obj

// LIBRARY: MW2SHELL 0x1004d310 SYMBOL
// __getstream

// frame.obj

// LIBRARY: MW2SHELL 0x1004d400 SYMBOL
// ___InternalCxxFrameHandler

// LIBRARY: MW2SHELL 0x1004d4c0 SYMBOL
// ?FindHandler@@YAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PAU_CONTEXT@@PAXPBU_s_FuncInfo@@EH1@Z

// LIBRARY: MW2SHELL 0x1004d760 SYMBOL
// ?FindHandlerForForeignException@@YAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PAU_CONTEXT@@PAXPBU_s_FuncInfo@@HH1@Z

// LIBRARY: MW2SHELL 0x1004d840 SYMBOL
// ?GetRangeOfTrysToCheck@@YAPBU_s_TryBlockMapEntry@@PBU_s_FuncInfo@@HHPAI1@Z

// LIBRARY: MW2SHELL 0x1004d8e0 SYMBOL
// ___FrameUnwindToState

// LIBRARY: MW2SHELL 0x1004d9b0 SYMBOL
// ?FrameUnwindFilter@@YAHPAU_EXCEPTION_POINTERS@@@Z

// LIBRARY: MW2SHELL 0x1004d9d0 SYMBOL
// ?CatchIt@@YAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PAU_CONTEXT@@PAXPBU_s_FuncInfo@@PBU_s_HandlerType@@PBU_s_CatchableType@@PBU_s_TryBlockMapEntry@@H1@Z

// LIBRARY: MW2SHELL 0x1004da70 SYMBOL
// ?CallCatchBlock@@YAPAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PAU_CONTEXT@@PBU_s_FuncInfo@@PAXHK@Z

// LIBRARY: MW2SHELL 0x1004db90 SYMBOL
// ?ExFilterRethrow@@YAHPAU_EXCEPTION_POINTERS@@@Z

// LIBRARY: MW2SHELL 0x1004dbc0 SYMBOL
// ?BuildCatchObject@@YAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PBU_s_HandlerType@@PBU_s_CatchableType@@@Z

// LIBRARY: MW2SHELL 0x1004ddc0 SYMBOL
// ?DestructExceptionObject@@YAXPAUEHExceptionRecord@@E@Z

// LIBRARY: MW2SHELL 0x1004de40 SYMBOL
// ?AdjustPointer@@YAPAXPAXABUPMD@@@Z

// lowhelpr.obj

// LIBRARY: MW2SHELL 0x1004de70 SYMBOL
// __CallSettingFrame@12

// LIBRARY: MW2SHELL 0x1004de95 SYMBOL
// __NLG_Return

// tidtable.obj

// LIBRARY: MW2SHELL 0x1004dec0 SYMBOL
// __mtinit

// LIBRARY: MW2SHELL 0x1004df20 SYMBOL
// __mtterm

// LIBRARY: MW2SHELL 0x1004df50 SYMBOL
// __initptd

// LIBRARY: MW2SHELL 0x1004df70 SYMBOL
// __getptd

// LIBRARY: MW2SHELL 0x1004dfe0 SYMBOL
// __freeptd

// setlocal.obj

// LIBRARY: MW2SHELL 0x1004e090 SYMBOL
// ___init_dummy

// aw_map.obj

// LIBRARY: MW2SHELL 0x1004e0a0 SYMBOL
// ___crtLCMapStringA

// LIBRARY: MW2SHELL 0x1004e2d0 SYMBOL
// _strncnt

// isctype.obj

// LIBRARY: MW2SHELL 0x1004e300 SYMBOL
// __isctype

// heapinit.obj

// LIBRARY: MW2SHELL 0x1004e3a0 SYMBOL
// __heap_init

// LIBRARY: MW2SHELL 0x1004e3c0 SYMBOL
// __heap_term

// commit.obj

// LIBRARY: MW2SHELL 0x1004e3d0 SYMBOL
// __commit

// write.obj

// LIBRARY: MW2SHELL 0x1004e480 SYMBOL
// __write

// LIBRARY: MW2SHELL 0x1004e500 SYMBOL
// __write_lk

// ioinit.obj

// LIBRARY: MW2SHELL 0x1004e700 SYMBOL
// __ioinit

// LIBRARY: MW2SHELL 0x1004e8e0 SYMBOL
// __ioterm

// closeall.obj

// LIBRARY: MW2SHELL 0x1004e940 SYMBOL
// __fcloseall

// _sftbuf.obj

// LIBRARY: MW2SHELL 0x1004e9e0 SYMBOL
// __stbuf

// LIBRARY: MW2SHELL 0x1004ea80 SYMBOL
// __ftbuf

// dosmap.obj

// LIBRARY: MW2SHELL 0x1004eac0 SYMBOL
// __dosmaperr

// LIBRARY: MW2SHELL 0x1004eb40 SYMBOL
// __errno

// LIBRARY: MW2SHELL 0x1004eb50 SYMBOL
// ___doserrno

// handler.obj

// LIBRARY: MW2SHELL 0x1004eb60 SYMBOL
// __callnewh

// ftime.obj

// LIBRARY: MW2SHELL 0x1004eba0 SYMBOL
// __ftime

// _getbuf.obj

// LIBRARY: MW2SHELL 0x1004ec50 SYMBOL
// __getbuf

// 87tran.obj

// LIBRARY: MW2SHELL 0x1004ecb1 SYMBOL
// __fFEXP

// LIBRARY: MW2SHELL 0x1004ed18 SYMBOL
// __rtinfpopse

// LIBRARY: MW2SHELL 0x1004ed1a SYMBOL
// __rtinfnpopse

// LIBRARY: MW2SHELL 0x1004ed2a SYMBOL
// __fFLN

// LIBRARY: MW2SHELL 0x1004eddf SYMBOL
// __rtinfpop

// LIBRARY: MW2SHELL 0x1004ede1 SYMBOL
// __rtinfnpop

// LIBRARY: MW2SHELL 0x1004edfe SYMBOL
// __ffexpm1

// 87cdisp.obj

// LIBRARY: MW2SHELL 0x1004eeb0 SYMBOL
// __cintrindisp2

// LIBRARY: MW2SHELL 0x1004eeee SYMBOL
// __cintrindisp1

// LIBRARY: MW2SHELL 0x1004ef2b SYMBOL
// __ctrandisp2

// LIBRARY: MW2SHELL 0x1004f0ab SYMBOL
// __ctrandisp1

// LIBRARY: MW2SHELL 0x1004f0de SYMBOL
// __fload

// lseek.obj

// LIBRARY: MW2SHELL 0x1004f120 SYMBOL
// __lseek

// LIBRARY: MW2SHELL 0x1004f1a0 SYMBOL
// __lseek_lk

// stdenvp.obj

// LIBRARY: MW2SHELL 0x1004f230 SYMBOL
// __setenvp

// stdargv.obj

// LIBRARY: MW2SHELL 0x1004f310 SYMBOL
// __setargv

// LIBRARY: MW2SHELL 0x1004f3b0 SYMBOL
// _parse_cmdline

// mbctype.obj

// LIBRARY: MW2SHELL 0x1004f580 SYMBOL
// __setmbcp

// LIBRARY: MW2SHELL 0x1004f7b0 SYMBOL
// _getSystemCP

// LIBRARY: MW2SHELL 0x1004f800 SYMBOL
// _CPtoLCID

// LIBRARY: MW2SHELL 0x1004f860 SYMBOL
// _setSBCS

// LIBRARY: MW2SHELL 0x1004f890 SYMBOL
// ___initmbctable

// aw_env.obj

// LIBRARY: MW2SHELL 0x1004f8a0 SYMBOL
// ___crtGetEnvironmentStringsA

// errmode.obj

// LIBRARY: MW2SHELL 0x1004fa40 SYMBOL
// ___set_app_type

// dllmain.obj

// LIBRARY: MW2SHELL 0x1004fa50 SYMBOL
// _DllMain@12

// crt0msg.obj

// LIBRARY: MW2SHELL 0x1004fa60 SYMBOL
// __FF_MSGBANNER

// LIBRARY: MW2SHELL 0x1004faa0 SYMBOL
// __NMSG_WRITE

// isatty.obj

// LIBRARY: MW2SHELL 0x1004fc90 SYMBOL
// __isatty

// wctomb.obj

// LIBRARY: MW2SHELL 0x1004fcc0 SYMBOL
// _wctomb

// LIBRARY: MW2SHELL 0x1004fd20 SYMBOL
// __wctomb_lk

// ulldiv.obj

// LIBRARY: MW2SHELL 0x1004fdc0 SYMBOL
// __aulldiv

// ullrem.obj

// LIBRARY: MW2SHELL 0x1004fe30 SYMBOL
// __aullrem

// ieee87.obj

// LIBRARY: MW2SHELL 0x1004feb0 SYMBOL
// __control87

// LIBRARY: MW2SHELL 0x1004fef0 SYMBOL
// __controlfp

// LIBRARY: MW2SHELL 0x1004ff10 SYMBOL
// __abstract_cw

// LIBRARY: MW2SHELL 0x1004ffc0 SYMBOL
// __hw_cw

// crt0fp.obj

// LIBRARY: MW2SHELL 0x10050050 SYMBOL
// __fptrap

// intrncvt.obj

// LIBRARY: MW2SHELL 0x10050060 SYMBOL
// __ZeroTail

// LIBRARY: MW2SHELL 0x100500d0 SYMBOL
// __IncMan

// LIBRARY: MW2SHELL 0x10050140 SYMBOL
// __RoundMan

// LIBRARY: MW2SHELL 0x100501f0 SYMBOL
// __CopyMan

// LIBRARY: MW2SHELL 0x10050210 SYMBOL
// __FillZeroMan

// LIBRARY: MW2SHELL 0x10050220 SYMBOL
// __IsZeroMan

// LIBRARY: MW2SHELL 0x10050240 SYMBOL
// __ShrMan

// LIBRARY: MW2SHELL 0x100502f0 SYMBOL
// __ld12cvt

// LIBRARY: MW2SHELL 0x100504c0 SYMBOL
// __ld12tod

// LIBRARY: MW2SHELL 0x100504e0 SYMBOL
// __ld12tof

// LIBRARY: MW2SHELL 0x10050500 SYMBOL
// __atodbl

// LIBRARY: MW2SHELL 0x10050540 SYMBOL
// __atoflt

// _fptostr.obj

// LIBRARY: MW2SHELL 0x10050580 SYMBOL
// __fptostr

// cfout.obj

// LIBRARY: MW2SHELL 0x10050610 SYMBOL
// __fltout2

// LIBRARY: MW2SHELL 0x100506a0 SYMBOL
// ___dtold

// osfinfo.obj

// LIBRARY: MW2SHELL 0x10050760 SYMBOL
// __alloc_osfhnd

// LIBRARY: MW2SHELL 0x100508a0 SYMBOL
// __set_osfhnd

// LIBRARY: MW2SHELL 0x10050950 SYMBOL
// __free_osfhnd

// LIBRARY: MW2SHELL 0x100509f0 SYMBOL
// __get_osfhandle

// LIBRARY: MW2SHELL 0x10050a40 SYMBOL
// __lock_fhandle

// LIBRARY: MW2SHELL 0x10050ab0 SYMBOL
// __unlock_fhandle

// open.obj

// LIBRARY: MW2SHELL 0x10050ae0 SYMBOL
// __open

// LIBRARY: MW2SHELL 0x10050b00 SYMBOL
// __sopen

// unhandld.obj

// LIBRARY: MW2SHELL 0x10050f60 SYMBOL
// _$E2

// LIBRARY: MW2SHELL 0x10050f70 SYMBOL
// _$E1

// LIBRARY: MW2SHELL 0x10050f90 SYMBOL
// ?__CxxUnhandledExceptionFilter@@YGJPAU_EXCEPTION_POINTERS@@@Z

// LIBRARY: MW2SHELL 0x10050ff0 SYMBOL
// ?__CxxRestoreUnhandledExceptionFilter@@YAXXZ

// hooks.obj

// LIBRARY: MW2SHELL 0x10051000 SYMBOL
// ?terminate@@YAXXZ

// LIBRARY: MW2SHELL 0x10051080 SYMBOL
// ?_inconsistency@@YAXXZ

// validate.obj

// LIBRARY: MW2SHELL 0x10051100 SYMBOL
// ?_ValidateRead@@YAHPBXI@Z

// LIBRARY: MW2SHELL 0x10051120 SYMBOL
// ?_ValidateWrite@@YAHPAXI@Z

// LIBRARY: MW2SHELL 0x10051140 SYMBOL
// ?_ValidateExecute@@YAHP6GHXZ@Z

// exsup3.obj

// LIBRARY: MW2SHELL 0x10051164 SYMBOL
// __except_handler3

// LIBRARY: MW2SHELL 0x10051221 SYMBOL
// __seh_longjmp_unwind@4

// inittime.obj

// LIBRARY: MW2SHELL 0x10051240 SYMBOL
// ___init_time

// LIBRARY: MW2SHELL 0x100512f0 SYMBOL
// __get_lc_time

// LIBRARY: MW2SHELL 0x10051670 SYMBOL
// __free_lc_time

// LIBRARY: MW2SHELL 0x100518b0 SYMBOL
// _storeTimeFmt

// initnum.obj

// LIBRARY: MW2SHELL 0x100519b0 SYMBOL
// ___init_numeric

// LIBRARY: MW2SHELL 0x10051bd0 SYMBOL
// _fix_grouping

// initmon.obj

// LIBRARY: MW2SHELL 0x10051c20 SYMBOL
// ___init_monetary

// LIBRARY: MW2SHELL 0x10051d10 SYMBOL
// __get_lc_lconv

// LIBRARY: MW2SHELL 0x10051e60 SYMBOL
// _fix_grouping

// LIBRARY: MW2SHELL 0x10051eb0 SYMBOL
// __free_lc_lconv

// initctyp.obj

// LIBRARY: MW2SHELL 0x10051f20 SYMBOL
// ___init_ctype

// initcoll.obj

// LIBRARY: MW2SHELL 0x100521d0 SYMBOL
// ___init_collate

// strpbrk.obj

// LIBRARY: MW2SHELL 0x100521e0 SYMBOL
// _strpbrk

// aw_str.obj

// LIBRARY: MW2SHELL 0x10052220 SYMBOL
// ___crtGetStringTypeW

// LIBRARY: MW2SHELL 0x100523e0 SYMBOL
// ___crtGetStringTypeA

// mktime.obj

// LIBRARY: MW2SHELL 0x10052510 SYMBOL
// _mktime

// LIBRARY: MW2SHELL 0x10052520 SYMBOL
// __make_time_t

// tzset.obj

// LIBRARY: MW2SHELL 0x10052780 SYMBOL
// ___tzset

// LIBRARY: MW2SHELL 0x100527c0 SYMBOL
// __tzset_lk

// LIBRARY: MW2SHELL 0x10052aa0 SYMBOL
// __isindst

// LIBRARY: MW2SHELL 0x10052ad0 SYMBOL
// __isindst_lk

// LIBRARY: MW2SHELL 0x10052d30 SYMBOL
// _cvtdate

// 87disp.obj

// LIBRARY: MW2SHELL 0x10052ec0 SYMBOL
// __trandisp1

// LIBRARY: MW2SHELL 0x10052f27 SYMBOL
// __trandisp2

// LIBRARY: MW2SHELL 0x10052fb3 SYMBOL
// __rttospopde

// LIBRARY: MW2SHELL 0x10052fb8 SYMBOL
// __rttospop

// LIBRARY: MW2SHELL 0x10052fba SYMBOL
// __rtnospop

// LIBRARY: MW2SHELL 0x10052fbc SYMBOL
// __rttosnpop

// LIBRARY: MW2SHELL 0x10052fbd SYMBOL
// __rtnospopde

// LIBRARY: MW2SHELL 0x10052fc4 SYMBOL
// __rtzeropop

// LIBRARY: MW2SHELL 0x10052fc6 SYMBOL
// __rtzeronpop

// LIBRARY: MW2SHELL 0x10052fcb SYMBOL
// __rtonepop

// LIBRARY: MW2SHELL 0x10052fcd SYMBOL
// __rtonenpop

// LIBRARY: MW2SHELL 0x10052fd2 SYMBOL
// __tosnan1

// LIBRARY: MW2SHELL 0x10052ffd SYMBOL
// __nosnan2

// LIBRARY: MW2SHELL 0x10052fff SYMBOL
// __tosnan2

// LIBRARY: MW2SHELL 0x10053027 SYMBOL
// __nan2

// LIBRARY: MW2SHELL 0x10053066 SYMBOL
// __rtindfpop

// LIBRARY: MW2SHELL 0x10053068 SYMBOL
// __rtindfnpop

// LIBRARY: MW2SHELL 0x10053079 SYMBOL
// __rttosnpopde

// LIBRARY: MW2SHELL 0x10053083 SYMBOL
// __rtchsifneg

// powhlp.obj

// LIBRARY: MW2SHELL 0x10053090 SYMBOL
// __powhlp

// LIBRARY: MW2SHELL 0x100532e0 SYMBOL
// __d_inttype

// adj_fdiv.obj

// LIBRARY: MW2SHELL 0x10053360 SYMBOL
// _fdiv_main_routine

// LIBRARY: MW2SHELL 0x10053477 SYMBOL
// __adj_fdiv_r

// LIBRARY: MW2SHELL 0x10053916 SYMBOL
// __fdivp_sti_st

// LIBRARY: MW2SHELL 0x10053929 SYMBOL
// __fdivrp_sti_st

// LIBRARY: MW2SHELL 0x1005393c SYMBOL
// __adj_fdiv_m32

// LIBRARY: MW2SHELL 0x10053988 SYMBOL
// __adj_fdiv_m64

// LIBRARY: MW2SHELL 0x100539d4 SYMBOL
// __adj_fdiv_m16i

// LIBRARY: MW2SHELL 0x10053a08 SYMBOL
// __adj_fdiv_m32i

// LIBRARY: MW2SHELL 0x10053a3c SYMBOL
// __adj_fdivr_m32

// LIBRARY: MW2SHELL 0x10053a88 SYMBOL
// __adj_fdivr_m64

// LIBRARY: MW2SHELL 0x10053ad4 SYMBOL
// __adj_fdivr_m16i

// LIBRARY: MW2SHELL 0x10053b08 SYMBOL
// __adj_fdivr_m32i

// LIBRARY: MW2SHELL 0x10053b3c SYMBOL
// __safe_fdiv

// LIBRARY: MW2SHELL 0x10053b51 SYMBOL
// __safe_fdivr

// LIBRARY: MW2SHELL 0x10053b66 SYMBOL
// __fprem_common

// LIBRARY: MW2SHELL 0x10053d6c SYMBOL
// __adj_fprem

// LIBRARY: MW2SHELL 0x10053e1e SYMBOL
// __fprem1_common

// LIBRARY: MW2SHELL 0x10054024 SYMBOL
// __adj_fprem1

// LIBRARY: MW2SHELL 0x100540d9 SYMBOL
// __safe_fprem

// LIBRARY: MW2SHELL 0x100540df SYMBOL
// __safe_fprem1

// LIBRARY: MW2SHELL 0x100540e5 SYMBOL
// __adj_fpatan

// LIBRARY: MW2SHELL 0x100540e8 SYMBOL
// __adj_fptan

// 87except.obj

// LIBRARY: MW2SHELL 0x100540f0 SYMBOL
// __87except

// wcslen.obj

// LIBRARY: MW2SHELL 0x10054210 SYMBOL
// _wcslen

// crtmbox.obj

// LIBRARY: MW2SHELL 0x10054230 SYMBOL
// ___crtMessageBoxA

// winsig.obj

// LIBRARY: MW2SHELL 0x100542d0 SYMBOL
// _raise

// LIBRARY: MW2SHELL 0x100544b0 SYMBOL
// _siglookup

// mantold.obj

// LIBRARY: MW2SHELL 0x100544f0 SYMBOL
// ___addl

// LIBRARY: MW2SHELL 0x10054520 SYMBOL
// ___add_12

// LIBRARY: MW2SHELL 0x10054590 SYMBOL
// ___shl_12

// LIBRARY: MW2SHELL 0x100545d0 SYMBOL
// ___shr_12

// LIBRARY: MW2SHELL 0x10054610 SYMBOL
// ___mtold12

// strgtold.obj

// LIBRARY: MW2SHELL 0x10054700 SYMBOL
// ___strgtold12

// x10fout.obj

// LIBRARY: MW2SHELL 0x10054e50 SYMBOL
// _$I10_OUTPUT

// chsize.obj

// LIBRARY: MW2SHELL 0x100551f0 SYMBOL
// __chsize_lk

// onexit.obj

// LIBRARY: MW2SHELL 0x10055330 SYMBOL
// __onexit

// LIBRARY: MW2SHELL 0x100553c0 SYMBOL
// _atexit

// LIBRARY: MW2SHELL 0x100553e0 SYMBOL
// ___onexitinit

// abort.obj

// LIBRARY: MW2SHELL 0x10055420 SYMBOL
// _abort

// inithelp.obj

// LIBRARY: MW2SHELL 0x10055440 SYMBOL
// ___getlocaleinfo

// aw_loc.obj

// LIBRARY: MW2SHELL 0x10055600 SYMBOL
// ___crtGetLocaleInfoW

// LIBRARY: MW2SHELL 0x10055730 SYMBOL
// ___crtGetLocaleInfoA

// stricmp.obj

// LIBRARY: MW2SHELL 0x10055870 SYMBOL
// __strcmpi

// gmtime.obj

// LIBRARY: MW2SHELL 0x10055940 SYMBOL
// _gmtime

// localtim.obj

// LIBRARY: MW2SHELL 0x10055aa0 SYMBOL
// _localtime

// mbstowcs.obj

// LIBRARY: MW2SHELL 0x10055c70 SYMBOL
// _mbstowcs

// wcstombs.obj

// LIBRARY: MW2SHELL 0x10055cd0 SYMBOL
// __wcstombs_lk

// LIBRARY: MW2SHELL 0x10055f40 SYMBOL
// _wcsncnt

// getenv.obj

// LIBRARY: MW2SHELL 0x10055f80 SYMBOL
// __getenv_lk

// util.obj

// LIBRARY: MW2SHELL 0x10056020 SYMBOL
// __set_exp

// LIBRARY: MW2SHELL 0x10056060 SYMBOL
// __sptype

// LIBRARY: MW2SHELL 0x100560d0 SYMBOL
// __decomp

// frnd.obj

// LIBRARY: MW2SHELL 0x100561c0 SYMBOL
// __frnd

// ieeemisc.obj

// LIBRARY: MW2SHELL 0x100561e0 SYMBOL
// __fpclass

// fpexcept.obj

// LIBRARY: MW2SHELL 0x10056290 SYMBOL
// __raise_exc

// LIBRARY: MW2SHELL 0x10056600 SYMBOL
// __handle_exc

// LIBRARY: MW2SHELL 0x100568c0 SYMBOL
// __set_errno

// strstrea.obj

// LIBRARY: MW2SHELL 0x100568f0 SYMBOL
// ?sync@strstreambuf@@UAEHXZ

// fpctrl.obj

// LIBRARY: MW2SHELL 0x10056900 SYMBOL
// __statfp

// LIBRARY: MW2SHELL 0x10056920 SYMBOL
// __clrfp

// LIBRARY: MW2SHELL 0x10056940 SYMBOL
// __ctrlfp

// LIBRARY: MW2SHELL 0x10056980 SYMBOL
// __set_statfp

// tenpow.obj

// LIBRARY: MW2SHELL 0x100569e0 SYMBOL
// ___ld12mul

// LIBRARY: MW2SHELL 0x10056c90 SYMBOL
// ___multtenpow12

// setmode.obj

// LIBRARY: MW2SHELL 0x10056d10 SYMBOL
// __setmode_lk

// mbsnbico.obj

// LIBRARY: MW2SHELL 0x10056d80 SYMBOL
// __mbsnbicoll

// wtombenv.obj

// LIBRARY: MW2SHELL 0x10056dc0 SYMBOL
// ___wtomb_environ

// aw_cmp.obj

// LIBRARY: MW2SHELL 0x10056e50 SYMBOL
// ___crtCompareStringA

// LIBRARY: MW2SHELL 0x10057140 SYMBOL
// _strncnt

// setenv.obj

// LIBRARY: MW2SHELL 0x10057170 SYMBOL
// ___crtsetenv

// LIBRARY: MW2SHELL 0x100573c0 SYMBOL
// _findenv

// LIBRARY: MW2SHELL 0x10057420 SYMBOL
// _copy_environ

// mbschr.obj

// LIBRARY: MW2SHELL 0x10057490 SYMBOL
// __mbschr

// mbsdup.obj

// LIBRARY: MW2SHELL 0x10057550 SYMBOL
// __mbsdup

// strlen.obj

// LIBRARY: MW2SHELL 0x10057580 SYMBOL
// _strlen

// mbscat.obj

// LIBRARY: MW2SHELL 0x10057600 SYMBOL
// __mbscpy

// LIBRARY: MW2SHELL 0x10057610 SYMBOL
// __mbscat

// drivemap.obj

// LIBRARY: MW2SHELL 0x100576f0 SYMBOL
// __getdrives

// strnicmp.obj

// LIBRARY: MW2SHELL 0x10057700 SYMBOL
// __strnicmp

// strnset.obj

// LIBRARY: MW2SHELL 0x10057800 SYMBOL
// __strnset

// flength.obj

// LIBRARY: MW2SHELL 0x10057830 SYMBOL
// __filelength

// _file.obj

// GLOBAL: MW2SHELL 0x10074a60
// _iob

#endif
