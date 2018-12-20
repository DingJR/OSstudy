#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
struct JOB
{
    int a;
};
/**
 * Executes the command "cat scores | grep Villanova | cut -b 1-10".
 * This quick-and-dirty version does no error checking.
 *
 * @author Jim Glenn
 * @version 0.1 10/4/2004
 */

int main(int argc, char **argv)
{
  char *line=(char*)malloc(sizeof(char)*1024);
  FILE *f = stdin;
  fgets(line,1024,f);
  puts(line);
  return 0;
}
