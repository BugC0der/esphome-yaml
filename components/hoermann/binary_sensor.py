import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor 
from esphome.const import CONF_TYPE
from . import HoermannHub, CONF_HOERMANN_ID, CONF_HOERMANN_ID_DEFAULT

DEPENDENCIES = ["hoermann"]

TYPES = {
    "error": cg.RawExpression("esphome::hoermann::HoermannBinarySensor::Error"),
    "prewarn": cg.RawExpression("esphome::hoermann::HoermannBinarySensor::Prewarn"),
    "option_relay": cg.RawExpression("esphome::hoermann::HoermannBinarySensor::OptionRelay"),
}

hoermann_ns = cg.esphome_ns.namespace("hoermann")
HoermannBinarySensor = hoermann_ns.class_("HoermannBinarySensor", binary_sensor.BinarySensor, cg.Component)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(HoermannBinarySensor).extend({
    cv.Optional(CONF_HOERMANN_ID, default=CONF_HOERMANN_ID_DEFAULT): cv.use_id(HoermannHub),
    cv.Required(CONF_TYPE): cv.enum(TYPES, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    enum_value = TYPES[config[CONF_TYPE]]
    hub = await cg.get_variable(config[CONF_HOERMANN_ID])
    var = await binary_sensor.new_binary_sensor(config, hub, enum_value)
    await cg.register_component(var, config)