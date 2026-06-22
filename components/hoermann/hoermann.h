#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"

namespace esphome
{
    namespace hoermann
    {

        class HoermannHub;

        class HoermannCover : public cover::Cover, public Component
        {
        public:
            HoermannCover(HoermannHub *hub) ;
            void control(const cover::CoverCall &call) override;
            cover::CoverTraits get_traits() override;

        protected:
            HoermannHub *hub_{nullptr};
        };
        class HoermannSwitch : public switch_::Switch, public Component
        {
        public:
            typedef enum switch_type_t { Light, Venting } switch_type_t;
            HoermannSwitch(HoermannHub *hub, switch_type_t type);
            void write_state(bool state) override;

        protected:
            HoermannHub * const hub_{nullptr};
            const switch_type_t type_;
        };

        class HoermannBinarySensor : public binary_sensor::BinarySensor, public Component
        {
        public:
            typedef enum sensor_type_t { Error, Prewarn, OptionRelay } sensor_type_t;
            HoermannBinarySensor(HoermannHub *hub, sensor_type_t type);

        protected:
            HoermannHub * const hub_{nullptr};
            const sensor_type_t type_;
        };

        class HoermannHub : public Component, public uart::UARTDevice
        {
        public:
            typedef enum hoermann_action_t
            {
                hoermann_action_stop = 0,
                hoermann_action_open,
                hoermann_action_close,
                hoermann_action_venting,
                hoermann_action_toggle_light,
                hoermann_action_emergency_stop,
                hoermann_action_impulse,
                hoermann_action_none
            } hoermann_action_t;

            void set_cover(HoermannCover *cover) { this->cover_ = cover; }
            void set_light(HoermannSwitch *light) { this->light_ = light; }
            void set_venting(HoermannSwitch *venting) { this->venting_ = venting; }
            void set_error(HoermannBinarySensor *error) { this->error_ = error; }
            void set_prewarn(HoermannBinarySensor *prewarn) { this->prewarn_ = prewarn; }
            void set_option_relay(HoermannBinarySensor *option_relay) { this->option_relay_ = option_relay; }

            void setup() override {}
            void loop() override;
            void dump_config() override;
            float get_setup_priority() const override { return setup_priority::DATA; }

            void send_command(hoermann_action_t action);

        protected:
            HoermannCover *cover_{nullptr};
            HoermannSwitch *light_{nullptr};
            HoermannSwitch *venting_{nullptr};
            HoermannBinarySensor *error_{nullptr};
            HoermannBinarySensor *prewarn_{nullptr};
            HoermannBinarySensor *option_relay_{nullptr};

            uint8_t rx_buffer[32];
            uint8_t counter{0};
            uint8_t len{0};

            void parse_input();
            uint8_t calc_checksum(uint8_t *p_data, uint8_t length);
        };

    } // namespace hoermann
} // namespace esphome