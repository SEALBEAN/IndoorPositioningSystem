
#include "remote.h"

#define LOG_MODULE_NAME remote
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

/* Declarations */
static K_SEM_DEFINE(bt_init_ok, 0, 1);

//Lấy tên thiết bị từ prj.conf
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

//Advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)};

/*Response data
Khi central gửi scan request để yêu cầu kết nối thì phải nhận đc 
response để confirm kết nối */
static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_REMOTE_SERV_VAL),
};


/* Declarations */
static uint8_t button_value = 0;
enum bt_button_notifications_enabled notifications_enabled;
static struct bt_remote_service_cb remote_callbacks;

static ssize_t read_button_characteristic_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
void button_chrc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);
void on_sent(struct bt_conn *conn, void *user_data);
static ssize_t on_write(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);




/*  
    Khởi tạo 1 service*/
BT_GATT_SERVICE_DEFINE(remote_srv,
                       BT_GATT_PRIMARY_SERVICE(BT_UUID_REMOTE_SERVICE),

                       // Khai báo character đầu tiên
                       // Gồm UUID, properties, permission, callback khi read, callback khi write, user data(?)
                       // Dấu | là cho phép cả 2 (phép OR trong Logic)
                       //Ở đây thêm vào 2 properties Read và Notify vì Muốn đọc được giá trị của nút bên thiết bị
                       // Và muốn có thêm option được notify khi mà gtrị của nút được thay đổi

                       BT_GATT_CHARACTERISTIC(BT_UUID_REMOTE_BUTTON_CHRC,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                                              BT_GATT_PERM_READ,
                                              read_button_characteristic_cb, NULL, NULL),
                    //    Client Characteristic Configuration (CCC)
                    //    Thêm chức năng dành cho Client, truyền vào callbacks và các permission
                    //    Ở đây là chức năng nhận Notify khi giá trị của nút thay đổi
                           BT_GATT_CCC(button_chrc_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

                        // Khai báo character thứ 2
                        // Thêm vào khả năng ghi dữ liệu vào server từ client và không cần response từ server
                        // Có callbacks được chạy khi data được ghi vào server
                       BT_GATT_CHARACTERISTIC(BT_UUID_REMOTE_MESSAGE_CHRC,
                                              BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                                              BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                              NULL, on_write, NULL), );

/* Callbacks */

//Callbacks được chạy khi nút được nhấn và callback này chạy sau khi callback button_handler bên main.c 
static ssize_t read_button_characteristic_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                             void *buf, uint16_t len, uint16_t offset)
{
            //Đọc giá trị từ biến được truyền vào và ghi vào buffer
            //buffer ở đây sẽ được hiển thị lên trên app Nrf Connect
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &button_value,
                             sizeof(button_value));
}


void button_chrc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{

    //Nếu Notify được thay đổi trạng thái thì in Log
    //value được truyền vào nên chỉ cần kiểm tra xem nó đang ở với trường hợp nào Notify hay cái khác thôi
    bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_INF("Notifications %s", notif_enabled ? "enabled" : "disabled");

    
    notifications_enabled = notif_enabled ? BT_BUTTON_NOTIFICATIONS_ENABLED : BT_BUTTON_NOTIFICATIONS_DISABLED;

    //Check xem có bị NULL không
    if (remote_callbacks.notif_changed)
    {
        //Gọi và truyền tham số vào callbacks đã được trỏ tới bên hàm main 
        remote_callbacks.notif_changed(notifications_enabled);
    }
}

static ssize_t on_write(struct bt_conn *conn,
                        const struct bt_gatt_attr *attr,
                        const void *buf,
                        uint16_t len,
                        uint16_t offset,
                        uint8_t flags)
{
    LOG_INF("Received data, handle %d, conn %p",
            attr->handle, (void *)conn);

    // Check xem có bị NULL không
    if (remote_callbacks.data_received)
    {
        // Gọi và truyền tham số vào callbacks đã được trỏ tới bên hàm main
        remote_callbacks.data_received(conn, buf, len);
    }
    return len;
}

int send_button_notification(struct bt_conn *conn, char *value, uint16_t length)
{
    int err = 0;

    struct bt_gatt_notify_params params = {0};
    const struct bt_gatt_attr *attr = &remote_srv.attrs[2];

    params.attr = attr;
    params.data = &value;
    params.len = length;
    params.func = on_sent;

    err = bt_gatt_notify_cb(conn, &params);

    return err;
}

// Callback được gọi bởi hàm trên
void on_sent(struct bt_conn *conn, void *user_data)
{
    ARG_UNUSED(user_data);
    LOG_INF("Notification sent on connection %p", (void *)conn);
}

//Callbacks đơn giản để báo lỗi do yêu cầu của hàm bt_enable
void bt_ready(int err)
{
    if (err)
    {
        LOG_ERR("bt_enable returned %d", err);
    }
    LOG_INF("Semaphore cb value = %d", bt_init_ok.count);
    k_sem_give(&bt_init_ok);
    LOG_INF("Semaphore cb value = %d", bt_init_ok.count);
}

/* Custom functions */


//Lấy gtri biến local từ hàm gán vào biến global của file
void set_button_value(uint8_t btn_value)
{
    button_value = btn_value;
}

int bluetooth_init(struct bt_conn_cb *bt_cb, struct bt_remote_service_cb *remote_cb)
{
    LOG_INF("Initializing Bluetooth");

    int err;

    if (bt_cb == NULL || remote_cb == NULL)
    {
        return -NRFX_ERROR_NULL;
    }
    /*  Đăng ký callback để quan sát trạng thái kết nối của thiết bị
        Khi kết nối thành công hay thất bại sẽ chạy callback đó
        Ở đây là đăng ký 2 callbacks connected và disconnected 
        của biến tham chiếu bt_cb(bluetooth_callbacks bên hàm main)
    */
    bt_conn_cb_register(bt_cb);

    /*
        Chuyển biến cục bộ (remote_cb) trong hàm thành biến toàn cục trong file (remote_callbacks)
        Gán dựa trên con trỏ nên dữ liệu luôn sync với nhau
        Ở đây là gán 2 hàm callbacks vào 2 att của remote_callbacks
    */
    remote_callbacks.notif_changed = remote_cb->notif_changed;
    remote_callbacks.data_received = remote_cb-> data_received;


    //Bật bluetooth và báo lỗi nếu có
    err = bt_enable(bt_ready);
    if (err)
    {
        LOG_ERR("bt_enable returned %d", err);
        return err;
    }
    k_sem_take(&bt_init_ok, K_FOREVER);
    LOG_INF("Semaphore value = %d", bt_init_ok.count);

    //Bắt đầu advertising  
                        //Param của adv   adv data           scan response data
    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err)
    {
        LOG_ERR("couldn't start advertising (err = %d", err);
        return err;
    }

    return err;
}