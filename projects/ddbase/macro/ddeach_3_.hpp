#ifndef ddbase_macro_ddeach_3__hpp_
#define ddbase_macro_ddeach_3__hpp_

#define ___DDEACH_CAT3(a, b) a ## b
#define __DDEACH_CAT3(a, b) a ## b
#define _DDEACH_CAT3(a, b) __DDEACH_CAT3(a, b)

#define _DDEACH_3_0(opt,   idx)
#define _DDEACH_3_1(opt,   idx, a, ...) opt(a, idx)
#define _DDEACH_3_2(opt,   idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_1(opt,  DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_3(opt,   idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_2(opt,  DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_4(opt,   idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_3(opt,  DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_5(opt,   idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_4(opt,  DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_6(opt,   idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_5(opt,  DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_7(opt,   idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_6(opt,  DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_8(opt,   idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_7(opt,  DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_9(opt,   idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_8(opt,  DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_10(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_9(opt,  DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_11(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_10(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_12(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_11(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_13(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_12(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_14(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_13(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_15(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_14(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_16(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_15(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_17(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_16(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_18(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_17(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_19(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_18(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_20(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_19(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_21(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_20(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_22(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_21(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_23(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_22(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_24(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_23(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_25(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_24(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_26(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_25(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_27(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_26(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_28(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_27(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_29(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_28(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_30(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_29(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_31(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_30(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_32(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_31(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_33(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_32(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_34(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_33(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_35(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_34(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_36(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_35(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_37(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_36(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_38(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_37(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_39(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_38(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_40(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_39(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_41(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_40(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_42(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_41(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_43(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_42(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_44(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_43(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_45(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_44(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_46(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_45(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_47(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_46(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_48(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_47(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_49(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_48(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_50(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_49(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_51(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_50(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_52(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_51(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_53(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_52(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_54(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_53(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_55(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_54(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_56(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_55(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_57(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_56(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_58(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_57(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_59(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_58(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_60(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_59(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_61(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_60(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_62(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_61(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_63(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_62(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_64(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_63(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_65(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_64(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_66(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_65(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_67(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_66(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_68(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_67(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_69(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_68(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_70(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_69(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_71(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_70(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_72(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_71(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_73(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_72(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_74(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_73(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_75(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_74(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_76(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_75(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_77(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_76(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_78(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_77(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_79(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_78(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_80(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_79(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_81(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_80(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_82(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_81(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_83(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_82(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_84(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_83(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_85(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_84(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_86(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_85(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_87(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_86(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_88(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_87(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_89(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_88(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_90(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_89(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_91(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_90(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_92(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_91(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_93(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_92(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_94(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_93(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_95(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_94(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_96(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_95(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_97(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_96(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_98(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_97(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_99(opt,  idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_98(opt, DDINC(idx), __VA_ARGS__),)
#define _DDEACH_3_100(opt, idx, a, ...) opt(a, idx) ___DDEACH_CAT3(_DDEACH_3_99(opt, DDINC(idx), __VA_ARGS__),)

#endif // ddbase_macro_ddeach_3__hpp_

