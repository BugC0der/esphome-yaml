import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover
from . import HoermannHub, CONF_HOERMANN_ID, CONF_HOERMANN_ID_DEFAULT

DEPENDENCIES = ["hoermann"]

hoermann_ns = cg.esphome_ns.namespace("hoermann")
HoermannCover = hoermann_ns.class_("HoermannCover", cover.Cover, cg.Component)

CONFIG_SCHEMA = cover.cover_schema(HoermannCover).extend({
    cv.Optional(CONF_HOERMANN_ID, default=CONF_HOERMANN_ID_DEFAULT): cv.use_id(HoermannHub),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_HOERMANN_ID])
    var = await cover.new_cover(config, hub)
    await cg.register_component(var, config)