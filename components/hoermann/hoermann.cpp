#include "hoermann.h"
#include "esphome/core/log.h"

namespace esphome
{
    namespace hoermann
    {
        static const char *const TAG = "hoermann";

        HoermannCover::HoermannCover(HoermannHub *hub)
            : hub_(hub)
        {
            if (hub_ != nullptr)
                hub_->set_cover(this);
        }

        HoermannSwitch::HoermannSwitch(HoermannHub *hub, switch_type_t type)
            : hub_(hub), type_(type)
        {
            if (hub_ == nullptr)
                return;
            if (type_ == Light)
                hub_->set_light(this);
            else if (type_ == Venting)
                hub_->set_venting(this);
        }
        HoermannBinarySensor::HoermannBinarySensor(HoermannHub *hub, sensor_type_t type)
            : hub_(hub), type_(type)
        {
            if (hub_ == nullptr)
                return;

            if (type_ == Error)
                hub_->set_error(this);
            else if (type_ == Prewarn)
                hub_->set_prewarn(this);
            else if (type_ == OptionRelay)
                hub_->set_option_relay(this);
        }
        void HoermannSwitch::write_state(bool state)
        {
            ESP_LOGD(TAG, "HoermannSwitch::write_state %02X", state);
            if (hub_ != nullptr)
            {
                switch (type_)
                {
                case Light:
                    ESP_LOGD(TAG, "write_state hoermann_action_toggle_light");
                    hub_->send_command(HoermannHub::hoermann_action_toggle_light);
                    break;
                case Venting:
                    ESP_LOGD(TAG, "write_state hoermann_action_venting");
                    hub_->send_command(HoermannHub::hoermann_action_venting);
                    break;
                }
            }
        }

        cover::CoverTraits HoermannCover::get_traits()
        {
            auto traits = cover::CoverTraits();
            traits.set_supports_stop(true);
            traits.set_supports_position(false);
            traits.set_supports_tilt(false);
            return traits;
        }

        void HoermannCover::control(const cover::CoverCall &call)
        {
            if (call.get_stop())
            {
                ESP_LOGD(TAG, "control hoermann_action_stop");
                this->hub_->send_command(HoermannHub::hoermann_action_stop); // Stop
            }
            else if (call.get_position().has_value())
            {
                float pos = *call.get_position();
                if (pos == cover::COVER_OPEN)
                {
                    ESP_LOGD(TAG, "control hoermann_action_open");
                    this->hub_->send_command(HoermannHub::hoermann_action_open); // Open
                }
                else if (pos == cover::COVER_CLOSED)
                {
                    ESP_LOGD(TAG, "control hoermann_action_close");
                    this->hub_->send_command(HoermannHub::hoermann_action_close); // Close
                }
            }
            this->publish_state();
        }

        void HoermannHub::loop()
        {
            ESP_LOGV(TAG, "loop begin");
            while (this->available() > 0)
            {
                const uint8_t data = this->read();
                ESP_LOGV(TAG, "received byte: 0x%02X", data);
                if ((data == 0x55) && (this->counter == 0))
                {
                    this->rx_buffer[this->counter++] = data;
                    this->len = 0;
                }
                else if (this->counter > 0)
                {
                    this->rx_buffer[this->counter++] = data;
                    if (this->counter == 3)
                    {
                        if (data < 16)
                        {
                            this->len = data + 4;
                        }
                        else
                        {
                            this->counter = 0;
                        }
                    }
                    else if (this->counter == this->len)
                    {
                        if (this->calc_checksum(this->rx_buffer, this->len - 1) == data)
                        {
                            this->parse_input();
                        }
                        else
                        {
                            ESP_LOGW(TAG, "failed calc_checksum");
                        }
                        this->counter = 0;
                    }
                }
            }
            ESP_LOGV(TAG, "loop end");
        }

        void HoermannHub::parse_input()
        {
            if (this->rx_buffer[1] == 0x00 && this->rx_buffer[2] == 0x02)
            {
                const uint8_t state_byte = this->rx_buffer[3];
                const uint8_t prewarn_byte = this->rx_buffer[4];

                if (this->cover_ != nullptr)
                {
                    if ((state_byte & 0x01) == 0x01)
                    {
                        ESP_LOGD(TAG, "cover::COVER_OPEN");
                        this->cover_->position = cover::COVER_OPEN;
                        this->cover_->current_operation = cover::COVER_OPERATION_IDLE;
                    }
                    else if ((state_byte & 0x02) == 0x02)
                    {
                        ESP_LOGD(TAG, "cover::COVER_CLOSED");
                        this->cover_->position = cover::COVER_CLOSED;
                        this->cover_->current_operation = cover::COVER_OPERATION_IDLE;
                    }
                    else if ((state_byte & 0x60) == 0x40)
                    {
                        ESP_LOGD(TAG, "cover::COVER_OPERATION_OPENING");
                        this->cover_->current_operation = cover::COVER_OPERATION_OPENING;
                    }
                    else if ((state_byte & 0x60) == 0x60)
                    {
                        ESP_LOGD(TAG, "cover::COVER_OPERATION_CLOSING");
                        this->cover_->current_operation = cover::COVER_OPERATION_CLOSING;
                    }
                    else
                    {
                        ESP_LOGD(TAG, "cover::COVER_OPERATION_IDLE");
                        this->cover_->current_operation = cover::COVER_OPERATION_IDLE;
                    }
                    ESP_LOGD(TAG, "cover publish_state");
                    this->cover_->publish_state();
                }

                if (this->light_ != nullptr)
                {
                    ESP_LOGD(TAG, "light state %02X", (state_byte & 0x08));
                    this->light_->publish_state((state_byte & 0x08) != 0);
                }
                if (this->venting_ != nullptr)
                {
                    ESP_LOGD(TAG, "venting state %02X", (state_byte & 0x80));
                    this->venting_->publish_state((state_byte & 0x80) != 0);
                }
                if (this->error_ != nullptr)
                {
                    ESP_LOGD(TAG, "error state %02X", (state_byte & 0x10));
                    this->error_->publish_state((state_byte & 0x10) != 0);
                }
                if (this->option_relay_ != nullptr)
                {
                    ESP_LOGD(TAG, "opton state %02X", (state_byte & 0x04));
                    this->option_relay_->publish_state((state_byte & 0x04) != 0);
                }
                if (this->prewarn_ != nullptr)
                {
                    ESP_LOGD(TAG, "prewarn_byte %02X", (prewarn_byte & 0x01));
                    this->prewarn_->publish_state((prewarn_byte & 0x01) != 0);
                }
            }
            else
            {
                ESP_LOGI(TAG, "parse_input condition not met, skipping");   
            }
        }

        void HoermannHub::send_command(hoermann_action_t action)
        {
            ESP_LOGD(TAG, "send_command action=%d", (uint8_t)action);

            uint8_t output_buffer[5];
            output_buffer[0] = 0x55;
            output_buffer[1] = 0x01;
            output_buffer[2] = 0x01;
            output_buffer[3] = (uint8_t)action;
            output_buffer[4] = output_buffer[0] + output_buffer[1] + output_buffer[2] + output_buffer[3];
            this->write_array(output_buffer, 5);
        }

        uint8_t HoermannHub::calc_checksum(uint8_t *p_data, uint8_t length)
        {
            uint8_t crc = 0;
            for (uint8_t i = 0; i < length; i++)
            {
                crc += p_data[i];
            }
            return crc;
        }

        void HoermannHub::dump_config()
        {
            ESP_LOGCONFIG(TAG, "Hoermann Garage Door Hub");
            if (this->cover_)   { ESP_LOGCONFIG(TAG, "  Cover: Configured"); }
            else                { ESP_LOGCONFIG(TAG, "  Cover: None"); }
            if (this->light_)   { ESP_LOGCONFIG(TAG, "  Light: Configured"); }
            else                { ESP_LOGCONFIG(TAG, "  Light: None"); }
            if (this->venting_) { ESP_LOGCONFIG(TAG, "  Venting: Configured"); }
            else                { ESP_LOGCONFIG(TAG, "  Venting: None"); }
            if (this->error_)   { ESP_LOGCONFIG(TAG, "  Error: Configured"); }
            else                { ESP_LOGCONFIG(TAG, "  Error: None"); }
            if (this->prewarn_) { ESP_LOGCONFIG(TAG, "  PreWarn: Configured"); }
            else                { ESP_LOGCONFIG(TAG, "  PreWarn: None"); }
            if (this->option_relay_) { ESP_LOGCONFIG(TAG, "  Option Relay: Configured"); }
            else                     { ESP_LOGCONFIG(TAG, "  Option Relay: None"); }
        }

    } // namespace hoermann
} // namespace esphome