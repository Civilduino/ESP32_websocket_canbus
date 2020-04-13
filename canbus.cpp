#include "canbus.h"

void CAN_begin(void)
{
    //Initialize configuration structures using macro initializers (changed GPIO type in esp idf library)
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(GPIO_NUM_22, GPIO_NUM_23, CAN_MODE_NORMAL);
    can_timing_config_t t_config = CAN_TIMING_CONFIG_25KBITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    //Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        printf("Driver installed\n");
    }
    else
    {
        printf("Failed to install driver\n");
        return;
    }

    //Start CAN driver
    if (can_start() == ESP_OK)
    {
        printf("Driver started\n");
    }
    else
    {
        printf("Failed to start driver\n");
        return;
    }
}

void CAN_read(void)
{
    can_status_info_t can_status;
    can_message_t message;

    //Check for received messages
    can_get_status_info(&can_status);
    if (can_status.msgs_to_rx >= 1)
    {
        if (can_receive(&message, pdMS_TO_TICKS(10)) == ESP_OK)
        {
            printf("Message received\n");
        }
        else
        {
            printf("Failed to receive message\n");
            return;
        }

        //Process received message
        printf("ID is %d\n", message.identifier);
        for (int i = 0; i < message.data_length_code; i++)
        {
            printf("Data byte %d = %d\n", i, message.data[i]);
        }
    }
}