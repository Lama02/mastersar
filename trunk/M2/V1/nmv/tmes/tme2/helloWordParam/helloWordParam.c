/*
 * =====================================================================================
 *
 *       Filename:  helloWordParam.c
 *
 *    Description:  Module "hello word" avec parametres pour noyau 2.6 
 *
 *        Version:  1.0
 *        Created:  09/10/2009 13:53:46
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  J. Sopena, 
 *        Company:  LIP6
 *
 * =====================================================================================
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_DESCRIPTION("Module \"hello word\" avec parametre pour noyau 2.6");
MODULE_AUTHOR("Julien Sopena, LIP6");
MODULE_LICENSE("GPL");

static char *whom = "world";
module_param(whom, charp, 0);

static int howmany = 1;
module_param(howmany, int, 0);

static int hello_init(void)
{
  int i;
  for (i = 0; i < howmany; i++)
   printk(KERN_ALERT "(%d) Hello, %s\n", i, whom);
  return 0;
}
static void hello_exit(void)
{
  printk(KERN_ALERT "Goodbye, cruel %s\n", whom);
}

module_init(hello_init);

module_exit(hello_exit);

