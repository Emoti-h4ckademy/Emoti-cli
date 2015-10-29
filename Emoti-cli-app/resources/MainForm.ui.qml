import QtQuick 2.4
import QtQuick.Controls 1.2

Rectangle {
    x: 0
    width: 196
    height: 330

    Image {
        id: image1
        x: 4
        y: 85
        width: 188
        height: 190
        fillMode: Image.PreserveAspectFit
        source: "emoti-logo-small.png"
    }

    ListModel {
        id: lmodel
        ListElement {
            name: "Bill Smith"
            number: "555 3264"
        }
        ListElement {
            name: "John Brown"
            number: "555 8426"
        }
        ListElement {
            name: "Sam Wise"
            number: "555 0473"
        }
    }

    Text {
        id: text1
        x: 12
        y: 24
        width: 172
        height: 42
        color: "#767676"
        text: qsTr("Welcome to Emoti!")
        font.pixelSize: 20
    }


}

