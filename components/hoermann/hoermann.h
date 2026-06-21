#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome
{
    namespace hoermann
    {

        class HoermannHub;

        class HoermannCover : public cover::Cover, public Component
        {
        public:
            void set_hub(HoermannHub *hub) { this->hub_ = hub; }
            void control(const cover::CoverCall &call) override;
            cover::CoverTraits get_traits() override;

        protected:
            HoermannHub *hub_{nullptr};
        };

        class HoermannHub : public Component, public uart::UARTDevice
        {
        public:
            void set_cover(HoermannCover *cover) { this->cover_ = cover; }
            void set_light(binary_sensor::BinarySensor *light) { this->light_ = light; }
            void set_error(binary_sensor::BinarySensor *error) { this->error_ = error; }
            void set_venting(binary_sensor::BinarySensor *venting) { this->venting_ = venting; }
            void set_prewarn(binary_sensor::BinarySensor *prewarn) { this->prewarn_ = prewarn; }
            void set_option_relay(binary_sensor::BinarySensor *option_relay) { this->option_relay_ = option_relay; }

            void setup() override {}
            void loop() override;
            void dump_config() override;
            float get_setup_priority() const override { return setup_priority::DATA; }

            void send_command(uint8_t action);

        protected:
            HoermannCover *cover_{nullptr};
            binary_sensor::BinarySensor *light_{nullptr};
            binary_sensor::BinarySensor *error_{nullptr};
            binary_sensor::BinarySensor *venting_{nullptr};
            binary_sensor::BinarySensor *prewarn_{nullptr};
            binary_sensor::BinarySensor *option_relay_{nullptr};

            uint8_t rx_buffer[32];
            uint8_t counter{0};
            uint8_t len{0};

            void parse_input();
            uint8_t calc_checksum(uint8_t *p_data, uint8_t length);
        };

    } // namespace hoermann
} // namespace esphome