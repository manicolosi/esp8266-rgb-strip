local module = {}

local function wifi_wait_ip()
    if wifi.sta.getip() == nil then
        uart.write(0, ".")
    else
        tmr.stop(1)
        uart.write(0, "\n")

        print("MODE: " .. wifi.getmode())
        print("MAC:  " .. wifi.ap.getmac())
        print("IP:   " .. wifi.sta.getip())

        app.start()
    end
end

function module.start()
    wifi.setmode(wifi.STATION)
    wifi.sta.config(config.wifi.SSID, config.wifi.PASSPHRASE)
    wifi.sta.connect()

    print("SSID: " .. config.wifi.SSID)
    uart.write(0, "Connecting..")

    tmr.alarm(1, 100, 1, wifi_wait_ip)
end

return module
