import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID, CONF_TX_PIN, CONF_RX_PIN, CONF_BAUD_RATE
from esphome import pins
from esphome.core import ID

AUTO_LOAD = ["binary_sensor", "cover", "switch"]
DEPENDENCIES = []
MULTI_CONF = False

CONF_HOERMANN_ID = "hoermann_id"
CONF_HOERMANN_ID_DEFAULT = "hoermann_hub"

hoermann_ns = cg.esphome_ns.namespace("hoermann")
HoermannHub = hoermann_ns.class_("HoermannHub", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_ID, default=CONF_HOERMANN_ID_DEFAULT): cv.declare_id(HoermannHub),
    cv.Optional(CONF_TX_PIN, default=15): pins.gpio_output_pin_schema,
    cv.Optional(CONF_RX_PIN, default=13): pins.gpio_input_pin_schema,
    cv.Optional(CONF_BAUD_RATE, default=19200): cv.int_range(min=1),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Create the implicit UART variable
    uart_id = ID(f"{config[CONF_ID].id}_uart", is_declaration=True, type=uart.UARTComponent)
    uart_var = cg.new_Pvariable(uart_id)

    # Configure pins and baud rate
    tx_pin = await cg.gpio_pin_expression(config[CONF_TX_PIN])
    rx_pin = await cg.gpio_pin_expression(config[CONF_RX_PIN])
    
    cg.add(uart_var.set_tx_pin(tx_pin))
    cg.add(uart_var.set_rx_pin(rx_pin))
    cg.add(uart_var.set_baud_rate(config[CONF_BAUD_RATE]))

    # FIX: Register directly in C++ App, bypassing Python's strict component registry
    cg.add(cg.App.register_component(uart_var))

    # Link your Hub to this implicit UART
    cg.add(var.set_uart_parent(uart_var))