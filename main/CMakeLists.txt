# Staged bring-up: raise HEXNET_BUILD_STEP after each hardware test passes.
# 0 LED | 1 relay | 2 dimmer | 3 RGB | 4 level | 5 DHT | 6 motor | 7 WiFi | 8 BLE | 9 CAN | 99 full
set(HEXNET_BUILD_STEP 99)

set(HEXNET_STEP_SRCS "app_main.c")
set(HEXNET_STEP_REQUIRES driver)
set(HEXNET_STEP_INCLUDE ".")

if(HEXNET_BUILD_STEP GREATER_EQUAL 99)
    set(HEXNET_STEP_SRCS
        "TroykaDHT.cpp"
        "dht_sensor.cpp"
        "adc_monitor.c"
        "esp_ws28xx.c"
        "hexnet_canbus.c"
        "hexnet_bluetooth.c"
        "hexnet_resolve_mqtt.c"
        "hexnet_wifi_portal.c"
        "dns_server.c"
        "hexnet_debug.c"
        "hexnet_ota_debug.c"
        "hexnet_log_init.c"
        "hexnet_http.c"
        "hexnet_wifi.c"
        "hexnet_mqtt.c"
        "hexnet_board.c"
        "hexnet_io_profile.c"
        "hexnet_io_labels.c"
        "hexnet_io_notify.c"
        "hexnet_cjson_psram.c"
        "hexnet_panel_sync.c"
        "hexnet_version.c"
        "hexnet_motor.c"
        "hexnet_shift_register.c"
        "hexnet_rgb_strip.c"
        "hexnet_dim_pwm.c"
        "hexnet_sensors.c"
        "hexnet_app.c"
        "UserCodes/Ota/OtaManager.cpp"
        "app_main.c"
    )
    set(HEXNET_STEP_INCLUDE "." "UserCodes/Ota")
    set(HEXNET_STEP_REQUIRES
        esp_system
        nvs_flash
        esp_event
        esp_netif
        protocol_examples_common
        driver
        mqtt
        esp_http_client
        esp_http_server
        esp_https_ota
        app_update
        esp_adc
        bt
        json
    )
elseif(HEXNET_BUILD_STEP GREATER_EQUAL 7)
    # Steps 7–9: expand in later bring-up passes (WiFi/BLE/CAN).
    message(FATAL_ERROR "HEXNET_BUILD_STEP ${HEXNET_BUILD_STEP}: not wired yet. Use 1..6 or 99.")
elseif(HEXNET_BUILD_STEP GREATER_EQUAL 1)
    # Steps 1–6: self-contained staged app_main (no WiFi/BLE/MQTT).
    set(HEXNET_STEP_SRCS "app_main.c")
    set(HEXNET_STEP_REQUIRES driver)
    if(HEXNET_BUILD_STEP GREATER_EQUAL 2)
        list(APPEND HEXNET_STEP_REQUIRES esp_driver_ledc)
    endif()
    if(HEXNET_BUILD_STEP GREATER_EQUAL 4)
        list(APPEND HEXNET_STEP_REQUIRES esp_adc)
    endif()
endif()

idf_component_register(SRCS ${HEXNET_STEP_SRCS}
                    INCLUDE_DIRS ${HEXNET_STEP_INCLUDE}
                    REQUIRES ${HEXNET_STEP_REQUIRES})

idf_build_get_property(project_ver PROJECT_VER)
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    HEXNET_BUILD_STEP=${HEXNET_BUILD_STEP}
    PROJECT_VER="${project_ver}"
)

if(HEXNET_BUILD_STEP GREATER_EQUAL 99)
    target_add_binary_data(${COMPONENT_LIB} "hexnet_logo.png" BINARY)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        HEXNET_LOG_WIFI=1
        HEXNET_LOG_MQTT=1
        HEXNET_LOG_BLE=1
        HEXNET_LOG_OTA=1
    )
endif()

message(STATUS "HEXNET_BUILD_STEP=${HEXNET_BUILD_STEP}")
