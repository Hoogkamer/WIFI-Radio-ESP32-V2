// THE PORTABLE RADIO VERSION

// Enable if you have a remote, and provide the correct remote codes
/*
#define HAS_REMOTE
#define RC_RADIO_ON 70386007955804
#define RC_STANDBY 70386011603005
#define RC_PREVIOUS_STATION 70386011640495
#define RC_NEXT_STATION 70386011624047
#define RC_NEXT_CATEGORY 70386013224938
#define RC_PREVIOUS_CATEGORY 70386013192042
#define RC_TURN_ON_SCREEN 70386011660258
#define RC_SHOW_DETAILS 70386011657704
#define RC_SAVE_STATION 70386010088896
#define RC_RADIO_OFF1 70386011651201
#define RC_RADIO_OFF2 70386013196293
#define RC_RADIO_OFF3 70386010039552
#define RC_VOLUME_PLUS
#define RC_VOLUME_MINUS
*/

// enable if you have rotary knobs
#define HAS_ROTARIES
// #define TFT_ROTATION 0 // 1 or 3 (landscape)
#define TFT_ROTATION 1 // 1 or 3 (landscape)

// enable if you need mono output (when you have 1 speaker you can use eighter L or R channel only)
#define MONO_OUTPUT

// set the max volume level and the total steps. In this example it can only get 1/3 of the total volume
#define VOLUME_MAX 20
// volume to use at start up
#define VOLUME_DEFAULT 3
// volume steps (set lower if you want to use bigger steps)
#define VOLUME_STEPS 64

// uncomment if you have an SD card connected (where you put the images and sounds)
// #define HAS_SDCARD

// uncomment if you want to show images for the stations: you need to have an SD card, and also a vertical tft layout
// #define SHOW_IMAGES

// -----------------------