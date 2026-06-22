import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_TYPE
from . import HoermannHub, CONF_HOERMANN_ID, CONF_HOERMANN_ID_DEFAULT

DEPENDENCIES = ["hoermann"]

TYPES = {
    "light": cg.RawExpression("esphome::hoermann::HoermannSwitch::Light"),
    "venting": cg.RawExpression("esphome::hoermann::HoermannSwitch::Venting")
}

hoermann_ns = cg.esphome_ns.namespace("hoermann")
HoermannSwitch = hoermann_ns.class_("HoermannSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = switch.switch_schema(HoermannSwitch).extend({
    cv.Optional(CONF_HOERMANN_ID, default=CONF_HOERMANN_ID_DEFAULT): cv.use_id(HoermannHub),
    cv.Required(CONF_TYPE): cv.enum(TYPES, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    enum_value = TYPES[config[CONF_TYPE]]
    hub = await cg.get_variable(config[CONF_HOERMANN_ID])
    var = await switch.new_switch(config, hub, enum_value)
    await cg.register_component(var, config)