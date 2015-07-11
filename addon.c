#include <stdio.h>
#include <ctype.h>
#include "sigmsgs.h"
#include "rc.h"


#if RC_ADDON

#include "let.tab.c"

#define KILL_USAGE "usage: kill [-signame|-signum] pid\n"
#define CHECK(cond, msg) if (cond) { fprint(2, (msg)); set(FALSE); return; }

void b_kill (char **av)
{
  char *sig_p, *proc_p;
  pid_t proc;
  int sig, r;

  if (av[1]  == NULL) {
    fprint(2, KILL_USAGE); set(FALSE); return; 
  }
  sig_p = av[1];
  if (av[2] == NULL) {
    sig = SIGTERM;
    proc_p = sig_p;
  } else {
    proc_p = av[2];
    CHECK((av[3] != NULL), KILL_USAGE);
    CHECK((*sig_p != '-'), KILL_USAGE)
    sig = a2u(sig_p+1);
    if (sig < 0) {
        int i;
        for (i=0; signals[i].name; ++i) {
            if (0 == strcasecmp(sig_p+1, signals[i].name)) {
                /* -sigkill */
                sig = signals[i].signum;
                break;
            } else if (0 == strcasecmp(sig_p+1, signals[i].name+3)) {
                /* -kill */
                sig = signals[i].signum;
                break;
            }
        }
    }
    CHECK((sig < 0), "bad signal\n")
  }

  proc = a2u(proc_p);
  CHECK((proc < 0), "bad process id\n")
  r = kill(proc, sig);
  CHECK((r < 0), "kill failed\n")
#undef KILL_USAGE
#undef CHECK
}

extern int LetDoParse(char *s, long *r);

#if 0
static void set_var (char *varname, long R)
{
  List *val = nnew(List); var->w = nprint("%ld", R);
  varassign(varname, val, FALSE);
}
#endif

static int check_var_name (char *p)
{
  if (!isalpha(*p) && (*p) != '_') {
    return 0;
  }
  for ( ; *p; ++p) {
    if (!isalnum(*p) && (*p) != '_') {
      return 0;
    }
  }
  return 1;
}


enum { VAL_ZERO = 1, VAL_NONZERO = 0, BAD_EXP = 2 };
static int ret_value(int parse_status, long value)
{
  if (0==parse_status) {
    if (0L == value) {
      return VAL_ZERO;
    } else {
      return VAL_NONZERO;
    }
  } else {
    return BAD_EXP;
  }
}

void b_let (char **av)
{
  long R = 0;
  int parse_status;
  int rc_status = BAD_EXP;
  char *exp, *varname;

  if (av[1] == 0) {
    fprint(2, "let: cannot have 0 args\n");
    goto returnLabel;
  }

  if (av[2] == 0) {
    exp = av[1];
    parse_status = LetDoParse(exp, &R);
    rc_status = ret_value(parse_status, R);
    goto returnLabel;
  }

  if (av[3] != 0) {
    fprint(2, "let: cannot have more than 2 args\n");
    goto returnLabel;
  }

  varname = av[1];
  exp = av[2];

  if (0 == strcmp(varname, "-p")) {
    parse_status = LetDoParse(exp, &R);
    if (0==parse_status) {
      fprint(1, "%ld\n", R);
    }
    rc_status = ret_value(parse_status, R);
    goto returnLabel;
  }

  if (!check_var_name(av[1])) {
    fprint(2, "let: bad variable name '%s'\n", av[1]);
    goto returnLabel;
  }

  parse_status = LetDoParse(av[2], &R);
  if (0 == parse_status) {
    List val;
    val.w = nprint("%ld", R);
    val.n = NULL;
    varassign(av[1], &val, FALSE);
  }
  rc_status = ret_value(parse_status, R);
  goto returnLabel;


returnLabel:
  setN(rc_status);
  return;
}

/*
#include "let.tab.c"
*/


#endif
