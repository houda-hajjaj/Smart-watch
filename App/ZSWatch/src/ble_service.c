#include "ble_service.h"
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(ble_service, LOG_LEVEL_INF);

/* ================================================================
 *  UUIDs standards BLE SIG (nRF Connect les décode automatiquement)
 * ================================================================
 *  Service : Environmental Sensing (0x181A)
 *  Chars   : Temperature (0x2A6E), Humidity (0x2A6F), Pressure (0x2A6D)
 *
 *  Service custom : mouvement 12345678-1234-5678-1234-56789abc0000
 *  Chars   : Accel (…0004), Magn (…0005), Steps (…0006)
 * ================================================================ */

/* ── UUIDs custom pour mouvement ──────────────────────────────── */
#define BT_UUID_MOTION_SVC_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abc0000)

#define BT_UUID_CHAR_ACCEL_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abc0004)

#define BT_UUID_CHAR_MAGN_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abc0005)

#define BT_UUID_CHAR_STEPS_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abc0006)

static struct bt_uuid_128 motion_svc_uuid = BT_UUID_INIT_128(BT_UUID_MOTION_SVC_VAL);
static struct bt_uuid_128 accel_uuid      = BT_UUID_INIT_128(BT_UUID_CHAR_ACCEL_VAL);
static struct bt_uuid_128 magn_uuid       = BT_UUID_INIT_128(BT_UUID_CHAR_MAGN_VAL);
static struct bt_uuid_128 steps_uuid      = BT_UUID_INIT_128(BT_UUID_CHAR_STEPS_VAL);

/* ================================================================
 *  Données internes
 * ================================================================ */
static struct ble_sensor_data current_data;
static struct bt_conn *current_conn;

/* ── Flags notification (CCC) ─────────────────────────────────── */
static bool notify_temp_enabled;
static bool notify_hum_enabled;
static bool notify_press_enabled;
static bool notify_accel_enabled;
static bool notify_magn_enabled;
static bool notify_steps_enabled;

/* ================================================================
 *  Lecture GATT – callbacks
 * ================================================================ */

/* ── Environmental Sensing chars ──────────────────────────────── */
static ssize_t read_temp(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                             &current_data.temperature,
                             sizeof(current_data.temperature));
}

static ssize_t read_hum(struct bt_conn *conn,
                        const struct bt_gatt_attr *attr,
                        void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                             &current_data.humidity,
                             sizeof(current_data.humidity));
}

static ssize_t read_press(struct bt_conn *conn,
                          const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                             &current_data.pressure,
                             sizeof(current_data.pressure));
}

/* ── Motion chars ─────────────────────────────────────────────── */
static ssize_t read_accel(struct bt_conn *conn,
                          const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                             &current_data.accel,
                             sizeof(current_data.accel));
}

static ssize_t read_magn(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                             &current_data.magn,
                             sizeof(current_data.magn));
}

static ssize_t read_steps(struct bt_conn *conn,
                          const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                             &current_data.steps,
                             sizeof(current_data.steps));
}

/* ── CCC changed callbacks ────────────────────────────────────── */
static void ccc_temp_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    notify_temp_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_INF("Temp notifications %s", notify_temp_enabled ? "ON" : "OFF");
}

static void ccc_hum_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    notify_hum_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_INF("Humidity notifications %s", notify_hum_enabled ? "ON" : "OFF");
}

static void ccc_press_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    notify_press_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_INF("Pressure notifications %s", notify_press_enabled ? "ON" : "OFF");
}

static void ccc_accel_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    notify_accel_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_INF("Accel notifications %s", notify_accel_enabled ? "ON" : "OFF");
}

static void ccc_magn_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    notify_magn_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_INF("Magn notifications %s", notify_magn_enabled ? "ON" : "OFF");
}

static void ccc_steps_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    notify_steps_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_INF("Steps notifications %s", notify_steps_enabled ? "ON" : "OFF");
}

/* ================================================================
 *  Service 1 : Environmental Sensing (UUID standard 0x181A)
 *  → nRF Connect affiche automatiquement "Temperature", "Humidity",
 *    "Pressure" avec les bonnes unités.
 * ================================================================ */
BT_GATT_SERVICE_DEFINE(ess_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),

    /* Temperature – UUID 0x2A6E – int16, résolution 0.01 °C */
    BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_temp, NULL, NULL),
    BT_GATT_CCC(ccc_temp_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    /* Humidity – UUID 0x2A6F – uint16, résolution 0.01 % */
    BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_hum, NULL, NULL),
    BT_GATT_CCC(ccc_hum_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    /* Pressure – UUID 0x2A6D – uint32, résolution 0.1 Pa */
    BT_GATT_CHARACTERISTIC(BT_UUID_PRESSURE,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_press, NULL, NULL),
    BT_GATT_CCC(ccc_press_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

/* ================================================================
 *  Service 2 : Motion (UUID custom) avec User Description
 *  → Les descripteurs permettent à nRF Connect d'afficher
 *    le nom de chaque caractéristique.
 * ================================================================ */
BT_GATT_SERVICE_DEFINE(motion_svc,
    BT_GATT_PRIMARY_SERVICE(&motion_svc_uuid),

    /* Accéléromètre (3 × int16, mg) */
    BT_GATT_CHARACTERISTIC(&accel_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_accel, NULL, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_GATT_CUD, BT_GATT_PERM_READ,
                        NULL, NULL, (void *)"Accelerometer XYZ (mg)"),
    BT_GATT_CCC(ccc_accel_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    /* Magnétomètre (3 × int16, mGauss) */
    BT_GATT_CHARACTERISTIC(&magn_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_magn, NULL, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_GATT_CUD, BT_GATT_PERM_READ,
                        NULL, NULL, (void *)"Magnetometer XYZ (mGauss)"),
    BT_GATT_CCC(ccc_magn_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    /* Compteur de pas (uint32) */
    BT_GATT_CHARACTERISTIC(&steps_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_steps, NULL, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_GATT_CUD, BT_GATT_PERM_READ,
                        NULL, NULL, (void *)"Step Counter"),
    BT_GATT_CCC(ccc_steps_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

/* ================================================================
 *  Advertising – annonce ESS standard pour que les scanners
 *  sachent qu'on propose des données environnementales.
 * ================================================================ */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS,
                  BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
                  BT_UUID_16_ENCODE(BT_UUID_ESS_VAL)),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, "Smart Watch Sensor Hub",
            sizeof("Smart Watch Sensor Hub") - 1),
};

/* ================================================================
 *  Connexion callbacks
 * ================================================================ */
static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection failed (err %u)", err);
        return;
    }
    LOG_INF("BLE Connected");
    current_conn = bt_conn_ref(conn);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("BLE Disconnected (reason %u)", reason);
    if (current_conn) {
        bt_conn_unref(current_conn);
        current_conn = NULL;
    }
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected    = connected,
    .disconnected = disconnected,
};

/* ================================================================
 *  API publique
 * ================================================================ */
int ble_service_init(void)
{
    int err;

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return err;
    }
    LOG_INF("Bluetooth initialized");

    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad),
                          sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return err;
    }
    LOG_INF("BLE Advertising started as \"Smart Watch Sensor Hub\"");

    return 0;
}

bool ble_service_is_connected(void)
{
    return (current_conn != NULL);
}

/* ── Helper : envoyer une notification ─────────────────────────── */
static void notify_attr(const struct bt_gatt_attr *attr,
                        const void *data, uint16_t len, bool enabled)
{
    if (!enabled || !current_conn) {
        return;
    }

    int err = bt_gatt_notify(current_conn, attr, data, len);
    if (err && err != -ENOTCONN) {
        LOG_WRN("Notify failed (err %d)", err);
    }
}

void ble_service_update(const struct ble_sensor_data *data)
{
    memcpy(&current_data, data, sizeof(current_data));

    /*
     * ess_svc.attrs[] indices :
     *   0  = Service ESS
     *   1  = Char decl temp,  2 = value temp,  3 = CCC temp
     *   4  = Char decl hum,   5 = value hum,   6 = CCC hum
     *   7  = Char decl press, 8 = value press,  9 = CCC press
     *
     * motion_svc.attrs[] indices :
     *   0  = Service Motion
     *   1  = Char decl accel, 2 = value accel, 3 = CUD, 4 = CCC accel
     *   5  = Char decl magn,  6 = value magn,  7 = CUD, 8 = CCC magn
     *   9  = Char decl steps,10 = value steps, 11= CUD, 12= CCC steps
     */
    notify_attr(&ess_svc.attrs[2],
                &current_data.temperature, sizeof(current_data.temperature),
                notify_temp_enabled);

    notify_attr(&ess_svc.attrs[5],
                &current_data.humidity, sizeof(current_data.humidity),
                notify_hum_enabled);

    notify_attr(&ess_svc.attrs[8],
                &current_data.pressure, sizeof(current_data.pressure),
                notify_press_enabled);

    notify_attr(&motion_svc.attrs[2],
                &current_data.accel, sizeof(current_data.accel),
                notify_accel_enabled);

    notify_attr(&motion_svc.attrs[6],
                &current_data.magn, sizeof(current_data.magn),
                notify_magn_enabled);

    notify_attr(&motion_svc.attrs[10],
                &current_data.steps, sizeof(current_data.steps),
                notify_steps_enabled);
}
