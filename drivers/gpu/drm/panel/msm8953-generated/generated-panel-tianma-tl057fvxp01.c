// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021 Julian Braha <julianbraha@gmail.com>
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct tianma_tl057fvxp01 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct gpio_desc *reset_gpio;
	bool prepared;
};

static inline struct tianma_tl057fvxp01 *to_tianma_tl057fvxp01(struct drm_panel *panel)
{
	return container_of(panel, struct tianma_tl057fvxp01, panel);
}

#define dsi_dcs_write_seq(dsi, seq...) do {				\
		static const u8 d[] = { seq };				\
		int ret;						\
		ret = mipi_dsi_dcs_write_buffer(dsi, d, ARRAY_SIZE(d));	\
		if (ret < 0)						\
			return ret;					\
	} while (0)

static void tianma_tl057fvxp01_reset(struct tianma_tl057fvxp01 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
}

static int tianma_tl057fvxp01_on(struct tianma_tl057fvxp01 *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	dsi_dcs_write_seq(dsi, 0x00, 0x00);
	dsi_dcs_write_seq(dsi, 0xff, 0x19, 0x11, 0x01);
	dsi_dcs_write_seq(dsi, 0x00, 0x80);
	dsi_dcs_write_seq(dsi, 0xff, 0x19, 0x11);
	dsi_dcs_write_seq(dsi, 0x00, 0xb0);
	dsi_dcs_write_seq(dsi, 0xb3, 0x04, 0x38, 0x08, 0x70);
	dsi_dcs_write_seq(dsi, 0x00, 0x00);
	dsi_dcs_write_seq(dsi, 0xff, 0xff, 0xff, 0xff);

	ret = mipi_dsi_dcs_set_tear_on(dsi, MIPI_DSI_DCS_TEAR_MODE_VBLANK);
	if (ret < 0) {
		dev_err(dev, "Failed to set tear on: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_display_brightness(dsi, 0x08cc);
	if (ret < 0) {
		dev_err(dev, "Failed to set display brightness: %d\n", ret);
		return ret;
	}

	dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_CONTROL_DISPLAY, 0x2c);
	dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_POWER_SAVE, 0x01);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to exit sleep mode: %d\n", ret);
		return ret;
	}
	msleep(120);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display on: %d\n", ret);
		return ret;
	}
	usleep_range(10000, 11000);

	return 0;
}

static int tianma_tl057fvxp01_off(struct tianma_tl057fvxp01 *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display off: %d\n", ret);
		return ret;
	}
	msleep(50);

	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to enter sleep mode: %d\n", ret);
		return ret;
	}
	msleep(70);

	return 0;
}

static int tianma_tl057fvxp01_prepare(struct drm_panel *panel)
{
	struct tianma_tl057fvxp01 *ctx = to_tianma_tl057fvxp01(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (ctx->prepared)
		return 0;

	tianma_tl057fvxp01_reset(ctx);

	ret = tianma_tl057fvxp01_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		return ret;
	}

	ctx->prepared = true;
	return 0;
}

static int tianma_tl057fvxp01_unprepare(struct drm_panel *panel)
{
	struct tianma_tl057fvxp01 *ctx = to_tianma_tl057fvxp01(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (!ctx->prepared)
		return 0;

	ret = tianma_tl057fvxp01_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);

	ctx->prepared = false;
	return 0;
}

static const struct drm_display_mode tianma_565_v0_mode = {
	.clock = (1080 + 53 + 4 + 53) * (2160 + 14 + 1 + 11) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 53,
	.hsync_end = 1080 + 53 + 4,
	.htotal = 1080 + 53 + 4 + 53,
	.vdisplay = 2160,
	.vsync_start = 2160 + 14,
	.vsync_end = 2160 + 14 + 1,
	.vtotal = 2160 + 14 + 1 + 11,
	.width_mm = 62,
	.height_mm = 110,
};

static int tianma_tl057fvxp01_get_modes(struct drm_panel *panel,
				   struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &tianma_tl057fvxp01_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs tianma_tl057fvxp01_panel_funcs = {
	.prepare = tianma_tl057fvxp01_prepare,
	.unprepare = tianma_tl057fvxp01_unprepare,
	.get_modes = tianma_tl057fvxp01_get_modes,
};

static int tianma_tl057fvxp01_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct tianma_tl057fvxp01 *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM;

	drm_panel_init(&ctx->panel, dev, &tianma_tl057fvxp01_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
		return ret;
	}

	return 0;
}

static int tianma_tl057fvxp01_remove(struct mipi_dsi_device *dsi)
{
	struct tianma_tl057fvxp01 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);

	return 0;
}

static const struct of_device_id tianma_tl057fvxp01_of_match[] = {
	{ .compatible = "tianma,tl057fvxp01" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, tianma_tl057fvxp01_of_match);

static struct mipi_dsi_driver tianma_tl057fvxp01_driver = {
	.probe = tianma_tl057fvxp01_probe,
	.remove = tianma_tl057fvxp01_remove,
	.driver = {
		.name = "panel-tianma-tl057fvxp01",
		.of_match_table = tianma_tl057fvxp01_of_match,
	},
};
module_mipi_dsi_driver(tianma_tl057fvxp01_driver);

MODULE_AUTHOR("Julian Braha <julianbraha@gmail.com>");
MODULE_DESCRIPTION("Tianma TL057FVXP01 panel driver");
MODULE_LICENSE("GPL v2");
