/*
  DMX Total Control
  AppFileDialog.qml

  Copyright (c) DMX Total Control contributors

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

import QtQuick
import QtQuick.Dialogs
import DMXTotalControl.Backend

/**
 * A file dialog that starts in a folder that makes sense (see
 * Core.dialogFolder): the folder of the show for shows, the folder used
 * last for pictures, sounds... Without it Windows starts every dialog of
 * the program where the last one was left (Open in the pictures, say).
 * Open it with start(), not open().
 */
FileDialog {
    /** "show" (open, save, import from a show), "image", "audio", "effects", "fixtures" */
    property string kind: "show"

    function start() {
        currentFolder = Core.dialogFolder(kind)
        open()
    }

    // (with several files: the first one)
    onAccepted: Core.dialogUsed(kind, selectedFile)
}
