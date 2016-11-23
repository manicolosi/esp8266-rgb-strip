local module = {}

GAMMA = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,
    3,  3,  4,  4,  4,  5,  5,  5,  6,  6,  7,  7,  7,  8,  8,  9,
    10, 10, 11, 11, 12, 13, 13, 14, 15, 15, 16, 17, 18, 19, 20, 20,
    21, 22, 23, 24, 25, 26, 27, 29, 30, 31, 32, 33, 35, 36, 37, 38,
    40, 41, 43, 44, 46, 47, 49, 50, 52, 54, 55, 57, 59, 61, 63, 64,
    66, 68, 70, 72, 74, 77, 79, 81, 83, 85, 88, 90, 92, 95, 97,100,
    102,105,107,110,113,115,118,121,124,127,130,133,136,139,142,145,
    149,152,155,158,162,165,169,172,176,180,183,187,191,195,199,203,
    207,211,215,219,223,227,232,236,240,245,249,254,258,263,268,273,
    277,282,287,292,297,302,308,313,318,323,329,334,340,345,351,357,
    362,368,374,380,386,392,398,404,410,417,423,429,436,442,449,455,
    462,469,476,483,490,497,504,511,518,525,533,540,548,555,563,571,
    578,586,594,602,610,618,626,634,643,651,660,668,677,685,694,703,
    712,721,730,739,748,757,766,776,785,795,804,814,824,833,843,853,
    863,873,884,894,904,915,925,936,946,957,968,979,990,1001,1012,1023
};

m = nil

RETAIN_FLAG = 1

R_PIN = 6
G_PIN = 7
B_PIN = 5

FREQ = 500

LIGHT_TOPIC = nil
RGB_TOPIC   = nil

current = {255, 255, 255}

local function publish(key, msg)
    topic = config.mqtt.TOPIC_PREFIX .. key
    print("PUBLISH", topic, msg)
    m:publish(topic, msg, 0, RETAIN_FLAG)
end

local function mqtt_stop()
    tmr.stop(6)
    m:disconnect()
end

local function set_output(pin, value)
    pwm.setduty(pin, GAMMA[value+1])
end

local function set_outputs()
    set_output(R_PIN, current[1])
    set_output(G_PIN, current[2])
    set_output(B_PIN, current[3])
end

local function set_outputs_new(r, g, b)
    set_output(R_PIN, r)
    set_output(G_PIN, g)
    set_output(B_PIN, b)
end

TRANSITION_MS = 1000
TRANSITION_TS = 256
current_t = 0

local function interopolate(a, b, t)
    return ((b - a) * ((t * TRANSITION_TS) / TRANSITION_TS) / TRANSITION_TS) + a
end
local function start_transition(final_color)
    start_color = {current[1], current[2], current[3]}
    tmr.register(2, TRANSITION_MS / TRANSITION_TS, tmr.ALARM_AUTO, function()
        if (current_t >= TRANSITION_TS) then
            tmr.unregister(2)
            current_t = 0
        else
            r = interopolate(start_color[1], final_color[1], current_t)
            g = interopolate(start_color[2], final_color[2], current_t)
            b = interopolate(start_color[3], final_color[3], current_t)

            set_outputs_new(r, g, b)

            current_t = current_t + 1
        end
    end)
    tmr.start(2)
end


local function rgb_set(data)
    t = {}
    i = 1
    for str in string.gmatch(data, "([^,]+)") do
        t[i] = tonumber(str)
        i = i + 1
    end

    start_transition(t)
    current = { t[1], t[2], t[3] }
    --set_outputs()

    publish("rgb/state", t[1] .. "," .. t[2] .. "," .. t[3])
end

local function light_set(data)
    if data == "ON" then
        --set_outputs()
    elseif data == "OFF" then
        pwm.setduty(R_PIN, 0)
        pwm.setduty(G_PIN, 0)
        pwm.setduty(B_PIN, 0)
    end

    publish("light/state", data)
end

local function mqtt_start()
    m = mqtt.Client(config.mqtt.CLIENT_ID, 60)

    m:lwt(config.mqtt.TOPIC_PREFIX .. "state", "offline", 0, RETAIN_FLAG)

    m:on("message", function(client, topic, data)
        if data ~= nil then
            print(topic .. ": " .. data)

            if topic == RGB_TOPIC then
                rgb_set(data)
            elseif topic == LIGHT_TOPIC then
                light_set(data)
            end
        end
    end)

    m:connect(config.mqtt.HOST, config.mqtt.PORT, 0, 1, function(con)
        print("MQTT Connected")

        m:subscribe(LIGHT_TOPIC, 0)
        m:subscribe(RGB_TOPIC, 0)

        publish("state", "online")
    end)

end

function module.start()
    print("Starting")

    LIGHT_TOPIC = config.mqtt.TOPIC_PREFIX .. "light/set"
    RGB_TOPIC   = config.mqtt.TOPIC_PREFIX .. "rgb/set"

    pwm.setup(R_PIN, FREQ, 512)
    pwm.setup(G_PIN, FREQ, 512)
    pwm.setup(B_PIN, FREQ, 512)

    mqtt_start()
end

function module.stop()
    print("Stopping")
    mqtt_stop()
end

return module
