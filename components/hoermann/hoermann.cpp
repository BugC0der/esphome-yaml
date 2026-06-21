#include "hoermann.h"
#include "esphome/core/log.h"

namespace esphome
{
    namespace hoermann
    {

        static const char *const TAG = "hoermann";

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
                this->hub_->send_command(0x03); // Stop
            }
            else if (call.get_position().has_value())
            {
                float pos = *call.get_position();
                if (pos == cover::COVER_OPEN)
                {
                    this->hub_->send_command(0x01); // Open
                }
                else if (pos == cover::COVER_CLOSED)
                {
                    this->hub_->send_command(0x02); // Close
                }
            }
            this->publish_state();
        }

        void HoermannHub::loop()
        {
            while (this->available() > 0)
            {
                uint8_t data = this->read();
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
                        this->counter = 0;
                    }
                }
            }
        }

        void HoermannHub::parse_input()
        {
            if (this->rx_buffer[1] == 0x00 && this->rx_buffer[2] == 0x02)
            {
                uint8_t state_byte = this->rx_buffer[3];
                uint8_t prewarn_byte = this->rx_buffer[4];

                if (this->cover_ != nullptr)
                {
                    if ((state_byte & 0x01) == 0x01)
                    {
                        this->cover_->position = cover::COVER_OPEN;
                        this->cover_->current_operation = cover::COVER_OPERATION_IDLE;
                    }
                    else if ((state_byte & 0x02) == 0x02)
                    {
                        this->cover_->position = cover::COVER_CLOSED;
                        this->cover_->current_operation = cover::COVER_OPERATION_IDLE;
                    }
                    else if ((state_byte & 0x60) == 0x40)
                    {
                        this->cover_->current_operation = cover::COVER_OPERATION_OPENING;
                    }
                    else if ((state_byte & 0x60) == 0x60)
                    {
                        this->cover_->current_operation = cover::COVER_OPERATION_CLOSING;
                    }
                    else
                    {
                        this->cover_->current_operation = cover::COVER_OPERATION_IDLE;
                    }
                    this->cover_->publish_state();
                }

                if (this->light_ != nullptr)
                    this->light_->publish_state((state_byte & 0x08) != 0);
                if (this->error_ != nullptr)
                    this->error_->publish_state((state_byte & 0x10) != 0);
                if (this->venting_ != nullptr)
                    this->venting_->publish_state((state_byte & 0x80) != 0);
                if (this->option_relay_ != nullptr)
                    this->option_relay_->publish_state((state_byte & 0x04) != 0);
                if (this->prewarn_ != nullptr)
                    this->prewarn_->publish_state((prewarn_byte & 0x01) != 0);
            }
        }

        void HoermannHub::send_command(uint8_t action)
        {
            uint8_t output_buffer[5];
            output_buffer[0] = 0x55;
            output_buffer[1] = 0x01;
            output_buffer[2] = 0x01;
            output_buffer[3] = action;
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
        }

    } // namespace hoermann
} // namespace esphome