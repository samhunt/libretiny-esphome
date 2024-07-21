import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate_ir, select
from esphome.const import (
    CONF_ID,
#    CONF_SUPPORTS_BOTH_SWING,
#    CONF_SUPPORTS_HORIZONTAL_SWING,
    CONF_SUPPORTS_VERTICAL_SWING,
)

AUTO_LOAD = ["climate_ir", "select"]

CONF_HORIZONTAL_SWING_SELECT = "horizontal_swing_select"
CONF_VERTICAL_SWING_SELECT = "vertical_swing_select"

panasonic_ns = cg.esphome_ns.namespace("panasonic_ir")
PanasonicClimate = panasonic_ns.class_("PanasonicClimate", climate_ir.ClimateIR)

PanasonicACSelect = panasonic_ns.class_(
    "PanasonicACSelect", select.Select, cg.PollingComponent
)

SELECT_SCHEMA = select.SELECT_SCHEMA.extend(
    {cv.GenerateID(CONF_ID): cv.declare_id(PanasonicACSelect)}
)

HORIZONTAL_SWING_OPTIONS = [
    "left",
    "center_left",
    "center",
    "center_right",
    "right",
    "auto",
]
VERTICAL_SWING_OPTIONS = ["top", "middle_top", "middle", "middle_bottom", "bottom", "auto"]

CONFIG_SCHEMA = climate_ir.CLIMATE_IR_WITH_RECEIVER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(PanasonicClimate),
#        cv.Optional(CONF_SUPPORTS_HORIZONTAL_SWING, default=False): cv.boolean,
        cv.Optional(CONF_SUPPORTS_VERTICAL_SWING, default=True): cv.boolean,
#        cv.Optional(CONF_SUPPORTS_BOTH_SWING, default=False): cv.boolean,
        cv.Optional(CONF_HORIZONTAL_SWING_SELECT): SELECT_SCHEMA,
        cv.Optional(CONF_VERTICAL_SWING_SELECT): SELECT_SCHEMA,
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await climate_ir.register_climate_ir(var, config)

    cg.add(
        var.set_supported_swing_modes(
#            config[CONF_SUPPORTS_HORIZONTAL_SWING],
            config[CONF_SUPPORTS_VERTICAL_SWING],
#            config[CONF_SUPPORTS_BOTH_SWING],
        )
    )

#    if CONF_HORIZONTAL_SWING_SELECT in config:
#        conf = config[CONF_HORIZONTAL_SWING_SELECT]
#        swing_select = await select.new_select(conf, options=HORIZONTAL_SWING_OPTIONS)
#        await cg.register_component(swing_select, conf)
#        cg.add(var.set_horizontal_vane_select(swing_select))

    if CONF_VERTICAL_SWING_SELECT in config:
        conf = config[CONF_VERTICAL_SWING_SELECT]
        swing_select = await select.new_select(conf, options=VERTICAL_SWING_OPTIONS)
        await cg.register_component(swing_select, conf)
        cg.add(var.set_vertical_vane_select(swing_select))
