#ifdef 0
// C runtime library code in MECH2.EXE: VC++ 2.2's LIBC.LIB (the single-threaded CRT),
// identified object by object against the library. Small wrappers that differ from
// another object's only in their relocations (memmove/memcpy, spawnvp/strtol) were
// resolved by their callers.
//
// Public functions are matched by their public symbol (SYMBOL). LIBC carries no CodeView
// records, so static functions have no name reccmp can match; they are annotated to mark
// their addresses as library code. The jmp [IAT] import thunks (0x4022be and the end of
// .text) are left to reccmp. Aliases at one address (__chkstk/__alloca_probe,
// __strcmpi/__stricmp) take the symbol reccmp keeps for that address.

// dospawn.obj

// LIBRARY: MECH2 0x0040234e SYMBOL
// __dospawn

// crt0dat.obj

// LIBRARY: MECH2 0x004025cb SYMBOL
// __cinit

// LIBRARY: MECH2 0x00402607 SYMBOL
// _exit

// LIBRARY: MECH2 0x0040261c SYMBOL
// __exit

// LIBRARY: MECH2 0x00402631 SYMBOL
// __cexit

// LIBRARY: MECH2 0x00402643 SYMBOL
// __c_exit

// LIBRARY: MECH2 0x00402655 SYMBOL
// _doexit

// LIBRARY: MECH2 0x004026f3 SYMBOL
// __initterm

// fclose.obj

// LIBRARY: MECH2 0x0040271b SYMBOL
// _fclose

// fread.obj

// LIBRARY: MECH2 0x0040278e SYMBOL
// _fread

// fopen.obj

// LIBRARY: MECH2 0x00402912 SYMBOL
// __fsopen

// LIBRARY: MECH2 0x0040293d SYMBOL
// _fopen

// calloc.obj

// LIBRARY: MECH2 0x00402954 SYMBOL
// _calloc

// free.obj

// LIBRARY: MECH2 0x0040299c SYMBOL
// _free

// sprintf.obj

// LIBRARY: MECH2 0x00402a1e SYMBOL
// _sprintf

// memmove.obj

// LIBRARY: MECH2 0x00402a90 SYMBOL
// _memmove

// fflush.obj

// LIBRARY: MECH2 0x00402c31 SYMBOL
// _fflush

// LIBRARY: MECH2 0x00402c90 SYMBOL
// __flush

// LIBRARY: MECH2 0x00402d0a SYMBOL
// __flushall

// LIBRARY: MECH2 0x00402d17 SYMBOL
// _flsall

// LIBRARY: MECH2 0x00402d9b SYMBOL
// ___endstdio

// fputs.obj

// LIBRARY: MECH2 0x00402db4 SYMBOL
// _fputs

// vsnprint.obj

// LIBRARY: MECH2 0x00402e1a SYMBOL
// __vsnprintf

// strncpy.obj

// LIBRARY: MECH2 0x00402e88 SYMBOL
// _strncpy

// wincrt0.obj

// LIBRARY: MECH2 0x00402eb5 SYMBOL
// _WinMainCRTStartup

// LIBRARY: MECH2 0x00403074 SYMBOL
// __amsg_exit

// dosmap.obj

// LIBRARY: MECH2 0x004030b2 SYMBOL
// __dosmaperr

// ioinit.obj

// LIBRARY: MECH2 0x00403135 SYMBOL
// __ioinit

// close.obj

// LIBRARY: MECH2 0x0040325e SYMBOL
// __close

// _freebuf.obj

// LIBRARY: MECH2 0x00403319 SYMBOL
// __freebuf

// _filbuf.obj

// LIBRARY: MECH2 0x0040335f SYMBOL
// __filbuf

// read.obj

// LIBRARY: MECH2 0x00403452 SYMBOL
// __read

// _open.obj

// LIBRARY: MECH2 0x004036ca SYMBOL
// __openfile

// stream.obj

// LIBRARY: MECH2 0x004038cb SYMBOL
// __getstream

// malloc.obj

// LIBRARY: MECH2 0x00403917 SYMBOL
// _malloc

// LIBRARY: MECH2 0x0040392e SYMBOL
// __nh_malloc

// LIBRARY: MECH2 0x00403a1c SYMBOL
// __heap_split_block

// heapinit.obj

// LIBRARY: MECH2 0x00403a72 SYMBOL
// __heap_init

// LIBRARY: MECH2 0x00403aa2 SYMBOL
// __heap_term

// LIBRARY: MECH2 0x00403b20 SYMBOL
// __heap_grow_emptylist

// LIBRARY: MECH2 0x00403b8b SYMBOL
// ___getempty

// hpabort.obj

// LIBRARY: MECH2 0x00403bb9 SYMBOL
// __heap_abort

// _flsbuf.obj

// LIBRARY: MECH2 0x00403bc6 SYMBOL
// __flsbuf

// output.obj

// LIBRARY: MECH2 0x00403d15 SYMBOL
// __output

// LIBRARY: MECH2 0x00404772 SYMBOL
// _write_char

// LIBRARY: MECH2 0x004047c2 SYMBOL
// _write_multi_char

// LIBRARY: MECH2 0x004047ff SYMBOL
// _write_string

// cprintf.obj

// LIBRARY: MECH2 0x00404843 SYMBOL
// _get_int_arg

// LIBRARY: MECH2 0x00404855 SYMBOL
// _get_int64_arg

// LIBRARY: MECH2 0x0040486e SYMBOL
// _get_short_arg

// commit.obj

// LIBRARY: MECH2 0x00404882 SYMBOL
// __commit

// write.obj

// LIBRARY: MECH2 0x004048e2 SYMBOL
// __write

// closeall.obj

// LIBRARY: MECH2 0x00404ae1 SYMBOL
// __fcloseall

// _sftbuf.obj

// LIBRARY: MECH2 0x00404b1e SYMBOL
// __stbuf

// LIBRARY: MECH2 0x00404bbf SYMBOL
// __ftbuf

// fwrite.obj

// LIBRARY: MECH2 0x00404c16 SYMBOL
// _fwrite

// exsup.obj

// LIBRARY: MECH2 0x00404dc0 SYMBOL
// __global_unwind2

// LIBRARY: MECH2 0x00404de0 SYMBOL
// __unwind_handler

// LIBRARY: MECH2 0x00404e02 SYMBOL
// __local_unwind2

// LIBRARY: MECH2 0x00404e5c SYMBOL
// __abnormal_termination

// winxfltr.obj

// LIBRARY: MECH2 0x00404eae SYMBOL
// __XcptFilter

// LIBRARY: MECH2 0x00405063 SYMBOL
// _xcptlookup

// ismbbyte.obj

// LIBRARY: MECH2 0x0040509b SYMBOL
// __ismbbkalnum

// LIBRARY: MECH2 0x004050ac SYMBOL
// __ismbbkprint

// LIBRARY: MECH2 0x004050bd SYMBOL
// __ismbbkpunct

// LIBRARY: MECH2 0x004050ce SYMBOL
// __ismbbalnum

// LIBRARY: MECH2 0x004050e3 SYMBOL
// __ismbbalpha

// LIBRARY: MECH2 0x004050f8 SYMBOL
// __ismbbgraph

// LIBRARY: MECH2 0x0040510d SYMBOL
// __ismbbprint

// LIBRARY: MECH2 0x00405122 SYMBOL
// __ismbbpunct

// LIBRARY: MECH2 0x00405137 SYMBOL
// __ismbblead

// LIBRARY: MECH2 0x00405148 SYMBOL
// __ismbbtrail

// LIBRARY: MECH2 0x00405159 SYMBOL
// __ismbbkana

// LIBRARY: MECH2 0x00405187 SYMBOL
// @x_ismbbtype@12

// stdenvp.obj

// LIBRARY: MECH2 0x004051c9 SYMBOL
// __setenvp

// stdargv.obj

// LIBRARY: MECH2 0x004052c6 SYMBOL
// __setargv

// LIBRARY: MECH2 0x00405381 SYMBOL
// _parse_cmdline

// mbctype.obj

// LIBRARY: MECH2 0x004055bd SYMBOL
// _getSystemCP

// LIBRARY: MECH2 0x00405606 SYMBOL
// _CPtoLCID

// LIBRARY: MECH2 0x00405654 SYMBOL
// _setSBCS

// LIBRARY: MECH2 0x00405681 SYMBOL
// __setmbcp

// LIBRARY: MECH2 0x00405872 SYMBOL
// __getmbcp

// LIBRARY: MECH2 0x00405878 SYMBOL
// ___initmbctable

// exsup3.obj

// LIBRARY: MECH2 0x00405890 SYMBOL
// __except_handler3

// LIBRARY: MECH2 0x0040593e SYMBOL
// __seh_longjmp_unwind@4

// crt0msg.obj

// LIBRARY: MECH2 0x0040598d SYMBOL
// __FF_MSGBANNER

// LIBRARY: MECH2 0x004059bc SYMBOL
// __NMSG_WRITE

// LIBRARY: MECH2 0x00405a2a SYMBOL
// __GET_RTERRMSG

// osfinfo.obj

// LIBRARY: MECH2 0x00405a6c SYMBOL
// __alloc_osfhnd

// LIBRARY: MECH2 0x00405aaf SYMBOL
// __set_osfhnd

// LIBRARY: MECH2 0x00405b33 SYMBOL
// __free_osfhnd

// LIBRARY: MECH2 0x00405bbb SYMBOL
// __get_osfhandle

// LIBRARY: MECH2 0x00405c01 SYMBOL
// __open_osfhandle

// _getbuf.obj

// LIBRARY: MECH2 0x00405cb2 SYMBOL
// __getbuf

// lseek.obj

// LIBRARY: MECH2 0x00405d0f SYMBOL
// __lseek

// open.obj

// LIBRARY: MECH2 0x00405da1 SYMBOL
// __open

// LIBRARY: MECH2 0x00405dbd SYMBOL
// __sopen

// heapsrch.obj

// LIBRARY: MECH2 0x004061d5 SYMBOL
// __heap_search

// heapgrow.obj

// LIBRARY: MECH2 0x00406301 SYMBOL
// __heap_grow

// LIBRARY: MECH2 0x0040637f SYMBOL
// __heap_new_region

// LIBRARY: MECH2 0x00406424 SYMBOL
// __heap_grow_region

// LIBRARY: MECH2 0x004064fc SYMBOL
// __heap_free_region

// isatty.obj

// LIBRARY: MECH2 0x00406544 SYMBOL
// __isatty

// wctomb.obj

// LIBRARY: MECH2 0x00406564 SYMBOL
// _wctomb

// ulldiv.obj

// LIBRARY: MECH2 0x004065f8 SYMBOL
// __aulldiv

// ullrem.obj

// LIBRARY: MECH2 0x00406670 SYMBOL
// __aullrem

// chsize.obj

// LIBRARY: MECH2 0x004066f4 SYMBOL
// __chsize

// heapadd.obj

// LIBRARY: MECH2 0x00406875 SYMBOL
// __heapadd

// LIBRARY: MECH2 0x004068a5 SYMBOL
// __heap_addblock

// LIBRARY: MECH2 0x00406b86 SYMBOL
// __before

// crt0fp.obj

// LIBRARY: MECH2 0x00406c09 SYMBOL
// __fptrap

// setmode.obj

// LIBRARY: MECH2 0x00406c16 SYMBOL
// __setmode

// chkstk.obj

// LIBRARY: MECH2 0x00406cb0 SYMBOL
// __chkstk

// findaddr.obj

// LIBRARY: MECH2 0x00406ce8 SYMBOL
// __heap_findaddr

// spawnlp.obj

// LIBRARY: MECH2 0x00406e14 SYMBOL
// __spawnlp

// stricmp.obj

// LIBRARY: MECH2 0x00406e30 SYMBOL
// __strcmpi

// strnicmp.obj

// LIBRARY: MECH2 0x00406ee0 SYMBOL
// __strnicmp

// spawnvp.obj

// LIBRARY: MECH2 0x00406fb8 SYMBOL
// __spawnvp

// tolower.obj

// LIBRARY: MECH2 0x00406fd4 SYMBOL
// __tolower

// LIBRARY: MECH2 0x00406fde SYMBOL
// _tolower

// spawnvpe.obj

// LIBRARY: MECH2 0x004070dd SYMBOL
// __spawnvpe

// aw_map.obj

// LIBRARY: MECH2 0x00407324 SYMBOL
// _strncnt

// LIBRARY: MECH2 0x0040735b SYMBOL
// _wcsncnt

// LIBRARY: MECH2 0x00407399 SYMBOL
// ___crtLCMapStringW

// LIBRARY: MECH2 0x0040764e SYMBOL
// ___crtLCMapStringA

// isctype.obj

// LIBRARY: MECH2 0x00407944 SYMBOL
// __isctype

// mbschr.obj

// LIBRARY: MECH2 0x004079dd SYMBOL
// __mbschr

// spawnve.obj

// LIBRARY: MECH2 0x00407a7e SYMBOL
// _comexecmd

// LIBRARY: MECH2 0x00407af1 SYMBOL
// __spawnve

// mbsrchr.obj

// LIBRARY: MECH2 0x00407d7c SYMBOL
// __mbsrchr

// getpath.obj

// LIBRARY: MECH2 0x00407dfe SYMBOL
// __getpath

// getenv.obj

// LIBRARY: MECH2 0x00407ea0 SYMBOL
// _getenv

// setmbval.obj

// LIBRARY: MECH2 0x00407f4f SYMBOL
// ___set_invalid_mb_chars

// aw_str.obj

// LIBRARY: MECH2 0x00407f88 SYMBOL
// ___crtGetStringTypeW

// LIBRARY: MECH2 0x004081d6 SYMBOL
// ___crtGetStringTypeA

// strchr.obj

// LIBRARY: MECH2 0x00408374 SYMBOL
// _strchr

// cenvarg.obj

// LIBRARY: MECH2 0x004083a2 SYMBOL
// __cenvarg

// access.obj

// LIBRARY: MECH2 0x004087b7 SYMBOL
// __access

// strrchr.obj

// LIBRARY: MECH2 0x00408818 SYMBOL
// _strrchr

// mbsnbico.obj

// LIBRARY: MECH2 0x00408848 SYMBOL
// __mbsnbicoll

// wtombenv.obj

// LIBRARY: MECH2 0x00408a89 SYMBOL
// ___wtomb_environ

// setfcntr.obj

// LIBRARY: MECH2 0x00408b27 SYMBOL
// ___set_fcntrlcomp

// aw_cmp.obj

// LIBRARY: MECH2 0x00408b6f SYMBOL
// _strncnt

// LIBRARY: MECH2 0x00408ba6 SYMBOL
// _wcsncnt

// LIBRARY: MECH2 0x00408be4 SYMBOL
// ___crtCompareStringW

// LIBRARY: MECH2 0x00408eb2 SYMBOL
// ___crtCompareStringA

// setenv.obj

// LIBRARY: MECH2 0x00409236 SYMBOL
// ___crtsetenv

// LIBRARY: MECH2 0x004094b2 SYMBOL
// _findenv

// LIBRARY: MECH2 0x00409526 SYMBOL
// _copy_environ

// realloc.obj

// LIBRARY: MECH2 0x004095b2 SYMBOL
// _realloc

// LIBRARY: MECH2 0x004096e5 SYMBOL
// __expand

// LIBRARY: MECH2 0x0040980c SYMBOL
// __heap_expand_block

// mbsdup.obj

// LIBRARY: MECH2 0x00409880 SYMBOL
// __mbsdup

// strlen.obj

// LIBRARY: MECH2 0x004098b0 SYMBOL
// _strlen

// mbscat.obj

// LIBRARY: MECH2 0x00409910 SYMBOL
// __mbscat

// LIBRARY: MECH2 0x00409914 SYMBOL
// __mbscpy

// Data

// GLOBAL: MECH2 0x0040d968
// _p_overlay

#endif
