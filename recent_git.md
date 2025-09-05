Based on your git history and the files I found, here are the custom
  keyboard files you were working on for your Work Louder Work Board:

  Main Files You Were Working On:

  1. /workspace/workspace/bb/qmk_firmware/keyboards/work_louder/work_board/k
  eymaps/eliot_midi/keymap.c - Your main MIDI keyboard implementation with:
    - Chord modifiers for music production
    - Session view support for Ableton Live
    - Toggle clips functionality
    - Custom RGB lighting that changes with MIDI channels
    - Multiple layers: NOTES, DRUMS, SESSION, ROUTER
    - MIDI CC controls mapped to keys
  2. /workspace/workspace/bb/qmk_firmware/keyboards/work_louder/work_board/k
  eymaps/eliot_midi/rgb_matrix_user.inc - Custom RGB matrix effects
  3. /workspace/workspace/bb/qmk_firmware/keyboards/work_louder/work_board/k
  eymaps/eliot_midi/rules.mk - Build configuration with MIDI enabled
  4. /workspace/workspace/bb/qmk_firmware/keyboards/work_louder/work_board/k
  eymaps/eliot/keymap.c - Your simpler keymap without MIDI
  5. /workspace/workspace/bb/qmk_firmware/eliot.json - JSON configuration
  file

  Your recent commits show you were working on:
  - Adding chord modifiers (not 100% working per your commit)
  - Toggle clips for Ableton session view
  - RGB lighting integration with MIDI channels
  - MIDI layout implementation

  The main work was in the eliot_midi keymap where you implemented extensive
   MIDI functionality for controlling Ableton Live from your keyboard.


could you tell me how to use this repo to change the code in my 
  keyboard? I need to modify my the keys for my custom keyboard