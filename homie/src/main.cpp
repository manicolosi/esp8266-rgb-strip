#include <Arduino.h>
#include <Homie.h>

static const uint8_t LED_PIN_R = D6;
static const uint8_t LED_PIN_G = D7;
static const uint8_t LED_PIN_B = D5;

static const char * color_names[] = {"red", "green", "blue"};
static const uint8_t color_pins[] = {D6, D7, D5};
static int color_values[] {0, 0, 0};

HomieNode stripNode("strip", "strip");

void setup_outputs() {
  for(int i = 0; i < 3; i++) {
    pinMode(color_pins[i], OUTPUT);
    digitalWrite(color_pins[i], LOW);
  }
}

uint8_t color_to_pin(const String& color) {
  uint8_t pin = -1;

  for(int i = 0; i < 3; i++) {
    if(strcmp(color.c_str(), color_names[i]) == 0) {
      pin = color_pins[i];
    }
  }

  return pin;
}

void update_color_value(const String& color, int value) {
  for(int i = 0; i < 3; i++) {
    if(strcmp(color.c_str(), color_names[i]) == 0) {
      color_values[i] = value;
    }
  }

  return;
}

void update_rgb_property() {
  char buffer[15];
  snprintf(buffer, 15, "%d,%d,%d", color_values[0], color_values[1], color_values[2]);

  stripNode.setProperty("rgb").send(buffer);
  return;
}

bool set_color(const String& color, const String& value) {
  uint8_t pin = color_to_pin(color);

  if(pin != -1) {
    Homie.getLogger() << color << " set to " << value.toInt() << endl;
    update_color_value(color, value.toInt());
    stripNode.setProperty(color).send(value);
    analogWrite(pin, value.toInt());
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

void setup() {
  setup_outputs();
  Serial.begin(115200);
  Homie_setFirmware("bare-minimum", "1.0.0");
  Homie.setup();

  stripNode.advertise("red").settable(red_handler);
  stripNode.advertise("green").settable(green_handler);
  stripNode.advertise("blue").settable(blue_handler);
  stripNode.advertise("rgb").settable(rgb_handler);
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
