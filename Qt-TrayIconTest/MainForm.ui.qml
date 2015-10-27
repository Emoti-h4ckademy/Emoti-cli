import QtQuick 2.4
import QtQuick.Controls 1.2

Rectangle {
    width: 196
    height: 370

    Image {
        id: image1
        x: 0
        y: 0
        source: "logo_square.png"
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


}

