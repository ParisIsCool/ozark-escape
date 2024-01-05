#include "config.h"
extern TTGOClass *ttgo;

// Define a struct to hold room information
struct RoomData {
  String name;
  String time;
  int yPos; // Y position for display
  bool selected; // Track if the room is selected
  bool wantsClue;
};

// Create an array to store room data
#define MAX_ROOMS 5
RoomData rooms[MAX_ROOMS]; // Define MAX_ROOMS as needed

int selectedRoomIndex = -1; // Track the index of the selected room, -1 means none selected

int findAvailablePosition(const String& name) {
  // Iterate through the room data array to find an available position
  for (int i = 0; i < MAX_ROOMS; i++) {
    if (rooms[i].name == name) {
      return i; // Already exists, let's go to that!
    } else if (rooms[i].name.isEmpty()) {
      return i; // Found an available position
    }
  }
  return -1; // No available position found
}

void drawRoomOutline(int yPos, bool isSelected) {
  if (isSelected) {
    ttgo->tft->drawRect(5, yPos * 30 + 65, 230, 30, TFT_BLUE); // Draw a blue outline when selected
  } else {
    ttgo->tft->drawRect(5, yPos * 30 + 65, 230, 30, TFT_BLACK); // Clear the outline when not selected
  }
}

void displayRoom(const String& roomName, const String& roomTime) {
  // Find an available position for displaying the room data
  int yPos = findAvailablePosition(roomName);

  if (yPos != -1) {
    // Store room data in the array
    rooms[yPos].name = roomName;
    rooms[yPos].time = roomTime;
    rooms[yPos].yPos = yPos;

    // Draw the outline for the selected room
    drawRoomOutline(yPos, rooms[yPos].wantsClue);

    // Display the room name in a small font
    ttgo->tft->setTextSize(2);
    ttgo->tft->setTextColor(TFT_WHITE, TFT_BLACK);
    ttgo->tft->setCursor(10, (yPos * 30) + 70);
    ttgo->tft->println(roomName);

    // Display the time below the room name
    ttgo->tft->setTextSize(2);
    ttgo->tft->setCursor(10, (yPos * 30) + 85); // Adjust the Y position for time
    ttgo->tft->println(roomTime);

  } else {
    // Handle a case where there are no available positions
  }
}

void setRoomDisplay(int selectedRoom) {
  // Draw an outline around the selected room
  if (selectedRoom == -1) { return; }
  rooms[selectedRoom].wantsClue = false;
  for (int i = 0; i < MAX_ROOMS; i++) {
    drawRoomOutline(rooms[i].yPos, rooms[selectedRoom].wantsClue);
    displayRoom(rooms[i].name, rooms[i].time);
  }
}

void RoomWantsClue(String name, bool Clue) {
  for (int i = 0; i < MAX_ROOMS; i++) {
    if (rooms[i].name == name) {
      rooms[i].wantsClue = Clue;
      setRoomDisplay(i);
    }
  }
  ttgo->motor->onec();
}


uint32_t lastTouch = 0;

void selectRoom() {
  int16_t x, y;

  boolean exitSelection = false; // Used to stay in the selection until a room is selected

  int selected = -1;

  if (ttgo->getTouch(x, y)) { // If you have touched something...

    while (ttgo->getTouch(x, y)) {} // Wait until you stop touching

    lastTouch = millis();

    for (int i = 0; i < MAX_ROOMS; i++) {
      int Y = ((rooms[i].yPos * 30) + 70);
      if ( y > Y && y < (Y+30) ) {
        selected = i;
      }
    }
  }
  setRoomDisplay(selected);
}


void LoopWatchTasks() {
  if (lastTouch + 60000 > millis()) {
    ttgo->setBrightness(255);
  } else {
    ttgo->setBrightness(32);
  }
  selectRoom();
}


void SetupWatchTasks() {
  ttgo->motor_begin();
}