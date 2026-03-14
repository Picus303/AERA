

#ifndef opcodes_h
#define opcodes_h

#include "../core/types.h"
#include "dll.h"


using namespace core;

namespace r_exec {

// Opcodes are initialized by Init().
class r_exec_dll Opcodes {
public:
  static uint16 View;
  static uint16 PgmView;
  static uint16 GrpView;

  static uint16 TI;

  static uint16 Ent;
  static uint16 Ont;
  static uint16 MkVal;

  static uint16 Grp;

  static uint16 Ptn;
  static uint16 AntiPtn;

  static uint16 IPgm;
  static uint16 ICppPgm;

  static uint16 Pgm;
  static uint16 AntiPgm;

  static uint16 ICmd;
  static uint16 Cmd;

  static uint16 Fact;
  static uint16 AntiFact;

  static uint16 Mdl;
  static uint16 Cst;

  static uint16 ICst;
  static uint16 IMdl;

  static uint16 Pred;
  static uint16 Goal;

  static uint16 Success;

  static uint16 MkGrpPair;

  static uint16 MkRdx;
  static uint16 Perf;

  static uint16 MkNew;

  static uint16 MkLowRes;
  static uint16 MkLowSln;
  static uint16 MkHighSln;
  static uint16 MkLowAct;
  static uint16 MkHighAct;
  static uint16 MkSlnChg;
  static uint16 MkActChg;

  static uint16 Sim;
  static uint16 Id;

  static uint16 Inject;
  static uint16 Eject;
  static uint16 Mod;
  static uint16 Set;
  static uint16 NewClass;
  static uint16 DelClass;
  static uint16 LDC;
  static uint16 Swap;
  static uint16 Prb;
  static uint16 Stop;

  static uint16 Gtr;
  static uint16 Lsr;
  static uint16 Gte;
  static uint16 Lse;
  static uint16 Add;
  static uint16 Sub;
  static uint16 Mul;
  static uint16 Div;

  static uint16 Var;
};
}


#endif
