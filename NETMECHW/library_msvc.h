#ifdef 0
// C runtime library code in NETMECHW.DLL: VC++ 2.2's LIBCMT.LIB (the multithreaded CRT),
// identified object by object against the library; the objects cover every byte from
// 0x10013b7c to the end of .text.
//
// Public functions are matched by their public symbol (SYMBOL). LIBCMT carries no CodeView
// records, so static functions have no name reccmp can match; they are left unannotated.
// The jmp [IAT] thunks of DPLAY.DLL's two ordinal imports (0x10013b70 and 0x10013b76, from
// the import library, between the game code and the CRT) are left to reccmp. Aliases at one
// address (__chkstk/__alloca_probe, __strcmpi/__stricmp) take the symbol reccmp keeps for
// that address.

// strrev.obj

// LIBRARY: NETMECHW 0x10013b7c SYMBOL
// __strrev

// isctype.obj

// LIBRARY: NETMECHW 0x10013bac SYMBOL
// __isctype

// delete.obj

// LIBRARY: NETMECHW 0x10013c27 SYMBOL
// ??3@YAXPAX@Z

// new.obj

// LIBRARY: NETMECHW 0x10013c34 SYMBOL
// ??2@YAPAXI@Z

// sprintf.obj

// LIBRARY: NETMECHW 0x10013c43 SYMBOL
// _sprintf

// unlink.obj

// LIBRARY: NETMECHW 0x10013c9c SYMBOL
// _remove

// strncpy.obj

// LIBRARY: NETMECHW 0x10013ccc SYMBOL
// _strncpy

// free.obj

// LIBRARY: NETMECHW 0x10013cf0 SYMBOL
// _free

// LIBRARY: NETMECHW 0x10013d11 SYMBOL
// __free_lk

// strncmp.obj

// LIBRARY: NETMECHW 0x10013d7c SYMBOL
// _strncmp

// calloc.obj

// LIBRARY: NETMECHW 0x10013db4 SYMBOL
// _calloc

// onexit.obj

// LIBRARY: NETMECHW 0x10013e04 SYMBOL
// __onexit

// LIBRARY: NETMECHW 0x10013e8c SYMBOL
// _atexit

// LIBRARY: NETMECHW 0x10013e9e SYMBOL
// ___onexitinit

// strtok.obj

// LIBRARY: NETMECHW 0x10013ed4 SYMBOL
// _strtok

// close.obj

// LIBRARY: NETMECHW 0x10013fa2 SYMBOL
// __close

// LIBRARY: NETMECHW 0x10013ff2 SYMBOL
// __close_lk

// adj_fdiv.obj

// LIBRARY: NETMECHW 0x10014068 SYMBOL
// _fdiv_main_routine

// LIBRARY: NETMECHW 0x1001417f SYMBOL
// __adj_fdiv_r

// LIBRARY: NETMECHW 0x1001461e SYMBOL
// _fdivp_sti_st

// LIBRARY: NETMECHW 0x10014631 SYMBOL
// _fdivrp_sti_st

// LIBRARY: NETMECHW 0x10014644 SYMBOL
// __adj_fdiv_m32

// LIBRARY: NETMECHW 0x10014690 SYMBOL
// __adj_fdiv_m64

// LIBRARY: NETMECHW 0x100146dc SYMBOL
// __adj_fdiv_m16i

// LIBRARY: NETMECHW 0x10014710 SYMBOL
// __adj_fdiv_m32i

// LIBRARY: NETMECHW 0x10014744 SYMBOL
// __adj_fdivr_m32

// LIBRARY: NETMECHW 0x10014790 SYMBOL
// __adj_fdivr_m64

// LIBRARY: NETMECHW 0x100147dc SYMBOL
// __adj_fdivr_m16i

// LIBRARY: NETMECHW 0x10014810 SYMBOL
// __adj_fdivr_m32i

// LIBRARY: NETMECHW 0x10014844 SYMBOL
// __safe_fdiv

// LIBRARY: NETMECHW 0x10014859 SYMBOL
// __safe_fdivr

// LIBRARY: NETMECHW 0x1001486e SYMBOL
// __fprem_common

// LIBRARY: NETMECHW 0x10014a74 SYMBOL
// __adj_fprem

// LIBRARY: NETMECHW 0x10014b26 SYMBOL
// __fprem1_common

// LIBRARY: NETMECHW 0x10014d2c SYMBOL
// __adj_fprem1

// LIBRARY: NETMECHW 0x10014de1 SYMBOL
// __safe_fprem

// LIBRARY: NETMECHW 0x10014de7 SYMBOL
// __safe_fprem1

// LIBRARY: NETMECHW 0x10014ded SYMBOL
// __adj_fpatan

// LIBRARY: NETMECHW 0x10014df0 SYMBOL
// __adj_fptan

// fpinit.obj

// LIBRARY: NETMECHW 0x10014df3 SYMBOL
// __fpmath

// LIBRARY: NETMECHW 0x10014e07 SYMBOL
// __fpclear

// LIBRARY: NETMECHW 0x10014e08 SYMBOL
// __cfltcvt_init

// ftol.obj

// LIBRARY: NETMECHW 0x10014e40 SYMBOL
// __ftol

// strchr.obj

// LIBRARY: NETMECHW 0x10014e68 SYMBOL
// _strchr

// rename.obj

// LIBRARY: NETMECHW 0x10014e8d SYMBOL
// _rename

// mbsdup.obj

// LIBRARY: NETMECHW 0x10014ec4 SYMBOL
// __mbsdup

// sscanf.obj

// LIBRARY: NETMECHW 0x10014eea SYMBOL
// _sscanf

// fclose.obj

// LIBRARY: NETMECHW 0x10014f28 SYMBOL
// _fclose

// LIBRARY: NETMECHW 0x10014f74 SYMBOL
// __fclose_lk

// fwrite.obj

// LIBRARY: NETMECHW 0x10014fcb SYMBOL
// _fwrite

// LIBRARY: NETMECHW 0x1001500d SYMBOL
// __fwrite_lk

// fopen.obj

// LIBRARY: NETMECHW 0x1001516d SYMBOL
// __fsopen

// LIBRARY: NETMECHW 0x100151b0 SYMBOL
// _fopen

// fread.obj

// LIBRARY: NETMECHW 0x100151c3 SYMBOL
// _fread

// LIBRARY: NETMECHW 0x10015205 SYMBOL
// __fread_lk

// snprintf.obj

// LIBRARY: NETMECHW 0x10015332 SYMBOL
// __snprintf

// fprintf.obj

// LIBRARY: NETMECHW 0x1001538a SYMBOL
// _fprintf

// msize.obj

// LIBRARY: NETMECHW 0x100153e2 SYMBOL
// __msize

// LIBRARY: NETMECHW 0x10015409 SYMBOL
// __msize_lk

// malloc.obj

// LIBRARY: NETMECHW 0x1001541a SYMBOL
// _malloc

// LIBRARY: NETMECHW 0x1001542d SYMBOL
// __nh_malloc

// LIBRARY: NETMECHW 0x10015478 SYMBOL
// __malloc_lk

// LIBRARY: NETMECHW 0x10015510 SYMBOL
// __heap_split_block

// memmove.obj (byte for byte memcpy.obj but for its relocations: the callers, the mono display's
// scroll and cvt.obj's _shift, call memmove)

// LIBRARY: NETMECHW 0x10015558 SYMBOL
// _memmove

// fflush.obj

// LIBRARY: NETMECHW 0x100156a6 SYMBOL
// _fflush

// LIBRARY: NETMECHW 0x100156ec SYMBOL
// __fflush_lk

// LIBRARY: NETMECHW 0x10015728 SYMBOL
// __flush

// LIBRARY: NETMECHW 0x1001578a SYMBOL
// __flushall

// LIBRARY: NETMECHW 0x10015795 SYMBOL
// _flsall

// LIBRARY: NETMECHW 0x10015846 SYMBOL
// ___endstdio

// fputs.obj

// LIBRARY: NETMECHW 0x1001585a SYMBOL
// _fputs

// vsnprint.obj

// LIBRARY: NETMECHW 0x100158d7 SYMBOL
// __vsnprintf

// dllcrt0.obj

// LIBRARY: NETMECHW 0x1001592e SYMBOL
// __CRT_INIT@12

// LIBRARY: NETMECHW 0x10015a06 SYMBOL
// __DllMainCRTStartup@12

// crt0.obj

// LIBRARY: NETMECHW 0x10015ad3 SYMBOL
// __amsg_exit

// aw_str.obj

// LIBRARY: NETMECHW 0x10015af3 SYMBOL
// ___crtGetStringTypeA

// _flsbuf.obj

// LIBRARY: NETMECHW 0x10015c3e SYMBOL
// __flsbuf

// output.obj

// LIBRARY: NETMECHW 0x10015d4a SYMBOL
// __output

// LIBRARY: NETMECHW 0x10016595 SYMBOL
// _write_char

// LIBRARY: NETMECHW 0x100165d5 SYMBOL
// _write_multi_char

// LIBRARY: NETMECHW 0x10016606 SYMBOL
// _write_string

// cprintf.obj

// LIBRARY: NETMECHW 0x1001663d SYMBOL
// _get_int_arg

// LIBRARY: NETMECHW 0x1001664c SYMBOL
// _get_int64_arg

// LIBRARY: NETMECHW 0x10016660 SYMBOL
// _get_short_arg

// dosmap.obj

// LIBRARY: NETMECHW 0x10016670 SYMBOL
// __dosmaperr

// LIBRARY: NETMECHW 0x100166e3 SYMBOL
// __errno

// LIBRARY: NETMECHW 0x100166ec SYMBOL
// ___doserrno

// mlock.obj

// LIBRARY: NETMECHW 0x100166f5 SYMBOL
// __mtinitlocks

// LIBRARY: NETMECHW 0x10016716 SYMBOL
// __mtdeletelocks

// LIBRARY: NETMECHW 0x1001676f SYMBOL
// __lock

// LIBRARY: NETMECHW 0x100167d7 SYMBOL
// __unlock

// heapinit.obj

// LIBRARY: NETMECHW 0x100167ed SYMBOL
// __heap_init

// LIBRARY: NETMECHW 0x10016814 SYMBOL
// __heap_term

// LIBRARY: NETMECHW 0x10016879 SYMBOL
// __heap_grow_emptylist

// LIBRARY: NETMECHW 0x100168cf SYMBOL
// ___getempty

// hpabort.obj

// LIBRARY: NETMECHW 0x100168f4 SYMBOL
// __heap_abort

// crt0dat.obj

// LIBRARY: NETMECHW 0x100168ff SYMBOL
// __cinit

// LIBRARY: NETMECHW 0x1001692f SYMBOL
// __exit

// LIBRARY: NETMECHW 0x10016940 SYMBOL
// __cexit

// LIBRARY: NETMECHW 0x1001694f SYMBOL
// _doexit

// LIBRARY: NETMECHW 0x100169da SYMBOL
// __lockexit

// LIBRARY: NETMECHW 0x100169e5 SYMBOL
// __unlockexit

// LIBRARY: NETMECHW 0x100169f0 SYMBOL
// __initterm

// realloc.obj

// LIBRARY: NETMECHW 0x10016a10 SYMBOL
// _realloc

// LIBRARY: NETMECHW 0x10016b1a SYMBOL
// __heap_expand_block

// tidtable.obj

// LIBRARY: NETMECHW 0x10016b76 SYMBOL
// __mtinit

// LIBRARY: NETMECHW 0x10016bd7 SYMBOL
// __mtterm

// LIBRARY: NETMECHW 0x10016bf8 SYMBOL
// __getptd

// LIBRARY: NETMECHW 0x10016c5b SYMBOL
// __freeptd

// ioinit.obj

// LIBRARY: NETMECHW 0x10016cfc SYMBOL
// __ioinit

// osfinfo.obj

// LIBRARY: NETMECHW 0x10016dee SYMBOL
// __alloc_osfhnd

// LIBRARY: NETMECHW 0x10016e5f SYMBOL
// __set_osfhnd

// LIBRARY: NETMECHW 0x10016ecb SYMBOL
// __free_osfhnd

// LIBRARY: NETMECHW 0x10016f3a SYMBOL
// __get_osfhandle

// fp8.obj

// LIBRARY: NETMECHW 0x10016f74 SYMBOL
// __setdefaultprecision

// testfdiv.obj

// LIBRARY: NETMECHW 0x10016f87 SYMBOL
// _ms_p5_test_fdiv

// LIBRARY: NETMECHW 0x10016fcd SYMBOL
// _ms_p5_mp_test_fdiv

// cvt.obj

// LIBRARY: NETMECHW 0x100170a0 SYMBOL
// __forcdecpt

// LIBRARY: NETMECHW 0x10017105 SYMBOL
// __cropzeros

// LIBRARY: NETMECHW 0x10017164 SYMBOL
// __positive

// LIBRARY: NETMECHW 0x10017179 SYMBOL
// __fassign

// LIBRARY: NETMECHW 0x100171bc SYMBOL
// __cftoe2

// LIBRARY: NETMECHW 0x10017299 SYMBOL
// __cftoe

// LIBRARY: NETMECHW 0x10017307 SYMBOL
// __cftof2

// LIBRARY: NETMECHW 0x100173c5 SYMBOL
// __cftof

// LIBRARY: NETMECHW 0x10017425 SYMBOL
// __cftog

// LIBRARY: NETMECHW 0x100174c7 SYMBOL
// __cfltcvt

// LIBRARY: NETMECHW 0x10017518 SYMBOL
// __shift

// strlen.obj

// LIBRARY: NETMECHW 0x10017544 SYMBOL
// _strlen

// mbscat.obj

// LIBRARY: NETMECHW 0x10017594 SYMBOL
// __mbscat

// LIBRARY: NETMECHW 0x10017598 SYMBOL
// __mbscpy

// input.obj

// LIBRARY: NETMECHW 0x10017625 SYMBOL
// __input

// cscanf.obj

// LIBRARY: NETMECHW 0x100180ab SYMBOL
// __hextodec

// input.obj

// LIBRARY: NETMECHW 0x100180e4 SYMBOL
// __inc

// LIBRARY: NETMECHW 0x10018107 SYMBOL
// __un_inc

// LIBRARY: NETMECHW 0x1001811e SYMBOL
// __whiteout

// _freebuf.obj

// LIBRARY: NETMECHW 0x10018149 SYMBOL
// __freebuf

// write.obj

// LIBRARY: NETMECHW 0x10018181 SYMBOL
// __write

// LIBRARY: NETMECHW 0x100181d9 SYMBOL
// __write_lk

// _open.obj

// LIBRARY: NETMECHW 0x1001834d SYMBOL
// __openfile

// stream.obj

// LIBRARY: NETMECHW 0x100184e8 SYMBOL
// __getstream

// _filbuf.obj

// LIBRARY: NETMECHW 0x10018563 SYMBOL
// __filbuf

// read.obj

// LIBRARY: NETMECHW 0x10018626 SYMBOL
// __read

// LIBRARY: NETMECHW 0x1001867e SYMBOL
// __read_lk

// _sftbuf.obj

// LIBRARY: NETMECHW 0x1001885b SYMBOL
// __stbuf

// LIBRARY: NETMECHW 0x100188dc SYMBOL
// __ftbuf

// heapsrch.obj

// LIBRARY: NETMECHW 0x10018911 SYMBOL
// __heap_search

// heapgrow.obj

// LIBRARY: NETMECHW 0x10018a01 SYMBOL
// __heap_grow

// LIBRARY: NETMECHW 0x10018a67 SYMBOL
// __heap_new_region

// LIBRARY: NETMECHW 0x10018aeb SYMBOL
// __heap_grow_region

// LIBRARY: NETMECHW 0x10018b98 SYMBOL
// __heap_free_region

// commit.obj

// LIBRARY: NETMECHW 0x10018bd2 SYMBOL
// __commit

// closeall.obj

// LIBRARY: NETMECHW 0x10018c51 SYMBOL
// __fcloseall

// stdenvp.obj

// LIBRARY: NETMECHW 0x10018c96 SYMBOL
// __setenvp

// stdargv.obj

// LIBRARY: NETMECHW 0x10018d61 SYMBOL
// __setargv

// LIBRARY: NETMECHW 0x10018df7 SYMBOL
// _parse_cmdline

// mbctype.obj

// LIBRARY: NETMECHW 0x10018fc1 SYMBOL
// _getSystemCP

// LIBRARY: NETMECHW 0x10018ffc SYMBOL
// _CPtoLCID

// LIBRARY: NETMECHW 0x1001903b SYMBOL
// _setSBCS

// LIBRARY: NETMECHW 0x1001905f SYMBOL
// __setmbcp

// LIBRARY: NETMECHW 0x10019244 SYMBOL
// ___initmbctable

// crt0msg.obj

// LIBRARY: NETMECHW 0x1001924f SYMBOL
// __FF_MSGBANNER

// LIBRARY: NETMECHW 0x10019275 SYMBOL
// __NMSG_WRITE

// setmbval.obj

// LIBRARY: NETMECHW 0x100192d1 SYMBOL
// ___set_invalid_mb_chars

// lseek.obj

// LIBRARY: NETMECHW 0x100192ff SYMBOL
// __lseek

// LIBRARY: NETMECHW 0x10019357 SYMBOL
// __lseek_lk

// _getbuf.obj

// LIBRARY: NETMECHW 0x100193a9 SYMBOL
// __getbuf

// isatty.obj

// LIBRARY: NETMECHW 0x100193f4 SYMBOL
// __isatty

// wctomb.obj

// LIBRARY: NETMECHW 0x1001940e SYMBOL
// _wctomb

// LIBRARY: NETMECHW 0x1001943b SYMBOL
// __wctomb_lk

// ulldiv.obj

// LIBRARY: NETMECHW 0x100194b4 SYMBOL
// __aulldiv

// ullrem.obj

// LIBRARY: NETMECHW 0x10019514 SYMBOL
// __aullrem

// ieee87.obj

// LIBRARY: NETMECHW 0x1001957e SYMBOL
// __control87

// LIBRARY: NETMECHW 0x100195b9 SYMBOL
// __controlfp

// LIBRARY: NETMECHW 0x100195d0 SYMBOL
// __abstract_cw

// LIBRARY: NETMECHW 0x10019663 SYMBOL
// __hw_cw

// crt0fp.obj

// LIBRARY: NETMECHW 0x100196f0 SYMBOL
// __fptrap

// tolower.obj

// LIBRARY: NETMECHW 0x100196fb SYMBOL
// _tolower

// LIBRARY: NETMECHW 0x1001973e SYMBOL
// __tolower_lk

// intrncvt.obj

// LIBRARY: NETMECHW 0x1001980a SYMBOL
// __ZeroTail

// LIBRARY: NETMECHW 0x1001986a SYMBOL
// __IncMan

// LIBRARY: NETMECHW 0x100198cb SYMBOL
// __RoundMan

// LIBRARY: NETMECHW 0x10019968 SYMBOL
// __CopyMan

// LIBRARY: NETMECHW 0x10019985 SYMBOL
// __FillZeroMan

// LIBRARY: NETMECHW 0x10019991 SYMBOL
// __IsZeroMan

// LIBRARY: NETMECHW 0x100199af SYMBOL
// __ShrMan

// LIBRARY: NETMECHW 0x10019a5a SYMBOL
// __ld12cvt

// LIBRARY: NETMECHW 0x10019bf6 SYMBOL
// __ld12tod

// LIBRARY: NETMECHW 0x10019c0c SYMBOL
// __ld12tof

// LIBRARY: NETMECHW 0x10019c22 SYMBOL
// __atodbl

// LIBRARY: NETMECHW 0x10019c53 SYMBOL
// __atoflt

// _fptostr.obj

// LIBRARY: NETMECHW 0x10019c84 SYMBOL
// __fptostr

// cfout.obj

// LIBRARY: NETMECHW 0x10019d02 SYMBOL
// __fltout2

// LIBRARY: NETMECHW 0x10019d79 SYMBOL
// ___dtold

// llmul.obj

// LIBRARY: NETMECHW 0x10019e34 SYMBOL
// __allmul

// llshl.obj

// LIBRARY: NETMECHW 0x10019e68 SYMBOL
// __allshl

// mbstowcs.obj

// LIBRARY: NETMECHW 0x10019e75 SYMBOL
// _mbstowcs

// mbtowc.obj

// LIBRARY: NETMECHW 0x10019ea4 SYMBOL
// __mbtowc_lk

// _ctype.obj

// LIBRARY: NETMECHW 0x10019fa4 SYMBOL
// _isspace

// ungetc.obj

// LIBRARY: NETMECHW 0x10019fcf SYMBOL
// __ungetc_lk

// open.obj

// LIBRARY: NETMECHW 0x1001a046 SYMBOL
// __open

// LIBRARY: NETMECHW 0x1001a05d SYMBOL
// __sopen

// heapadd.obj

// LIBRARY: NETMECHW 0x1001a3fc SYMBOL
// __heap_addblock

// LIBRARY: NETMECHW 0x1001a64a SYMBOL
// __before

// aw_cmp.obj

// LIBRARY: NETMECHW 0x1001a6b3 SYMBOL
// _strncnt

// aw_map.obj

// LIBRARY: NETMECHW 0x1001a6df SYMBOL
// ___crtLCMapStringA

// mantold.obj

// LIBRARY: NETMECHW 0x1001a944 SYMBOL
// ___addl

// LIBRARY: NETMECHW 0x1001a967 SYMBOL
// ___add_12

// LIBRARY: NETMECHW 0x1001a9c5 SYMBOL
// ___shl_12

// LIBRARY: NETMECHW 0x1001aa03 SYMBOL
// ___shr_12

// LIBRARY: NETMECHW 0x1001aa36 SYMBOL
// ___mtold12

// strgtold.obj

// LIBRARY: NETMECHW 0x1001ab1a SYMBOL
// ___strgtold12

// x10fout.obj

// LIBRARY: NETMECHW 0x1001b156 SYMBOL
// _$I10_OUTPUT

// chsize.obj

// LIBRARY: NETMECHW 0x1001b48c SYMBOL
// __chsize_lk

// findaddr.obj

// LIBRARY: NETMECHW 0x1001b5b5 SYMBOL
// __heap_findaddr

// tenpow.obj

// LIBRARY: NETMECHW 0x1001b61f SYMBOL
// ___ld12mul

// LIBRARY: NETMECHW 0x1001b85f SYMBOL
// ___multtenpow12

// setmode.obj

// LIBRARY: NETMECHW 0x1001b8d4 SYMBOL
// __setmode_lk

// chkstk.obj

// LIBRARY: NETMECHW 0x1001b940 SYMBOL
// __chkstk

// strupr.obj

// LIBRARY: NETMECHW 0x1001b96d SYMBOL
// __strupr

// strnicmp.obj

// LIBRARY: NETMECHW 0x1001ba48 SYMBOL
// __strnicmp

// strnset.obj

// LIBRARY: NETMECHW 0x1001bb10 SYMBOL
// __strnset

// stricmp.obj

// LIBRARY: NETMECHW 0x1001bb3c SYMBOL
// __strcmpi

// flength.obj

// LIBRARY: NETMECHW 0x1001bbde SYMBOL
// __filelength

// CRT data the game code reads, through the ctype macros (isspace).

// GLOBAL: NETMECHW 0x10025390
// _pctype

// GLOBAL: NETMECHW 0x10025398
// __ctype

// GLOBAL: NETMECHW 0x1002559c
// __mb_cur_max

#endif
