#include "key_constants.hpp"
#include <algorithm>
#include <dlfcn.h>
#include <iostream>
#include <set>
#include <unistd.h>

#include "RtMidi.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

// This function should be embedded in a try/catch block in case of
// an exception.  It offers the user a choice of MIDI ports to open.
// It returns false if there are no ports available.
bool chooseMidiPort(RtMidiOut *rtmidi);

RtMidi::Api chooseMidiApi();

RtMidiOut *midiout = 0;
std::vector<unsigned char> message;

using WootingAnalogResult = int;
using wooting_analog_initialise_t = WootingAnalogResult (*)();
using wooting_analog_uninitialise_t = WootingAnalogResult (*)();
using wooting_analog_read_full_buffer_t = int (*)(unsigned short *code_buffer,
                                                  float *analog_buffer,
                                                  unsigned int len);

bool global_sustain_mode = false; // bad global

unsigned int microsecond = 1000000;

const int MAX_KEYS_TO_CHECK = 16;

float time_until_actuation = 0.016;

// 40 points each with delta time of 0.0004 means that in total we get 40 *
// 0.0004 = 0.016 of a second
const int NUM_ANALOG_DATA_POINTS_BEFORE_ACTUATION = 128;
float time_between_data_collection_seconds =
    time_until_actuation / (float)NUM_ANALOG_DATA_POINTS_BEFORE_ACTUATION;

bool key_currently_pressed[48] = {false};
bool actuated_keys[48] = {0};
bool previous_tick_actuated_keys[48] = {0};
bool just_actuated_keys[48] = {false};
int analog_data_points_collected[48] = {0};
float analog_data_points[48][NUM_ANALOG_DATA_POINTS_BEFORE_ACTUATION] = {
    {0.0f}};
float speed[48] = {0};

void clear_keys_pressed(bool key_currently_pressed[48]) {
  for (int i = 0; i < 48; i++) {
    key_currently_pressed[i] = false;
  }
}

bool at_least_one_key_actuated(bool actuated_keys[48]) {
  for (int i = 0; i < 48; i++) {
    if (actuated_keys[i]) {
      return true;
    }
  }
  return false;
}

void update_previously_actuated_keys(bool actuated_keys[48],
                                     bool previously_actuated_keys[48]) {
  for (int i = 0; i < 48; i++) {
    previously_actuated_keys[i] = actuated_keys[i];
  }
}

void print_actuated_key_info() {
  int cutoff = 48;
  for (int i = 0; i < cutoff; i++) {
    if (just_actuated_keys[i]) {
      printf("skn: %d, pressed: %d, actuated: %d, apc: %d, speed: %f dp: ", i,
             key_currently_pressed[i], actuated_keys[i],
             analog_data_points_collected[i], speed[i]);
      for (int j = 0; j < analog_data_points_collected[i]; j++) {
        printf("%f, ", analog_data_points[i][j]);
      }
      printf("\n");
    }
  }
}

int transpose = 3;
// Center at C4, and move down two octaves, yields base note of C2
int lowest_note = 60 - 2 * 12 + transpose;

void update_transpose(int new_transpose) {
  lowest_note = 60 - 2 * 12 + new_transpose;
}

int convert_key_to_note(int pressed_key) { return lowest_note + pressed_key; }

/**
 * \note this only turns off notes in the range 0 to 127
 */
void turn_off_all_midi_notes() {
  for (int i = 0; i < 127; i++) {
    // Note Off: 128, 64, 40
    message[0] = 128; // off
    message[1] = i;
    message[2] = 40; // vel
    midiout->sendMessage(&message);
  }
}

void play_just_actuated_notes(bool actuated_keys[48],
                              bool previous_tick_actuated_keys[48],
                              float speed[48]) {
  //    printf("starting note batch \n");
  for (int i = 0; i < 48; i++) {
    int note = convert_key_to_note(i);
    int note_velocity = speed[i];
    if (!previous_tick_actuated_keys[i] && actuated_keys[i]) {
      printf("playing %d at velocity %d\n", note, note_velocity);
      // Note On: 144, 64, 90
      message[0] = 144; // on
      message[1] = note;
      message[2] = note_velocity; // vel
      midiout->sendMessage(&message);
    }
    if (previous_tick_actuated_keys[i] && !actuated_keys[i]) {
      if (!global_sustain_mode) {
        // Note Off: 128, 64, 40
        message[0] = 128; // off
        message[1] = note;
        message[2] = 40; // vel
        midiout->sendMessage(&message);
      }
    }
  }
  //    printf("ending note batch\n\n");
}

/**
 * @param set_a
 * @param set_b
 * @return true iff set_a is a subset of set_b
 */
bool subset(const std::set<int> &set_a, const std::set<int> &set_b) {
  return std::includes(set_b.begin(), set_b.end(), set_a.begin(), set_a.end());
}

/**
 * @param set_a
 * @param set_b
 * @param result an empty set which will contain the result of taking A \ B
 * A \ B is a set containing the elements of set_a which are not in set_b
 */
void difference(const std::set<int> &set_a, const std::set<int> &set_b,
                std::set<int> *result) {
  std::set_difference(set_a.begin(), set_a.end(), set_b.begin(), set_b.end(),
                      std::inserter(*result, result->end()));
}

void print_set(const std::set<int> &set) {
  printf("set contents:\n");
  for (int j : set) {
    printf("%d ", j);
  }
  printf("\n");
}

void process_commands(std::set<int> HID_keys_pressed) {

  std::set<int> transpose_command = {KEY_SPACE, KEY_T};
  std::set<int> sustain_mode_on = {KEY_SPACE};
  std::set<int> sustain_mode_off = {KEY_LEFTSHIFT, KEY_SPACE};

  if (HID_keys_pressed.size() == 1) {
    int pressed_key = *HID_keys_pressed.begin();
    if (pressed_key == KEY_SPACE) {
      global_sustain_mode = true;
    }
  }

  if (HID_keys_pressed.size() == 2) { // redundant
    if (HID_keys_pressed == sustain_mode_off) {
      global_sustain_mode = false;
    }
  }

  if (HID_keys_pressed.size() == 3) {
    if (subset(transpose_command, HID_keys_pressed)) {
      std::set<int> other_keys;
      difference(HID_keys_pressed, transpose_command, &other_keys);
      assert(other_keys.size() == 1);
      int transpose_key = *other_keys.begin();
      bool invalid_transpose_key =
          HID_number_row_to_value.find(transpose_key) ==
          HID_number_row_to_value.end();
      if (not invalid_transpose_key) {
        // global variable, this is bad.
        update_transpose(HID_number_row_to_value[transpose_key]);
        turn_off_all_midi_notes();
      }
    }
  }
}

int main() {

  // RtMidiOut constructor
  try {
    midiout = new RtMidiOut(chooseMidiApi());
  } catch (RtMidiError &error) {
    error.printMessage();
    exit(EXIT_FAILURE);
  }

  // Call function to select port.
  try {
    if (chooseMidiPort(midiout) == false) {
      delete midiout;
      return 0;
    }
  } catch (RtMidiError &error) {
    error.printMessage();
    delete midiout;
    return 0;
  }

  // Send out a series of MIDI messages.

  // Program change: 192, 5
  message.push_back(192);
  message.push_back(5);
  midiout->sendMessage(&message);

  // Control Change: 176, 7, 100 (volume)
  message[0] = 176;
  message[1] = 7;
  message.push_back(100);
  midiout->sendMessage(&message);

  // SetEnvironmentVariableA("RUST_LOG", "off");
  auto sdk = dlopen("libwooting_analog_sdk.so", RTLD_LAZY);

  if (sdk == NULL) {
    fprintf(stderr, "dlopen failed: %s\n", dlerror());
    return 0;
  }

  ((wooting_analog_initialise_t)dlsym(sdk, "wooting_analog_initialise"))();
  auto read_full_buffer = (wooting_analog_read_full_buffer_t)dlsym(
      sdk, "wooting_analog_read_full_buffer");

  std::set<int> prev_set_of_skn;

  int num_iterations = 0;

  while (true) {

    num_iterations = (1 + num_iterations) % 50000;
    unsigned short code_buffer[MAX_KEYS_TO_CHECK];
    float analog_buffer[MAX_KEYS_TO_CHECK];

    // the code buffer stores all the keys that are currently pressed
    // the analog buffer stores how depressed the key is
    // they are both indexed in the same fashion
    const int num_keys_pressed =
        read_full_buffer(code_buffer, analog_buffer, MAX_KEYS_TO_CHECK);
    bool got_key_press =
        num_keys_pressed >= 0 && num_keys_pressed < MAX_KEYS_TO_CHECK;

    clear_keys_pressed(key_currently_pressed);
    // skn: sequential key number
    std::set<int> set_of_skn;
    std::set<int> HID_keys_pressed;

    if (got_key_press) {
      for (int i = 0; i < num_keys_pressed;
           i++) { // keep going until you find the right key

        int HID_key_code = code_buffer[i];
        HID_keys_pressed.insert(HID_key_code);

        float key_depression = analog_buffer[i];
        int sequential_key_number = HID_to_sequential_querty[HID_key_code];

        set_of_skn.insert(sequential_key_number);

        bool already_actuated = actuated_keys[sequential_key_number];

        // printf("%d | %d", sequential_key_number, already_actuated);

        if (already_actuated) { // key has already been actuated, stop caring
          just_actuated_keys[sequential_key_number] = false;
          continue;
        }

        key_currently_pressed[sequential_key_number] = true;
        int num_analog_data_points_collected =
            analog_data_points_collected[sequential_key_number];

        if (num_analog_data_points_collected <
            NUM_ANALOG_DATA_POINTS_BEFORE_ACTUATION) {
          analog_data_points[sequential_key_number]
                            [num_analog_data_points_collected] = key_depression;
          analog_data_points_collected[sequential_key_number] += 1;
        } else { // collected enough datapoints, start actuation

          assert(analog_data_points_collected[sequential_key_number] ==
                 NUM_ANALOG_DATA_POINTS_BEFORE_ACTUATION);

          actuated_keys[sequential_key_number] = true;
          just_actuated_keys[sequential_key_number] = true;

          float *max_depression_ptr =
              std::max_element(analog_data_points[sequential_key_number],
                               analog_data_points[sequential_key_number] +
                                   NUM_ANALOG_DATA_POINTS_BEFORE_ACTUATION);
          float max_depression = *max_depression_ptr;
          int index_max_acheived = std::distance(
              analog_data_points[sequential_key_number], max_depression_ptr);
          int plus_one_index = index_max_acheived + 1;

          float average_speed = max_depression / (float)plus_one_index;
          float scaled_speed = average_speed * 1000; // make it

          float avg_depression = 0.0;
          for (int j = 0; j < plus_one_index; j++) {
            avg_depression += analog_data_points[sequential_key_number][j];
          }
          avg_depression = avg_depression / (float)plus_one_index;
          // in practice this number is between 0.1 and 0.6 so to map 0.6 to 127
          // we need 127 / 0.6 = 211.6666, but that's too loud, tried 200
          scaled_speed = avg_depression * 200;

          speed[sequential_key_number] = scaled_speed;

          if (speed[sequential_key_number] > 127) {
            speed[sequential_key_number] = 127;
          }

          //            int max_velocity = 127;
          //
          //            float avg_speed = 0.0;
          //            for (int j = 0; j <
          //            NUM_ANALOG_DATA_POINTS_BEFORE_ACTUATION; j++) {
          //                avg_speed +=
          //                analog_data_points[sequential_key_number][j];
          //            }
          //
          //            avg_speed /=
          //            (float)NUM_ANALOG_DATA_POINTS_BEFORE_ACTUATION;
          //            speed[sequential_key_number] = avg_speed * max_velocity;
          //
          //            speed[sequential_key_number] *= 1.5;
          //
          //            if (speed[sequential_key_number] > 127) {
          //                speed[sequential_key_number] = 127;
          //            }
        }

        //         printf("skn: %d apc: %d, actutated: %d \n",
        //         sequential_key_number, num_analog_data_points_collected,
        //         actuated_keys[sequential_key_number]);

        // std::cout << sequential_key_mapping << ": " << analog_buffer[i] <<
        // "\n";
        //  if (code_buffer[i] == 0x14) {
        //      std::cout << "pressing q" << "\n";
        //  }
      }

      // process any commands
      process_commands(HID_keys_pressed);
    }

    std::set<int> skns_to_be_unactuated;
    // everything that was pressed, minus everything that is pressed, is
    // everything that is no longer pressed
    std::set_difference(
        prev_set_of_skn.begin(), prev_set_of_skn.end(), set_of_skn.begin(),
        set_of_skn.end(),
        std::inserter(skns_to_be_unactuated, skns_to_be_unactuated.end()));

    // print_actuated_key_info();
    // print_actuated_key_info();
    // printf("num pressed keys %d \n", set_of_skn.size());
    // printf("num prev keys %d \n", prev_set_of_skn.size());
    // printf("num to be de-ac keys %d \n", skns_to_be_unactuated.size());

    for (auto skn : skns_to_be_unactuated) {
      //            printf("de-actuating: %d\n", skn);
      key_currently_pressed[skn] = false;
      actuated_keys[skn] = false;
      analog_data_points_collected[skn] = 0;
      speed[skn] = 0.0;
    }

    play_just_actuated_notes(actuated_keys, previous_tick_actuated_keys, speed);

    prev_set_of_skn = set_of_skn;
    update_previously_actuated_keys(actuated_keys, previous_tick_actuated_keys);

    usleep(time_between_data_collection_seconds *
           microsecond); // sleep so that when we collect datapoints
                         // there is spacing
  }

  return 0;
}

bool chooseMidiPort(RtMidiOut *rtmidi) {
  std::cout << "\nWould you like to open a virtual output port? [y/N] ";

  std::string keyHit;
  std::getline(std::cin, keyHit);
  if (keyHit == "y") {
    rtmidi->openVirtualPort();
    return true;
  }

  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if (nPorts == 0) {
    std::cout << "No output ports available!" << std::endl;
    return false;
  }

  if (nPorts == 1) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  } else {
    for (i = 0; i < nPorts; i++) {
      portName = rtmidi->getPortName(i);
      std::cout << "  Output port #" << i << ": " << portName << '\n';
    }

    do {
      std::cout << "\nChoose a port number: ";
      std::cin >> i;
    } while (i >= nPorts);
  }

  std::cout << "\n";
  rtmidi->openPort(i);

  return true;
}

RtMidi::Api chooseMidiApi() {
  std::vector<RtMidi::Api> apis;
  RtMidi::getCompiledApi(apis);

  if (apis.size() <= 1)
    return RtMidi::Api::UNSPECIFIED;

  std::cout << "\nAPIs\n  API #0: unspecified / default\n";
  for (size_t n = 0; n < apis.size(); n++)
    std::cout << "  API #" << apis[n] << ": "
              << RtMidi::getApiDisplayName(apis[n]) << "\n";

  std::cout << "\nChoose an API number: ";
  unsigned int i;
  std::cin >> i;

  std::string dummy;
  std::getline(std::cin, dummy); // used to clear out stdin

  return static_cast<RtMidi::Api>(i);
}
