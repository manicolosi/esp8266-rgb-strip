#include <Arduino.h>
#include <Homie.h>

static const uint8_t LED_PIN_R = D6;
static const uint8_t LED_PIN_G = D7;
static const uint8_t LED_PIN_B = D5;

static const char * color_names[] = {"red", "green", "blue"};
static const uint8_t color_pins[] = {D6, D7, D5};
static int color_values[] {0, 0, 0};
static bool state_is_on = true;

HomieNode stripNode("strip", "strip");

void setup_outputs() {
  for(int i = 0; i < 3; i++) {
    pinMode(color_pins[i], OUTPUT);
    digitalWrite(color_pins[i], LOW);
  }
}

int color_index(const String& color) {
  for(int i = 0; i < 3; i++) {
    if(strcmp(color.c_str(), color_names[i]) == 0) {
      return i;
    }
  }

  return -1;
}

void update_rgb_property() {
  char buffer[15];
  snprintf(buffer, 15, "%d,%d,%d", color_values[0], color_values[1], color_values[2]);

  stripNode.setProperty("rgb").send(buffer);
  return;
}

void set_pin_color(int idx, int value, bool update_cache) {
  uint8_t pin = color_pins[idx];

  if(update_cache) {
    color_values[idx] = value;
  }

  if(state_is_on) {
    analogWrite(pin, value);
  }
}

bool set_color(const String& color, const String& value) {
  int idx = color_index(color);

  if(idx != -1) {
    Homie.getLogger() << color << " set to " << value.toInt() << endl;
    set_pin_color(idx, value.toInt(), true);
    stripNode.setProperty(color).send(value);
  } else {
    Homie.getLogger() << color << " is invalid" << endl;
    return false;
  }

  return true;
}

bool red_handler(const HomieRange& range, const String& value) {
  bool ret_val = set_color("red", value);
  update_rgb_property();
  return ret_val;
}

bool green_handler(const HomieRange& range, const String& value) {
  bool ret_val = set_color("green", value);
  update_rgb_property();
  return ret_val;
}

bool blue_handler(const HomieRange& range, const String& value) {
  bool ret_val = set_color("blue", value);
  update_rgb_property();
  return ret_val;
}

bool rgb_handler(const HomieRange& range, const String& value) {
  int delim1 = value.indexOf(",");
  int delim2 = value.indexOf(",", delim1 + 1);

  String r = value.substring(0, delim1);
  String g = value.substring(delim1 + 1, delim2);
  String b = value.substring(delim2 + 1);

  set_color("red", r);
  set_color("green", g);
  set_color("blue", b);
  update_rgb_property();

  return true;
}

bool state_handler(const HomieRange& range, const String& value) {
  if(value == "on") {
    state_is_on = true;
    for(int i = 0; i < 3; i++) {
      set_pin_color(i, color_values[i], false);
    }
    stripNode.setProperty("state").send("on");
  } else if(value == "off") {
    for(int i = 0; i < 3; i++) {
      set_pin_color(i, 0, false);
    }
    state_is_on = false;
    stripNode.setProperty("state").send("off");
  } else {
    Homie.getLogger() << "state '" << value << "' is invalid" << endl;
    return false;
  }

  return true;
}

void setup() {
  setup_outputs();
  Serial.begin(115200);
  Homie_setFirmware("bare-minimum", "1.0.0");
  Homie.setup();

  stripNode.advertise("red").settable(red_handler);
  stripNode.advertise("green").settable(green_handler);
  stripNode.advertise("blue").settable(blue_handler);
  stripNode.advertise("rgb").settable(rgb_handler);
  stripNode.advertise("state").settable(state_handler);
}

void loop() {
  Homie.loop();
}

// TOPICS:
// strip0/red
// strip0/green
// strip0/blue
// strip0/rgb
//
// strip0/hue
// strip0/saturation
// strip0/luminosity
// strip0/hsl
//
// strip0/state (on/off)
// strip0/mode (normal/party/whatever)
