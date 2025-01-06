#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/time.h>

#define PROCFS_NAME "tsulab"
static struct proc_dir_entry *lab4_proc_file;

static unsigned long voyager1LaunchDays(void) {
    struct timespec64 today;
    ktime_get_real_ts64(&today);
    time64_t launchDate = mktime64(1977, 9, 5, 12, 56, 0);
    int diffDays = (today.tv_sec - launchDate) / 86400;
    return diffDays;
}

static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t *offset) {
    unsigned long time = voyager1LaunchDays();
    char text[64];
    size_t len;
    len = snprintf(text, sizeof(text), "It's been %ld days since the launch of Voyager 1\n", time);

    if (*offset > 0){
        return 0;
    }
    
    if (copy_to_user(buffer, text, len)){
        return -EFAULT;
    }

    *offset += len;
    pr_info ("procfile read %s\n",file_pointer->f_path.dentry->d_name.name);
    return len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
};
#else
static const struct file_operations proc_file_fops = {
    .read = procfile_read,
};
#endif

static int __init f_init(void) {
    lab4_proc_file = proc_create(PROCFS_NAME, 0664, NULL, &proc_file_fops);
    if (lab4_proc_file == NULL){
        pr_alert("Could not initialize /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }
    pr_info("/proc/%s created\n", PROCFS_NAME);
    return 0;
}

static void __exit f_exit(void) {
    proc_remove(lab4_proc_file);
    pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(f_init);
module_exit(f_exit);
MODULE_LICENSE("GPL");