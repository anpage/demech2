#ifdef 0
// C runtime library code in MW2.DLL: VC++ 4.1's LIBCMTD.LIB
// (the debug multithreaded CRT, linked /DEBUG: whole objects), identified object by object against the library.
//
// Public functions are matched by their public symbol (SYMBOL). The debug and release CRT
// share those, while the names reccmp derives from CodeView differ between them.
// Static functions have no public symbol; they are named by the C name that LIBCMTD's
// CodeView records carry. Those without one (assembler routines) and those whose name
// recurs in several objects (wcsncnt, fix_grouping ...) keep the unmatchable SYMBOL form
// so that reccmp can't pair them with the wrong copy.
// Aliases at one address (__chkstk/__alloca_probe, __strcmpi/__stricmp) take the symbol
// reccmp keeps for that address.

// sprintf.obj

// LIBRARY: MW2 0x10080490 SYMBOL
// _sprintf

// ftol.obj

// LIBRARY: MW2 0x1008057c SYMBOL
// __ftol

// 87csqrt.obj

// LIBRARY: MW2 0x100805b0 SYMBOL
// _sqrt

// LIBRARY: MW2 0x100805ba SYMBOL
// __CIsqrt

// fpinit.obj

// LIBRARY: MW2 0x100805d0 SYMBOL
// __fpmath

// LIBRARY: MW2 0x100805f0 SYMBOL
// __fpclear

// LIBRARY: MW2 0x10080600 SYMBOL
// __cfltcvt_init

// user.obj

// LIBRARY: MW2 0x10080640 SYMBOL
// ?__set_inconsistency@@YAP6AXXZP6AXXZ@Z

// strncpy.obj

// LIBRARY: MW2 0x10080650 SYMBOL
// _strncpy

// sscanf.obj

// LIBRARY: MW2 0x10080750 SYMBOL
// _sscanf

// atox.obj

// LIBRARY: MW2 0x10080820 SYMBOL
// _atol

// LIBRARY: MW2 0x10080940 SYMBOL
// _atoi

// strchr.obj

// LIBRARY: MW2 0x10080970 SYMBOL
// _strchr

// LIBRARY: MW2 0x10080976 SYMBOL
// ___from_strstr_to_strchr

// isctype.obj

// LIBRARY: MW2 0x10080a30 SYMBOL
// __isctype

// fclose.obj

// LIBRARY: MW2 0x10080af0 SYMBOL
// _fclose

// LIBRARY: MW2 0x10080b80 SYMBOL
// __fclose_lk

// fprintf.obj

// LIBRARY: MW2 0x10080c50 SYMBOL
// _fprintf

// fopen.obj

// LIBRARY: MW2 0x10080d20 SYMBOL
// __fsopen

// LIBRARY: MW2 0x10080e30 SYMBOL
// _fopen

// fread.obj

// LIBRARY: MW2 0x10080e60 SYMBOL
// _fread

// fwrite.obj

// LIBRARY: MW2 0x10080eb0 SYMBOL
// __fwrite_lk

// strtok.obj

// LIBRARY: MW2 0x100810d0 SYMBOL
// _strtok

// toupper.obj

// LIBRARY: MW2 0x10081250 SYMBOL
// __toupper

// LIBRARY: MW2 0x10081270 SYMBOL
// _toupper

// LIBRARY: MW2 0x10081320 SYMBOL
// __toupper_lk

// strtol.obj

// LIBRARY: MW2 0x10081460 SYMBOL
// _strtol

// LIBRARY: MW2 0x10081490
// strtoxl

// LIBRARY: MW2 0x10081830 SYMBOL
// _strtoul

// rmdir.obj

// LIBRARY: MW2 0x10081860 SYMBOL
// __rmdir

// unlink.obj

// LIBRARY: MW2 0x100818c0 SYMBOL
// __unlink

// printf.obj

// LIBRARY: MW2 0x100818e0 SYMBOL
// _printf

// memmove.obj

// LIBRARY: MW2 0x100819a0 SYMBOL
// _memmove

// fflush.obj

// LIBRARY: MW2 0x10081af0 SYMBOL
// _fflush

// LIBRARY: MW2 0x10081b50 SYMBOL
// __fflush_lk

// LIBRARY: MW2 0x10081bc0 SYMBOL
// __flush

// LIBRARY: MW2 0x10081c80 SYMBOL
// __flushall

// LIBRARY: MW2 0x10081ca0
// flsall

// fputs.obj

// LIBRARY: MW2 0x10081e00 SYMBOL
// _fputs

// vsnprint.obj

// LIBRARY: MW2 0x10081ef0 SYMBOL
// __vsnprintf

// crt0dat.obj

// LIBRARY: MW2 0x10081fe0 SYMBOL
// __cinit

// LIBRARY: MW2 0x10082030 SYMBOL
// _exit

// LIBRARY: MW2 0x10082050 SYMBOL
// __exit

// LIBRARY: MW2 0x10082070 SYMBOL
// __cexit

// LIBRARY: MW2 0x10082090 SYMBOL
// __c_exit

// LIBRARY: MW2 0x100820b0
// doexit

// LIBRARY: MW2 0x100821c0 SYMBOL
// __lockexit

// LIBRARY: MW2 0x100821e0 SYMBOL
// __unlockexit

// LIBRARY: MW2 0x10082200
// _initterm

// vsprintf.obj

// LIBRARY: MW2 0x10082240 SYMBOL
// _vsprintf

// tolower.obj

// LIBRARY: MW2 0x10082330 SYMBOL
// __tolower

// LIBRARY: MW2 0x10082350 SYMBOL
// _tolower

// LIBRARY: MW2 0x10082400 SYMBOL
// __tolower_lk

// qsort.obj

// LIBRARY: MW2 0x10082540 SYMBOL
// _qsort

// LIBRARY: MW2 0x100827a0
// shortsort

// LIBRARY: MW2 0x10082830
// swap

// strstr.obj

// LIBRARY: MW2 0x10082890 SYMBOL
// _strstr

// chkstk.obj

// LIBRARY: MW2 0x10082910 SYMBOL
// __chkstk

// 87ctran.obj

// LIBRARY: MW2 0x10082940 SYMBOL
// _pow

// LIBRARY: MW2 0x1008294a SYMBOL
// _log

// LIBRARY: MW2 0x10082954 SYMBOL
// _log10

// LIBRARY: MW2 0x1008295b SYMBOL
// _exp

// LIBRARY: MW2 0x10082962 SYMBOL
// __CIpow

// LIBRARY: MW2 0x1008296c SYMBOL
// __CIlog

// LIBRARY: MW2 0x10082976 SYMBOL
// __CIlog10

// LIBRARY: MW2 0x1008297d SYMBOL
// __CIexp

// strncat.obj

// LIBRARY: MW2 0x10082990 SYMBOL
// _strncat

// exsup.obj

// LIBRARY: MW2 0x10082ab4 SYMBOL
// __global_unwind2

// LIBRARY: MW2 0x10082ad4
// _unwind_handler

// LIBRARY: MW2 0x10082af6 SYMBOL
// __local_unwind2

// LIBRARY: MW2 0x10082b4e SYMBOL
// __NLG_Return2

// LIBRARY: MW2 0x10082b5e SYMBOL
// __abnormal_termination

// LIBRARY: MW2 0x10082b81 SYMBOL
// __NLG_Notify1

// LIBRARY: MW2 0x10082b8a SYMBOL
// __NLG_Notify

// LIBRARY: MW2 0x10082b9d SYMBOL
// __NLG_Dispatch

// getenv.obj

// LIBRARY: MW2 0x10082bb0 SYMBOL
// _getenv

// LIBRARY: MW2 0x10082bf0 SYMBOL
// __getenv_lk

// exsup3.obj

// LIBRARY: MW2 0x10082cdc SYMBOL
// __except_handler3

// LIBRARY: MW2 0x10082d99 SYMBOL
// __seh_longjmp_unwind@4

// ftell.obj

// LIBRARY: MW2 0x10082dc0 SYMBOL
// _ftell

// LIBRARY: MW2 0x10082e30 SYMBOL
// __ftell_lk

// strspn.obj

// LIBRARY: MW2 0x100830f0 SYMBOL
// _strspn

// fgets.obj

// LIBRARY: MW2 0x10083130 SYMBOL
// _fgets

// fseek.obj

// LIBRARY: MW2 0x10083280 SYMBOL
// _fseek

// LIBRARY: MW2 0x10083300 SYMBOL
// __fseek_lk

// ctime.obj

// LIBRARY: MW2 0x10083440 SYMBOL
// _ctime

// time.obj

// LIBRARY: MW2 0x10083490 SYMBOL
// _time

// strncmp.obj

// LIBRARY: MW2 0x10083620 SYMBOL
// _strncmp

// rand.obj

// LIBRARY: MW2 0x10083660 SYMBOL
// _srand

// LIBRARY: MW2 0x10083680 SYMBOL
// _rand

// dbgheap.obj

// LIBRARY: MW2 0x100836d0 SYMBOL
// _malloc

// LIBRARY: MW2 0x10083700 SYMBOL
// __malloc_dbg

// LIBRARY: MW2 0x10083730 SYMBOL
// __nh_malloc

// LIBRARY: MW2 0x10083760 SYMBOL
// __nh_malloc_dbg

// LIBRARY: MW2 0x100837e0 SYMBOL
// __heap_alloc

// LIBRARY: MW2 0x10083810 SYMBOL
// __heap_alloc_dbg

// LIBRARY: MW2 0x10083b40 SYMBOL
// _calloc

// LIBRARY: MW2 0x10083b70 SYMBOL
// __calloc_dbg

// LIBRARY: MW2 0x10083be0 SYMBOL
// _realloc

// LIBRARY: MW2 0x10083c10 SYMBOL
// __realloc_dbg

// LIBRARY: MW2 0x10083c60
// realloc_help

// LIBRARY: MW2 0x100841e0 SYMBOL
// __expand

// LIBRARY: MW2 0x10084210 SYMBOL
// __expand_dbg

// LIBRARY: MW2 0x10084260 SYMBOL
// _free

// LIBRARY: MW2 0x10084280 SYMBOL
// __free_lk

// LIBRARY: MW2 0x100842a0 SYMBOL
// __free_dbg

// LIBRARY: MW2 0x100842d0 SYMBOL
// __free_dbg_lk

// LIBRARY: MW2 0x10084700 SYMBOL
// __msize

// LIBRARY: MW2 0x10084720 SYMBOL
// __msize_dbg

// LIBRARY: MW2 0x100848a0 SYMBOL
// __CrtSetBreakAlloc

// LIBRARY: MW2 0x100848d0 SYMBOL
// __CrtSetDbgBlockType

// LIBRARY: MW2 0x10084990 SYMBOL
// __CrtSetAllocHook

// LIBRARY: MW2 0x100849c0
// CheckBytes

// LIBRARY: MW2 0x10084a50 SYMBOL
// __CrtCheckMemory

// LIBRARY: MW2 0x10084df0 SYMBOL
// __CrtSetDbgFlag

// LIBRARY: MW2 0x10084e20 SYMBOL
// __CrtDoForAllClientObjects

// LIBRARY: MW2 0x10084ea0 SYMBOL
// __CrtIsValidPointer

// LIBRARY: MW2 0x10084f00 SYMBOL
// __CrtIsValidHeapPointer

// LIBRARY: MW2 0x10084f70 SYMBOL
// __CrtIsMemoryBlock

// LIBRARY: MW2 0x10085090 SYMBOL
// __CrtSetDumpClient

// LIBRARY: MW2 0x100850c0 SYMBOL
// __CrtMemCheckpoint

// LIBRARY: MW2 0x10085210 SYMBOL
// __CrtMemDifference

// LIBRARY: MW2 0x10085350 SYMBOL
// __CrtMemDumpAllObjectsSince

// LIBRARY: MW2 0x10085630
// _printMemBlockData

// LIBRARY: MW2 0x10085730 SYMBOL
// __CrtDumpMemoryLeaks

// LIBRARY: MW2 0x100857c0 SYMBOL
// __CrtMemDumpStatistics

// fputc.obj

// LIBRARY: MW2 0x10085890 SYMBOL
// _fputc

// rotl.obj

// LIBRARY: MW2 0x10085940 SYMBOL
// __lrotl

// 87ctriga.obj

// LIBRARY: MW2 0x10085960 SYMBOL
// _asin

// LIBRARY: MW2 0x1008596a SYMBOL
// _acos

// LIBRARY: MW2 0x10085971 SYMBOL
// _atan

// LIBRARY: MW2 0x10085978 SYMBOL
// _atan2

// LIBRARY: MW2 0x10085982 SYMBOL
// __CIasin

// LIBRARY: MW2 0x1008598c SYMBOL
// __CIacos

// LIBRARY: MW2 0x10085993 SYMBOL
// __CIatan

// LIBRARY: MW2 0x1008599a SYMBOL
// __CIatan2

// 87ctrig.obj

// LIBRARY: MW2 0x100859b0 SYMBOL
// _sin

// LIBRARY: MW2 0x100859ba SYMBOL
// _cos

// LIBRARY: MW2 0x100859c1 SYMBOL
// _tan

// LIBRARY: MW2 0x100859c8 SYMBOL
// __CIsin

// LIBRARY: MW2 0x100859d2 SYMBOL
// __CIcos

// LIBRARY: MW2 0x100859d9 SYMBOL
// __CItan

// dllcrt0.obj

// LIBRARY: MW2 0x100859e0 SYMBOL
// __CRT_INIT@12

// LIBRARY: MW2 0x10085bc0 SYMBOL
// __DllMainCRTStartup@12

// LIBRARY: MW2 0x10085d10 SYMBOL
// __amsg_exit

// _flsbuf.obj

// LIBRARY: MW2 0x10085d70 SYMBOL
// __flsbuf

// output.obj

// LIBRARY: MW2 0x10086010 SYMBOL
// __output

// LIBRARY: MW2 0x10086dc0
// write_char

// LIBRARY: MW2 0x10086e40
// write_multi_char

// LIBRARY: MW2 0x10086e90
// write_string

// cprintf.obj

// LIBRARY: MW2 0x10086ef0
// get_int_arg

// LIBRARY: MW2 0x10086f10
// get_int64_arg

// LIBRARY: MW2 0x10086f40
// get_short_arg

// dbgrpt.obj

// LIBRARY: MW2 0x10086f60 SYMBOL
// __CrtDbgBreak

// LIBRARY: MW2 0x10086f80 SYMBOL
// __CrtSetReportMode

// LIBRARY: MW2 0x10087000 SYMBOL
// __CrtSetReportFile

// LIBRARY: MW2 0x100870b0 SYMBOL
// __CrtSetReportHook

// LIBRARY: MW2 0x100870e0 SYMBOL
// __CrtDbgReport

// LIBRARY: MW2 0x100874c0
// CrtMessageWindow

// 87sqrt.obj

// LIBRARY: MW2 0x100877f0
// FSQRT

// 87cdisp.obj

// LIBRARY: MW2 0x10087810 SYMBOL
// __cintrindisp2

// LIBRARY: MW2 0x1008784e SYMBOL
// __cintrindisp1

// LIBRARY: MW2 0x1008788b SYMBOL
// __ctrandisp2

// LIBRARY: MW2 0x10087a0b SYMBOL
// __ctrandisp1

// LIBRARY: MW2 0x10087a3e SYMBOL
// __fload

// fp8.obj

// LIBRARY: MW2 0x10087a80 SYMBOL
// __setdefaultprecision

// testfdiv.obj

// LIBRARY: MW2 0x10087aa0 SYMBOL
// __ms_p5_test_fdiv

// LIBRARY: MW2 0x10087af0 SYMBOL
// __ms_p5_mp_test_fdiv

// cvt.obj

// LIBRARY: MW2 0x10087c40 SYMBOL
// __forcdecpt

// LIBRARY: MW2 0x10087cb0 SYMBOL
// __cropzeros

// LIBRARY: MW2 0x10087d10 SYMBOL
// __positive

// LIBRARY: MW2 0x10087d30 SYMBOL
// __fassign

// LIBRARY: MW2 0x10087d80 SYMBOL
// __cftoe

// LIBRARY: MW2 0x10087e00
// _cftoe2

// LIBRARY: MW2 0x10087ef0 SYMBOL
// __cftof

// LIBRARY: MW2 0x10087f60
// _cftof2

// LIBRARY: MW2 0x10088020 SYMBOL
// __cftog

// LIBRARY: MW2 0x100880d0 SYMBOL
// __cfltcvt

// LIBRARY: MW2 0x10088140
// _shift

// input.obj

// LIBRARY: MW2 0x10088170 SYMBOL
// __input

// cscanf.obj

// LIBRARY: MW2 0x10089520
// _hextodec

// input.obj

// LIBRARY: MW2 0x10089590
// _inc

// LIBRARY: MW2 0x100895e0
// _un_inc

// LIBRARY: MW2 0x10089610
// _whiteout

// strlen.obj

// LIBRARY: MW2 0x10089660 SYMBOL
// _strlen

// aw_str.obj

// LIBRARY: MW2 0x100896e0 SYMBOL
// ___crtGetStringTypeW

// LIBRARY: MW2 0x10089940 SYMBOL
// ___crtGetStringTypeA

// mlock.obj

// LIBRARY: MW2 0x10089ae0 SYMBOL
// __mtinitlocks

// LIBRARY: MW2 0x10089b20 SYMBOL
// __mtdeletelocks

// LIBRARY: MW2 0x10089be0 SYMBOL
// __lock

// LIBRARY: MW2 0x10089c90 SYMBOL
// __unlock

// LIBRARY: MW2 0x10089cb0 SYMBOL
// __lock_file

// LIBRARY: MW2 0x10089d10 SYMBOL
// __lock_file2

// LIBRARY: MW2 0x10089d50 SYMBOL
// __unlock_file

// LIBRARY: MW2 0x10089db0 SYMBOL
// __unlock_file2

// LIBRARY: MW2 0x10089df0 SYMBOL
// __lockerr_exit

// close.obj

// LIBRARY: MW2 0x10089e20 SYMBOL
// __close

// LIBRARY: MW2 0x10089ec0 SYMBOL
// __close_lk

// _freebuf.obj

// LIBRARY: MW2 0x10089f90 SYMBOL
// __freebuf

// _sftbuf.obj

// LIBRARY: MW2 0x1008a020 SYMBOL
// __stbuf

// LIBRARY: MW2 0x1008a170 SYMBOL
// __ftbuf

// _open.obj

// LIBRARY: MW2 0x1008a210 SYMBOL
// __openfile

// stream.obj

// LIBRARY: MW2 0x1008a5d0 SYMBOL
// __getstream

// write.obj

// LIBRARY: MW2 0x1008a770 SYMBOL
// __write

// LIBRARY: MW2 0x1008a820 SYMBOL
// __write_lk

// memcpy.obj

// LIBRARY: MW2 0x1008ab20 SYMBOL
// _memcpy

// tidtable.obj

// LIBRARY: MW2 0x1008ac70 SYMBOL
// __mtinit

// LIBRARY: MW2 0x1008ad10 SYMBOL
// __mtterm

// LIBRARY: MW2 0x1008ad50 SYMBOL
// __initptd

// LIBRARY: MW2 0x1008ad70 SYMBOL
// __getptd

// LIBRARY: MW2 0x1008ae20 SYMBOL
// __freeptd

// LIBRARY: MW2 0x1008af30 SYMBOL
// ___threadid

// LIBRARY: MW2 0x1008af50 SYMBOL
// ___threadhandle

// setlocal.obj

// LIBRARY: MW2 0x1008af70 SYMBOL
// _setlocale

// LIBRARY: MW2 0x1008b380
// _setlocale_set_cat

// LIBRARY: MW2 0x1008b530
// _setlocale_get_all

// LIBRARY: MW2 0x1008b650 SYMBOL
// __expandlocale

// LIBRARY: MW2 0x1008b810 SYMBOL
// ___init_dummy

// LIBRARY: MW2 0x1008b830 SYMBOL
// __strcats

// LIBRARY: MW2 0x1008b890 SYMBOL
// ___lc_strtolc

// LIBRARY: MW2 0x1008ba40 SYMBOL
// ___lc_lctostr

// aw_map.obj

// LIBRARY: MW2 0x1008bac0 SYMBOL
// ___crtLCMapStringW

// LIBRARY: MW2 0x1008bdc0 SYMBOL
// _wcsncnt

// LIBRARY: MW2 0x1008be30 SYMBOL
// ___crtLCMapStringA

// LIBRARY: MW2 0x1008c150 SYMBOL
// _strncnt

// dosmap.obj

// LIBRARY: MW2 0x1008c1c0 SYMBOL
// __dosmaperr

// LIBRARY: MW2 0x1008c2a0 SYMBOL
// __errno

// LIBRARY: MW2 0x1008c2c0 SYMBOL
// ___doserrno

// _file.obj

// LIBRARY: MW2 0x1008c2e0 SYMBOL
// ___initstdio

// LIBRARY: MW2 0x1008c440 SYMBOL
// ___endstdio

// commit.obj

// LIBRARY: MW2 0x1008c470 SYMBOL
// __commit

// 87tran.obj

// LIBRARY: MW2 0x1008c5a1 SYMBOL
// __fFEXP

// LIBRARY: MW2 0x1008c608 SYMBOL
// __rtinfpopse

// LIBRARY: MW2 0x1008c60a SYMBOL
// __rtinfnpopse

// LIBRARY: MW2 0x1008c61a SYMBOL
// __fFLN

// LIBRARY: MW2 0x1008c6cf SYMBOL
// __rtinfpop

// LIBRARY: MW2 0x1008c6d1 SYMBOL
// __rtinfnpop

// LIBRARY: MW2 0x1008c6ee SYMBOL
// __ffexpm1

// mbsnbico.obj

// LIBRARY: MW2 0x1008c7a0 SYMBOL
// __mbsnbicoll

// wtombenv.obj

// LIBRARY: MW2 0x1008c810 SYMBOL
// ___wtomb_environ

// ioinit.obj

// LIBRARY: MW2 0x1008c8e0 SYMBOL
// __ioinit

// LIBRARY: MW2 0x1008cc20 SYMBOL
// __ioterm

// lseek.obj

// LIBRARY: MW2 0x1008cce0 SYMBOL
// __lseek

// LIBRARY: MW2 0x1008cd90 SYMBOL
// __lseek_lk

// _filbuf.obj

// LIBRARY: MW2 0x1008ce80 SYMBOL
// __filbuf

// asctime.obj

// LIBRARY: MW2 0x1008d070 SYMBOL
// _asctime

// LIBRARY: MW2 0x1008d220
// store_dt

// localtim.obj

// LIBRARY: MW2 0x1008d260 SYMBOL
// _localtime

// dtoxtime.obj

// LIBRARY: MW2 0x1008d4c0 SYMBOL
// ___loctotime_t

// handler.obj

// LIBRARY: MW2 0x1008d5e0 SYMBOL
// ?_set_new_handler@@YAP6AHI@ZP6AHI@Z@Z

// LIBRARY: MW2 0x1008d620 SYMBOL
// ?_query_new_handler@@YAP6AHI@ZXZ

// LIBRARY: MW2 0x1008d640 SYMBOL
// __callnewh

// memset.obj

// LIBRARY: MW2 0x1008d6a0 SYMBOL
// _memset

// malloc.obj

// LIBRARY: MW2 0x1008d700 SYMBOL
// __malloc_base

// LIBRARY: MW2 0x1008d730 SYMBOL
// __nh_malloc_base

// LIBRARY: MW2 0x1008d7b0 SYMBOL
// __heap_alloc_base

// dbghook.obj

// LIBRARY: MW2 0x1008d7e0 SYMBOL
// __CrtDefaultAllocHook

// expand.obj

// LIBRARY: MW2 0x1008d800 SYMBOL
// __expand_base

// realloc.obj

// LIBRARY: MW2 0x1008d850 SYMBOL
// __realloc_base

// free.obj

// LIBRARY: MW2 0x1008d910 SYMBOL
// __free_base

// heapchk.obj

// LIBRARY: MW2 0x1008d940 SYMBOL
// __heapchk

// LIBRARY: MW2 0x1008d9b0 SYMBOL
// __heapset

// heapinit.obj

// LIBRARY: MW2 0x1008db80 SYMBOL
// __heap_init

// LIBRARY: MW2 0x1008dbb0 SYMBOL
// __heap_term

// 87triga.obj

// LIBRARY: MW2 0x1008dbec SYMBOL
// __fFATN2

// LIBRARY: MW2 0x1008dc3c SYMBOL
// __rtpiby2

// 87trig.obj

// LIBRARY: MW2 0x1008dc80 SYMBOL
// __fFCOS

// LIBRARY: MW2 0x1008dc8a SYMBOL
// __fFSIN

// stdenvp.obj

// LIBRARY: MW2 0x1008dcb0 SYMBOL
// __setenvp

// stdargv.obj

// LIBRARY: MW2 0x1008ddf0 SYMBOL
// __setargv

// LIBRARY: MW2 0x1008dec0
// parse_cmdline

// mbctype.obj

// LIBRARY: MW2 0x1008e280 SYMBOL
// __setmbcp

// LIBRARY: MW2 0x1008e600
// getSystemCP

// LIBRARY: MW2 0x1008e690
// CPtoLCID

// LIBRARY: MW2 0x1008e730
// setSBCS

// LIBRARY: MW2 0x1008e7b0 SYMBOL
// __getmbcp

// LIBRARY: MW2 0x1008e7d0 SYMBOL
// ___initmbctable

// aw_env.obj

// LIBRARY: MW2 0x1008e7f0 SYMBOL
// ___crtGetEnvironmentStringsW

// LIBRARY: MW2 0x1008eab0 SYMBOL
// ___crtGetEnvironmentStringsA

// errmode.obj

// LIBRARY: MW2 0x1008ed00 SYMBOL
// __set_error_mode

// LIBRARY: MW2 0x1008ed80 SYMBOL
// ___set_app_type

// dllmain.obj

// LIBRARY: MW2 0x1008eda0 SYMBOL
// _DllMain@12

// crt0msg.obj

// LIBRARY: MW2 0x1008edc0 SYMBOL
// __FF_MSGBANNER

// LIBRARY: MW2 0x1008ee20 SYMBOL
// __NMSG_WRITE

// LIBRARY: MW2 0x1008f030 SYMBOL
// __GET_RTERRMSG

// _getbuf.obj

// LIBRARY: MW2 0x1008f0b0 SYMBOL
// __getbuf

// isatty.obj

// LIBRARY: MW2 0x1008f170 SYMBOL
// __isatty

// wctomb.obj

// LIBRARY: MW2 0x1008f1c0 SYMBOL
// _wctomb

// LIBRARY: MW2 0x1008f240 SYMBOL
// __wctomb_lk

// ulldiv.obj

// LIBRARY: MW2 0x1008f320 SYMBOL
// __aulldiv

// ullrem.obj

// LIBRARY: MW2 0x1008f390 SYMBOL
// __aullrem

// xtoa.obj

// LIBRARY: MW2 0x1008f410 SYMBOL
// __itoa

// LIBRARY: MW2 0x1008f470
// xtoa

// LIBRARY: MW2 0x1008f530 SYMBOL
// __ltoa

// LIBRARY: MW2 0x1008f590 SYMBOL
// __ultoa

// mbscat.obj

// LIBRARY: MW2 0x1008f5c0 SYMBOL
// __mbscpy

// LIBRARY: MW2 0x1008f5d0 SYMBOL
// __mbscat

// snprintf.obj

// LIBRARY: MW2 0x1008f6b0 SYMBOL
// __snprintf

// winsig.obj

// LIBRARY: MW2 0x1008f7a0 SYMBOL
// _signal

// LIBRARY: MW2 0x1008fa20
// ctrlevent_capture@4

// LIBRARY: MW2 0x1008fad0 SYMBOL
// _raise

// LIBRARY: MW2 0x1008fd60
// siglookup

// LIBRARY: MW2 0x1008fdd0 SYMBOL
// ___fpecode

// LIBRARY: MW2 0x1008fdf0 SYMBOL
// ___pxcptinfoptrs

// crtmbox.obj

// LIBRARY: MW2 0x1008fe10 SYMBOL
// ___crtMessageBoxA

// 87disp.obj

// LIBRARY: MW2 0x1008fef0 SYMBOL
// __trandisp1

// LIBRARY: MW2 0x1008ff57 SYMBOL
// __trandisp2

// LIBRARY: MW2 0x1008ffe3 SYMBOL
// __rttospopde

// LIBRARY: MW2 0x1008ffe8 SYMBOL
// __rttospop

// LIBRARY: MW2 0x1008ffea SYMBOL
// __rtnospop

// LIBRARY: MW2 0x1008ffec SYMBOL
// __rttosnpop

// LIBRARY: MW2 0x1008ffed SYMBOL
// __rtnospopde

// LIBRARY: MW2 0x1008fff4 SYMBOL
// __rtzeropop

// LIBRARY: MW2 0x1008fff6 SYMBOL
// __rtzeronpop

// LIBRARY: MW2 0x1008fffb SYMBOL
// __rtonepop

// LIBRARY: MW2 0x1008fffd SYMBOL
// __rtonenpop

// LIBRARY: MW2 0x10090002 SYMBOL
// __tosnan1

// LIBRARY: MW2 0x1009002d SYMBOL
// __nosnan2

// LIBRARY: MW2 0x1009002f SYMBOL
// __tosnan2

// LIBRARY: MW2 0x10090057 SYMBOL
// __nan2

// LIBRARY: MW2 0x10090096 SYMBOL
// __rtindfpop

// LIBRARY: MW2 0x10090098 SYMBOL
// __rtindfnpop

// LIBRARY: MW2 0x100900a9 SYMBOL
// __rttosnpopde

// LIBRARY: MW2 0x100900b3 SYMBOL
// __rtchsifneg

// 87except.obj

// LIBRARY: MW2 0x100900c0 SYMBOL
// __87except

// ieee87.obj

// LIBRARY: MW2 0x100901e0 SYMBOL
// __statusfp

// LIBRARY: MW2 0x10090200 SYMBOL
// __clearfp

// LIBRARY: MW2 0x10090220 SYMBOL
// __control87

// LIBRARY: MW2 0x10090260 SYMBOL
// __controlfp

// LIBRARY: MW2 0x10090280 SYMBOL
// __fpreset

// LIBRARY: MW2 0x100902c0
// _abstract_cw

// LIBRARY: MW2 0x10090370
// _hw_cw

// LIBRARY: MW2 0x10090400
// _abstract_sw

// crt0fp.obj

// LIBRARY: MW2 0x10090440 SYMBOL
// __fptrap

// intrncvt.obj

// LIBRARY: MW2 0x10090460 SYMBOL
// __ZeroTail

// LIBRARY: MW2 0x100904d0 SYMBOL
// __IncMan

// LIBRARY: MW2 0x10090540 SYMBOL
// __RoundMan

// LIBRARY: MW2 0x100905f0 SYMBOL
// __CopyMan

// LIBRARY: MW2 0x10090610 SYMBOL
// __FillZeroMan

// LIBRARY: MW2 0x10090620 SYMBOL
// __IsZeroMan

// LIBRARY: MW2 0x10090640 SYMBOL
// __ShrMan

// LIBRARY: MW2 0x100906f0 SYMBOL
// __ld12cvt

// LIBRARY: MW2 0x100908c0 SYMBOL
// __ld12tod

// LIBRARY: MW2 0x100908e0 SYMBOL
// __ld12tof

// LIBRARY: MW2 0x10090900 SYMBOL
// __ld12told

// LIBRARY: MW2 0x10090990 SYMBOL
// __atodbl

// LIBRARY: MW2 0x100909d0 SYMBOL
// __atoldbl

// LIBRARY: MW2 0x10090a10 SYMBOL
// __atoflt

// _fptostr.obj

// LIBRARY: MW2 0x10090a50 SYMBOL
// __fptostr

// cfout.obj

// LIBRARY: MW2 0x10090b30 SYMBOL
// __fltout2

// LIBRARY: MW2 0x10090bc0 SYMBOL
// ___dtold

// mbtowc.obj

// LIBRARY: MW2 0x10090c80 SYMBOL
// _mbtowc

// LIBRARY: MW2 0x10090d00 SYMBOL
// __mbtowc_lk

// _ctype.obj

// LIBRARY: MW2 0x10090eb0 SYMBOL
// _isalpha

// LIBRARY: MW2 0x10090f00 SYMBOL
// _isupper

// LIBRARY: MW2 0x10090f50 SYMBOL
// _islower

// LIBRARY: MW2 0x10090fa0 SYMBOL
// _isdigit

// LIBRARY: MW2 0x10090ff0 SYMBOL
// _isxdigit

// LIBRARY: MW2 0x10091040 SYMBOL
// _isspace

// LIBRARY: MW2 0x10091090 SYMBOL
// _ispunct

// LIBRARY: MW2 0x100910e0 SYMBOL
// _isalnum

// LIBRARY: MW2 0x10091130 SYMBOL
// _isprint

// LIBRARY: MW2 0x10091180 SYMBOL
// _isgraph

// LIBRARY: MW2 0x100911d0 SYMBOL
// _iscntrl

// LIBRARY: MW2 0x10091220 SYMBOL
// ___isascii

// LIBRARY: MW2 0x10091250 SYMBOL
// ___toascii

// LIBRARY: MW2 0x10091270 SYMBOL
// ___iscsymf

// LIBRARY: MW2 0x100912f0 SYMBOL
// ___iscsym

// ungetc.obj

// LIBRARY: MW2 0x10091370 SYMBOL
// _ungetc

// LIBRARY: MW2 0x100913e0 SYMBOL
// __ungetc_lk

// osfinfo.obj

// LIBRARY: MW2 0x10091510 SYMBOL
// __alloc_osfhnd

// LIBRARY: MW2 0x10091700 SYMBOL
// __set_osfhnd

// LIBRARY: MW2 0x10091810 SYMBOL
// __free_osfhnd

// LIBRARY: MW2 0x10091950 SYMBOL
// __get_osfhandle

// LIBRARY: MW2 0x100919f0 SYMBOL
// __open_osfhandle

// LIBRARY: MW2 0x10091b10 SYMBOL
// __lock_fhandle

// LIBRARY: MW2 0x10091bb0 SYMBOL
// __unlock_fhandle

// open.obj

// LIBRARY: MW2 0x10091bf0 SYMBOL
// __open

// LIBRARY: MW2 0x10091c40 SYMBOL
// __sopen

// winxfltr.obj

// LIBRARY: MW2 0x10092280 SYMBOL
// __XcptFilter

// LIBRARY: MW2 0x10092490
// xcptlookup

// inittime.obj

// LIBRARY: MW2 0x100924f0 SYMBOL
// ___init_time

// LIBRARY: MW2 0x100925f0
// _get_lc_time

// LIBRARY: MW2 0x10092aa0
// _free_lc_time

// LIBRARY: MW2 0x10092dc0
// storeTimeFmt

// initnum.obj

// LIBRARY: MW2 0x10092f50 SYMBOL
// ___init_numeric

// LIBRARY: MW2 0x10093210 SYMBOL
// _fix_grouping

// initmon.obj

// LIBRARY: MW2 0x100932a0 SYMBOL
// ___init_monetary

// LIBRARY: MW2 0x100933e0
// _get_lc_lconv

// LIBRARY: MW2 0x100935c0 SYMBOL
// _fix_grouping

// LIBRARY: MW2 0x10093650
// _free_lc_lconv

// initctyp.obj

// LIBRARY: MW2 0x10093700 SYMBOL
// ___init_ctype

// initcoll.obj

// LIBRARY: MW2 0x10093b00 SYMBOL
// ___init_collate

// strcmp.obj

// LIBRARY: MW2 0x10093b20 SYMBOL
// _strcmp

// strcspn.obj

// LIBRARY: MW2 0x10093bb0 SYMBOL
// _strcspn

// strpbrk.obj

// LIBRARY: MW2 0x10093bf0 SYMBOL
// _strpbrk

// getqloc.obj

// LIBRARY: MW2 0x10093c30 SYMBOL
// ___get_qualified_locale

// LIBRARY: MW2 0x10094030
// trans_lang_lang

// LIBRARY: MW2 0x100940d0
// trans_ctry_ctry

// LIBRARY: MW2 0x10094170
// trans_ctry_lang

// LIBRARY: MW2 0x10094200
// testSpecialCtry

// LIBRARY: MW2 0x10094250
// match_ctry_lang

// closeall.obj

// LIBRARY: MW2 0x10094380 SYMBOL
// __fcloseall

// powhlp.obj

// LIBRARY: MW2 0x10094470 SYMBOL
// __powhlp

// LIBRARY: MW2 0x100946c0 SYMBOL
// __d_inttype

// adj_fdiv.obj

// LIBRARY: MW2 0x10094740 SYMBOL
// _fdiv_main_routine

// LIBRARY: MW2 0x10094857 SYMBOL
// __adj_fdiv_r

// LIBRARY: MW2 0x10094cf6 SYMBOL
// __fdivp_sti_st

// LIBRARY: MW2 0x10094d09 SYMBOL
// __fdivrp_sti_st

// LIBRARY: MW2 0x10094d1c SYMBOL
// __adj_fdiv_m32

// LIBRARY: MW2 0x10094d68 SYMBOL
// __adj_fdiv_m64

// LIBRARY: MW2 0x10094db4 SYMBOL
// __adj_fdiv_m16i

// LIBRARY: MW2 0x10094de8 SYMBOL
// __adj_fdiv_m32i

// LIBRARY: MW2 0x10094e1c SYMBOL
// __adj_fdivr_m32

// LIBRARY: MW2 0x10094e68 SYMBOL
// __adj_fdivr_m64

// LIBRARY: MW2 0x10094eb4 SYMBOL
// __adj_fdivr_m16i

// LIBRARY: MW2 0x10094ee8 SYMBOL
// __adj_fdivr_m32i

// LIBRARY: MW2 0x10094f1c SYMBOL
// __safe_fdiv

// LIBRARY: MW2 0x10094f31 SYMBOL
// __safe_fdivr

// LIBRARY: MW2 0x10094f46 SYMBOL
// __fprem_common

// LIBRARY: MW2 0x1009514c SYMBOL
// __adj_fprem

// LIBRARY: MW2 0x100951fe SYMBOL
// __fprem1_common

// LIBRARY: MW2 0x10095404 SYMBOL
// __adj_fprem1

// LIBRARY: MW2 0x100954b9 SYMBOL
// __safe_fprem

// LIBRARY: MW2 0x100954bf SYMBOL
// __safe_fprem1

// LIBRARY: MW2 0x100954c5 SYMBOL
// __adj_fpatan

// LIBRARY: MW2 0x100954c8 SYMBOL
// __adj_fptan

// aw_cmp.obj

// LIBRARY: MW2 0x100954d0 SYMBOL
// ___crtCompareStringW

// LIBRARY: MW2 0x100957c0 SYMBOL
// _wcsncnt

// LIBRARY: MW2 0x10095830 SYMBOL
// ___crtCompareStringA

// LIBRARY: MW2 0x10095c90 SYMBOL
// _strncnt

// setenv.obj

// LIBRARY: MW2 0x10095d00 SYMBOL
// ___crtsetenv

// LIBRARY: MW2 0x10096070
// findenv

// LIBRARY: MW2 0x10096110
// copy_environ

// read.obj

// LIBRARY: MW2 0x10096210 SYMBOL
// __read

// LIBRARY: MW2 0x100962c0 SYMBOL
// __read_lk

// tzset.obj

// LIBRARY: MW2 0x10096780 SYMBOL
// ___tzset

// LIBRARY: MW2 0x100967d0 SYMBOL
// __tzset

// LIBRARY: MW2 0x10096800
// _tzset_lk

// LIBRARY: MW2 0x10096b90 SYMBOL
// __isindst

// LIBRARY: MW2 0x10096bd0
// _isindst_lk

// LIBRARY: MW2 0x10096f40
// cvtdate

// gmtime.obj

// LIBRARY: MW2 0x10097150 SYMBOL
// _gmtime

// wcslen.obj

// LIBRARY: MW2 0x10097380 SYMBOL
// _wcslen

// fpexcept.obj

// LIBRARY: MW2 0x100973d0 SYMBOL
// __handle_qnan1

// LIBRARY: MW2 0x10097420 SYMBOL
// __handle_qnan2

// LIBRARY: MW2 0x10097490 SYMBOL
// __except1

// LIBRARY: MW2 0x10097550 SYMBOL
// __except2

// LIBRARY: MW2 0x10097640 SYMBOL
// __raise_exc

// LIBRARY: MW2 0x100979b0 SYMBOL
// __handle_exc

// LIBRARY: MW2 0x10097c70 SYMBOL
// __umatherr

// LIBRARY: MW2 0x10097d20 SYMBOL
// __set_errno

// LIBRARY: MW2 0x10097d50
// _get_fname

// LIBRARY: MW2 0x10097d80 SYMBOL
// __errcode

// matherr.obj

// LIBRARY: MW2 0x10097dc0 SYMBOL
// __matherr

// fpctrl.obj

// LIBRARY: MW2 0x10097dd0 SYMBOL
// __statfp

// LIBRARY: MW2 0x10097df0 SYMBOL
// __clrfp

// LIBRARY: MW2 0x10097e10 SYMBOL
// __ctrlfp

// LIBRARY: MW2 0x10097e50 SYMBOL
// __set_statfp

// mantold.obj

// LIBRARY: MW2 0x10097eb0 SYMBOL
// ___addl

// LIBRARY: MW2 0x10097ee0 SYMBOL
// ___add_12

// LIBRARY: MW2 0x10097f50 SYMBOL
// ___shl_12

// LIBRARY: MW2 0x10097f90 SYMBOL
// ___shr_12

// LIBRARY: MW2 0x10097fd0 SYMBOL
// ___mtold12

// strgtold.obj

// LIBRARY: MW2 0x100980c0 SYMBOL
// ___strgtold12

// LIBRARY: MW2 0x10098810 SYMBOL
// ___STRINGTOLD

// x10fout.obj

// LIBRARY: MW2 0x10098860 SYMBOL
// _$I10_OUTPUT

// chsize.obj

// LIBRARY: MW2 0x10098c00 SYMBOL
// __chsize

// LIBRARY: MW2 0x10098ca0 SYMBOL
// __chsize_lk

// strftime.obj

// LIBRARY: MW2 0x10098f30 SYMBOL
// _strftime

// LIBRARY: MW2 0x100990a0
// _expandtime

// LIBRARY: MW2 0x100996c0
// _store_str

// LIBRARY: MW2 0x10099710
// _store_num

// LIBRARY: MW2 0x100997c0
// _store_number

// LIBRARY: MW2 0x10099860
// _store_winword

// inithelp.obj

// LIBRARY: MW2 0x10099d10 SYMBOL
// ___getlocaleinfo

// lconv.obj

// LIBRARY: MW2 0x10099fe0 SYMBOL
// _localeconv

// aw_loc.obj

// LIBRARY: MW2 0x1009a000 SYMBOL
// ___crtGetLocaleInfoW

// LIBRARY: MW2 0x1009a1c0 SYMBOL
// ___crtGetLocaleInfoA

// wcstol.obj

// LIBRARY: MW2 0x1009a390 SYMBOL
// _wcstol

// LIBRARY: MW2 0x1009a3c0
// wcstoxl

// LIBRARY: MW2 0x1009a700 SYMBOL
// _wcstoul

// stricmp.obj

// LIBRARY: MW2 0x1009a730 SYMBOL
// __strcmpi

// util.obj

// LIBRARY: MW2 0x1009a800 SYMBOL
// __set_exp

// LIBRARY: MW2 0x1009a840 SYMBOL
// __get_exp

// LIBRARY: MW2 0x1009a860 SYMBOL
// __add_exp

// LIBRARY: MW2 0x1009a890 SYMBOL
// __set_bexp

// LIBRARY: MW2 0x1009a8d0 SYMBOL
// __sptype

// LIBRARY: MW2 0x1009a940 SYMBOL
// __decomp

// frnd.obj

// LIBRARY: MW2 0x1009aa30 SYMBOL
// __frnd

// ieeemisc.obj

// LIBRARY: MW2 0x1009aa50 SYMBOL
// __copysign

// LIBRARY: MW2 0x1009aa80 SYMBOL
// __chgsign

// LIBRARY: MW2 0x1009aaa0 SYMBOL
// __scalb

// LIBRARY: MW2 0x1009aac0 SYMBOL
// __logb

// LIBRARY: MW2 0x1009abe0 SYMBOL
// __nextafter

// LIBRARY: MW2 0x1009aef0 SYMBOL
// __finite

// LIBRARY: MW2 0x1009af10 SYMBOL
// __isnan

// LIBRARY: MW2 0x1009af40 SYMBOL
// __fpclass

// mbschr.obj

// LIBRARY: MW2 0x1009aff0 SYMBOL
// __mbschr

// wcstombs.obj

// LIBRARY: MW2 0x1009b100 SYMBOL
// _wcstombs

// LIBRARY: MW2 0x1009b180 SYMBOL
// __wcstombs_lk

// LIBRARY: MW2 0x1009b500 SYMBOL
// _wcsncnt

// tenpow.obj

// LIBRARY: MW2 0x1009b570 SYMBOL
// ___ld12mul

// LIBRARY: MW2 0x1009b820 SYMBOL
// ___multtenpow12

// setmode.obj

// LIBRARY: MW2 0x1009b8a0 SYMBOL
// __setmode

// LIBRARY: MW2 0x1009b940 SYMBOL
// __setmode_lk

// towupper.obj

// LIBRARY: MW2 0x1009ba60 SYMBOL
// _towupper

// LIBRARY: MW2 0x1009bb20 SYMBOL
// __towupper_lk

// iswctype.obj

// LIBRARY: MW2 0x1009bc00 SYMBOL
// _iswctype

// LIBRARY: MW2 0x1009bcb0 SYMBOL
// _is_wctype

// ldexp.obj

// LIBRARY: MW2 0x1009bcd0 SYMBOL
// _ldexp

// mbsdup.obj

// LIBRARY: MW2 0x1009c100 SYMBOL
// __mbsdup

// getch.obj

// LIBRARY: MW2 0x1009c130 SYMBOL
// __ungetch

// putch.obj

// LIBRARY: MW2 0x1009c170 SYMBOL
// __putch_lk

// flength.obj

// LIBRARY: MW2 0x1009c1e0 SYMBOL
// __filelength

// strnset.obj

// LIBRARY: MW2 0x1009c2d0 SYMBOL
// __strnset

// strnicmp.obj

// LIBRARY: MW2 0x1009c300 SYMBOL
// __strnicmp

// initcon.obj

// LIBRARY: MW2 0x1009c400 SYMBOL
// ___initconin

// LIBRARY: MW2 0x1009c430 SYMBOL
// ___initconout

// LIBRARY: MW2 0x1009c460 SYMBOL
// ___termcon

#endif
