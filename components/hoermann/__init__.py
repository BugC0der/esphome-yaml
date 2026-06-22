import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

AUTO_LOAD = ["binary_sensor", "cover", "switch"]
DEPENDENCIES = ["uart"]
MULTI_CONF = False

CONF_HOERMANN_ID = "hoermann_id"

hoermann_ns = cg.esphome_ns.namespace("hoermann")
HoermannHub = hoermann_ns.class_("HoermannHub", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(HoermannHub),
}).extend(uart.UART_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)