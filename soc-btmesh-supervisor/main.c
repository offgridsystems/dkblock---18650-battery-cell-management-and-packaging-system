/***********************************************************************************************//**
 * \file   main.c
 * \brief  Supervisor Main code
 *
 * 11/27/18 Make it do something
 *
 *
 *
 *
 *
 * This example demonstrates the bare minimum needed for a Blue Gecko BT Mesh C application.
 * The application starts unprovisioned Beaconing after boot
 ***************************************************************************************************
 * <b> (C) Copyright 2017 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* C Standard Library headers */
#include <stdlib.h>
#include <stdio.h>

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"
#include "retargetserial.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include <gecko_configuration.h>
#include "mesh_generic_model_capi_types.h"
#include "mesh_lib.h"
#include <mesh_sizes.h>

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include <em_gpio.h>

/* Device initialization header */
#include "hal-config.h"

/* Display Interface header */
#include "display_interface.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

// Maximum number of simultaneous Bluetooth connections
#define MAX_CONNECTIONS 2

// heap for Bluetooth stack
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS) + BTMESH_HEAP_SIZE + 1760];

// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;

// Bluetooth advertisement set configuration
//
// At minimum the following is required:
// * One advertisement set for Bluetooth LE stack (handle number 0)
// * One advertisement set for Mesh data (handle number 1)
// * One advertisement set for Mesh unprovisioned beacons (handle number 2)
// * One advertisement set for Mesh unprovisioned URI (handle number 3)
// * N advertisement sets for Mesh GATT service advertisements
// (one for each network key, handle numbers 4 .. N+3)
//
#define MAX_ADVERTISERS (4 + MESH_CFG_MAX_NETKEYS)

// bluetooth stack configuration
extern const struct bg_gattdb_def bg_gattdb_data;

const gecko_configuration_t config =
{
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.max_advertisers = MAX_ADVERTISERS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap) - BTMESH_HEAP_SIZE,
  .bluetooth.sleep_clock_accuracy = 100,
  .gattdb = &bg_gattdb_data,
  .btmesh_heap_size = BTMESH_HEAP_SIZE,
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .max_timers = 16,
};

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);

void mesh_native_bgapi_init(void);

bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

/*
 * block status request - request for block to update the block managers data about specific block
 */
static void block_status_request(uint16_t model_id,
                              	 uint16_t element_index,
								 uint16_t client_addr,
								 uint16_t server_addr,
								 uint16_t appkey_index,
								 const struct mesh_generic_request *request,
								 uint32_t transition_ms,
								 uint16_t delay_ms,
								 uint8_t request_flags)
{
	char buf[30];
	sprintf(buf, "%x", client_addr);
	DI_Print(buf, 1);
	sprintf(buf, "ob_ntc1: %x", request->block_temp.temp_ob_ntc1);
	DI_Print(buf, 2);


}

static void block_status_change(uint16_t model_id,
                                uint16_t element_index,
								const struct mesh_generic_state *current,
								const struct mesh_generic_state *target,
								uint32_t remaining_ms)
{

}

/**
 * Initialization of the models supported by this node. This function registers callbacks for
 * each of the supported models.
 */
static void init_models(void)
{
  mesh_lib_generic_server_register_handler(MESH_DK_BLOCK_STATUS_SERVER_MODEL_ID,
                                           0,
                                           block_status_request,
                                           block_status_change);
}


int main()
{
  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  // Initialize application
  initApp();

  gecko_stack_init(&config);
  gecko_bgapi_class_dfu_init();
  gecko_bgapi_class_system_init();
  gecko_bgapi_class_le_gap_init();
  gecko_bgapi_class_le_connection_init();
  gecko_bgapi_class_gatt_init();
  gecko_bgapi_class_gatt_server_init();
  gecko_bgapi_class_endpoint_init();
  gecko_bgapi_class_hardware_init();
  gecko_bgapi_class_flash_init();
  gecko_bgapi_class_test_init();
  gecko_bgapi_class_sm_init();
  mesh_native_bgapi_init();

  gecko_initCoexHAL();
  RETARGET_SerialInit();

  DI_Init();

  while (1) {
    struct gecko_cmd_packet *evt = gecko_wait_event();
    bool pass = mesh_bgapi_listener(evt);
    if (pass) {
      handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
    }
  }
}

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{
  switch (evt_id) {
    case gecko_evt_dfu_boot_id:
      //gecko_cmd_le_gap_set_advertising_timing(0, 1000*adv_interval_ms/625, 1000*adv_interval_ms/625, 0, 0);
      gecko_cmd_le_gap_set_mode(2, 2);
      break;
    case gecko_evt_system_boot_id:
      // Initialize Mesh stack in Node operation mode, wait for initialized event
      gecko_cmd_mesh_node_init();
      break;
    case gecko_evt_mesh_node_initialized_id:
      // The Node is now initialized, start unprovisioned Beaconing using PB-Adv Bearer
      gecko_cmd_mesh_node_start_unprov_beaconing(0x1);
      break;
    case gecko_evt_le_connection_closed_id:
      /* Check if need to boot to dfu mode */
      if (boot_to_dfu) {
        /* Enter to DFU OTA mode */
        gecko_cmd_system_reset(2);
      }
      break;
    case gecko_evt_gatt_server_user_write_request_id:
      if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
        /* Set flag to enter to OTA mode */
        boot_to_dfu = 1;
        /* Send response to Write Request */
        gecko_cmd_gatt_server_send_user_write_response(
          evt->data.evt_gatt_server_user_write_request.connection,
          gattdb_ota_control,
          bg_err_success);

        /* Close connection to enter to DFU OTA mode */
        gecko_cmd_le_connection_close(evt->data.evt_gatt_server_user_write_request.connection);
      }
      break;
    default:
      break;
  }
}
