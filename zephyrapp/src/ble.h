int ble_main(void);
void ble_update();
void ble_notify_sensors(void);
void ble_refresh_adv(void);
void update_bthome_adv_data(void);
void ble_broadcast_bthome(void);

extern bool ble_ok;
extern bool ble_connected;
