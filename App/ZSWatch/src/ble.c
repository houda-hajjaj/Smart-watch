#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/settings/settings.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <string.h>

LOG_MODULE_REGISTER(ble, LOG_LEVEL_INF);

/* UUIDs standard */
#define BT_UUID_ESS_VAL              0x181A  // Environmental Sensing Service
#define BT_UUID_TEMPERATURE_VAL       0x2A6E
#define BT_UUID_HUMIDITY_VAL           0x2A6F
#define BT_UUID_PRESSURE_VAL           0x2A6D  // Pression en pascals (ou 0.01 hPa)
#define BT_UUID_MAGNETIC_FLUX_3D_VAL   0x2AA0
#define BT_UUID_ACCELEROMETER_3D_VAL   0x2AA1  // Accélération 3 axes

/* Déclaration des UUID */
static const struct bt_uuid_16 ess_uuid = BT_UUID_INIT_16(BT_UUID_ESS_VAL);
static const struct bt_uuid_16 temp_uuid = BT_UUID_INIT_16(BT_UUID_TEMPERATURE_VAL);
static const struct bt_uuid_16 humi_uuid = BT_UUID_INIT_16(BT_UUID_HUMIDITY_VAL);
static const struct bt_uuid_16 press_uuid = BT_UUID_INIT_16(BT_UUID_PRESSURE_VAL);
static const struct bt_uuid_16 mag_uuid = BT_UUID_INIT_16(BT_UUID_MAGNETIC_FLUX_3D_VAL);
static const struct bt_uuid_16 accel_uuid = BT_UUID_INIT_16(BT_UUID_ACCELEROMETER_3D_VAL);

/* Variables pour stocker les valeurs */
static int16_t temp_value;
static uint16_t humi_value;
static uint32_t press_value;          // Pression en pascals
static int16_t mag_value[3];
static int16_t accel_value[3];

/* Callbacks CCC (pour tracer l'activation des notifications) */
static void temp_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    LOG_INF("Notifications température %s", (value == BT_GATT_CCC_NOTIFY) ? "activées" : "désactivées");
}

static void humi_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    LOG_INF("Notifications humidité %s", (value == BT_GATT_CCC_NOTIFY) ? "activées" : "désactivées");
}

static void press_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    LOG_INF("Notifications pression %s", (value == BT_GATT_CCC_NOTIFY) ? "activées" : "désactivées");
}

static void mag_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    LOG_INF("Notifications magnétomètre %s", (value == BT_GATT_CCC_NOTIFY) ? "activées" : "désactivées");
}

static void accel_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    LOG_INF("Notifications accélération %s", (value == BT_GATT_CCC_NOTIFY) ? "activées" : "désactivées");
}

/* Fonctions de lecture (optionnelles) */
static ssize_t read_temp(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &temp_value, sizeof(temp_value));
}

static ssize_t read_humi(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &humi_value, sizeof(humi_value));
}

static ssize_t read_press(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &press_value, sizeof(press_value));
}

static ssize_t read_mag(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                        void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, mag_value, sizeof(mag_value));
}

static ssize_t read_accel(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, accel_value, sizeof(accel_value));
}

/* Définition du service GATT (ESS avec 5 caractéristiques) */
BT_GATT_SERVICE_DEFINE(sensor_svc,
    /* Service primaire ESS */
    BT_GATT_PRIMARY_SERVICE(&ess_uuid),

    /* --- Température --- */
    BT_GATT_CHARACTERISTIC(&temp_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_temp, NULL, &temp_value),
    BT_GATT_CCC(temp_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    /* --- Humidité --- */
    BT_GATT_CHARACTERISTIC(&humi_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_humi, NULL, &humi_value),
    BT_GATT_CCC(humi_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    /* --- Pression --- */
    BT_GATT_CHARACTERISTIC(&press_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_press, NULL, &press_value),
    BT_GATT_CCC(press_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    /* --- Magnétomètre (3 axes) --- */
    BT_GATT_CHARACTERISTIC(&mag_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_mag, NULL, mag_value),
    BT_GATT_CCC(mag_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    /* --- Accéléromètre (3 axes) --- */
    BT_GATT_CHARACTERISTIC(&accel_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_accel, NULL, accel_value),
    BT_GATT_CCC(accel_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

/* Récupération des attributs pour les notifications (index après chaque caractéristique) */
#define TEMP_ATTR  (&sensor_svc.attrs[2])
#define HUMI_ATTR  (&sensor_svc.attrs[5])
#define PRESS_ATTR (&sensor_svc.attrs[8])
#define MAG_ATTR   (&sensor_svc.attrs[11])
#define ACCEL_ATTR (&sensor_svc.attrs[14])

/* Callbacks de connexion */
static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Échec de connexion (err %u)", err);
    } else {
        LOG_INF("Connecté");
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Déconnecté (raison %u)", reason);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

/* Publicité */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME)-1),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_ESS_VAL & 0xFF, BT_UUID_ESS_VAL >> 8),
};

/* Initialisation */
void ble_init(void)
{
    int err;

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("bt_enable failed (err %d)", err);
        return;
    }

    LOG_INF("Bluetooth initialisé");

    /* Charger les settings (bonding) */
    settings_load();

    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Publicité échouée (err %d)", err);
    } else {
        LOG_INF("Publicité active. Nom : %s", CONFIG_BT_DEVICE_NAME);
    }
}

/* Fonctions de mise à jour et notification */
void ble_update_temperature(int16_t temp_100)
{
    temp_value = temp_100;
    uint8_t buf[2];
    sys_put_le16(temp_value, buf);
    bt_gatt_notify(NULL, TEMP_ATTR, buf, sizeof(buf));
}

void ble_update_humidity(uint16_t humi_100)
{
    humi_value = humi_100;
    uint8_t buf[2];
    sys_put_le16(humi_value, buf);
    bt_gatt_notify(NULL, HUMI_ATTR, buf, sizeof(buf));
}

void ble_update_pressure(uint32_t pressure)
{
    press_value = pressure;
    uint8_t buf[4];
    sys_put_le32(press_value, buf);
    bt_gatt_notify(NULL, PRESS_ATTR, buf, sizeof(buf));
}

void ble_update_magnetometer(int16_t x_100, int16_t y_100, int16_t z_100)
{
    mag_value[0] = x_100;
    mag_value[1] = y_100;
    mag_value[2] = z_100;
    bt_gatt_notify(NULL, MAG_ATTR, mag_value, sizeof(mag_value));
}

void ble_update_acceleration(int16_t x_100, int16_t y_100, int16_t z_100)
{
    accel_value[0] = x_100;
    accel_value[1] = y_100;
    accel_value[2] = z_100;
    bt_gatt_notify(NULL, ACCEL_ATTR, accel_value, sizeof(accel_value));
}