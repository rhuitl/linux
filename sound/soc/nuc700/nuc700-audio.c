/*
 * Copyright (c) 2010 Nuvoton technology corporation.
 *
 * Wan ZongShun <mcuos.com@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>

#include "nuc700-audio.h"

static struct snd_soc_dai_link nuc700evb_ac97_dai = {
	.name		= "AC97",
	.stream_name	= "AC97 HiFi",
	.cpu_dai_name	= "nuc700-ac97",
	.codec_dai_name	= "ac97-hifi",
	.codec_name	= "ac97-codec",
	.platform_name	= "nuc700-pcm-audio",
};

static struct snd_soc_card nuc700evb_audio_machine = {
	.name		= "NUC700EVB_AC97",
	.dai_link	= &nuc700evb_ac97_dai,
	.num_links	= 1,
};

static struct platform_device *nuc700evb_asoc_dev;

static int __init nuc700evb_audio_init(void)
{
	int ret;

	ret = -ENOMEM;
	nuc700evb_asoc_dev = platform_device_alloc("soc-audio", -1);
	if (!nuc700evb_asoc_dev)
		goto out;

	/* nuc700 board audio device */
	platform_set_drvdata(nuc700evb_asoc_dev, &nuc700evb_audio_machine);

	ret = platform_device_add(nuc700evb_asoc_dev);

	if (ret) {
		platform_device_put(nuc700evb_asoc_dev);
		nuc700evb_asoc_dev = NULL;
	}

out:
	return ret;
}

static void __exit nuc700evb_audio_exit(void)
{
	platform_device_unregister(nuc700evb_asoc_dev);
}

module_init(nuc700evb_audio_init);
module_exit(nuc700evb_audio_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("NUC700 Series ASoC audio support");
MODULE_AUTHOR("Wan ZongShun");
