#include "debug.h"

#include "fnSystem.h"
#include "fnWiFi.h"
#include "fnFsSD.h"
#include "fnFsSPIF.h"
#include "fnConfig.h"
#include "keys.h"
#include "led.h"
#include "sio.h"
#include "interface.h" // c64
#include "fuji.h"
#include "modem.h"
#include "apetime.h"
#include "voice.h"
#include "httpService.h"
#include "printerlist.h"
#include "midimaze.h"
#include "siocpm.h"

#include <esp_system.h>
#include <nvs_flash.h>

#include <esp32/spiram.h>
#include <esp32/himem.h>

#ifdef BLUETOOTH_SUPPORT
#include "fnBluetooth.h"
#endif

// fnSystem is declared and defined in fnSystem.h/cpp
// fnBtManager is declared and defined in fnBluetooth.h/cpp
// fnLedManager is declared and defined in led.h/cpp
// fnKeyManager is declared and defined in keys.h/cpp
// fnHTTPD is declared and defineid in HttpService.h/cpp

// sioFuji theFuji; // moved to fuji.h/.cpp
sioApeTime apeTime;
sioVoice sioV;
sioMIDIMaze sioMIDI;
// sioCassette sioC; // now part of sioFuji theFuji object
sioModem *sioR;
sioCPM sioZ;

#define DEVICE_MASK 0b00000000000000000000111100000000 //  Devices 8-11
Interface drive;

void main_shutdown_handler()
{
    Debug_println("Shutdown handler called");
    // Give devices an opportunity to clean up before rebooting
    // SIO.shutdown();
}

// Initial setup
void main_setup()
{
#ifdef DEBUG
    fnUartDebug.begin(DEBUG_SPEED);
    unsigned long startms = fnSystem.millis();
    Debug_printf("\n\n--~--~--~--\nFujiNet %s Started @ %lu\n", fnSystem.get_fujinet_version(), startms);
    Debug_printf("Starting heap: %u\n", fnSystem.get_free_heap_size());
    Debug_printf("PsramSize %u\n", fnSystem.get_psram_size());
    Debug_printf("himem phys %u\n", esp_himem_get_phys_size());
    Debug_printf("himem free %u\n", esp_himem_get_free_size());
    Debug_printf("himem reserved %u\n", esp_himem_reserved_area_size());
#endif
    // Install a reboot handler
    esp_register_shutdown_handler(main_shutdown_handler);

    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        Debug_println("Erasing flash");
        ESP_ERROR_CHECK(nvs_flash_erase());
        e = nvs_flash_init();
    }
    ESP_ERROR_CHECK(e);

    fnKeyManager.setup();
    fnLedManager.setup();

    fnSPIFFS.start();
    fnSDFAT.start();

    // Load our stored configuration
    Config.load();

    if ( Config.get_bt_status() )
    {
        // Start SIO2BT mode if we were in it last shutdown
        fnLedManager.set(eLed::LED_BT, true); // BT LED ON
        fnBtManager.start();
    }
    else
    {
        // Set up the WiFi adapter
        fnWiFi.start();
        // Go ahead and try reconnecting to WiFi
        fnWiFi.connect();
    }

    theFuji.setup(&SIO);
    SIO.addDevice(&theFuji, SIO_DEVICEID_FUJINET); // the FUJINET!

    SIO.addDevice(&apeTime, SIO_DEVICEID_APETIME); // APETime

    SIO.addDevice(&sioMIDI, SIO_DEVICEID_MIDI); // MIDIMaze

    // Create a new printer object, setting its output depending on whether we have SD or not
    FileSystem *ptrfs = fnSDFAT.running() ? (FileSystem *)&fnSDFAT : (FileSystem *)&fnSPIFFS;
    sioPrinter::printer_type ptype = Config.get_printer_type(0);
    if (ptype == sioPrinter::printer_type::PRINTER_INVALID)
        ptype = sioPrinter::printer_type::PRINTER_FILE_TRIM;

    Debug_printf("Creating a default printer using %s storage and type %d\n", ptrfs->typestring(), ptype);

    sioPrinter *ptr = new sioPrinter(ptrfs, ptype);
    fnPrinters.set_entry(0, ptr, ptype, Config.get_printer_port(0));

    SIO.addDevice(ptr, SIO_DEVICEID_PRINTER + fnPrinters.get_port(0)); // P:

    sioR = new sioModem(ptrfs, false); // turned off by default.
    
    SIO.addDevice(sioR, SIO_DEVICEID_RS232); // R:

    SIO.addDevice(&sioV, SIO_DEVICEID_FN_VOICE); // P3:

    SIO.addDevice(&sioZ, SIO_DEVICEID_CPM); // (ATR8000 CPM)

    // Go setup SIO
    SIO.setup();

#ifdef DEBUG
    unsigned long endms = fnSystem.millis();
    Debug_printf("Available heap: %u\nSetup complete @ %lu (%lums)\n", fnSystem.get_free_heap_size(), endms, endms - startms);
#endif
}

void cbm_setup()
{

#ifdef DEBUG
    fnUartDebug.begin(DEBUG_SPEED);
    unsigned long startms = fnSystem.millis();
    Debug_printf("\n\n--~--~--~--\nFujiNet %s Started @ %lu\n", fnSystem.get_fujinet_version(), startms);
    Debug_printf("\nIEC Version!\n\n");
    Debug_printf("Starting heap: %u\n", fnSystem.get_free_heap_size());
    Debug_printf("PsramSize %u\n", fnSystem.get_psram_size());
    Debug_printf("himem phys %u\n", esp_himem_get_phys_size());
    Debug_printf("himem free %u\n", esp_himem_get_free_size());
    Debug_printf("himem reserved %u\n", esp_himem_reserved_area_size());
#endif
    // Install a reboot handler
    esp_register_shutdown_handler(main_shutdown_handler);

    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        Debug_println("Erasing flash");
        ESP_ERROR_CHECK(nvs_flash_erase());
        e = nvs_flash_init();
    }
    ESP_ERROR_CHECK(e);

    fnKeyManager.setup();
    fnLedManager.setup();

    fnSPIFFS.start();
    fnSDFAT.start();

    // Load our stored configuration
    Config.load();

    {
        // Set up the WiFi adapter
        fnWiFi.start();
        // Go ahead and try reconnecting to WiFi
        fnWiFi.connect();
    }

    // Go setup IEC
    iec.enabledDevices = DEVICE_MASK;
    iec.init();
    Debug_println("IEC Bus Initialized");

    drive.begin(iec, &fnSDFAT);
    Debug_print("Virtual Device(s) Started: [ ");
    for (int i = 0; i < 31; i++)
    {
        if (iec.isDeviceEnabled(i))
        {
            Debug_printf("%.02d ", i);
        }
    }
    Debug_println("]");

#ifdef DEBUG
    unsigned long endms = fnSystem.millis();
    Debug_printf("Available heap: %u\nSetup complete @ %lu (%lums)\n", fnSystem.get_free_heap_size(), endms, endms - startms);
#endif
}

// Main high-priority service loop
void fn_service_loop(void *param)
{
    bool c = DIGI_LOW;
    while (true)
    {
        // We don't have any delays in this loop, so IDLE threads will be starved
        // Shouldn't be a problem, but something to keep in mind...
        // Go service BT if it's active
#ifdef BLUETOOTH_SUPPORT
        if (fnBtManager.isActive())
            fnBtManager.service();
        else
#endif

            //     fnSystem.set_pin_mode(22, gpio_mode_t::GPIO_MODE_OUTPUT);
            // for (int i = 0; i < 100; i++)
            // {
            //     fnSystem.digital_write(22, c);
            //     fnSystem.delay_microseconds(100);
            //     c = not c;
            // }

            // THIS IS WHERE WE CAN SELECT THE HOST MACHINE
            //    SIO.service();
            drive.loop();
    }
}

/*
* This is the start/entry point for an ESP-IDF program (must use "C" linkage)
*/
extern "C"
{
    void app_main()
    {
        bool c = DIGI_LOW;
        fnSystem.set_pin_mode(22, gpio_mode_t::GPIO_MODE_OUTPUT);
        fnSystem.digital_write(22, DIGI_LOW);
        fnSystem.digital_write(22, DIGI_HIGH);
        fnSystem.digital_write(22, DIGI_LOW);
        for (int i = 0; i < 100; i++)
        {
            fnSystem.digital_write(22, c);
            fnSystem.delay_microseconds(10);
            c = not c;
        }

        // Call our setup routine
        cbm_setup();
        
        //main_setup();

        // Create a new high-priority task to handle the main loop
        // This is assigned to CPU1; the WiFi task ends up on CPU0
        #define MAIN_STACKSIZE 8192
        #define MAIN_PRIORITY 10
        #define MAIN_CPUAFFINITY 1
        xTaskCreatePinnedToCore(fn_service_loop, "fnLoop",
            MAIN_STACKSIZE, nullptr, MAIN_PRIORITY, nullptr, MAIN_CPUAFFINITY);
            
        // Sit here twiddling our thumbs
        while (true)
            vTaskDelay(9000 / portTICK_PERIOD_MS);
    }
}
