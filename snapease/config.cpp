#include "main.h"

void config_readstr(const char *what, char *out, int outsz)
{
  GetPrivateProfileString("snapease",what,"",out,outsz,g_ini_file.Get());
}

int config_readint(const char *what, int def)
{
  return GetPrivateProfileInt("snapease",what,def,g_ini_file.Get());
}

void config_writestr(const char *what, const char *value)
{
  WritePrivateProfileString("snapease",what,value,g_ini_file.Get());
}

void config_writeint(const char *what, int value)
{
  char valuestr[256];
  sprintf(valuestr,"%d",value);
  WritePrivateProfileString("snapease",what,valuestr,g_ini_file.Get());
}