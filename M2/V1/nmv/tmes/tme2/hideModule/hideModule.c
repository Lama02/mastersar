/*
 * =====================================================================================
 *
 *       Filename:  hideModule.c
 *
 *    Description:  Module "module cache" pour noyau 2.6 
 *
 *        Version:  1.0
 *        Created:  09/10/2009 13:53:46
 *       Revision:  none
 *       Compiler:  gcc
 *
 *        Authors:  A. Rachid, C. Monir, 
 *        Company:  UPMC
 *
 * =====================================================================================
 */

/*
 * =====================================================================================
 * ATTENTION : CE MODULE UNE FOIS INSERE, NE PEUT PLUS ETRE ENLEVE DU NOYAU  
 * EN UTILISANT LA FONCTION "rmmod". LA SEUL SOLUTION EST DE REDEMARRER LA    
 * MACHINE.                                                                  
 * =====================================================================================
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

MODULE_DESCRIPTION("Module \"hide module\" pour noyau 2.6");
MODULE_AUTHOR("A. Rachid, C. Monir, UPMC");
MODULE_LICENSE("GPL");


static int hello_init(void)
{
  /* cacher le module en le supprimant de la liste des modules */
  /* inseres dans le noyau */
  __list_del(__this_module.list.prev, __this_module.list.next);
  printk(KERN_ALERT "Je suis le module cache\n");
  return 0;
}

static void hello_exit(void)
{
  /* ATTENTION : cette fonction ne sera jamais appelee car     */
  /* le module insere est cache. L'insertion du ce meme module */
  /* n'est pas possible car l'entree correspondant dans le     */
  /* repertoire /proc n'est pas encore supprimee. Ceci sera    */
  /* l'objet de la version 4 de ce code.                       */
  printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);

module_exit(hello_exit);

