/*
 * Copyright © 2017, Nikit Sharma "Dreamstar" <nikitsharma92@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>
#include "super_charger_control.h"

int super_charge = 0;

#define DRIVER_VERSION  1
#define DRIVER_SUBVER 0

static ssize_t cust_ac_level_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d mA", cust_ac_lev);
 
}

static ssize_t cust_ac_level_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
 
 	unsigned int ret = -EINVAL;
 	int val;
 
 	ret = sscanf(buf, "%d", &val);
 
 	if (super_charge == 1 && val > AC_MAX_LEVEL)
 	{
 		val = AC_MAX_LEVEL;
 	}
 
 	if (super_charge == 1 && val < AC_MIN_LEVEL)
 	{
 		val = AC_MIN_LEVEL;
 	}
 
 	cust_ac_lev = val;
 
 	return count;
}

static ssize_t super_ver_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "Super Charger Control %u.%u", DRIVER_VERSION, DRIVER_SUBVER);
}

static ssize_t super_charge_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d", super_charge);
}

static ssize_t super_charge_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int val;
	sscanf(buf, "%d", &val);
	switch(val)
	{
	case 0:
        super_charge=0;
        break;  
	case 1:
	super_charge=1;
	break;
	default:
	pr_info("invalid value specified");
}

return count;

}

static struct kobj_attribute super_charge_attribute =
 	__ATTR(enabled,
 		0666,
 		super_charge_show,
 		super_charge_store);

static struct kobj_attribute super_ver_attribute =
	__ATTR(version,
		0444,
		super_ver_show, NULL);

static struct kobj_attribute cust_ac_level_attribute =
	__ATTR(cust_ac_level,
		0666,
		cust_ac_level_show,
		cust_ac_level_store);

static struct attribute *charger_level_control_attrs[] =
	{
		&cust_ac_level_attribute.attr,
                &super_charge_attribute.attr,
                &super_ver_attribute.attr,
		NULL,
	};

static struct attribute_group chgr_level_attr_group =
	{
		.attrs = charger_level_control_attrs,
	};

static struct kobject *charger_level_control_kobj;

static int charger_level_control_probe(void)
{
	int sysfs_result;
	printk(KERN_DEBUG "[%s]\n",__func__);

	charger_level_control_kobj = kobject_create_and_add("super_charge", kernel_kobj);

	if (!charger_level_control_kobj) {
		pr_err("%s Interface create failed!\n",
			__FUNCTION__);
		return -ENOMEM;
        }

	sysfs_result = sysfs_create_group(charger_level_control_kobj,
			&chgr_level_attr_group);

	if (sysfs_result) {
		pr_info("%s sysfs create failed!\n", __FUNCTION__);
		kobject_put(charger_level_control_kobj);
	}
	return sysfs_result;
}

static void charger_level_control_remove(void)
{
	if (charger_level_control_kobj != NULL)
		kobject_put(charger_level_control_kobj);
}

module_init(charger_level_control_probe);
module_exit(charger_level_control_remove);
MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Nikit Sharma <nikitsharma92@gmail.com>");
MODULE_DESCRIPTION("Samsung Charge level control driver");
