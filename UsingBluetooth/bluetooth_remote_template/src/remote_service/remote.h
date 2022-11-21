
#include <zephyr.h>
#include <logging/log.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>

// Thay UUID = UUID 16bit của SIG-adopted
// https://btprodspecificationrefs.blob.core.windows.net/assigned-values/16-bit%20UUID%20Numbers%20Document.pdf
// #define BT_UUID_REMOTE_SERV_VAL \
//     BT_UUID_16(0x1821)

#define BT_UUID_REMOTE_SERV_VAL \
    BT_UUID_128_ENCODE(0xe9ea0001, 0xe19b, 0x482d, 0x9293, 0xc7907585fc48)
#define BT_UUID_REMOTE_BUTTON_CHRC_VAL \
    BT_UUID_128_ENCODE(0xe9ea0002, 0xe19b, 0x482d, 0x9293, 0xc7907585fc48)

                                            // 128 hay 16 tùy theo UUID đã khai báo ở trên
#define BT_UUID_REMOTE_SERVICE       BT_UUID_DECLARE_128(BT_UUID_REMOTE_SERV_VAL)
#define BT_UUID_REMOTE_BUTTON_CHRC   BT_UUID_DECLARE_128(BT_UUID_REMOTE_BUTTON_CHRC_VAL)

/** @brief UUID of the Message Characteristic. **/
#define BT_UUID_REMOTE_MESSAGE_CHRC_VAL \
    BT_UUID_128_ENCODE(0xe9ea0003, 0xe19b, 0x482d, 0x9293, 0xc7907585fc48)

#define BT_UUID_REMOTE_MESSAGE_CHRC BT_UUID_DECLARE_128(BT_UUID_REMOTE_MESSAGE_CHRC_VAL)

enum bt_button_notifications_enabled
{
    BT_BUTTON_NOTIFICATIONS_ENABLED,
    BT_BUTTON_NOTIFICATIONS_DISABLED,
};

struct bt_remote_service_cb
{
    void (*notif_changed)(enum bt_button_notifications_enabled status);
    void (*data_received)(struct bt_conn *conn, const uint8_t *const data, uint16_t len);
};

int send_button_notification(struct bt_conn *conn, char *value, uint16_t length);
void set_button_value(uint8_t btn_value);
int bluetooth_init(struct bt_conn_cb *bt_cb, struct bt_remote_service_cb *remote_cb);


