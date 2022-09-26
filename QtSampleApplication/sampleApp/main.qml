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
    width: 640
    height: 480
//    visibility:Window.Maximized
    property alias comboBox: deviceListComboBox
    title: qsTr("Sample Application")

    RowLayout{
        id:rowlayout
        width:parent.width
        height: parent.height

        ColumnLayout{
            id:columnlayout1
//            anchors.top: rowlayout.top
//            anchors.topMargin: 20
            Layout.margins: 20

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
//                    camera.selectDevice(currentIndex)
                    sideBar.getTab(0).item.children[0].enabled = true


                }
//                Component.onCompleted:
//                {
//                    camera.deviceEnumeration()
//                }
            }

            TabView{
                id: sideBar
                implicitWidth: 250
                implicitHeight: 250
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
    //                            camera.enumResolution(currentIndex)
                                camera.selectDevice(currentIndex)
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
    //                            camera.enumResolution(currentIndex)
                                fps_combo_box.enabled = true
                            }
                            Component.onCompleted: enabled = false
                        }
//                        ComboBox
//                        {
//                            implicitWidth: 250
//                            id:fps_combo_box
//                            model: fps
//                            textRole: "display"
//                            onCurrentIndexChanged:
//                            {
//    //                            camera.enumResolution(currentIndex)
//                            }
//                            Component.onCompleted: enabled = false
//                        }
//                    }
                }
                Tab{
                    id: tab2
                    title: "Extention Controls"
                    ColumnLayout{

                    }
                }
            }

        }

    }
    Devices{
        id: camera
    }

}


//                    GridLayout{
//                        id:gridlayout1
//                        column:3
//                        Text{
//                            id:text1
//                            text:"Brightness"
//                        }
//                        Slider{
//                            id:brightness
//                            stepSize: 1
//                            minimumValue: 0
//                            maximumValue: m_brightness
//                            onValueChanged:
//                            {
////                                camera.set_brightness(brightness.value);
//                            }
//                        }

                    }
