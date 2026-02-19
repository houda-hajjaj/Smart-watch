#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>
#include <string.h>  // pour memcpy

LOG_MODULE_REGISTER(ble, LOG_LEVEL_INF);

/* Définition des UUIDs personnalisés */
#define BT_UUID_SENSOR_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)
#define BT_UUID_SENSOR_DATA_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1)

static struct bt_uuid_128 sensor_service_uuid = BT_UUID_INIT_128(BT_UUID_SENSOR_SERVICE_VAL);
static struct bt_uuid_128 sensor_data_uuid    = BT_UUID_INIT_128(BT_UUID_SENSOR_DATA_VAL);

/* Tampon pour la valeur courante de la caractéristique */
static uint8_t sensor_data_value[64];

/* Configuration du CCC (Client Characteristic Configuration) */
static struct bt_gatt_ccc_cfg sensor_data_ccc_cfg[BT_GATT_CCC_MAX] = {};

/* Callback appelé quand le client active/désactive les notifications */
static void sensor_data_ccc_cfg_changed(const struct bt_gatt_attr *attr,
					uint16_t value)
{
	ARG_UNUSED(attr);
	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);
	LOG_INF("Notifications %s", notif_enabled ? "activées" : "désactivées");
}

/* Lecture de la caractéristique (optionnel, pour compatibilité) */
static ssize_t read_sensor_data(struct bt_conn *conn,
				const struct bt_gatt_attr *attr,
				void *buf, uint16_t len, uint16_t offset)
{
	const uint8_t *value = attr->user_data;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				sizeof(sensor_data_value));
}

/* Déclaration du service GATT */
BT_GATT_SERVICE_DEFINE(sensor_service,
	BT_GATT_PRIMARY_SERVICE(&sensor_service_uuid),

	/* Caractéristique : données capteurs (lecture + notification) */
	BT_GATT_CHARACTERISTIC(&sensor_data_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ,
			       read_sensor_data, NULL, sensor_data_value),

	/* Descripteur CCC (obligatoire pour les notifications) */
	BT_GATT_CCC_MANAGED(sensor_data_ccc_cfg,
			    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

/* Données de publicité (advertising) */
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
		sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

/* Données de réponse au scan (inclut l'UUID de notre service) */
static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_SENSOR_SERVICE_VAL),
};

/* Callbacks de connexion */
static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_ERR("Échec de connexion (err %u)", err);
	} else {
		LOG_INF("Connecté");
		/* L'échange de MTU sera initié par le client si nécessaire */
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

/* Initialisation du BLE */
int ble_init(void)
{
	int err;

	/* Initialisation de la stack Bluetooth */
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("bt_enable a échoué (err %d)", err);
		return err;
	}
	LOG_INF("Bluetooth initialisé");

	/* Démarrage de la publicité en mode connectable */
	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("La publicité n'a pas démarré (err %d)", err);
		return err;
	}
	LOG_INF("Publicité active. Nom : %s", CONFIG_BT_DEVICE_NAME);

	return 0;
}

/* Envoi des données des capteurs en notification */
void ble_notify_sensor_data(const uint8_t *data, size_t len)
{
	/* Vérifier que la taille ne dépasse pas le tampon */
	if (len > sizeof(sensor_data_value)) {
		len = sizeof(sensor_data_value);
	}

	/* Mettre à jour la valeur locale */
	memcpy(sensor_data_value, data, len);

	/* Envoyer une notification à tous les clients abonnés.
	 * L'index 2 correspond à la valeur de la caractéristique
	 * (après le service primaire et la déclaration de la caractéristique)
	 */
	bt_gatt_notify(NULL, &sensor_service.attrs[2], data, len);
}