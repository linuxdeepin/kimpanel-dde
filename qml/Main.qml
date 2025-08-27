import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

pragma ComponentBehavior: Bound

// A modern, clean input method panel with excellent text visibility
Window {
  id: main
  visible: true
  flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
  color: "transparent"

  // Improved sizing for better readability
  property int cellW: 320
  property int cellH: 42
  property int panelPadding: 8
  width: cellW + (panelPadding * 2)
  height: (listModel.count > 0 ? Math.min(7, listModel.count) : 1) * cellH + (panelPadding * 2)

  // Color scheme for excellent contrast and modern appearance
  readonly property color backgroundColor: "#ffffff"
  readonly property color borderColor: "#e0e0e0"
  readonly property color shadowColor: "#00000020"
  readonly property color primaryTextColor: "#1a1a1a"
  readonly property color secondaryTextColor: "#666666"
  readonly property color selectedBackgroundColor: "#f0f7ff"
  readonly property color selectedBorderColor: "#4a90e2"
  readonly property color labelColor: "#4a90e2"

  // Position is handled by layer-shell margins in main.cpp
  Connections {
    target: PanelAdaptor
    function onLookupChanged() { listView.forceLayout(); }
  }

  // Build model from adaptor's lists
  ListModel { id: listModel }
  Timer {
    id: rebuild
    interval: 10; running: true; repeat: true
    onTriggered: {
      listModel.clear()
      for (let i = 0; i < PanelAdaptor.texts.length; ++i) {
        listModel.append({
          label: PanelAdaptor.labels[i] ?? "",
          text: PanelAdaptor.texts[i] ?? "",
          comment: PanelAdaptor.comments[i] ?? ""
        })
      }
    }
  }

      // Main panel container with modern styling
  Rectangle {
    anchors.fill: parent
    radius: 12
    color: main.backgroundColor
    border.width: 1
    border.color: main.borderColor

    // Subtle drop shadow effect
    layer.enabled: true
    layer.effect: DropShadow {
      radius: 16
      samples: 33
      color: main.shadowColor
      verticalOffset: 4
      horizontalOffset: 0
    }

    ListView {
      id: listView
      anchors.fill: parent
      anchors.margins: main.panelPadding
      model: listModel
      spacing: 2
      
      delegate: Rectangle {
        required property int index
        required property var model

        width: listView.width
        height: main.cellH
        radius: 8
        color: (index === PanelAdaptor.cursor) ? main.selectedBackgroundColor : "transparent"
        border.width: (index === PanelAdaptor.cursor) ? 1 : 0
        border.color: main.selectedBorderColor

        // Smooth transitions for selection
        Behavior on color {
          ColorAnimation { duration: 150; easing.type: Easing.OutCubic }
        }
        Behavior on border.width {
          NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
        }

        Row {
          anchors.fill: parent
          anchors.leftMargin: 12
          anchors.rightMargin: 12
          anchors.verticalCenter: parent.verticalCenter
          spacing: 12

          // Candidate number/label
          Label {
            text: parent.parent.model.label
            width: 24
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: main.labelColor
            font.pixelSize: 14
            font.weight: Font.Medium
          }

          // Main candidate text
          Label {
            text: parent.parent.model.text
            color: main.primaryTextColor
            font.pixelSize: 16
            font.weight: (parent.parent.index === PanelAdaptor.cursor) ? Font.DemiBold : Font.Normal
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
          }

          // Comment/description
          Label {
            text: parent.parent.model.comment
            color: main.secondaryTextColor
            font.pixelSize: 13
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            opacity: 0.8
          }
        }
      }
    }

    // Navigation indicators with improved styling
    Row {
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.margins: 12
      spacing: 8

      Label {
        text: PanelAdaptor.hasPrev ? "◀" : ""
        color: main.labelColor
        font.pixelSize: 12
        opacity: PanelAdaptor.hasPrev ? 0.8 : 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
      }

      Label {
        text: PanelAdaptor.hasNext ? "▶" : ""
        color: main.labelColor
        font.pixelSize: 12
        opacity: PanelAdaptor.hasNext ? 0.8 : 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
      }
    }
  }
}



