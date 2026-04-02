#include "ble_service.h"

#include <errno.h>
#include <string.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/byteorder.h>

#include "rtc/rtc.h"
#include "thread/data_init_thread.h"

LOG_MODULE_REGISTER(ble_service, LOG_LEVEL_INF);

#define BT_UUID_MOTION_SVC_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abc0000)

#define BT_UUID_CHAR_ACCEL_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abc0004)

#define BT_UUID_CHAR_MAGN_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abc0005)

#define BT_UUID_CHAR_STEPS_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abc0006)

#define BT_UUID_CHAR_RTC_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abc0007)

#define BT_UUID_CHAR_HEADING_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abc0008)

#define BT_UUID_CHAR_DIRECTION_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abc0009)

#define ESS_TEMP_VAL_ATTR_IDX     2
#define ESS_HUM_VAL_ATTR_IDX      5
#define ESS_PRESS_VAL_ATTR_IDX    8
#define MOTION_ACCEL_VAL_ATTR_IDX 2
#define MOTION_MAGN_VAL_ATTR_IDX  6
#define MOTION_STEPS_VAL_ATTR_IDX 10
#define MOTION_RTC_VAL_ATTR_IDX   14
#define MOTION_HEADING_VAL_ATTR_IDX 18
#define MOTION_DIR_VAL_ATTR_IDX   22

enum ble_notify_bits {
    BLE_NOTIFY_TEMP  = BIT(0),
    BLE_NOTIFY_HUM   = BIT(1),
    BLE_NOTIFY_PRESS = BIT(2),
    BLE_NOTIFY_ACCEL = BIT(3),
    BLE_NOTIFY_MAGN  = BIT(4),
    BLE_NOTIFY_STEPS = BIT(5),
    BLE_NOTIFY_RTC   = BIT(6),
    BLE_NOTIFY_HEADING = BIT(7),
    BLE_NOTIFY_DIR   = BIT(8),
};

static struct bt_uuid_128 motion_svc_uuid = BT_UUID_INIT_128(BT_UUID_MOTION_SVC_VAL);
static struct bt_uuid_128 accel_uuid = BT_UUID_INIT_128(BT_UUID_CHAR_ACCEL_VAL);
static struct bt_uuid_128 magn_uuid = BT_UUID_INIT_128(BT_UUID_CHAR_MAGN_VAL);
static struct bt_uuid_128 steps_uuid = BT_UUID_INIT_128(BT_UUID_CHAR_STEPS_VAL);
static struct bt_uuid_128 rtc_uuid = BT_UUID_INIT_128(BT_UUID_CHAR_RTC_VAL);
static struct bt_uuid_128 heading_uuid = BT_UUID_INIT_128(BT_UUID_CHAR_HEADING_VAL);
static struct bt_uuid_128 direction_uuid = BT_UUID_INIT_128(BT_UUID_CHAR_DIRECTION_VAL);

static struct ble_sensor_data current_data;
static struct bt_conn *current_conn;
static uint32_t notify_mask;
static bool ble_started;

K_MUTEX_DEFINE(ble_state_mutex);

static void notify_attr(struct bt_conn *conn,
                        const struct bt_gatt_attr *attr,
                        const void *data, size_t len);

static int ble_parse_decimal(const char *text, size_t count)
{
    int value = 0;
    size_t i;

    for (i = 0; i < count; ++i) {
        if ((text[i] < '0') || (text[i] > '9')) {
            return -EINVAL;
        }
        value = (value * 10) + (text[i] - '0');
    }

    return value;
}

static bool ble_is_leap_year(int year)
{
    return ((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0));
}

static int ble_days_in_month(int year, int month)
{
    static const uint8_t days_per_month[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
    };

    if ((month < 1) || (month > 12)) {
        return -EINVAL;
    }

    if ((month == 2) && ble_is_leap_year(year)) {
        return 29;
    }

    return days_per_month[month - 1];
}

static int ble_parse_rtc_text(const char *text,
                              int *year,
                              int *month,
                              int *day,
                              int *hour,
                              int *minute,
                              int *second)
{
    int max_day;

    if ((text == NULL) || (year == NULL) || (month == NULL) || (day == NULL) ||
        (hour == NULL) || (minute == NULL) || (second == NULL)) {
        return -EINVAL;
    }

    if ((text[4] != '-') || (text[7] != '-') || (text[10] != ' ') ||
        (text[13] != ':') || (text[16] != ':') || (text[19] != '\0')) {
        return -EINVAL;
    }

    *year = ble_parse_decimal(&text[0], 4);
    *month = ble_parse_decimal(&text[5], 2);
    *day = ble_parse_decimal(&text[8], 2);
    *hour = ble_parse_decimal(&text[11], 2);
    *minute = ble_parse_decimal(&text[14], 2);
    *second = ble_parse_decimal(&text[17], 2);

    max_day = ble_days_in_month(*year, *month);
    if ((*year < 2000) || (max_day < 0) || (*day < 1) || (*day > max_day) ||
        (*hour < 0) || (*hour > 23) || (*minute < 0) || (*minute > 59) ||
        (*second < 0) || (*second > 59)) {
        return -EINVAL;
    }

    return 0;
}

static ssize_t write_rtc(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr,
                         const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    char rtc_text[BLE_RTC_TEXT_LEN];
    uint16_t copy_len;
    DataInitContext *ctx;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int err;

    ARG_UNUSED(flags);

    if (offset != 0U) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if ((buf == NULL) || (len < (BLE_RTC_TEXT_LEN - 1U)) || (len > BLE_RTC_TEXT_LEN)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    copy_len = len;
    if ((copy_len == BLE_RTC_TEXT_LEN) && (((const char *)buf)[BLE_RTC_TEXT_LEN - 1] == '\0')) {
        copy_len -= 1U;
    }

    if (copy_len != (BLE_RTC_TEXT_LEN - 1U)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    memcpy(rtc_text, buf, copy_len);
    rtc_text[copy_len] = '\0';

    err = ble_parse_rtc_text(rtc_text, &year, &month, &day, &hour, &minute, &second);
    if (err != 0) {
        LOG_WRN("BLE RTC write rejected: invalid format");
        return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }

    ctx = data_init_thread_get_context_mutable();
    if ((ctx == NULL) || !ctx->rtc_ready) {
        LOG_WRN("BLE RTC write rejected: RTC not ready");
        return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
    }

    err = watch_rtc_set(&ctx->rtc, year, month, day, hour, minute, second);
    if (err != 0) {
        LOG_ERR("BLE RTC write failed (err %d)", err);
        return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
    }

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    memcpy(current_data.rtc_text, rtc_text, sizeof(current_data.rtc_text));
    k_mutex_unlock(&ble_state_mutex);

    LOG_INF("RTC updated over BLE: %s", rtc_text);

    if ((conn != NULL) && ((notify_mask & BLE_NOTIFY_RTC) != 0U)) {
        notify_attr(conn, attr, rtc_text, copy_len);
    }

    return len;
}

static ssize_t read_temperature(struct bt_conn *conn,
                                const struct bt_gatt_attr *attr,
                                void *buf, uint16_t len, uint16_t offset)
{
    int16_t value;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    value = current_data.temperature;
    k_mutex_unlock(&ble_state_mutex);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static ssize_t read_humidity(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr,
                             void *buf, uint16_t len, uint16_t offset)
{
    uint16_t value;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    value = current_data.humidity;
    k_mutex_unlock(&ble_state_mutex);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static ssize_t read_pressure(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr,
                             void *buf, uint16_t len, uint16_t offset)
{
    uint32_t value;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    value = current_data.pressure;
    k_mutex_unlock(&ble_state_mutex);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static ssize_t read_accel(struct bt_conn *conn,
                          const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset)
{
    int16_t value[ARRAY_SIZE(current_data.accel)];

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    memcpy(value, current_data.accel, sizeof(value));
    k_mutex_unlock(&ble_state_mutex);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(value));
}

static ssize_t read_magn(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset)
{
    int16_t value[ARRAY_SIZE(current_data.magn)];

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    memcpy(value, current_data.magn, sizeof(value));
    k_mutex_unlock(&ble_state_mutex);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(value));
}

static ssize_t read_steps(struct bt_conn *conn,
                          const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset)
{
    uint32_t value;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    value = current_data.steps;
    k_mutex_unlock(&ble_state_mutex);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static ssize_t read_rtc(struct bt_conn *conn,
                        const struct bt_gatt_attr *attr,
                        void *buf, uint16_t len, uint16_t offset)
{
    char value[BLE_RTC_TEXT_LEN];
    size_t value_len;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    memcpy(value, current_data.rtc_text, sizeof(value));
    k_mutex_unlock(&ble_state_mutex);

    value_len = strnlen(value, sizeof(value));
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, value_len);
}

static ssize_t read_heading(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr,
                            void *buf, uint16_t len, uint16_t offset)
{
    uint16_t value;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    value = current_data.heading_tenths_deg;
    k_mutex_unlock(&ble_state_mutex);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static ssize_t read_direction(struct bt_conn *conn,
                              const struct bt_gatt_attr *attr,
                              void *buf, uint16_t len, uint16_t offset)
{
    char value[BLE_COMPASS_DIR_LEN];
    size_t value_len;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    memcpy(value, current_data.compass_direction, sizeof(value));
    k_mutex_unlock(&ble_state_mutex);

    value_len = strnlen(value, sizeof(value));
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, value_len);
}

static void set_notify_enabled(uint32_t bit, uint16_t value, const char *label)
{
    bool enabled = (value == BT_GATT_CCC_NOTIFY);
    bool changed;
    uint32_t new_mask;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    new_mask = enabled ? (notify_mask | bit) : (notify_mask & ~bit);
    changed = (new_mask != notify_mask);
    notify_mask = new_mask;
    k_mutex_unlock(&ble_state_mutex);

    if (changed) {
        LOG_INF("%s notifications %s", label, enabled ? "ON" : "OFF");
    }
}

static void ccc_temp_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    set_notify_enabled(BLE_NOTIFY_TEMP, value, "Temp");
}

static void ccc_hum_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    set_notify_enabled(BLE_NOTIFY_HUM, value, "Humidity");
}

static void ccc_press_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    set_notify_enabled(BLE_NOTIFY_PRESS, value, "Pressure");
}

static void ccc_accel_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    set_notify_enabled(BLE_NOTIFY_ACCEL, value, "Accel");
}

static void ccc_magn_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    set_notify_enabled(BLE_NOTIFY_MAGN, value, "Magn");
}

static void ccc_steps_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    set_notify_enabled(BLE_NOTIFY_STEPS, value, "Steps");
}

static void ccc_rtc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    set_notify_enabled(BLE_NOTIFY_RTC, value, "RTC");
}

static void ccc_heading_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    set_notify_enabled(BLE_NOTIFY_HEADING, value, "Heading");
}

static void ccc_direction_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    set_notify_enabled(BLE_NOTIFY_DIR, value, "Direction");
}

BT_GATT_SERVICE_DEFINE(ess_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),
    BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_temperature, NULL, NULL),
    BT_GATT_CCC(ccc_temp_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_humidity, NULL, NULL),
    BT_GATT_CCC(ccc_hum_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(BT_UUID_PRESSURE,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_pressure, NULL, NULL),
    BT_GATT_CCC(ccc_press_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

BT_GATT_SERVICE_DEFINE(motion_svc,
    BT_GATT_PRIMARY_SERVICE(&motion_svc_uuid),
    BT_GATT_CHARACTERISTIC(&accel_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_accel, NULL, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_GATT_CUD, BT_GATT_PERM_READ,
                       NULL, NULL, (void *)"Accelerometer XYZ (mg)"),
    BT_GATT_CCC(ccc_accel_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(&magn_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_magn, NULL, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_GATT_CUD, BT_GATT_PERM_READ,
                       NULL, NULL, (void *)"Magnetometer XYZ (mGauss)"),
    BT_GATT_CCC(ccc_magn_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(&steps_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_steps, NULL, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_GATT_CUD, BT_GATT_PERM_READ,
                       NULL, NULL, (void *)"Step Counter"),
    BT_GATT_CCC(ccc_steps_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(&rtc_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                           read_rtc, write_rtc, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_GATT_CUD, BT_GATT_PERM_READ,
                       NULL, NULL, (void *)"RTC Date Time"),
    BT_GATT_CCC(ccc_rtc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(&heading_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_heading, NULL, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_GATT_CUD, BT_GATT_PERM_READ,
                       NULL, NULL, (void *)"Compass Heading (0.1 deg)"),
    BT_GATT_CCC(ccc_heading_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(&direction_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_direction, NULL, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_GATT_CUD, BT_GATT_PERM_READ,
                       NULL, NULL, (void *)"Compass Direction"),
    BT_GATT_CCC(ccc_direction_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_ESS_VAL)),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
            sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
    struct bt_conn *new_conn;
    struct bt_conn *old_conn;

    if (err) {
        LOG_ERR("Connection failed (err %u)", err);
        return;
    }

    new_conn = bt_conn_ref(conn);
    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    old_conn = current_conn;
    current_conn = new_conn;
    k_mutex_unlock(&ble_state_mutex);

    if (old_conn != NULL) {
        bt_conn_unref(old_conn);
    }

    LOG_INF("BLE connected");
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    struct bt_conn *old_conn = NULL;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    if (current_conn == conn) {
        old_conn = current_conn;
        current_conn = NULL;
    }
    notify_mask = 0U;
    k_mutex_unlock(&ble_state_mutex);

    if (old_conn != NULL) {
        bt_conn_unref(old_conn);
    }

    LOG_INF("BLE disconnected (reason %u)", reason);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

static void notify_attr(struct bt_conn *conn,
                        const struct bt_gatt_attr *attr,
                        const void *data, size_t len)
{
    int err = bt_gatt_notify(conn, attr, data, len);
    if (err && err != -ENOTCONN) {
        LOG_WRN("Notify failed (err %d)", err);
    }
}

int ble_service_init(void)
{
    int err;
    bool already_started;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    already_started = ble_started;
    k_mutex_unlock(&ble_state_mutex);

    if (already_started) {
        return 0;
    }

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return err;
    }
    LOG_INF("Bluetooth initialized");

    err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad),
                          sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return err;
    }

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    memset(&current_data, 0, sizeof(current_data));
    notify_mask = 0U;
    ble_started = true;
    k_mutex_unlock(&ble_state_mutex);

    LOG_INF("BLE advertising started as \"%s\"", CONFIG_BT_DEVICE_NAME);
    return 0;
}

bool ble_service_is_connected(void)
{
    bool is_connected;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    is_connected = (current_conn != NULL);
    k_mutex_unlock(&ble_state_mutex);

    return is_connected;
}

bool ble_service_get_rssi_dbm(int8_t *out_rssi_dbm)
{
    struct bt_conn *conn = NULL;
    struct net_buf *cmd_buf;
    struct net_buf *rsp = NULL;
    struct bt_hci_cp_read_rssi *cp;
    struct bt_hci_rp_read_rssi *rp;
    uint16_t conn_handle;
    int err;
    bool ok = false;

    if (out_rssi_dbm == NULL) {
        return false;
    }

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    if (current_conn != NULL) {
        conn = bt_conn_ref(current_conn);
    }
    k_mutex_unlock(&ble_state_mutex);

    if (conn == NULL) {
        return false;
    }

    err = bt_hci_get_conn_handle(conn, &conn_handle);
    if (err != 0) {
        LOG_DBG("Failed to get BLE connection handle (err %d)", err);
        goto out;
    }

    cmd_buf = bt_hci_cmd_alloc(K_FOREVER);
    if (cmd_buf == NULL) {
        LOG_DBG("Failed to allocate HCI RSSI command buffer");
        goto out;
    }

    cp = net_buf_add(cmd_buf, sizeof(*cp));
    cp->handle = sys_cpu_to_le16(conn_handle);

    err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_RSSI, cmd_buf, &rsp);
    if (err != 0) {
        LOG_DBG("Failed to read BLE RSSI (err %d)", err);
        goto out;
    }

    if ((rsp == NULL) || (rsp->len < sizeof(*rp))) {
        LOG_DBG("BLE RSSI response too short");
        goto out;
    }

    rp = (struct bt_hci_rp_read_rssi *)rsp->data;
    if (rp->status != 0U || sys_le16_to_cpu(rp->handle) != conn_handle ||
        rp->rssi == BT_HCI_LE_RSSI_NOT_AVAILABLE) {
        goto out;
    }

    *out_rssi_dbm = rp->rssi;
    ok = true;

out:
    if (rsp != NULL) {
        net_buf_unref(rsp);
    }
    bt_conn_unref(conn);
    return ok;
}

void ble_service_update(const struct ble_sensor_data *data)
{
    struct ble_sensor_data snapshot;
    struct bt_conn *conn = NULL;
    uint32_t active_notify_mask;
    size_t rtc_len;
    size_t dir_len;

    k_mutex_lock(&ble_state_mutex, K_FOREVER);
    current_data = *data;
    snapshot = current_data;
    active_notify_mask = notify_mask;
    if (current_conn != NULL) {
        conn = bt_conn_ref(current_conn);
    }
    k_mutex_unlock(&ble_state_mutex);

    if (conn == NULL || active_notify_mask == 0U) {
        if (conn != NULL) {
            bt_conn_unref(conn);
        }
        return;
    }

    if ((active_notify_mask & BLE_NOTIFY_TEMP) != 0U) {
        notify_attr(conn, &ess_svc.attrs[ESS_TEMP_VAL_ATTR_IDX],
                    &snapshot.temperature, sizeof(snapshot.temperature));
    }

    if ((active_notify_mask & BLE_NOTIFY_HUM) != 0U) {
        notify_attr(conn, &ess_svc.attrs[ESS_HUM_VAL_ATTR_IDX],
                    &snapshot.humidity, sizeof(snapshot.humidity));
    }

    if ((active_notify_mask & BLE_NOTIFY_PRESS) != 0U) {
        notify_attr(conn, &ess_svc.attrs[ESS_PRESS_VAL_ATTR_IDX],
                    &snapshot.pressure, sizeof(snapshot.pressure));
    }

    if ((active_notify_mask & BLE_NOTIFY_ACCEL) != 0U) {
        notify_attr(conn, &motion_svc.attrs[MOTION_ACCEL_VAL_ATTR_IDX],
                    snapshot.accel, sizeof(snapshot.accel));
    }

    if ((active_notify_mask & BLE_NOTIFY_MAGN) != 0U) {
        notify_attr(conn, &motion_svc.attrs[MOTION_MAGN_VAL_ATTR_IDX],
                    snapshot.magn, sizeof(snapshot.magn));
    }

    if ((active_notify_mask & BLE_NOTIFY_STEPS) != 0U) {
        notify_attr(conn, &motion_svc.attrs[MOTION_STEPS_VAL_ATTR_IDX],
                    &snapshot.steps, sizeof(snapshot.steps));
    }

    rtc_len = strnlen(snapshot.rtc_text, sizeof(snapshot.rtc_text));
    if ((active_notify_mask & BLE_NOTIFY_RTC) != 0U && rtc_len > 0U) {
        notify_attr(conn, &motion_svc.attrs[MOTION_RTC_VAL_ATTR_IDX],
                    snapshot.rtc_text, rtc_len);
    }

    if ((active_notify_mask & BLE_NOTIFY_HEADING) != 0U) {
        notify_attr(conn, &motion_svc.attrs[MOTION_HEADING_VAL_ATTR_IDX],
                    &snapshot.heading_tenths_deg, sizeof(snapshot.heading_tenths_deg));
    }

    dir_len = strnlen(snapshot.compass_direction, sizeof(snapshot.compass_direction));
    if ((active_notify_mask & BLE_NOTIFY_DIR) != 0U && dir_len > 0U) {
        notify_attr(conn, &motion_svc.attrs[MOTION_DIR_VAL_ATTR_IDX],
                    snapshot.compass_direction, dir_len);
    }

    bt_conn_unref(conn);
}
