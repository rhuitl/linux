/*
 * OHCI HCD (Host Controller Driver) for USB.
 *
 * Copyright (C) 2011 Nuvoton corp.
 *
 * Author : Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

#include <linux/clk.h>
#include <linux/platform_device.h>

struct clk *nuc700_clk;

static int ohci_nuc700_start(struct usb_hcd *hcd)
{
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);

	ohci_hcd_init(ohci);
	ohci_init(ohci);
	ohci_run(ohci);
	hcd->state = HC_STATE_RUNNING;
	return 0;
}

static const struct hc_driver ohci_nuc700_hc_driver = {
	.description =		hcd_name,
	.product_desc =		"NUC700 OHCI",
	.hcd_priv_size =	sizeof(struct ohci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq =			ohci_irq,
	.flags =		HCD_USB11 | HCD_MEMORY,

	/*
	 * basic lifecycle operations
	 */
	.start =		ohci_nuc700_start,
	.stop =			ohci_stop,
	.shutdown =		ohci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		ohci_urb_enqueue,
	.urb_dequeue =		ohci_urb_dequeue,
	.endpoint_disable =	ohci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number =	ohci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =	ohci_hub_status_data,
	.hub_control =		ohci_hub_control,
#ifdef	CONFIG_PM
	.bus_suspend =		ohci_bus_suspend,
	.bus_resume =		ohci_bus_resume,
#endif
	.start_port_reset =	ohci_start_port_reset,
};

/*-------------------------------------------------------------------------*/

static int ohci_hcd_nuc700_probe(struct platform_device *pdev)
{
	struct resource *res = NULL;
	struct usb_hcd *hcd = NULL;
	int irq = -1;
	int ret;

	if (usb_disabled())
		return -ENODEV;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		err("platform_get_resource error.");
		return -ENODEV;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		err("platform_get_irq error.");
		return -ENODEV;
	}

	nuc700_clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(nuc700_clk))
		return PTR_ERR(nuc700_clk);
	clk_enable(nuc700_clk);

	/* initialize hcd */
	hcd = usb_create_hcd(&ohci_nuc700_hc_driver, &pdev->dev,
							(char *)hcd_name);
	if (!hcd) {
		err("Failed to create hcd");
		clk_put(nuc700_clk);
		return -ENOMEM;
	}

	hcd->regs = (void __iomem *)res->start;
	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);
	ret = usb_add_hcd(hcd, irq, IRQF_DISABLED | IRQF_SHARED);
	if (ret != 0) {
		err("Failed to add hcd");
		clk_put(nuc700_clk);
		usb_put_hcd(hcd);
		return ret;
	}

	return ret;
}

static int ohci_hcd_nuc700_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	clk_disable(nuc700_clk);
	clk_put(nuc700_clk);
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);

	return 0;
}

static struct platform_driver ohci_hcd_nuc700_driver = {
	.probe		= ohci_hcd_nuc700_probe,
	.remove		= ohci_hcd_nuc700_remove,
	.shutdown	= usb_hcd_platform_shutdown,
	.driver		= {
		.name	= "nuc700-ohci",
		.owner	= THIS_MODULE,
	},
};

MODULE_ALIAS("platform:nuc700-ohci");
