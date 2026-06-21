import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover
from . import HoermannHub, CONF_HOERMANN_ID

DEPENDENCIES = ["hoermann"]

hoermann_ns = cg.esphome_ns.namespace("hoermann")
HoermannCover = hoermann_ns.class_("HoermannCover", cover.Cover, cg.Component)

CONFIG_SCHEMA = cover.cover_schema(HoermannCover).extend(
    {
        cv.GenerateID(CONF_HOERMANN_ID): cv.use_id(HoermannHub),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = await cover.new_cover(config)
    await cg.register_component(var, config)
    
    hub = await cg.get_variable(config[CONF_HOERMANN_ID])
    cg.add(hub.set_cover(var))