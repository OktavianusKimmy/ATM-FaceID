
import paho.mqtt.client as mqtt

# MQTT Broker
BROKER = "192.168.1.4"
PORT = 1883

# MQTT Topics
RFID_TOPIC = "rfid/card"
SERVO_TOPIC = "servo/control"


def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("Berhasil terhubung ke MQTT Broker!")

        client.subscribe(RFID_TOPIC)
        print(f"Subscribe: {RFID_TOPIC}")

    else:
        print(f"Koneksi gagal: {reason_code}")


def on_message(client, userdata, msg):
    uid = msg.payload.decode("utf-8")

    print("\n===== RFID CARD DETECTED =====")
    print(f"UID: {uid}")

    # Logika pengujian sementara:
    # Setiap kartu yang terbaca akan menggerakkan servo.
    print("Mengirim perintah membuka servo...")

    client.publish(SERVO_TOPIC, "-1")

    print("Perintah 1 berhasil dikirim!")
    print("==============================")


client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2,
    client_id="Python_RFID_Controller"
)

client.on_connect = on_connect
client.on_message = on_message

print("Menghubungkan ke MQTT Broker...")

client.connect(BROKER, PORT, 60)

client.loop_forever()