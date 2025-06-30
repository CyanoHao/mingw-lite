#include <locale.h>
#include <stdio.h>

#include <libintl.h>

int main()
{
  textdomain("test");

  printf("%s\n", gettext("Hello, world!"));
}
