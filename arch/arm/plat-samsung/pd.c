/* linux/arch/arm/plat-samsung/pd.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Samsung Power domain support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/pm_runtime.h>

#include <plat/pd.h>

void samsung_pd_set_persistent(struct platform_device *pdev)
{
	struct samsung_pd_info *pdata = pdev->dev.platform_data;

	pdata->persistent = true;
}

static int samsung_pd_probe(struct platform_device *pdev)
{
	struct samsung_pd_info *pdata = pdev->dev.platform_data;
	struct device *dev = &pdev->dev;
	int ret = 0;

	if (!pdata) {
		dev_err(dev, "no device data specified\n");
		return -ENOENT;
	}

	if (!pdata->persistent && pdata->enable)
		ret = pdata->enable(dev);

	if (pdata->persistent)
		pm_runtime_set_suspended(dev);
	else
		pm_runtime_set_active(dev);

	pm_runtime_enable(dev);

	if (!ret)
		dev_info(dev, "power domain registered\n");

	return ret;
}

static int __devexit samsung_pd_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	pm_runtime_disable(dev);
	return 0;
}

static int samsung_pd_runtime_suspend(struct device *dev)
{
	struct samsung_pd_info *pdata = dev->platform_data;
	int ret = 0;

	if (pdata->disable)
		ret = pdata->disable(dev);

	dev_dbg(dev, "suspended\n");
	return ret;
}

static int samsung_pd_runtime_resume(struct device *dev)
{
	struct samsung_pd_info *pdata = dev->platform_data;
	int ret = 0;

	if (pdata->enable)
		ret = pdata->enable(dev);

	dev_dbg(dev, "resumed\n");
	return ret;
}

static int samsung_pd_resume(struct device *dev)
{
	struct samsung_pd_info *pdata = dev->platform_data;
	int ret = 0;

	if (pdata->enable)
		ret = pdata->enable(dev);

	pm_runtime_disable(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	dev_dbg(dev, "resumed\n");
	return ret;
}

static const struct dev_pm_ops samsung_pd_pm_ops = {
	.resume			= samsung_pd_resume,
	.runtime_suspend	= samsung_pd_runtime_suspend,
	.runtime_resume		= samsung_pd_runtime_resume,
};

static struct platform_driver samsung_pd_driver = {
	.driver		= {
		.name		= "samsung-pd",
		.owner		= THIS_MODULE,
		.pm		= &samsung_pd_pm_ops,
	},
	.probe		= samsung_pd_probe,
	.remove		= __devexit_p(samsung_pd_remove),
};

static int __init samsung_pd_init(void)
{
	int ret;

	ret = platform_driver_register(&samsung_pd_driver);
	if (ret)
		printk(KERN_ERR "%s: failed to add PD driver\n", __func__);

	return ret;
}
arch_initcall(samsung_pd_init);
