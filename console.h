#pragma once

#define printfoot FILE *lf = fopen(config.logfile, "a+");
#define printhead fclose(lf);
#define logbody "==========%s==========\n"

#define infocond if (config.loglvlint >= LOGLVL_INFO)
#define warncond if (config.loglvlint >= LOGLVL_WARN)
#define failcond if (config.loglvlint >= LOGLVL_FAIL)
 
#define printinfo1(b) infocond { printfoot fprintf(lf, logbody b "\n", LOGLVL_INFO_STR); printf(logbody b "\n", LOGLVL_INFO_STR); printhead }
#define printinfo2(b,a) infocond { printfoot fprintf(lf, logbody b "\n", LOGLVL_INFO_STR, a); printf(logbody b "\n", LOGLVL_INFO_STR, a); printhead }
#define printinfo3(b,a,c) infocond { printfoot fprintf(lf, logbody b "\n", LOGLVL_INFO_STR, a,c); printf(logbody b "\n", LOGLVL_INFO_STR, a,c); printhead }

#define printwarn1(b) warncond { printfoot fprintf(lf, logbody b "\n", LOGLVL_WARN_STR); printf(logbody b "\n", LOGLVL_WARN_STR); printhead }
#define printwarn2(b,a) warncond { printfoot fprintf(lf, logbody b "\n", LOGLVL_WARN_STR, a); printf(logbody b "\n", LOGLVL_WARN_STR, a); printhead }
#define printwarn3(b,a,c) warncond { printfoot fprintf(lf, logbody b "\n", LOGLVL_WARN_STR, a,c); printf(logbody b "\n", LOGLVL_WARN_STR, a,c); printhead }

#define printfail1(b) failcond { printfoot fprintf(lf, logbody b "\n", LOGLVL_FAIL_STR); printf(logbody b "\n", LOGLVL_FAIL_STR); printhead }
#define printfail2(b,a) failcond { printfoot fprintf(lf, logbody b "\n", LOGLVL_FAIL_STR, a); printf(logbody b "\n", LOGLVL_FAIL_STR, a); printhead }
#define printfail3(b,a,c) failcond { printfoot fprintf(lf, logbody b "\n", LOGLVL_FAIL_STR, a,c); printf(logbody b "\n", LOGLVL_FAIL_STR, a,c); printhead }