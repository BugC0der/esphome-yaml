import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_TYPE
from . import HoermannHub, CONF_HOERMANN_ID

DEPENDENCIES = ["hoermann"]

TYPES = {
    "light": "set_light",
    "error": "set_error",
    "venting": "set_venting",
    "prewarn": "set_prewarn",
    "option_relay": "set_option_relay",
}

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema().extend(
    {
        cv.GenerateID(CONF_HOERMANN_ID): cv.use_id(HoermannHub),
        cv.Required(CONF_TYPE): cv.enum(TYPES, lower=True),
    }
)

async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    hub = await cg.get_variable(config[CONF_HOERMANN_ID])
    cg.add(getattr(hub, TYPES[config[CONF_TYPE]])(var))