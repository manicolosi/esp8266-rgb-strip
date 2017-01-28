#include <Arduino.h>
#include <Homie.h>
#include "color.h"

static const uint16_t GAMMA[] = {
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

static const uint8_t LED_PIN_R = D6;
static const uint8_t LED_PIN_G = D7;
static const uint8_t LED_PIN_B = D5;

static const char * COLOR_NAMES[] = {"red", "green", "blue"};
static const uint8_t COLOR_PINS[] = {D6, D7, D5};

static int color_values[] {0, 0, 0};
static bool state_is_on = true;

HomieNode stripNode("strip", "strip");

void setup_outputs() {
  for(int i = 0; i < 3; i++) {
    pinMode(COLOR_PINS[i], OUTPUT);
    digitalWrite(COLOR_PINS[i], LOW);
  }
}

int color_index(const String& color) {
  for(int i = 0; i < 3; i++) {
    if(strcmp(color.c_str(), COLOR_NAMES[i]) == 0) {
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
  uint8_t pin = COLOR_PINS[idx];

  if(update_cache) {
    color_values[idx] = value;
  }

  if(state_is_on) {
    analogWrite(pin, GAMMA[value]);
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

  stripNode.advertise("state").settable(state_handler);

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
