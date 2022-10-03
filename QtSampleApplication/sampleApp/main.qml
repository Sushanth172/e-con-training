import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick 2.6
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.0
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import qml.components.sample 1.0


ApplicationWindow {
    id: applicationWindow
    visible: true
    visibility:Window.Maximized
    property alias comboBox: deviceListComboBox
    /*
      qsTr - The result is returned to the text property and the user
      interface will show the appropriate translation of "Back" for
      the current locale
    */
    title: qsTr("Sample Application")


    ColumnLayout{
        id:columnlayout1

        ComboBox {
            id: deviceListComboBox
            model: device
            textRole: "display"
            implicitWidth: 250
            MouseArea
            {
                anchors.fill: parent
                onPressed:
                {
                    if(pressed)
                    {
                        camera.deviceEnumeration()
                    }
                    mouse.accepted = false
                }

            }
            onCurrentIndexChanged:
            {
                console.log("Current_Index:"+currentIndex)
                camera.selectDevice(currentIndex)
//                sideBar.getTab(0).item.children[0].enabled = true

            }
        }

        TabView{
            id: sideBar
            implicitWidth: 250
            Tab{
                id: uvcControls
                title:qsTr("UVC Settings");
                ColumnLayout{
                    id:tab1_combobox
                    ComboBox
                    {
                        implicitWidth: 250
                        id:format_combo_box
                        model: format
                        textRole: "display"
                        onCurrentIndexChanged:
                        {
//                            camera.formatType(currentIndex)
                            resolution_combo_box.enabled = true
                        }
                        Component.onCompleted: enabled = false
                    }
                    ComboBox
                    {
                        implicitWidth: 250
                        id:resolution_combo_box
                        model: resolution
                        textRole: "display"
                        onCurrentIndexChanged:
                        {
                            fps_combo_box.enabled = true
                        }
                        Component.onCompleted: enabled = false
                    }
                    ComboBox
                    {
                        implicitWidth: 250
                        id:fps_combo_box
                        model: fps
                        textRole: "display"
                        onCurrentIndexChanged:
                        {
                        }
                        Component.onCompleted: enabled = false
                    }
                }
            }
            Tab{
                id: tab2
                title: "Extention Controls"
            }
        }

    }
    Devices{
        id: camera
    }

}
