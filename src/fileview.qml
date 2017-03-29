import QtQuick 2.0

Item
{
    width: 300
    height: 200

    ListModel
    {
        id: myModel2

        ListElement { text: "List Item 1" }
        ListElement { text: "List Item 2" }
        ListElement { text: "List Item 3" }
        ListElement { text: "List Item 4" }
        ListElement { text: "List Item 5" }
        ListElement { text: "List Item 6" }
    }

    Component
    {
        id: beerDelegate

        Rectangle
        {
            id: beerDelegateRectangle
            height: beerDelegateText.height * 1.5
            width: parent.width

            Text
            {
                id: beerDelegateText
                text: "<b>" + modelData + "</b> <i>(" + modelData + ")</i>"
            }

            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    console.log("clicked: " + modelData + " at index: " + index);
                    beerList.currentIndex = index;
                    beerList.highlight = highlightBar; // Load this as a seperate component onclick of mouse
                }
            }
        }
    }

    ListView
    {
        id: beerList
        anchors.fill: parent
        model: myModel2
        delegate: beerDelegate

        //highlightFollowsCurrentItem: true
        //highlight: Rectangle
        //{
        // width: parent.width
        // color: "red"
        //}

        focus: true
    }

    // Component to control ListView highlight property
    Component {
        id: highlightBar
        Rectangle {
            //width: 200; height: 50
            color: "lightgrey"
            // y: listView.currentItem.y;
            // Behavior on y { SpringAnimation { spring: 2; damping: 0.1 } }
        }
    }
}
