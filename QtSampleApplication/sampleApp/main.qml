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
//        MessageDialog
//        {
//            id:dialog
//            title: "Notification"
//            text: "Device connection lost."
//            Component.onCompleted: visible = false
//        }

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
                console.log("Current_Index on device Enumeration:"+currentIndex)
                camera.selectDevice(currentIndex)
                sideBar.getTab(0).item.children[0].enabled = true
            }
            Component.onCompleted:
            {
                camera.deviceEnumeration()
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
                            console.log("Current_Index on format box:"+currentIndex)
                            camera.enumFormat(currentIndex)
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
                            console.log("Current_Index on resolution box:"+currentIndex)
                            camera.enumResolution(currentIndex)
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
                            console.log("Current_Index on fps box:"+currentIndex)
                            camera.enumFps(currentIndex)
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
        onEmitformats: {
            sideBar.getTab(0).item.children[0].currentIndex = deviceIndex
        }
        onEmitresolution: {
            sideBar.getTab(0).item.children[1].currentIndex = deviceIndex
        }
        onEmitfps: {
            sideBar.getTab(0).item.children[2].currentIndex = deviceIndex
        }
        onDevicedisconnected: {
            dialog.visible = true

            sideBar.getTab(0).item.children[0].currentIndex=0
            sideBar.getTab(0).item.children[1].currentIndex=0
            sideBar.getTab(0).item.children[2].currentIndex=0

            sideBar.getTab(0).item.children[0].currentIndex.enabled=false
            sideBar.getTab(0).item.children[1].currentIndex.enabled=false
            sideBar.getTab(0).item.children[2].currentIndex.enabled=false
        }

//        onEmitformat: {
//            sideBar.getTab(0).item.children[0].currentIndex = index
//        }
    }

}
