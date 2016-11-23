local module = {}

module.wifi = {}
module.wifi.SSID = "redacted"
module.wifi.PASSPHRASE = "redacted"

module.mqtt = {}
module.mqtt.HOST = "redacted"
module.mqtt.PORT = 1883
module.mqtt.CLIENT_ID = node.chipid()
module.mqtt.TOPIC_PREFIX = "esp/" .. node.chipid() .. "/"

return module
